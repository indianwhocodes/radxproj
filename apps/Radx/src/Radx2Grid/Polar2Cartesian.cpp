
#include "Polar2Cartesian.hh"
#include <cmath>

// constructor
Polar2Cartesian::Polar2Cartesian(Repository *store) { _store = store; }

Polar2Cartesian::~Polar2Cartesian() {}

void Polar2Cartesian::calculateCartesianCoords() {
    calculateHeight();
    calculateRoI();
}

void Polar2Cartesian::calculateHeight() {
//    _store->_outGate
    std::vector<float> r = _store->_outGate;
    std::vector<float> e;
    std::vector<float> h0;
    float R = 4 * 6371008 / 3;
    for(size_t i = 0 ; i < _store->_outElevation.size(); i++)
    {
        e.push_back(_store->_outElevation[i] * M_PI / 180.0);
        h0.push_back(sqrt(pow(r[i],2.0) + (r[i] * 2 * R) * sin(e[i]) + pow(R,2.0)) - R);
        _store->_gateDistance.push_back(R * asin(r[i]*cos(e[i]) / (R + h0[i])));
        _store->_gateZr.push_back(h0[i] + 0.0);
    }
//    for(size_t i = 0 ; i < r.size(); i++)
//    {
//        h0.push_back(sqrt(pow(r[i],2.0) + (r[i] * 2 * R) * sin(e[i]) + pow(R,2.0)) - R);
//    }
//    for(size_t i = 0 ; i < r.size(); i++)
//    {
//        _store->_gateDistance.push_back(R * asin(r[i]*cos(e[i]) / (R + h0[i])));
//    }

}

void Polar2Cartesian::calculateRoI() {}

// getter
float *Polar2Cartesian::getGateX() {
  //    _store->_gateX;
}
float *Polar2Cartesian::getGateY() {
  //    _store->_gateY;
}
float *Polar2Cartesian::getGateZ() {
  //   _store->_gateZ;
}
float *Polar2Cartesian::getGateRoI() {
  //    _store->_gateRoI;
}
