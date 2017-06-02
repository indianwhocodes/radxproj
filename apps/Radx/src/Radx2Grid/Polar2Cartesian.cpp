#include "Polar2Cartesian.hh"
#include <cmath>
#include <tbb/tbb.h>
#include <vector>

// constructor
Polar2Cartesian::Polar2Cartesian(std::shared_ptr<Repository> store)
{
  _store = store;
}

Polar2Cartesian::~Polar2Cartesian()
{
}

void
Polar2Cartesian::calculateXYZ()
{
  const std::vector<double>& r = _store->_outGate;
  _store->_gateGroundDistance.resize(_store->_nPoints);
  _store->_gateZr.resize(_store->_nPoints);
  _store->_gateX.resize(_store->_nPoints);
  _store->_gateY.resize(_store->_nPoints);
  _store->_gateZ.resize(_store->_nPoints);
  _store->_gateRoI.resize(_store->_nPoints);

  const double R = 4.0 * 6371008.0 / 3.0;
#pragma ivdep
  tbb::parallel_for(size_t(0), _store->_nPoints, [=](size_t i) {
    // Calculate ground distance and relative altitude
    double rad_e = _store->_outElevation[i] * M_PI / 180.0;
    float h0 = sqrt(r[i] * r[i] + (r[i] * 2 * R) * sin(rad_e) + R * R) - R;
    _store->_gateGroundDistance[i] = (R * asin(r[i] * cos(rad_e) / (R + h0)));
    _store->_gateZr[i] = h0;
    // Calculate (x,y,z) for each gate
    float gateAngleRad = (90.0 - _store->_outAzimuth[i]) * M_PI / 180.00;
    _store->_gateX[i] = _store->_gateGroundDistance[i] * cos(gateAngleRad);
    _store->_gateY[i] = _store->_gateGroundDistance[i] * sin(gateAngleRad);
    _store->_gateZ[i] = _store->_gateZr[i] + _store->_altitudeAgl;
    float roi = _store->_gateGroundDistance[i] * 1.5 / 180.0 * M_PI +
                _store->_gateZr[i] * 0.02;
    _store->_gateRoI[i] = std::min(roi, 2000.0f);
  });
}
