
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

    float* _gateSize;
    int* _rayNGates;
    int* _rayStartIndex;
    int* _rayStartRange;

    float* _azimuth;
    float* _elevation;
    float* _timeVar;
    float* _rangeVar;

    float* _reflectivity;       //name change. Accomodate multiple values.
    
    // input file name;
    std::string _inputFile;

    // output values
    float* _outElevation;
    float* _outAzimuth;
    float* _outGate;	

    float _scalingFactor;
    short _fillValue;
    float _addOffset;

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
        
    _store->_timeVar = new float[_store->_timeDim];
    netCDF::NcVar timeVar = dataFile.getVar("time");
    timeVar.getVar(_store->_timeVar);

    _store->_rangeVar = new float[_store->_rangeDim];
    netCDF::NcVar range = dataFile.getVar("range");
    range.getVar(_store->_rangeVar);
       
    _store->_rayNGates = new int[_store->_timeDim];
    netCDF::NcVar ray_n_gates = dataFile.getVar("ray_n_gates");
    ray_n_gates.getVar(_store->_rayNGates);
        

    //_store->_gateSize.resize(_store->_timeDim);
    //int* gateSizePtr = _store->_gateSize.data();
        
    _store->_gateSize=new float[_store->_timeDim];
    netCDF::NcVar gateSize = dataFile.getVar("ray_gate_spacing");
    gateSize.getVar(_store->_gateSize);

        
    _store->_rayStartIndex = new int[_store->_timeDim];
    netCDF::NcVar ray_start_index = dataFile.getVar("ray_start_index");
    ray_start_index.getVar(_store->_rayStartIndex);
        

    _store->_rayStartRange = new int[_store->_timeDim];
    netCDF::NcVar ray_start_range = dataFile.getVar("ray_start_range");
    ray_start_range.getVar(_store->_rayStartRange);
        

    _store->_azimuth = new float[_store->_timeDim];
    netCDF::NcVar azimuth = dataFile.getVar("azimuth");
    azimuth.getVar(_store->_azimuth);

    _store->_elevation = new float[_store->_timeDim];
    netCDF::NcVar elevation = dataFile.getVar("elevation");
    elevation.getVar(_store->_elevation);
        
    // store all the values of reflectivity and copy htem by indexing
    _store->_reflectivity = new float[_store->_timeDim * _store->_rangeDim];
    netCDF::NcVar ref = dataFile.getVar("REF");
    ref.getVar(_store->_reflectivity);
        
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
    _store->_outGate = new float[_store->_nPoints];
    _store->_outElevation = new float[_store->_nPoints];
    _store->_outAzimuth = new float[_store->_nPoints];
    _store->_outRef = new float[_store->_nPoints];
    
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
float* PolarDataStream::getOutElevation()
{

    return _store->_outElevation;
}

float* PolarDataStream::getOutAzimuth()
{
    return _store->_azimuth;
    // return _store->_outAzimuth; // which one is rignt?
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
