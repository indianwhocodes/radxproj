
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
// ProjInfo.cc
//
// Projection Info object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2008
//
//////////////////////////////////////////////////////////////////////////
//
//  ProjInfo object handles reprojecting the 2D netCDF input
//  grid to an Mdv supported output grid. This is done by producing 
//  a look up table for indices of the input 2D grid which correspond
//  to indices of the output 2D grid. This class handles only the remapping
//  of the 2D(XY) grid since 2D, 3D, and 4D grids with the same x and y
//  coordinate variables can use this information.
//
///////////////////////////////////////////////////////////////////////////


#include <cstring>
#include <NcfMdv/ProjInfo.hh>

template <class T> string toString( T value );

//
// Constructor
//
ProjInfo::ProjInfo(NcVar *projVar, CoordinateVar *yVar,  CoordinateVar *xVar, bool debug)
{
 
  _debug = debug;
  
  _inputGridY = yVar;

  _inputGridX = xVar;

  _projVar = projVar;


  //
  // Initialize data members. 
  //
  _inputProj = NULL;

  _indexLut = NULL;

  _falseEasting = PROJ_INFO_UNKNOWN;

  _falseNorthing = PROJ_INFO_UNKNOWN;

  _gridMappingName = NULL;

  _gridNorthPoleLatitude = PROJ_INFO_UNKNOWN;

  _gridNorthPoleLongitude = PROJ_INFO_UNKNOWN;

  _latitudeProjectionOrigin = PROJ_INFO_UNKNOWN;

  _longitudeProjectionOrigin = PROJ_INFO_UNKNOWN;

  _longitudeCentralMeridian = PROJ_INFO_UNKNOWN;

  _northPoleGridLongitude = PROJ_INFO_UNKNOWN;
  
  _scaleFactorAtCentralMeridian = PROJ_INFO_UNKNOWN;

  _scaleFactorAtProjectionOrigin = PROJ_INFO_UNKNOWN;

  _standardParallel[0] = PROJ_INFO_UNKNOWN;

  _standardParallel[1] = PROJ_INFO_UNKNOWN;

  _straightVerticalLongitudeFromPole = PROJ_INFO_UNKNOWN;
} 

//
// Destructor
//
ProjInfo::~ProjInfo()
{
  //
  // Free memory
  //
  if (_indexLut)
  delete[] _indexLut;

  if (_inputProj)
    pj_free(_inputProj);

  if(_gridMappingName)
    delete[] _gridMappingName;
}

//
// Get attributes of NcVar projection variable if relevant,
// Initialize Proj4 projection object.
//
int ProjInfo::init()
{
  _getAttributes();

  if (_initializeProj4())
    return 1;

  return 0;
}

