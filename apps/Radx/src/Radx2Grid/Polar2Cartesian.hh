
#ifndef Polar2Cartesian_h
#define Polar2Cartesian_h

#include "PolarDataStream.hh"

class Polar2Cartesian {
public:
  // constructor & destructor
  Polar2Cartesian(std::shared_ptr<Repository> store);
  ~Polar2Cartesian();

  void calculateXYZ();

private:
  std::shared_ptr<Repository> _store;
};

#endif
