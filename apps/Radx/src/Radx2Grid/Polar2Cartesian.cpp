#include "Polar2Cartesian.hh"
#include <cmath>
#include <tbb/tbb.h>
#include <vector>

// constructor
Polar2Cartesian::Polar2Cartesian(std::shared_ptr<Repository> store) {
  _store = store;
}

Polar2Cartesian::~Polar2Cartesian() {}

void Polar2Cartesian::calculateCartesianCoords() {
  calculateHeight();
  calculateXYZ();
  float minimalRoI = 500.0;
  calculateRoI(minimalRoI);
}

void Polar2Cartesian::calculateXYZ() {
  for (size_t i = 0; i < _store->_outAzimuth.size(); i++) {
    float gateAngleRad = (90.0 - _store->_outAzimuth[i]) * M_PI / 180.00;
    _store->_gateX.push_back(_store->_gateDistance[i] * cos(gateAngleRad));
    _store->_gateY.push_back(_store->_gateDistance[i] * sin(gateAngleRad));
    _store->_gateZ.push_back(_store->_gateZr[i] + _store->_altitudeAgl);
  }
}

void Polar2Cartesian::calculateHeight() {
  std::vector<float> r = _store->_outGate;
  float rad_e;
  float h0;
  float R = 4.0 * 6371008.0 / 3.0;
  for (size_t i = 0; i < _store->_outElevation.size(); i++) {
    rad_e = _store->_outElevation[i] * M_PI / 180.0;
    h0 = sqrt(r[i] * r[i] + (r[i] * 2 * R) * sin(rad_e) + R * R - R);
    _store->_gateDistance.push_back(R * asin(r[i] * cos(rad_e) / (R + h0)));
    _store->_gateZr.push_back(h0);
  }
}

void Polar2Cartesian::calculateRoI(float minimalRoI) {
  for (size_t i = 0; i < _store->_gateDistance.size(); i++) {
    float roi = _store->_gateDistance[i] * 1.5 / 180.0 * M_PI +
                _store->_gateZr[i] * 0.02;
    _store->_gateRoI.push_back(std::max(minimalRoI, roi));
  }
}
