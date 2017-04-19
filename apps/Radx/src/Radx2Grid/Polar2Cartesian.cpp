
#include "Polar2Cartesian.hh"
#include <vector>
#include <cmath>

struct Repository // PolarDataStream::Repository
{
    // dimenssions
    int _nPoints;
    int _timeDim;
    int _rangeDim;

    // variables
    float _latitude;
    float _longitude;
    float _altitude;
    float _altitudeAgl;

    float _scalingFactor;
    short _fillValue;
    float _addOffset;

    std::vector<float> _gateSize;
    std::vector<int> _rayNGates;
    std::vector<int> _rayStartIndex;
    std::vector<int> _rayStartRange;

    std::vector<float> _azimuth;
    std::vector<float> _elevation;
    std::vector<float> _timeVar;
    std::vector<float> _rangeVar;

    std::vector<float> _reflectivity; // name change. Accomodate multiple values.

    // input file name;
    std::string _inputFile;

    // output values
    std::vector<float> _outElevation;
    std::vector<float> _outAzimuth;
    std::vector<float> _outGate;
    std::vector<float> _outRef;

    // Cartesian Coords
    std::vector<float> _gateDistance;
    std::vector<float> _gateZr;
    std::vector<float> _gateX;
    std::vector<float> _gateY;
    std::vector<float> _gateZ;
    std::vector<float> _gateRoI;
};


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
