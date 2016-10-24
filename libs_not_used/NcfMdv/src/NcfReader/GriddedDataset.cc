/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 2008
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/////////////////////////////////////////////////////////////
// GriddedDataset.cc
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2008
//
//////////////////////////////////////////////////////////////////////////
//
//  GriddedDataset extracts metadata and data from netCDF gridded data
//  variable. The gridded data values are unpacked( using scale factor
//  and bias) if relevant and stored stored as floats since netCDF and 
//  Mdv formats have only the float data type in common. An output
//  data array gets created after converting input grid indices to 
//  output grid indices. Some more work should be done in interpreting
//  flag values and perhaps creating gridded data from flag value fields.
//   
//
///////////////////////////////////////////////////////////////////////////

#include <NcfMdv/GriddedDataset.hh>

//
// Constructor
//
GriddedDataset::GriddedDataset(NcVar *var, CoordinateVar *tCoordVar, 
			       CoordinateVar *zCoordVar, ProjInfo *projInfo, 
			       bool debug, int outputGridNx, int outputGridNy)
{
  _var = var;
  
  _debug = debug;

  _outNx = outputGridNx;

  _outNy = outputGridNy;

  _tCoordVar = tCoordVar;

  _zCoordVar = zCoordVar;

  _projInfo = projInfo;

  _isFlagData = false;
  
  _scaleFactor = 1.0;

  _addOffset = 0.0;

  _fillValue = GRIDDED_DATASET_UNKNOWN;

  _validMin = GRIDDED_DATASET_UNKNOWN;

  _validMax = GRIDDED_DATASET_UNKNOWN;

  _validMinSet = false;

  _validMaxSet = false;

  _fillValueSet = false;

  _data = NULL;

  _outputData = NULL;

  _dataDimensions = NULL;

  _standardName = NULL;

  _longName = NULL;

  _units = NULL;

  _ancillaryVariables = NULL;

  _flagValues = NULL;

  _flagMeanings = NULL;

  _numFlags = 0;

  _dataType =  ncNoType;

  _inputDataArraySize = 1;
} 


//
// Destructor
//
GriddedDataset::~GriddedDataset()
{
  //
  // Free memory
  //
  if (_data)
    delete[] _data;

  if (_outputData)
    delete[] _outputData;

  if (_dataDimensions)
    delete[] _dataDimensions;

  if( _standardName)
    delete[]  _standardName;

  if (_longName)
    delete[] _longName;

  if (_units)
    delete[] _units;

  if (_ancillaryVariables)
    delete[] _ancillaryVariables;
  
  if (_flagValues)
    delete[] _flagValues;

  if (_flagMeanings)
    delete[] _flagMeanings; 

}

//
// Extract data from the NcVar and reproject it
//
int GriddedDataset::init()
{
  _getAttributes();

  if (_getDimensionInfo())
    return 1;

  if (_getGriddedDataArray())
    return 1;

  if(_projectData())
    return 1;

  return 0;
}

