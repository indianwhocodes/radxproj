#ifndef CART2GRID_HH
#define CART2GRID_HH
#include "PolarDataStream.hh"

#include <chrono>
#include <memory>
#include <tbb/atomic.h>
#include <vector>

#define IR 8494677.33f

#define calculate_elevation(s, h, Z0)                                          \
  (std::atan2(std::cos(s / IR) - (IR / (IR + h - Z0)), std::sin(s / IR)))

#define calculate_range_gate(s, h, e, Z0)                                      \
  (std::sin(s / IR) * (IR + h - Z0) / std::cos(e))

class Cart2Grid {

private:
  shared_ptr<Repository> _store;
  map<string, std::shared_ptr<vector<vector<vector<tbb::atomic<float>>>>>>
      _outputGridSum;
  map<string, std::shared_ptr<vector<vector<vector<tbb::atomic<float>>>>>>
      _outputGridWeight;
  map<string, std::shared_ptr<vector<vector<vector<tbb::atomic<int>>>>>>
      _outputGridCount;
  map<string, std::shared_ptr<vector<vector<vector<float>>>>> _outputFinalGrid;

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

public:
  Cart2Grid(std::shared_ptr<Repository> store, const Params &params);
  void interpGrid();
  void computeGrid();
};

#endif // CART2GRID_HH