//
// Get attributes for NcVar projection information variable 
//
int ProjInfo::_getAttributes()
{
  //
  // If the NcVar containing projection attributes is NULL, we
  // assume the grid is a lat lon grid.
  //
  if (_projVar != NULL)
    {
      //
      // Following are the CF-1.0 list of projection related attributes.
      // Not all are required for each projection but we will get them
      // if available.
      //
      
      //
      // Get false_easting
      //
      NcAtt *att = _projVar->get_att("false_easting");
      
      if (att != 0)
	if  (att->is_valid())
	  {
	    _falseEasting =  att->as_float(0);
	    
	    delete att;

	    att = NULL;
	  }

      //
      // Get false_northing
      //
      att = _projVar->get_att("false_northing");
      if (att != 0)
	if (att->is_valid())
	  {
	    _falseNorthing =  att->as_float(0);
	    
	    delete att;

	    att = NULL;
	  }
      
      //
      // Get grid_mapping_name
      //
      att = _projVar->get_att("grid_mapping_name");
      if (att != 0)
	if (att->is_valid())
	  {
	    _gridMappingName =  att->as_string(0);
      
	    delete att;

	    att = NULL;
	  }
   
      //
      // Get grid_north_pole_latitude
      //
      att = _projVar->get_att("grid_north_pole_latitude");
      if (att != 0)
	if (att->is_valid())
	  {
	    _gridNorthPoleLatitude =  att->as_float(0);
      
	    delete att;

	    att = NULL;
	  }
     
      //
      // Get grid_north_pole_longitude
      //
      att = _projVar->get_att("grid_north_pole_longitude");
      if (att != 0)
	if (att->is_valid())
	  {
	    _gridNorthPoleLongitude =  att->as_float(0);

	    delete att;

	    att = NULL;
	  }
      
      //
      // Get latitude_of_projection_origin
      //
      att = _projVar->get_att("latitude_of_projection_origin");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _latitudeProjectionOrigin =  att->as_float(0);
      
	    delete att;

	    att = NULL;
	  }
     
      //
      // Get longitude_of_projection_origin
      //
      att = _projVar->get_att("longitude_of_projection_origin");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _longitudeProjectionOrigin =  att->as_float(0); 
      
	    delete att;

	    att = NULL;
	  }
     
      //
      // Get longitude_of_central_meridian
      //
      att = _projVar->get_att("longitude_of_central_meridian");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _longitudeCentralMeridian =  att->as_float(0);
	    
	     delete att;

	    att = NULL;
	  }

      //
      // Get scale_factor_at_central_meridian
      //
      att = _projVar->get_att("scale_factor_at_central_meridian");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _scaleFactorAtCentralMeridian =  att->as_float(0);
      
	     delete att;

	    att = NULL;
	  }

      //
      // Get north_pole_grid_longitude
      //
      att = _projVar->get_att("north_pole_grid_longitude");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _northPoleGridLongitude =  att->as_float(0);
      
	     delete att;

	     att = NULL;
	  }
   
      //
      // Get scale_factor_at_projection_origin
      //
      att = _projVar->get_att("scale_factor_at_projection_origin");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _scaleFactorAtProjectionOrigin =  att->as_float(0);
      
	    delete att;

	     att = NULL;
	  }
      //
      // Get standard_parallel
      //
      att = _projVar->get_att("standard_parallel");
      if (att != 0)
	  if (att->is_valid())
	    {
	      if (att->num_vals() == 1)
		_standardParallel[0] =  att->as_float(0);
	      else if ( att->num_vals() == 2)
		{
		  _standardParallel[0] =  att->as_float(0);
		  
		  _standardParallel[1] =  att->as_float(1);
		}
	      
	      delete att;

	      att = NULL;
	    }

      //
      // Get straight_vertical_longitude_from_pole
      //
      att = _projVar->get_att("straight_vertical_longitude_from_pole");
      if (att != 0) 
	if (att->is_valid())
	  {
	    _straightVerticalLongitudeFromPole =  att->as_float(0); 
      
	    delete att;
	    
	    att = NULL; 
	  }

      //
      // Set projection type
      //
      if ( strcmp(_gridMappingName, "albers_conical_equal_area") == 0)
	_projType = PROJ_ALBERS_CONICAL_EQUAL_AREA;
      
      else if  ( strcmp(_gridMappingName,"azimuthal_equdistant") == 0)
	_projType = PROJ_AZIMUTHAL_EQUIDISTANT;
      
      else if  ( strcmp(_gridMappingName,"lambert_azimuthal_equal_area") == 0)
	_projType = PROJ_LAMBERT_AZIMUTHAL_EQUAL_AREA;
      
      else if  ( strcmp(_gridMappingName,"lambert_conformal_conic")== 0)
	_projType = PROJ_LAMBERT_CONFORMAL;
      
      else if  ( strcmp(_gridMappingName,"polar_stereographic") == 0)
	_projType = PR0J_POLAR_STEREOGRAPHIC;
      
      else if  ( strcmp(_gridMappingName,"rotated_latitude_longitude") == 0)
	_projType = PROJ_ROTATED_POLE;
      
      else if  ( strcmp(_gridMappingName,"stereographic") == 0)
	_projType = PROJ_STEREOGRAPHIC;
      
      else if  ( strcmp(_gridMappingName,"transverse_mercator") == 0)
	_projType = PROJ_TRANSVERSE_MERCATOR;
      else
	{
	  cerr << "ProjInfo::_getProjectionInfo(): ERROR: Unknown projection type! " << endl;
	  return 1;
	}
    }
  else
    _projType = PROJ_LATLON;

  return 0;
}