//
// Get attributes for gridded data variable.
// We make the assumption that the following are gridded data attributes:
// _FillValue, valid_min, valid_max, standard_name, long_name, units
// Make sure the attributes get cleaned up since netCDF libs do not 
// handle cleaning up memory for these objects
// 
void GriddedDataset::_getAttributes()
{
  _varName = _var->name();

  //
  // Get _FillValue
  //
  NcAtt *att = NULL;

  att = _var->get_att("_FillValue");
  
  if (att != 0)
    if  (att->is_valid())
      {
	_fillValue =  att->as_float(0);

	_fillValueSet = true;
 
	delete att;

	att = NULL;
      }

  //
  // Get scale
  //
  att = _var->get_att("scale_factor"); 

  if (att != 0)
    if  (att->is_valid())
      {
	_scaleFactor =  att->as_float(0);
	
	delete att;

	att = NULL;
      }

  //
  // Get offset
  //
  att = _var->get_att("add_offset"); 

  if (att != 0)
    if  (att->is_valid())
      {
	_addOffset =  att->as_float(0);

	delete att;

	att = NULL;
      }

  //
  // Get valid_min
  //
  att = _var->get_att("valid_min");

  if (att != 0)
    if (att->is_valid())
      {
	_validMin =  att->as_float(0);

	_validMinSet = true;
  
	delete att;

	att = NULL;
      }

  //
  // Get valid_max
  //
  att = _var->get_att("valid_max");
  if (att != 0)
    if (att->is_valid())
      {
	_validMax =  att->as_float(0);

	_validMaxSet = true;

	delete att;

	att = NULL;
      }
  
  //
  // Get standard_name
  //
  att = _var->get_att("standard_name");
  if (att != 0)
    if (att->is_valid())
      {
	_standardName = att->as_string(0);

	delete att;

	att = NULL;
      }
  
  //
  // Get long_name
  //
  att = _var->get_att("long_name");
  if (att != 0)
    if (att->is_valid())
      {
	_longName =  att->as_string(0);     
	
	delete att;

	att = NULL;
      }
 
  //
  // Get units
  //
  att = _var->get_att("units");
  if (att != 0) 
    if (att->is_valid())
      {
	_units = att->as_string(0);
  
	delete att;

	att = NULL;
      } 

  //
  // Get flag values
  //
  att = _var->get_att("flag_values");

  if (att != 0) 
    if (att->is_valid())
      {
	_getFlagValues(att);

	delete att;

	att = NULL;
      }

  //
  // Get flag meanings string
  //
  att = _var->get_att("flag_meanings");
  if (att != 0) 
    if (att->is_valid())
      {
	_flagMeanings = att->as_string(0);
	
	delete att;

	att = NULL;
      }
 
  if (_debug)
    {
      if (_standardName)
	{
	  cerr << "GriddedDataset::_getAttributes():\"standard_name\": " 
	       << _standardName << endl;
	}

      if (_longName)
	cerr << "GriddedDataset::_getAttributes():\"long_name\": " << _longName << endl;
      
      if (_fillValueSet)
      cerr << "GriddedDataset::_getAttributes():\"_FillValue\": " << _fillValue << endl;
      
      if (_validMinSet)
	cerr << "GriddedDataset::_getAttributes():\"valid_min\": " << _validMin << endl;
      
      if (_validMaxSet)
	cerr << "GriddedDataset::_getAttributes():\"valid_max\": " << _validMax << endl;

      if (_units)
	cerr << "GriddedDataset::_getAttributes():\"units\": " << _units << endl;
      
      if(_isFlagData) 
	{
	  
	  cerr << "GriddedDataset::_getAttributes():\"flag_values\": ";

	  for (int i = 0; i < _numFlags; i++)
	    {
	      cerr <<  _flagValues[i] << endl;
	    }
	
	  cerr << "GriddedDataset::_getAttributes():\"flag_meanings\": " 
	       << _flagMeanings << endl;
	}
    }      
}

int GriddedDataset::_getDimensionInfo()
{
  //
  // Allocate memory for  dimensions array
  // 
  _numDimensions = _var->num_dims();

  _dataDimensions = new long[_numDimensions];
  
  //
  // initialize input data size
  //
   _inputDataArraySize = 1;

  //
  // Fill in the data dimensions array and get the data array
  // size. dim(0) is slowest varying variable, dim(1) is
  // the next slowest. ( so if you have vars x,y,z,t the 
  // order should be dim(0) == dim t, dim(1) == dim z,
  // dim(2) == dim y ...
  //
  for (int i = 0; i <_numDimensions ; i ++)
     {
       NcDim *dim = _var->get_dim(i);

       _inputDataArraySize *= dim->size();
       
       _dataDimensions[i] = dim->size();
     }

   return 0;
}

