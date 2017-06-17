#include <assert.h>
#include <iostream>
#include<typeinfo>
#include "tbb/blocked_range3d.h"
#include "tbb/compat/thread"
#include "tbb/parallel_for.h"

#include "Cart2Grid.hh"

ptr_vector3d<double> _grid_el;
ptr_vector3d<double> _grid_gate;
ptr_vector3d<double> _grid_ground;
ptr_vector3d<double> _grid_x;
ptr_vector3d<double> _grid_y;
ptr_vector3d<double> _grid_z;

template<typename T>
inline void
Cart2Grid::_makeGrid(ptr_vector3d<T>& grid)
{
  grid = std::make_shared<vector3d<T>>();
  resizeArray(grid, _DSizeI, _DSizeJ, _DSizeK);
}

template<typename T>
inline void
Cart2Grid::_makeGrid(ptr_vector3d<T>& grid, T value)
{
  grid = std::make_shared<vector3d<T>>();
  resizeArray(grid, _DSizeI, _DSizeJ, _DSizeK, value);
}

Cart2Grid::Cart2Grid(std::shared_ptr<Repository> store, const Params& params)
  : _store(store)
  , _params(params)
{
  _xy_geom = _params.grid_xy_geom;
  _z_geom = _params.grid_z_geom;

  // Initialize the grid
  _DSizeI = _xy_geom.nx;
  _DSizeJ = _xy_geom.ny;
  _DSizeK = _z_geom.nz;

  // Determine how many field in Repository;
  _clock = _currentTimestamp();

  // Initialize Grid
  if (_grid_el == nullptr) {

    // Initialize The Grid
    _makeGrid(_grid_x);
    _makeGrid(_grid_y);
    _makeGrid(_grid_z);

    // Initialize Radar Relative Grid
    _makeGrid(_grid_el);
    _makeGrid(_grid_gate);
    _makeGrid(_grid_ground);

    // Fill in value;
    const double DMinX = _xy_geom.minx * 1000.0;
    const double DMinY = _xy_geom.miny * 1000.0;
    const double DMinZ = _z_geom.minz * 1000.0;
    const double CellX = _xy_geom.dx * 1000.0;
    const double CellY = _xy_geom.dy * 1000.0;
    const double CellZ = _z_geom.dz * 1000.0;
    const double Z0 = _store->altitudeAgl;

    tbb::parallel_for(
      tbb::blocked_range3d<int>(0, _DSizeI, 0, _DSizeJ, 0, _DSizeK),
      [=](const tbb::blocked_range3d<int>& r) {
#ifdef __GNUC__
#pragma GCC ivdep
#else
#pragma ivdep
#endif
        for (auto i = r.pages().begin(); i != r.pages().end(); ++i)
          for (auto j = r.rows().begin(); j != r.rows().end(); ++j)
            for (auto k = r.cols().begin(); k != r.cols().end(); ++k) {
              // We're using a Fortran order at here. So be careful.
              double posx = DMinX + i * CellX;
              double posy = DMinY + j * CellY;
              double posz = DMinZ + k * CellZ;
              _grid_x->at(i).at(j).at(k) = posx;
              _grid_y->at(i).at(j).at(k) = posy;
              _grid_z->at(i).at(j).at(k) = posz;
              float s = std::sqrt(posx * posx + posy * posy);
              float el = calculate_elevation(s, posz, Z0);
              float rg = calculate_range_gate(s, posz, el, Z0);
              el = el / M_PI * 180.0;
              _grid_el->at(i).at(j).at(k) = el;
              _grid_ground->at(i).at(j).at(k) = s;
              _grid_gate->at(i).at(j).at(k) = rg;
            }
      });
  }

  // Initialize Feild
  for (auto it = _store->outFields.cbegin(); it != _store->outFields.cend();
       ++it) {
    string name = (*it).first;
    auto fieldsum = std::make_shared<vector3d<tbb::atomic<double>>>();
    auto fieldweight = std::make_shared<vector3d<tbb::atomic<double>>>();
    auto fieldcount = std::make_shared<vector3d<tbb::atomic<int>>>();
    // resize and fill
    resizeArray(fieldsum, _DSizeI, _DSizeJ, _DSizeK);
    resizeArray(fieldweight, _DSizeI, _DSizeJ, _DSizeK);
    resizeArray(fieldcount, _DSizeI, _DSizeJ, _DSizeK);
    // Add to output field list
    _outputGridSum.insert(std::make_pair(name, fieldsum));
    _outputGridWeight.insert(std::make_pair(name, fieldweight));
    _outputGridCount.insert(std::make_pair(name, fieldcount));
  }
  if (_params.debug) {
    _timeit("Allocating grids");
  }
}

