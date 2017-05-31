
#ifndef Polar2Cartesian_h
#define Polar2Cartesian_h

#include "PolarDataStream.hh"

class Polar2Cartesian {
public:
  // constructor & destructor
  Polar2Cartesian(std::shared_ptr<Repository> store);
  ~Polar2Cartesian();

  void calculateHeight();
  void calculateRoI(float minimalRoI);
  void calculateXYZ();

  // this function called from outside
  void calculateCartesianCoords();

  // setter, getter
  // float *getGateX();
  // float *getGateY();
  // float *getGateZ();
  // float *getGateRoI();

private:
  std::shared_ptr<Repository> _store;
};

#endif