int GriddedDataset::_getGriddedDataArray()
{
  _dataType = _var->type();

  //
  // We'll cast the input data to floats. That is the only common
  // datatype between Mdv and NetCDF
  //
  _data = new float [_inputDataArraySize];

  if (_data == NULL)
    {
      cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
      return (1);  
    }	 
  
  if (_fillValueSet)
    _fillValue = _fillValue * _scaleFactor + _addOffset;

  //
  // Variable 'data' will hold the native netCDF type, '_data' will hold the casted float data
  //
  void *data;
  
  NcBool gotData = FALSE;

  switch( (int)_dataType )
    {
      //
      // ncByte = 1, ncChar = 2, ncShort = 3 , 
      // ncInt = 4, ncFloat =5 , or ncDouble = 6.
      // 
    case 1:
 
      data = malloc(_inputDataArraySize * sizeof (ncbyte));
         
      if (data == NULL)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
	  
	  return (1);  
	}	 
  
      gotData =_var->get((ncbyte*)data, _dataDimensions);
     
      if (!gotData)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failed to get byte data" << endl;
	  
	  return 1;
	}

      for (int i = 0; i < _inputDataArraySize; i++)
	{
	  _data[i] = (float)(((si08*)data)[i]) * _scaleFactor + _addOffset;
	}
    
	break;

    case 2:
      
      data = malloc(_inputDataArraySize * sizeof (char));
      
      if (data == NULL)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
	  
	  return (1);  
	}	 

      gotData =_var->get((char*)data,_dataDimensions);
      
      if (!gotData)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray():Failed to get char data" << endl;

	  return 1;
	}

      for (int i = 0; i < _inputDataArraySize; i++)
	_data[i] = (float)(((char*)data)[i]) * _scaleFactor + _addOffset;
      
      break;

    case 3:     
      
      data =  malloc(_inputDataArraySize * sizeof (short));
      
      if (data == NULL)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
	  
	  return (1);  
	}	 

      gotData =_var->get((short*)data, _dataDimensions);
      
      if (!gotData)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failed to get short data" << endl;
	 
	  return 1;
	}
      for (int i = 0; i < _inputDataArraySize; i++)
	_data[i] = (float)(((short*)data)[i]) * _scaleFactor + _addOffset;
      
      break;

    case 4:      
      
      data =  malloc(_inputDataArraySize * sizeof (int));
      
      if (data == NULL)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
	  
	  return (1);  
	}	 

      gotData = _var->get((int*)data,_dataDimensions); 
      
      if (!gotData)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failed to get int data" << endl;
	
	  return 1;
	}
      for (int i = 0; i < _inputDataArraySize; i++)
	  _data[i] = (float)(((int*)data)[i]) * _scaleFactor + _addOffset;
      
      break;

    case 5:
      
      data = malloc(_inputDataArraySize * sizeof (float));
      
      if (data == NULL)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
	  
	  return (1);  
	}	 

      gotData = _var->get((float*)data,_dataDimensions);
      
      if (!gotData)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failed to get float data" << endl;
	 
	  return 1;
	} 
	  
      for (int i = 0; i < _inputDataArraySize; i++)
	  
	_data[i] = (float)(((float*)data)[i]) * _scaleFactor + _addOffset;
      
      break;

    case 6:
      
      data = malloc(_inputDataArraySize * sizeof (float));
      
      if (data == NULL)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray(): Failure to allocate memory!" << endl;
	  
	  return (1);  
	}	 

      gotData =  _var->get((double*)data,_dataDimensions);
      
      if (!gotData)
	{
	  cerr << "GriddedDataset::_getGriddedDataArray():Failed to get double data" << endl;
	  
	  return 1;
	}
      
      for (int i = 0; i < _inputDataArraySize; i++)
	
	_data[i] = (float)(((double*)data)[i]) * _scaleFactor + _addOffset;
      
      break;

    default:

      cerr << "GriddedDataset::_getGriddedDataArray(): Failed to recognize data type." << endl;

      break;
    }

  //
  // Now we have float data, reset scale and bias
  //
  _scaleFactor = 1;

  _addOffset = 0;
 
  //
  // Cleanup
  //
  free(data);
  
  return 0;
}
 