void
Cart2Grid::interpGrid()
{
  // Convert everything to 0;
  _clock = _currentTimestamp();

  const double DMinX = _xy_geom.minx * 1000.0;
  const double DMinY = _xy_geom.miny * 1000.0;
  const double DMinZ = _z_geom.minz * 1000.0;
  const double CellX = _xy_geom.dx * 1000.0;
  const double CellY = _xy_geom.dy * 1000.0;
  const double CellZ = _z_geom.dz * 1000.0;
  const double GateSize = _store->gateSize[0];

  tbb::parallel_for(size_t(0), _store->nPoints, [&](size_t m) {

    if (_params.debug == _params.DEBUG_VERBOSE) {
      if (m % 10000 == 0)
        std::cout << m << std::endl;
    }

    // Check if there is any valid data on this point
    // This is greatly useful when we do reflectivity or KDP only

    std::vector<string> validname;

    for (auto it = _store->outFields.cbegin(); it != _store->outFields.cend();
         ++it) {
      string name = (*it).first;
      if (name.find("REF") == 0 && (*it).second->at(m) >= 0.0) {
        validname.push_back(name);
      }
      // TODO, for more types
    }

    if (validname.size() == 0)
      return;

    double X = _store->gateX[m];
    double Y = _store->gateY[m];
    double Z = _store->gateZ[m];
    double RoI = _store->gateRoI[m];
    double E = _store->outElevation[m];
    double G = _store->outGate[m];
    double S = _store->gateGroundDistance[m];

    // Put it at grid
    int ci = int((X - DMinX) / CellX);
    int cj = int((Y - DMinY) / CellY);
    int ck = int((Z - DMinZ) / CellZ);

    // Search range
    int si = int(RoI / CellX) + 1;
    int sj = int(RoI / CellY) + 1;
    int sk = int(RoI / CellZ) + 1;

    int starti = std::max(0, ci - si);
    int endi = std::min(_DSizeI - 1, ci + si);
    if (endi < starti)
      return;
    int startj = std::max(0, cj - sj);
    int endj = std::min(_DSizeJ - 1, cj + sj);
    if (endj < startj)
      return;
    int startk = std::max(0, ck - sk);
    int endk = std::min(_DSizeK - 1, ck + sk);
    if (endk < startk)
      return;

    // Grab gates
    for (int i = starti; i <= endi; ++i) {
      for (int j = startj; j <= endj; ++j) {
#ifdef __GNUC__
#pragma GCC ivdep
#else
#pragma ivdep
#endif
        for (int k = startk; k <= endk; ++k) {

          double s, el, rg, posx, posy;

          //          if (_grid_computed->at(i).at(j).at(k)) {
          s = _grid_ground->at(i).at(j).at(k);
          el = _grid_el->at(i).at(j).at(k);
          rg = _grid_gate->at(i).at(j).at(k);
          posx = _grid_x->at(i).at(j).at(k);
          posy = _grid_y->at(i).at(j).at(k);
          //          } else {
          //            s = std::sqrt(posx * posx + posy * posy);
          //            el = calculate_elevation(s, posz, Z0);
          //            rg = calculate_range_gate(s, posz, el, Z0);
          //            el = el / M_PI * 180.0;
          //            _grid_ground->at(i).at(j).at(k) = s;
          //            _grid_el->at(i).at(j).at(k) = el;
          //            _grid_gate->at(i).at(j).at(k) = rg;
          //            _grid_computed->at(i).at(j).at(k) = true;
          //          }

          if (std::abs(rg - G) > 2.0 * GateSize) {
            continue;
          }

          double max_e_diff = (E < 6.0) ? 1.0 : 3.0;
          if (std::abs(el - E) > max_e_diff) {
            continue;
          }

          double dot = std::min(1.0, (posx * X + posy * Y) / S / s);
          double adot = std::acos(dot);
          if (adot > 1.0) {
            continue;
          }
          double term1 = std::cos(std::abs(el - E));
          double term2 = dot;
          double e_u = std::acos(term1 * term2) * 180.0 / M_PI;

          double alpha = e_u;
          double gate_diff = abs(rg - G) / (2 * GateSize) + 1e-8;

          double w =
            std::pow(0.005, std::pow(alpha, 3.0)) / std::pow(gate_diff, 2.0) +
            1e-8;

          for (auto& name : validname) {

            double v = _store->outFields[name]->at(m);
            double vw = v * w + 1e-8;

            atomicAdd(_outputGridSum[name]->at(i).at(j).at(k), vw);
            atomicAdd(_outputGridWeight[name]->at(i).at(j).at(k), w);
            (_outputGridCount[name]->at(i).at(j).at(k))++;
          }
        } // Loop k
      }   // Loop j
    }     // Loop i
  });     // Parfor m
  if (_params.debug) {
    _timeit("Computation");
  }
  _clock = _currentTimestamp();
  computeGrid();
  if (_params.debug) {
    _timeit("Masking");
  }
}

void
Cart2Grid::computeGrid()
{
  for (auto m = _store->outFields.cbegin(); m != _store->outFields.cend();
       ++m) {
    string name = (*m).first;
    auto field = std::make_shared<vector3d<double>>();
    resizeArray(field, _DSizeI, _DSizeJ, _DSizeK);
    tbb::parallel_for(0, _DSizeI, [=](int i) {
      for (int j = 0; j < _DSizeJ; j++) {
        for (int k = 0; k < _DSizeK; k++) {
          if (_outputGridCount[name]->at(i).at(j).at(k) < 3 ||
              _outputGridWeight[name]->at(i).at(j).at(k) == 0) {
            field->at(i).at(j).at(k) = INVALID_DATA;
          } else {
            field->at(i).at(j).at(k) =
              _outputGridSum[name]->at(i).at(j).at(k) /
              _outputGridWeight[name]->at(i).at(j).at(k);
          }
        } // Loop k
      }   // Loop j
    });   // Parfor i
    _outputFinalGrid.insert(std::make_pair(name, field));
  } // Loop m
}

std::shared_ptr<Repository>
Cart2Grid::getRepository()
{
  return _store;
}

int 
Cart2Grid::getGridDimX()
{
  return _DSizeI;
}

int 
Cart2Grid::getGridDimY()
{
  return _DSizeJ;
}

int 
Cart2Grid::getGridDimZ()
{
  return _DSizeK;
}

map <string, ptr_vector3d<double>>
Cart2Grid::getOutputFinalGrid()
{
	return _outputFinalGrid;
}

