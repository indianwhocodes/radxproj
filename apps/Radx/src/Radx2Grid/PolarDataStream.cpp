
#include "PolarDataStream.h"

struct Repository // PolarDataStream::Repository
{
    // dimenssions
    size_t _nPoints;
    size_t _timeDim;
    size_t _rangeDim;

    // variables
    float _latitude;
    float _longitude;
    float _altitude;
    float _altitudeAgl;

    size_t* _gateSize;
    size_t* _rayNGates;
    size_t* _rayStartIndex;
    size_t* _rayStartRange;

    float* _azimuth;
    float* _elevation;
    float* _timeVar;
    
    // input file name;
    std::string _inputFile;

    // output values
    float* _outElevation;
    float* _outAzimuth;
    float* _outGate;
    float* _outRef;	

    // Cartesian Coords
    float* _gateX;
    float* _gateY;
    float* _gateZ;

    float* _gateRoI;

};
// constructor
PolarDataStream::PolarDataStream(const std::string& inputFile) 
{
    _store  = new Repository();
    _store->_inputFile = inputFile;
}

PolarDataStream::~PolarDataStream()
{
	
}

// read dimenssions, variables from NetCDF file and fill the repository
void PolarDataStream::LoadDataFromNetCDFFilesIntoRepository()
{
    std::cout << "LoadDataFromNetCDFFilesIntoRepository function" << std::endl;
	
}

// create 1D array for Elevation, Azimuth, Gate and field values such as Ref
void PolarDataStream::populateOutputValues()
{
    
}

// getter for output values
float* PolarDataStream::getOutElevation()
{

    return _store->_outElevation;
}

float* PolarDataStream::getOutAzimuth()
{

    return _store->_outAzimuth;
}

float* PolarDataStream::getOutGate()
{

    return _store->_outGate;
}

float* PolarDataStream::getOutRef()
{

    return _store->_outRef;
}

Repository* PolarDataStream::getRepository()
{
    return _store;
}