int GriddedDataset::_getFlagValues(NcAtt *att)
{
  int dataType = (int)att->type();

  if ((int)dataType != _dataType)
    {
      cerr << "GriddedDataset::_getFlagValues: ERROR: By CF-1.0 convention, the flag values " 
	   << " should be the same type as the data. We will not retrieve flag values" << endl;

      return 1;
    }

  _numFlags =  att->num_vals();

  _flagValues = new float[_numFlags];

   if (_flagValues == NULL)
    {
      cerr << "GriddedDataset::_getFlagValues(): Failure to allocate memory!" << endl;
      return (1);  
    }	 

  //
  // We'll cast the input data to floats. That is common
  // datatype between Mdv and NetCDF
  // 
  for (int i = 0; i < _numFlags; i++)
    {
      switch( (int)dataType )
	{
	  //
	  // ncByte = 1, ncChar = 2, ncShort = 3 , 
	  // ncInt = 4, ncFloat =5 , or ncDouble = 6.
	  // 
	case 1:
	  
	  _flagValues[i] = (float) (att->as_ncbyte(i));
	  
	  break;

	case 2:
	  
	  _flagValues[i] = (float) (att->as_char(i));
    
	  break;

	case 3:     
      
	  _flagValues[i] = (float) (att->as_short(i));
    
	  break;

	case 4:      
	  _flagValues[i] = (float) (att->as_int(i));
	  
	  break;

	case 5:
	  _flagValues[i] = (float) (att->as_float(i));
	  
	  break;
      
	case 6:
      
	  _flagValues[i] = (float) (att->as_float(i));
	  
	  break;	  

	default:
	  
	  cerr << "GriddedDataset::_getFlagValues(): Failed to recognize data type." << endl;
	  
	  break;
	}
    }

  return 0;
}

 
//
// _projectData(): load the data in output data arrays.
//                 We make use of a ProjInfo object to convert
//                 output grid indices to input indices when
//                 loading the output data array.
// 
int GriddedDataset::_projectData()  
{
  //
  // Create output data array. XY grid dimensions may differ 
  // between input and output grids. The time and z dimensions 
  // are the same for input and output grids.
  //
  int nt;

  if ( _tCoordVar == NULL)
    nt = 1;
  else
    nt = _tCoordVar->getDimension();

  int nz;

  if ( _zCoordVar == NULL)
    nz = 1;
  else
    nz = _zCoordVar->getDimension();

  int inNy = _projInfo->getNy();

  int inNx = _projInfo->getNx(); 
  
  _outputData = new float[ nt * nz * _outNy * _outNx];

  for (int t = 0; t < nt; t++)
    {
      for ( int z = 0; z < nz; z++)
	{
	  for (int j = 0; j <  _outNy; j++)
	    {
	      for (int i = 0; i <  _outNx; i++)
		{ 
		  int outputIndex = t*(_outNx * _outNy * nz) + z*(_outNx* _outNy) + 
		    j * _outNx + i;
		  		  
		  int inputXYindex = _projInfo->getInputXYIndex(j * _outNx + i);

		  int inputIndex =  t*(inNx * inNy * nz) + z*(inNx* inNy) + inputXYindex;
		  	    
		  if (inputIndex >= 0 )
		    _outputData[outputIndex ] = _data[inputIndex];
		  else
		    _outputData[outputIndex ] = _fillValue;
		}
	    }  
	}
    }
  
  return 0;
}

const char * GriddedDataset::getStandardName() const
{
  //
  // Standard name does not always exist so we return long name if 
  // standard name is not set.
  //
  if (_standardName)
    return _standardName;
  else 
    return _longName;
}

double GriddedDataset::getZ( int i)  const
{ 
  if ( _zCoordVar != NULL )
    return (_zCoordVar->get(i));
  else
    return -9999;
}

double GriddedDataset::getT( int i) const
{ 
  if ( _tCoordVar != NULL )
    return (_tCoordVar->get(i));
  else
    return -9999;
}

int GriddedDataset::getNt() const
{
  if ( _tCoordVar != NULL )
    return (_tCoordVar->getDimension());
  else
    return 1;
}

int GriddedDataset::getNz() const
{
  if ( _zCoordVar != NULL )
    return (_zCoordVar->getDimension());
  else
    return 1;
}

bool GriddedDataset::hasVlevelInfo() const
{
  if (_zCoordVar != NULL )
    return true;
  else
    return false;
}

bool GriddedDataset::hasTimeInfo() const
{
  if (_tCoordVar != NULL )
    return true;
  else
    return false;
}
