#include "Cart2Grid.hh"
#include "tbb/compat/thread"
#include "tbb/parallel_for.h"
#include <assert.h>
#include <iostream>

Cart2Grid::Cart2Grid(std::shared_ptr<Repository> store, const Params &params)
    : _store(store), _params(params) {
  _xy_geom = _params.grid_xy_geom;
  _z_geom = _params.grid_z_geom;

  // Initialize the grid
  _DSizeI = _xy_geom.nx;
  _DSizeJ = _xy_geom.ny;
  _DSizeK = _z_geom.nz;

  // Determine how many field in Repository;
  _clock = _currentTimestamp();
  for (auto it = _store->_outFields.cbegin(); it != _store->_outFields.cend();
       ++it) {
    string name = (*it).first;
    auto fieldsum =
        std::make_shared<vector<vector<vector<tbb::atomic<float>>>>>();
    auto fieldweight =
        std::make_shared<vector<vector<vector<tbb::atomic<float>>>>>();
    auto fieldcount =
        std::make_shared<vector<vector<vector<tbb::atomic<int>>>>>();
    // resize and fill
    fieldsum->resize(_DSizeI,
                     vector<vector<tbb::atomic<float>>>(
                         _DSizeJ, vector<tbb::atomic<float>>(_DSizeK, 0)));
    fieldweight->resize(_DSizeI,
                        vector<vector<tbb::atomic<float>>>(
                            _DSizeJ, vector<tbb::atomic<float>>(_DSizeK, 0)));
    fieldcount->resize(_DSizeI,
                       vector<vector<tbb::atomic<int>>>(
                           _DSizeJ, vector<tbb::atomic<int>>(_DSizeK, 0)));
    // Add to output field list
    _outputGridSum.insert(
        std::pair<string,
                  std::shared_ptr<vector<vector<vector<tbb::atomic<float>>>>>>(
            name, fieldsum));
    _outputGridWeight.insert(
        std::pair<string,
                  std::shared_ptr<vector<vector<vector<tbb::atomic<float>>>>>>(
            name, fieldweight));
    _outputGridCount.insert(
        std::pair<string,
                  std::shared_ptr<vector<vector<vector<tbb::atomic<int>>>>>>(
            name, fieldcount));
  }
  if (_params.debug) {
    _timeit("Allocating grids");
  }
}

inline void atomicAdd(tbb::atomic<float> &x, float addend) {
  float o, n;
  do {
    // It is meaning less to add such a small number
    if (std::fabs(addend) < 1e-5f)
      break;
    o = x;
    n = o + addend;
  } while (x.compare_and_swap(n, o) !=
           o); // This is safe, because addend is at least 2e-5
}

void Cart2Grid::interpGrid() {
  // Convert everything to 0;
  _clock = _currentTimestamp();

  float DMinX = _xy_geom.minx * 1000.0f;
  float DMinY = _xy_geom.miny * 1000.0f;
  float DMinZ = _z_geom.minz * 1000.0f;
  float CellX = _xy_geom.dx * 1000.0f;
  float CellY = _xy_geom.dy * 1000.0f;
  float CellZ = _z_geom.dz * 1000.0f;
  float Z0 = _store->_altitudeAgl;

  tbb::parallel_for(0, _store->_nPoints, [=](int m) {
    std::cout << "=================" << m << "===============" << std::endl;
    // Grab gates
    float X = _store->_gateX[m];
    float Y = _store->_gateY[m];
    float Z = _store->_gateZ[m];
    float RoI = _store->_gateRoI[m];
    float E = _store->_outElevation[m];
    float G = _store->_outGate[m];
    float S = _store->_gateGroundDistance[m];
    float GateSize = _store->_gateSize[0];

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

    for (int i = starti; i <= endi; ++i) {
      for (int j = startj; j <= endj; ++j) {
        for (int k = startk; k <= endk; ++k) {
          float posx = DMinX + i * CellX;
          float posy = DMinY + j * CellY;
          float posz = DMinZ + k * CellZ;

          float s = std::sqrt(posx * posx + posy * posy);
          float el = calculate_elevation(s, posz, Z0);
          float rg = calculate_range_gate(s, posz, el, Z0);

          if ((el < 0.0) || (el > 20)) {
            continue;
          }

          if (std::abs(rg - G) > 2.0 * GateSize) {
            continue;
          }

          float max_e_diff = (E < 6.0) ? 1.0 : 3.0;
          if (std::abs(el / M_PI * 180.0 - E) > max_e_diff) {
            continue;
          }

          float dot = std::min(1.0f, (posx * X + posy * Y) / (S * s));
          float term1 = std::cos(std::abs(el - E));
          float term2 = dot;
          float e_u = std::acos(term1 * term2) * 180.0 / M_PI;

          float alpha = e_u / max_e_diff;

          float gate_diff = fabs(rg - G) / (2 * GateSize) + 1e-5;

          float w = std::pow(0.005f, alpha * alpha * alpha) /
                        (gate_diff * gate_diff) +
                    2e-5;

          for (auto it = _outputGridCount.cbegin();
               it != _outputGridCount.cend(); ++it) {
            string name = (*it).first;
            float v = _store->_outFields[name]->at(m);
            if (v < -100.0f) {
              continue;
            }

            float vw = v * w + 2e-5f;

            //           atomicAdd(_outputGridSum[name]->at(i).at(j).at(k), vw);
            //           atomicAdd(_outputGridWeight[name]->at(i).at(j).at(k),
            //           w);
            //           (_outputGridCount[name]->at(i).at(j).at(k))++;
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

void Cart2Grid::computeGrid() {
  for (auto m = _store->_outFields.cbegin(); m != _store->_outFields.cend();
       ++m) {
    string name = (*m).first;
    auto field = std::make_shared<vector<vector<vector<float>>>>();
    field->resize(_DSizeI,
                  vector<vector<float>>(_DSizeJ, vector<float>(_DSizeK, 0)));
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
    _outputFinalGrid.insert(
        std::pair<string, shared_ptr<vector<vector<vector<float>>>>>(name,
                                                                     field));
  } // Loop m
}