//
// _initializeProj4(): 
// We are using the Proj.4 projection library (see http://remotesensing.org/proj or
// http://www.remotesensing.org/geotiff/proj_list and the CF-1.0 conventions for
// parameters of supported projections. 
// (http://cf-pcmdi.llnl.gov/documents/cf-conventions/1.0/apf.html#id2976188)
// 
// 
int ProjInfo::_initializeProj4()
{

  //
  // If grid is a lat lon grid, dont bother with the projection translator
  //
  if (_projType == PROJ_LATLON)
    {
      if (_debug) 
	cerr << "ProjInfo::_initializeProj4(): grid is a lat lon grid. No projection "
	     << " translator being instantiated. " << endl;
      return 0;
    }
  
  //
  // First determine the units argument for Proj.4 pj_init function.
  // We will support units of meters (m), kilometers (km), and feet (ft).
  // Support for more units could be added as necessary. 
  //
  // Create udunits object from x coordinate variable and create 
  // udunits objects for "meter", "kilometer", and "feet" and
  // compare. If we find a match fill out the "unit=*" arg for proj object
  // else return error. 
  //
   
  //
  // Define a udunits2 units system
  //
  ut_system* unitSys = ut_read_xml(NULL);
  
  if (unitSys == NULL )
    {
      cerr << "ProjInfo::_initializeProj4(): ERROR: Trouble opening udunits2 xml unit "
	   << "system file. Cannot initialize Proj.4 projection object." << endl;
      
      return 1;
    }
 
  //
  // Create a ut_unit from the units of the x coordinate variable.
  //
  ut_unit*  xUnit = ut_parse(unitSys,_inputGridX->getUnits(), UT_ASCII);
  
  if (xUnit == NULL)
    {
      cerr << "ProjInfo::_initializeProj4(): ERROR: Cannot create a ut_unit " 
	   << "object from string " << _inputGridX->getUnits() << endl;
      
      return 1;
    }
  
  //
  // Create ut_unit for "meter", "kilometer", "feet"
  //
  ut_unit*  mUnit =  ut_parse(unitSys,"meter", UT_ASCII);
  
  if (mUnit == NULL)
    {
      cerr << "ProjInfo::_initializeProj4(): ERROR: Cannot create a ut_unit " 
	   << "object from string \'meter\'" << endl;
      
      return 1;
    }
  
  ut_unit* kmUnit =  ut_parse(unitSys,"kilometer", UT_ASCII); 
  
  if (kmUnit == NULL)
    {
      cerr << "ProjInfo::_initializeProj4(): ERROR: Cannot create a ut_unit " 
	   << "object from string \'kilometer\'" << endl;
      
      return 1;
    }
  
  ut_unit* ftUnit =  ut_parse(unitSys,"feet",  UT_ASCII); 
  
  if (ftUnit == NULL)
    {
      cerr << "ProjInfo::_initializeProj4(): ERROR: Cannot create a ut_unit " 
	   << "object from string \'feet\'" << endl;
      
      return 1;
    }

  bool unitsSet = false;
  
  string units;

  if ( !ut_compare( xUnit, mUnit))
    {
      units = "units=m";
      
      unitsSet = true;
    }
  else if ( !ut_compare( xUnit, kmUnit))
    {
      units = "units=km";
      
      unitsSet = true;
    }
  else if ( !ut_compare( xUnit, ftUnit))
    {
      units = "units=ft";
      
      unitsSet = true;
    }

  //
  // udunits2 cleanup
  //
  if (mUnit)
    ut_free(mUnit);

  if (kmUnit)
    ut_free(kmUnit);

  if (ftUnit)
    ut_free(ftUnit);

  if (xUnit)
    ut_free(xUnit);

  if(unitSys)
    ut_free_system(unitSys);

  if (!unitsSet)
    {
      cerr << "ProjInfo::_initializeProj4(): ERROR: Could not resolve units " 
	   << "of coordinate x and y variables." << endl;    
      
      return 1;
    }
  
  //
  // initialize projection
  //
  string proj = "proj=";
  string lat_0 = "lat_0=";
  string lon_0 = "lon_0=";
  string ellps = "ellps=WGS84";
  string x_0 = "x_0=";
  string y_0 = "y_0=";
  string lat_1 = "lat_1=";
  string lat_2 = "lat_2=";
  string lat_ts = "lat_ts=";
  string k_0 = "k_0=";
  string k = "k=";

  switch( _projType)
    {
    case PROJ_ALBERS_CONICAL_EQUAL_AREA:
      {

	proj = proj + "aea";

	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	lat_0 = lat_0 + toString(_latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_longitudeCentralMeridian);
       
	string lat_1 = toString(_standardParallel[0]);
	
	string lat_2 = toString(_standardParallel[1]);
	
	char *inputPrjArgs[] = {(char*) proj.c_str(), (char*)x_0.c_str(), (char*)y_0.c_str(), 
				(char*)lat_0.c_str(), (char*)lon_0.c_str(), (char*)lat_1.c_str(), 
				(char*)lat_2.c_str(), (char*)units.c_str()};
	
	if ( (_inputProj = pj_init(8,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		 << "projection object." << endl;
	    return(1);
	  }
      }

      break;
      
    case PROJ_AZIMUTHAL_EQUIDISTANT:
      {
	proj = proj + "aeqd";
	
	lat_0 = lat_0 + toString(_latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_longitudeProjectionOrigin);
	
	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	char *inputPrjArgs[] = { (char*)proj.c_str(), (char*)lat_0.c_str(), (char*)lon_0.c_str(), 
				 (char*)x_0.c_str(), (char*)y_0.c_str(), (char*)units.c_str()};


	if ( (_inputProj = pj_init(6,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		 << "projection object." << endl;
	    return(1);
	  }
      }

      break;

    case PROJ_LAMBERT_AZIMUTHAL_EQUAL_AREA:
      {
	proj = proj + "laea";
	
	lat_0 = lat_0 + toString(_latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_longitudeProjectionOrigin);
	
	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	char *inputPrjArgs[] = {  (char*)proj.c_str(),  (char*)lat_0.c_str(),  (char*)lon_0.c_str(),  
				  (char*)x_0.c_str(),  (char*)y_0.c_str(), (char*)units.c_str() };

	if ( (_inputProj = pj_init(6,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		 << "projection object." << endl;
	   return(1);
	  }
      }

      break;

    case PROJ_LAMBERT_CONFORMAL:
      {
	proj = proj + "lcc";

	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	lat_0 = lat_0 +toString( _latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_longitudeCentralMeridian);
	
	string lat_1 = toString(_standardParallel[0]);
	
	string lat_2 = toString(_standardParallel[1]);
	
	char *inputPrjArgs[] = { (char*)proj.c_str(), (char*)x_0.c_str(), (char*)y_0.c_str(), 
			 (char*)lat_0.c_str(), (char*)lon_0.c_str(), (char*)lat_1.c_str(), 
			 (char*)lat_2.c_str(), (char*)units.c_str()};
	
	if ( (_inputProj = pj_init(8,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		<< "projection object." << endl; 
	    return(1);
	  }
      }
      break;

      
    case PR0J_POLAR_STEREOGRAPHIC:
      {
	proj = proj + "stere";
	
	lat_ts = lat_ts + toString(_standardParallel[0]);
	
	lat_0 = lat_0 + toString(_latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_straightVerticalLongitudeFromPole);
	
	k_0 = k_0 + toString(_scaleFactorAtProjectionOrigin);
	
	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	char *inputPrjArgs[] = { (char*)proj.c_str(), (char*)lat_ts.c_str(), (char*)x_0.c_str(), 
				 (char*)y_0.c_str(), (char*)lat_0.c_str(), (char*)lon_0.c_str(), 
				 (char*)k_0.c_str(), (char*)units.c_str()};
		
	if ( (_inputProj = pj_init(8,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		 << "projection object." << endl; 
	    return(1);
	  }
      }
      
      break;


    case PROJ_STEREOGRAPHIC:      
      {
	proj = proj + "stere";
	
	lat_0 = lat_0 + toString(_latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_longitudeProjectionOrigin);
	
	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	// scale factor defaults to zero when not available according to Proj.4
	//k_0 = k_0 + _scaleFactorAtProjectionOrigin; 
	

	char *inputPrjArgs[] = {(char*) proj.c_str(), (char*)lat_0.c_str(), (char*)lon_0.c_str(), 
				 (char*)x_0.c_str(), (char*)y_0.c_str(), (char*)units.c_str() };

			
	if ( (_inputProj = pj_init(6,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		  << "projection object." << endl; 
	    return(1);
	  }
      }

      break;


    case  PROJ_TRANSVERSE_MERCATOR:
      {
	lat_0 = lat_0 + toString(_latitudeProjectionOrigin);
	
	lon_0 = lon_0 + toString(_longitudeCentralMeridian);
	
	x_0 = x_0 + toString(_falseEasting);
	
	y_0 = y_0 + toString(_falseNorthing);
	
	k =  k + toString(_scaleFactorAtCentralMeridian);
	
	char *inputPrjArgs[] = { (char*)proj.c_str(), (char*)x_0.c_str(), (char*)y_0.c_str(), 
				 (char*)lat_0.c_str(), (char*)lon_0.c_str(), (char*)k.c_str(),
				 (char*)units.c_str()};
      
	if ( (_inputProj = pj_init(7,inputPrjArgs )) == NULL)
	  {
	    cerr << "ProjInfo::_projectData(): ERROR: Failed to initialize Proj.4 "
		 << "projection object." << endl; 
	    return(1);
	  }
	
      }
      break;

    default:
      {
	_inputProj = NULL;

	cerr << "ProjInfo::_projectData(): Projection " << _gridMappingName 
	     << " not supported." << endl;

	return 1;
      }
    } 

  if (_debug)
    cerr << "ProjInfo::_initializeProj4(): Proj.4 projection set: " << proj.c_str() << endl;
  
  return 0;

}


//
// createLut(): To facilitate reprojecting input data to output grid,  we
//               create a look up table containing 2D input grid indices 
//               for each output grid 2D index
// 
int ProjInfo::createLut(Pjg *outputGrid)  
{ 
 
  //
  // Get output grid dimensions. These will be the dimensions of the
  // lookup table. 
  //
  int outNx = outputGrid->getNx();

  int outNy = outputGrid->getNy();

  //
  // Allocate memory for index look up table for output grid
  //
  _indexLut = new int[outNy * outNx];
  
  if ( _indexLut == NULL )
    {
      cerr << "ProjInfo::_projectData(): Failure to allocate memory for index look up table!" << endl;
      return (1);  
    }	
  
  //
  // Get some information about the output grid
  //
  const double outputMinX = outputGrid->getMinx();
  
  double outputDx = outputGrid->getDx();
  
  const double outputMinY = outputGrid->getMiny();
  
  double outputDy = outputGrid->getDy();

  //
  // Get info about input grid coordinates (the netCDF coordinate variables)
  //
  double inputMinX = _inputGridX->get(0);
  
  double inputMinY = _inputGridY->get(0);
  
  double inputDx = _inputGridX->get(1) - inputMinX;

  double inputDy = _inputGridY->get(1) - inputMinY;

  int inNx = _inputGridX->getDimension();

  int inNy = _inputGridY->getDimension();

  if (_debug)
    {
      cerr << "ProjInfo::_createLut(): inputDx " << inputDx << " inputDy " 
	   << inputDy << " inputMinX " << inputMinX << " inputMinY " << inputMinY  << endl;
    }

  //
  // March through output grid coordinate by coordinate doing
  // following: find the lat and lon of the output coordinate pair, 
  // use the inputProj object and Proj.4 API method pj_fwd to find the corresponding
  // input grid coordinates, convert these coordinates to i, j indices of the input data
  // array and grab that data and put it into the output data array.
  //
  projUV inputCartCoord, outputLatLon;
  
  double outputLat, outputLon;
  
  for (int j = 0; j <  outNy; j++)
    {
      for (int i = 0; i <  outNx; i++)
	{
	  //
	  // Get lat, lon of output grid coordinate pair
	  // 
	  outputGrid->xy2latlon(outputMinX + i * outputDx, outputMinY + j * outputDy, 
				outputLat, outputLon);
	  
	  int inputIndexI,  inputIndexJ;
	  
	  if ( _projType != PROJ_LATLON)
	    {
	      //
	      // Convert to radians which is expected by Proj.4 api
	      //
	      outputLatLon.u = outputLon * DEG_TO_RAD;
	      
	      outputLatLon.v  = outputLat * DEG_TO_RAD;
	      
	      //
	      // Find cartesian coordinates of input data corresponding to the 
	      // output grid lon, lat pair
	      //
	      inputCartCoord = pj_fwd(outputLatLon,_inputProj);
	      
	      //
	      // Find the closest input grid I J coordinates correpsonding 
	      // to the caretesian coordinates
	      //
	      inputIndexI = (int)(roundl( (inputCartCoord.u - inputMinX)/inputDx) );
	      
	      inputIndexJ = (int)(roundl( (inputCartCoord.v - inputMinY)/inputDy) );
	    }
	  else
	    {
	      //
	      // Find the closest input grid I J coordinates correpsonding 
	      // to the output latitude longitude coordinates
	      //
	      inputIndexI = (int)(roundl( (outputLon - inputMinX)/inputDx) );
	      
	      inputIndexJ = (int)(roundl( (outputLat - inputMinY)/inputDy) );
	    }
	  
	  //
	  // Calculate output array 2D grid index
	  //
	  int outputArrayIndex = j * outNx + i;
	  
	  //
	  // Find array index corresponding to IJ
	  //
	  int inputArrayIndex;
	  
	  if ( inputIndexJ >= 0 && inputIndexJ < inNy && inputIndexI >= 0 && inputIndexI < inNx)
	    { 
	      inputArrayIndex = inputIndexJ * inNx  + inputIndexI;
	     
	      _indexLut[ outputArrayIndex] = inputArrayIndex;
	    }
	  else
	    _indexLut[outputArrayIndex] = -1;	
	}
    }
  
  return 0;
}

bool ProjInfo::operator==(const ProjInfo &other)
{
  
  if ( (fabs( _falseEasting - other._falseEasting ) < PROJ_INFO_EPSILON ) &&

       (fabs( _falseNorthing - other._falseNorthing ) < PROJ_INFO_EPSILON ) &&
       
       _gridMappingName ==  other._gridMappingName &&
       
       (fabs( _gridNorthPoleLatitude - other._gridNorthPoleLatitude ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _gridNorthPoleLongitude - other._gridNorthPoleLongitude ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _latitudeProjectionOrigin - other._latitudeProjectionOrigin ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _longitudeProjectionOrigin - other._longitudeProjectionOrigin ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _longitudeCentralMeridian - other._longitudeCentralMeridian ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _northPoleGridLongitude - other. _northPoleGridLongitude ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _scaleFactorAtCentralMeridian - other._scaleFactorAtCentralMeridian ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _scaleFactorAtProjectionOrigin - other._scaleFactorAtProjectionOrigin ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _standardParallel[0] - other._standardParallel[0] ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _standardParallel[1] - other._standardParallel[1] ) < PROJ_INFO_EPSILON ) &&
       
       (fabs( _straightVerticalLongitudeFromPole - other._straightVerticalLongitudeFromPole ) < PROJ_INFO_EPSILON ) &&

       _inputGridX == other._inputGridX &&

       _inputGridY == other._inputGridY )
    
    return true;
  else
    return false;

}

template <class T>
std::string toString( T value )
{
   ostringstream oss;

   oss << value;

   return oss.str();
}
