#ifndef RADX_RADX2GRID_CART2GRID_H_
#define RADX_RADX2GRID_CART2GRID_H_

#include "PolarDataStream.hh"
#include <chrono>
#include <memory>
#include <tbb/atomic.h>
#include <vector>

#define IR 8494677.33

#define calculate_elevation(s, h, Z0)                                          \
  (std::atan2(std::cos(s / IR) - (IR / (IR + h - Z0)), std::sin(s / IR)))

#define calculate_range_gate(s, h, e, Z0)                                      \
  (std::sin(s / IR) * (IR + h - Z0) / std::cos(e))

template <typename T> using vector3d = vector<vector<vector<T>>>;

template <typename T> using ptr_vector3d = std::shared_ptr<vector3d<T>>;

template <typename T>
inline void resizeArray(ptr_vector3d<T> &c, size_t x, size_t y, size_t z) {
  c->resize(x, vector<vector<T>>(y, vector<T>(z, 0)));
}

inline void resizeArray(ptr_vector3d<bool> &c, size_t x, size_t y, size_t z,
                        bool value) {
  c->resize(x, vector<vector<bool>>(y, vector<bool>(z, value)));
}

// inline void atomicAdd(tbb::atomic<double> &x, double addend) {
//  double o, n;
//  do {
//    o = x;
//    n = o + addend;
//  } while (x.compare_and_swap(n, o) != o);
//}

class Cart2Grid {
public:

  Cart2Grid(std::shared_ptr<Repository> store, const Params& params, int nthreads);
  void interpGrid(int nthreads);
  void computeGrid(int nthreads);

  std::shared_ptr<Repository> getRepository();
  map<string, ptr_vector3d<double>> getOutputFinalGrid();
  int getGridDimX();
  int getGridDimY();
  int getGridDimZ();
  inline Params::grid_xy_geom_t getStructXYGeom(){ return _xy_geom;}
  inline Params::grid_z_geom_t getStructZGeom(){ return _z_geom;}
  inline float getDMinX() { return _xy_geom.minx; }
  inline float getDMinY() { return _xy_geom.miny; }
  inline float getDMinZ() { return _z_geom.minz; }


private:
  shared_ptr<Repository> _store;
  map<string, ptr_vector3d<double>> _outputGridSum;
  map<string, ptr_vector3d<double>> _outputGridWeight;
  map<string, ptr_vector3d<int>> _outputGridCount;
  map<string, ptr_vector3d<double>> _outputFinalGrid;

  const Params _params;
  Params::grid_xy_geom_t _xy_geom;
  Params::grid_z_geom_t _z_geom;

  long _clock = 0;

  inline long _currentTimestamp() {
    std::chrono::microseconds start =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    return start.count();
  }

  inline void _timeit(string promotion = "") {
    std::cerr << promotion << ": " << (_currentTimestamp() - _clock) / 1.0E6
              << " sec" << std::endl;
  }

  int _DSizeI, _DSizeJ, _DSizeK; // Size of the grid

  template <typename T> inline void _makeGrid(ptr_vector3d<T> &grid);

  template <typename T> inline void _makeGrid(ptr_vector3d<T> &grid, T value);
};

#endif // RADX_RADX2GRID_CART2GRID_H_
