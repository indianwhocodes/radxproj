#include <vector>
#include "PolarDataStream.h"
#include "netcdf" 

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

    std::vector<float> _gateSize;
    std::vector<int> _rayNGates;
    std::vector<int> _rayStartIndex;
    std::vector<int> _rayStartRange;

    std::vector<float> _azimuth;
    std::vector<float> _elevation;
    std::vector<float> _timeVar;
    std::vector<float> _rangeVar;

    std::vector<float> _reflectivity;       //name change. Accomodate multiple values.
    
    // input file name;
    std::string _inputFile;

    // output values
    std::vector<float> _outElevation;
    std::vector<float> _outAzimuth;
    std::vector<float> _outGate;	

    float _scalingFactor;
    short _fillValue;
    float _addOffset;

    std::vector<float> _outRef;

    // Cartesian Coords
    std::vector<float> _gateX;
    std::vector<float> _gateY;
    std::vector<float> _gateZ;

    std::vector<float> _gateRoI;

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

    netCDF::NcFile dataFile(_store->_inputFile, netCDF::NcFile::read);
    
    netCDF::NcDim TimeDim = dataFile.getDim("time");
    _store->_timeDim = TimeDim.getSize();
    
    netCDF::NcDim RangeDim = dataFile.getDim("range");
    _store->_rangeDim = RangeDim.getSize();
        
    netCDF::NcDim n_points = dataFile.getDim("n_points");
    _store->_nPoints = n_points.getSize();
       
    netCDF::NcVar lat = dataFile.getVar("latitude");
    lat.getVar(&_store->_latitude);      

    netCDF::NcVar lon = dataFile.getVar("longitude");
    lon.getVar(&_store->_longitude);

    netCDF::NcVar alt_agl = dataFile.getVar("altitude_agl");
    alt_agl.getVar(&_store->_altitudeAgl);
        
   
    _store->_timeVar.resize(_store->_timeDim);
     float* timeVarPtr=_store->_timeVar.data();
    netCDF::NcVar timeVar = dataFile.getVar("time");
    timeVar.getVar(timeVarPtr);

    _store->_rangeVar.resize(_store->_timeDim);
    float* rangeVarPtr=_store->_rangeVar.data();
    netCDF::NcVar range = dataFile.getVar("range");
    range.getVar(rangeVarPtr);
       
    _store->_rayNGates.resize(_store->_timeDim);
    int *rayNGatesPtr=_store->_rayNGates.data();
    netCDF::NcVar ray_n_gates = dataFile.getVar("ray_n_gates");
    ray_n_gates.getVar(rayNGatesPtr);
        

    _store->_gateSize.resize(_store->_timeDim);
    float* gateSizePtr = _store->_gateSize.data();        
    netCDF::NcVar gateSize = dataFile.getVar("ray_gate_spacing");
    gateSize.getVar(gateSizePtr);
   
// _store->_gateSize=new float[_store->_timeDim];
   // netCDF::NcVar gateSize = dataFile.getVar("ray_gate_spacing");
   // gateSize.getVar(_store->_gateSize);

        
    _store->_rayStartIndex.resize(_store->_timeDim);
    int* rayStartIndexPtr=_store->_rayStartIndex.data();
    netCDF::NcVar ray_start_index = dataFile.getVar("ray_start_index");
    ray_start_index.getVar(rayStartIndexPtr);
        

    _store->_rayStartRange.resize(_store->_timeDim);
    int* rayStartRangePtr=_store->_rayStartRange.data();
    netCDF::NcVar ray_start_range = dataFile.getVar("ray_start_range");
    ray_start_range.getVar(rayStartRangePtr);
        

    _store->_azimuth.resize(_store->_timeDim);
    float* azimuthPtr=_store->_azimuth.data();
    netCDF::NcVar azimuth = dataFile.getVar("azimuth");
    azimuth.getVar(azimuthPtr);

    _store->_elevation.resize(_store->_timeDim);
    float* elevationPtr= _store->_elevation.data();
    netCDF::NcVar elevation = dataFile.getVar("elevation");
    elevation.getVar(elevationPtr);
        
    // store all the values of reflectivity and copy htem by indexing
    _store->_reflectivity.resize(_store->_timeDim * _store->_rangeDim);
    float* reflectivityPtr=_store->_reflectivity.data();
    netCDF::NcVar ref = dataFile.getVar("REF");
    ref.getVar(reflectivityPtr);
        
    netCDF::NcVarAtt scale_factor = ref.getAtt("scale_factor");
    scale_factor.getValues(&_store->_scalingFactor);
       
    netCDF::NcVarAtt fillValue = ref.getAtt("_FillValue");
    fillValue.getValues(&_store->_fillValue);
        
    netCDF::NcVarAtt offset = ref.getAtt("add_offset");
    offset.getValues(&_store->_addOffset);
	
}

// create 1D array for Elevation, Azimuth, Gate and field values such as Ref
void PolarDataStream::populateOutputValues()
{
     //Size of outGate, outRef, outAzi etc = sum(ray_n_gates) = n_points
    //Pre-allocate for speed. 
    
    //std::cout<<"DEBUG: in populateValues"<<std::endl;
    _store->_outGate.resize(_store->_nPoints);
    _store->_outElevation.resize(_store->_nPoints);
    _store->_outAzimuth.resize(_store->_nPoints);
    _store->_outRef.resize(_store->_nPoints);
    
    for(size_t i=0; i < _store->_timeDim; i++)
    {
        long r0 = _store->_rayStartRange[i];
        //std::cout<<"DEBUG: Before accessing gateSize"<<std::endl;
        long g  = _store->_gateSize[i];
        //std::cout<<"DEBUG: After"<<std::endl;
        long start = _store->_rayStartIndex[i];
        long end = start + _store->_rayNGates[i];
        //std::cout << "DEBUG:" << start << ":" << _store->_rayNGates[i] << std::endl;
        //std::cout<<"DEBUG: After "<< i<<std::endl;
        float replicateElevation = _store->_elevation[i];
        
        for(size_t j=start; j<end; j++)
        {
            _store->_outGate[j] = (j-start)*g + r0;
            _store->_outElevation[j] = replicateElevation; 
            _store->_outAzimuth[j] = _store->_azimuth[i];
            if(_store->_reflectivity[i] == _store->_fillValue )
            {
                _store->_outRef[j] = -999999.0;
            }
            else
            {
                _store->_outRef[j] = _store->_reflectivity[j] * _store->_scalingFactor + _store->_addOffset;            
            }
            
        }
        
    }
}

// getter for output values
std::vector<float> PolarDataStream::getOutElevation()
{

    return _store->_outElevation;
}

std::vector<float> PolarDataStream::getOutAzimuth()
{
    return _store->_azimuth;
    // return _store->_outAzimuth; // which one is rignt?
}

std::vector<float> PolarDataStream::getOutGate()
{

    return _store->_outGate;
}

std::vector<float> PolarDataStream::getOutRef()
{

    return _store->_outRef;
}

Repository* PolarDataStream::getRepository()
{
    return _store;
}
