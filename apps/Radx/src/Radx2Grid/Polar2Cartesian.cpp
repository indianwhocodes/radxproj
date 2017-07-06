#include <cmath>
#include <vector>
#include <tbb/tbb.h>
#include "Polar2Cartesian.hh"

// constructor
Polar2Cartesian::Polar2Cartesian(std::shared_ptr<Repository> store)
{
  _store = store;
}

Polar2Cartesian::~Polar2Cartesian() {}

void Polar2Cartesian::calculateXYZ(int nthreads)
{
  const std::vector<double>& gate = _store->outGate;
  _store->gateGroundDistance.resize(_store->nPoints);
  _store->gateZr.resize(_store->nPoints);
  _store->gateX.resize(_store->nPoints);
  _store->gateY.resize(_store->nPoints);
  _store->gateZ.resize(_store->nPoints);
  _store->gateRoI.resize(_store->nPoints);

  tbb::task_scheduler_init init(nthreads);
  const double IR = 4.0 * 6371008.0 / 3.0;
  #pragma ivdep
  tbb::parallel_for(size_t(0), _store->nPoints, [=](size_t i) {
    // Calculate ground distance and relative altitude
    double radianElev = _store->outElevation[i] * M_PI / 180.0;
    // (Eq 2.28b)
    double h0 =
      sqrt(gate[i] * gate[i] + (gate[i] * 2.0 * IR) * 
      sin(radianElev) + IR * IR) - IR;
    // (Eq 2.28c)
    _store->gateGroundDistance[i] = 
      (IR * asin(gate[i] * cos(radianElev) / (IR + h0)));
    
    _store->gateZr[i] = h0;
    
    // Calculate (x,y,z) for each gate
    double gateAngleRad = (90.0 - _store->outAzimuth[i]) * M_PI / 180.00;
    _store->gateX[i] = _store->gateGroundDistance[i] * cos(gateAngleRad);
    _store->gateY[i] = _store->gateGroundDistance[i] * sin(gateAngleRad);
    _store->gateZ[i] = _store->gateZr[i] + _store->altitudeAgl;
    double radiusOfInfluence = 
      _store->gateGroundDistance[i] * 1.5 / 180.0 * M_PI +
      _store->gateZr[i] * 0.02;
    
    _store->gateRoI[i] = std::min(max(radiusOfInfluence, 500.0), 2000.0);
  });
}
