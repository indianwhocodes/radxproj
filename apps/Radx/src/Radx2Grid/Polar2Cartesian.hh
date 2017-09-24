#ifndef RADX_RADX2GRID_POLAR_2_CARTESIAN_H_
#define RADX_RADX2GRID_POLAR_2_CARTESIAN_H_

#include "PolarDataStream.hh"
#include "tbb/partitioner.h"

class Polar2Cartesian
{
public:
  // constructor & destructor
  Polar2Cartesian(std::shared_ptr<Repository> store);
  ~Polar2Cartesian();

  void calculateXYZ(int nthreads);

  static tbb::affinity_partitioner ap;

private:
  std::shared_ptr<Repository> _store;
};

#endif // RADX_RADX2GRID_POLAR_2_CARTESIAN_H_
