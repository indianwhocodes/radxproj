
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 2008
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// CoordinateVar.cc
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2008
//
//////////////////////////////////////////////////////////////////////////
//
//  CoordinateVar object extracts data from netCDF coordinate variable.
//
///////////////////////////////////////////////////////////////////////////

#include <NcfMdv/CoordinateVar.hh>
#include <math.h>

//
// Constructor
//
CoordinateVar::CoordinateVar(NcVar *var)
{
  //
  // Initialize members 
  // 
  _units = NULL;

  _standardName = NULL;

  _longName = NULL;

  _dimension = -1;

  _dataType = -1;

  _data = NULL;

  _var = var;

  //
  // Get standard_name. Note that the caller must delete 
  // NcAtt* and memory created by as_string() method ( the destructor
  // will take care of that).
  //
  NcAtt *att = _var->get_att("standard_name");
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
	_longName = att->as_string(0);
	
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
  // Get coordiante data.
  // We are assuming coordinate data is 1 dimensional
  //
  NcDim* dim =  _var->get_dim(0);

  if (dim->is_valid())
    _dimension =  dim->size();
 
  _dataType = (int)_var->type();
  
  switch( (int)_dataType )
    {
      //
      // ncByte = 1, ncChar = 2, ncShort = 3 , 
      // ncInt = 4, ncFloat =5 , or ncDouble = 6.
      // 
    case 1:
      _data = malloc(_dimension * sizeof (ncbyte));
      _var->get((ncbyte*)_data, _dimension );
      break;

    case 2:
      _data = malloc(_dimension * sizeof (char));
      _var->get((char*)_data, _dimension);
      break;

    case 3:     
      _data =  malloc(_dimension * sizeof (short));
      _var->get((short*)_data, _dimension);
      break;

    case 4:      
      _data =  malloc(_dimension * sizeof (int));
      _var->get((int*)_data,_dimension); 
      break;

    case 5:
      _data = malloc(_dimension * sizeof (float));
      _var->get((float*)_data,_dimension);
      break;

    case 6:
      _data = malloc(_dimension * sizeof (double));
      _var->get((double*)_data,_dimension);
      break;

    default:
      cerr << "CoordinateVar::_extractData(): Failed to recognize datatype." << endl;
      break;
    }
}

//
// Destructor
//
CoordinateVar::~CoordinateVar()
{

  if(_units)
    delete[] _units;

  if (_standardName)
    delete[] _standardName;

  if(_longName)
    delete[] _longName;

  if (_data != NULL)
    free( _data);
}

double CoordinateVar::get(int i) const
{
  if ( i >= _dimension)
    {
      cerr << "CoordinateVar::get(): ERROR: requesting out of bounds dimension.\n" << endl;
      return COORDINATE_VAR_UNKNOWN;
    }
  
  //
  // ncByte = 1, ncChar = 2, ncShort = 3 , 
  // ncInt = 4, ncFloat =5 , or ncDouble = 6.
  // 
  double dataPt;

  switch( (int)_dataType )
    {
      //
      // ncByte = 1, ncChar = 2, ncShort = 3 , 
      // ncInt = 4, ncFloat =5 , or ncDouble = 6.
      // 
    case 1:
      
      dataPt = (double) ( ((ncbyte*)_data)[i]);
      
      break;
      
    case 2:
      
	dataPt =  (double)( ((char*)_data)[i]);
	
	break;
	
    case 3:    

      dataPt = (double) ( ((short*)_data)[i]);
      
      break;
  
    case 4:    

      dataPt = (double) ( ((int*)_data)[i]);
      
      break;
    
    case 5:    

      dataPt = (double) ( ((float*)_data)[i]);
      
      break;
    
    case 6:    

      dataPt = (double) ( ((double*)_data)[i]);
      
      break;  
    
    default:
      
      cerr << "CoordinateVar::get(): ERROR! Could not return array element" << endl; 

      break;

    }
  
   return dataPt;
}

bool CoordinateVar::operator==(const CoordinateVar &other)
{
  //
  // Check that the data is the same
  // 
  if (_dimension == other._dimension && _dataType == other._dataType )
    {
      double dataPt1;
      
      double dataPt2;

      for (int i = 0; i < _dimension; i++)
	{  
	switch( (int)_dataType )
	  {
	    //
	    // ncByte = 1, ncChar = 2, ncShort = 3 , 
	    // ncInt = 4, ncFloat =5 , or ncDouble = 6.
	    // 
	  case 1:
	    
	    dataPt1 = (double) ( ((ncbyte*)_data)[i]);
	    
	    dataPt2 = (double) ( ((ncbyte*)other._data)[i]);

	    break;
	    
	  case 2:
	    
	    dataPt1 =  (double)( ((char*)_data)[i]);

	    dataPt2 =  (double)( ((char*)other._data)[i]);
	    
	    break;
	    
	  case 3:    
	    
	    dataPt1 = (double) ( ((short*)_data)[i]);

	    dataPt2 = (double) ( ((short*)other._data)[i]);
	    
	    break;
	    
	  case 4:    
	    
	    dataPt1 = (double) ( ((int*)_data)[i]);
	    
	    dataPt2 = (double) ( ((int*)other._data)[i]);
	    
	    break;
	    
	  case 5:    
	    
	    dataPt1 = (double) ( ((float*)_data)[i]);
	    
	    dataPt2 = (double) ( ((float*)other._data)[i]);

	    break;
	    
	  case 6:    
	    
	    dataPt1 = (double) ( ((double*)_data)[i]);

	    dataPt2 = (double) ( ((double*)other._data)[i]);
	    
	    break;  
	    
	  default:
	    
	    cerr << "CoordinateVar::get(): ERROR! Could not return array element" << endl; 
	    
	    break;	    
	  }
	if ( fabs(dataPt1 -dataPt2) > COORDINATE_VAR_EPSILON)
	  return false;
	}
    } 
	   
  //
  // Check the other metadata 
  //
  if( _standardName == other._standardName &&
      _longName == other._longName &&
      _dataType == other._dataType)
    return true;
  else
    return false;
}
