
#include "Polar2Cartesian.h"

// constructor
Polar2Cartesian::Polar2Cartesian(Repository* store)
{
    _store = store;
}

Polar2Cartesian::~Polar2Cartesian()
{

}

void Polar2Cartesian::calculateCartesianCoords()
{
    calculateHeight();
    calculateRoI();
}

void Polar2Cartesian::calculateHeight()
{

}

void Polar2Cartesian::calculateRoI()
{

}


// getter
float* Polar2Cartesian::getGateX()
{
//    _store->_gateX;
}
float* Polar2Cartesian::getGateY()
{
//    _store->_gateY;
}
float* Polar2Cartesian::getGateZ()
{
 //   _store->_gateZ;
}
float* Polar2Cartesian::getGateRoI()
{
//    _store->_gateRoI;
}
