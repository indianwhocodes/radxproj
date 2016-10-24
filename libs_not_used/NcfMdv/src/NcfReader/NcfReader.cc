/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 2008
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

////////////////////////////////////////////////////////////
// NcfReader.hh
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2008
//
//////////////////////////////////////////////////////////////////////////
//
// NcfReader object reads gridded datasets from a mostly CF compliant netCDF 
// file. Some assumptions are made about the file that are not in the CF 
// document including the existence of "start_time", "stop_time", and 
// "forecast_period" vars. We assume the attributes "title", "institution", 
// "source", "history", "references", and "comment" exist and are global.
// 
///////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <NcfMdv/NcfReader.hh>

NcfReader::NcfReader(char *filename, bool debug, Pjg *outputGrid)
{
  _ncfFileStr = filename;

  _debug = debug;

  _outputGrid = outputGrid;

  _datasetName = NULL;
  
  _title = NULL;
  
  _institution = NULL;
  
  _source = NULL;
  
  _history = NULL;
  
  _references = NULL;
  
  _comment = NULL;
  
  _conventions = NULL;


}

NcfReader::~NcfReader()
{
  //
  // Memory cleanup
  //
  delete _ncFile;

  if (_datasetName)
    delete[] _datasetName;
  
  if (_title)
    delete[] _title;
  
  if (_institution)
    delete[] _institution;

  if(_source)
    delete[] _source;
  
  if (_history)
    delete[] _history;
  
  if (_references)
    delete[] _references;
  
  if (_comment)
    delete[] _comment;
  
  if (_conventions)
    delete[] _conventions;

  vector < GriddedDataset* >::iterator i;

  //
  // Delete gridded datasets
  //
  for(  i = _griddedDatasets.begin(); i != _griddedDatasets.end(); i++)
    delete *i;

  _griddedDatasets.erase(_griddedDatasets.begin(), _griddedDatasets.end());
  
  //
  // Delete projection information objects
  // 
  vector < ProjInfo* >::iterator j;
 
  for( j = _projInfo.begin(); j != _projInfo.end(); j++)
    delete *j;

  _projInfo.erase( _projInfo.begin(), _projInfo.end());

  // 
  // Delete x coordinates
  //   
  vector < CoordinateVar* >::iterator m;

  for( m = _xCoordinates.begin(); m != _xCoordinates.end(); m++)
    delete *m;

  _xCoordinates.erase( _xCoordinates.begin(),_xCoordinates.end());

  
  // 
  // Delete y coordinates
  //   
  vector < CoordinateVar* >::iterator n;

  for( n = _yCoordinates.begin(); n != _yCoordinates.end(); n++)
    delete *n;

  _yCoordinates.erase( _yCoordinates.begin(),_yCoordinates.end());

  //
  // Delete vertical coordinates
  //
  vector < CoordinateVar* >::iterator k;

  for(  k =_zCoordinates.begin(); k !=  _zCoordinates.end(); k++)
    delete *k;

  _zCoordinates.erase( _zCoordinates.begin(), _zCoordinates.end());
  
  // 
  // Delete time coordinates
  //   
  vector < CoordinateVar* >::iterator l;

  for( l = _tCoordinates.begin(); l != _tCoordinates.end(); l++)
    delete *l;

  _tCoordinates.erase( _tCoordinates.begin(),_tCoordinates.end());

}

int NcfReader::readFile()
{

  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  //
  // Open the file. 
  // 
  _ncFile = new NcFile(_ncfFileStr.c_str(), NcFile::ReadOnly);

  //
  // Check that constructor succeeded
  //
  if (!_ncFile->is_valid())
    {
      cerr << _ncfFileStr.c_str() << " could not be opened!\n";
      return 1;
    }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program. In the case of this example, we just exit with
  // an NC_ERR error code.
  
  NcError err(NcError::silent_nonfatal);
 
  //
  // Get the number of variables
  //
  _numVars = _ncFile->num_vars();

  //
  // Get the number of dimensions
  //
  _numDims = _ncFile->num_dims();

  _getDatasetName();

  _getGlobalAttributes();
  
  _getTimeInfo();
 
  if (_getGriddedData())
    {
      cerr << "NcfReader::readFile(): ERROR." << endl;
      return 1;
    }

  return 0;
}

void NcfReader::_getDatasetName()
{
  //
  // We assume this variable is in the file
  //
  NcVar *nameVar = _ncFile->get_var("data_name");
  
  if (nameVar != NULL)
      if (!nameVar->is_valid())
	_datasetName = nameVar->as_string(0);
  
}

//
// _getGlobalAttributes():
//
// We will assume the attributes "title", "institution", "source",
// "history", "references", and "comment" exist and are global.
//
int NcfReader::_getGlobalAttributes()
{  
  //
  // Get the number of attributes
  //
  int numAtts = _ncFile->num_atts();

  NcAtt* globalAtt = NULL;

  //
  // Loop through the attributes and grab the ones we are expecting.
  //
  for (int i = 0; i < numAtts; i++)
    {
      globalAtt = _ncFile->get_att(i);
      
      if (!globalAtt->is_valid())
	continue;
      
      if ( strcmp( globalAtt->name(), "title") == 0 )
	_title = globalAtt->as_string(0);
       
      if ( strcmp( globalAtt->name(), "institution") == 0 )
	_institution = globalAtt->as_string(0);
      
      if ( strcmp( globalAtt->name(), "source") == 0 ) 
	_source = globalAtt->as_string(0);
      
      if ( strcmp( globalAtt->name(), "history") == 0 )
	_history = globalAtt->as_string(0);

      
      if ( strcmp( globalAtt->name(), "references") == 0 )
	 _references = globalAtt->as_string(0);
      
      if ( strcmp( globalAtt->name(), "comment") == 0 )
	_comment = globalAtt->as_string(0);
 
      if ( strcmp( globalAtt->name(), "Conventions") == 0 )
	 _conventions = globalAtt->as_string(0);
       
       //
       // Caller must delete NcAtt*
       //
       if(globalAtt)
	 {
	   delete globalAtt;
	   
	   globalAtt = NULL;
	 } 
    }

  if (_debug)
    {
      cerr << "NcfReader::_getGlobalAttributes(): \"title\": " << _title << endl;
      
      cerr << "NcfReader::_getGlobalAttributes(): \"institution\": " << _institution << endl;
      
      cerr << "NcfReader::_getGlobalAttributes(): \"source\": " << _source << endl;
      
      cerr << "NcfReader::_getGlobalAttributes(): \"history\": " << _history << endl;
      
      cerr << "NcfReader::_getGlobalAttributes(): \"references\": " << _references << endl;
      
      cerr << "NcfReader::_getGlobalAttributes(): \"comment\": " << _comment << endl;
      
      cerr << "NcfReader::_getGlobalAttributes(): \"Conventions\": " << _conventions << endl;    
    }
  return 0;
} 

//
// There are only two standard names relating to time attributes in the standard name table
// (http://cf-pcmdi.llnl.gov/documents/cf-standard-names/6/cf-standard-name-table.html)
// time, and forecast_reference_time. Therefore the assumption is made that 
// "time", "start_time", "stop_time", "forecast_period", and  "forecast_reference_time" are variables in the file.
//
int NcfReader::_getTimeInfo()
{

  long timeData;
 
  int ret = _getTimeVar((char *)"time", timeData);

  if (ret)
    _time.set(DateTime::NEVER);
  else
    _time.set(timeData);

  ret = _getTimeVar((char *)"start_time",timeData);
  if (ret)
    _startTime.set(DateTime::NEVER);
  else
    _startTime.set(timeData);
  
  ret = _getTimeVar((char *)"stop_time",timeData);
  if (ret)
    _stopTime.set(DateTime::NEVER);
  else
    _stopTime.set(timeData);

  ret = _getTimeVar((char *)"forecast_period",timeData);

  if (ret)
    _forecastPeriod = -1;
  else
    _forecastPeriod = timeData;

  ret = _getTimeVar((char *)"forecast_reference_time",timeData);
  
  if (ret)
    _forecastReferenceTime.set(DateTime::NEVER);
  else
    _forecastReferenceTime.set(timeData);

  return 0;
}


int NcfReader::_getTimeVar(char *timeVarName, long& timeData)
{
  
  NcVar* timeVar;
  
  timeVar = _ncFile->get_var(timeVarName);

  if ( timeVar == 0 )
      return 1;
  else if ( !timeVar->is_valid())
    return 1;
  else
    timeData = timeVar->as_long(0);
 
  if (_debug)
      cerr << "NcfReader::_getTime(): " << timeVarName << " = " << timeData << endl;

  return 0;
}

//
// Loop through the variables and their attributes. Find variables 
// with dimensions corresponding to standard x and y variables and create
// a GriddedDataset object with pointers to ProjInfo object (including 
// x and y CoordinateVar objects), vertical level, and time CoordinateVar objects. 
//
int NcfReader::_getGriddedData()
{
  NcVar* var;

  for (int i = 0; i < _numVars; i++)
    {
      var = _ncFile->get_var(i);

      if (var->is_valid())
	{
	  //
	  // Dataset is a gridded field if they have x and y coordinate variables. x and y 
	  // coordinate variables have standard names "latitude", "longitude", 
	  // "projection_x_coordinate" or "projection_y_coordinate"). 
	  // Determine z and t coordinate variables if relevant. Note that a time coordinate variable
	  // *may* have the standard_name "time" but it is not required; however, the 
	  // units attribute can be checked for time units. A vertical level coordinate will either
	  // have pressure units OR it will have the "positive" attribute.
	  //
	  int numDims = var->num_dims();

	  if (numDims >= 2  && numDims <= 4)
	    {    
	      NcVar *xVar, *yVar, *zVar, *tVar;

	      xVar = yVar = zVar = tVar = NULL;

	      bool xset = false;
	      
	      bool yset = false;

	      for (int j = 0; j < numDims ; j++)
		{
		  NcDim* dim;
		  
		  dim =  var->get_dim(j);
		  
		  if (dim->is_valid())
		    {
		      //
		      // Get the coordinate variable associated with dataset dimension
		      // 	 
		      NcVar *coordVar = _ncFile->get_var(dim->name());

		      //
		      // Try to get some of the attributes which distinguish 
		      // coordinate variables from others
		      //
		      NcAtt *standardNameAtt = coordVar->get_att("standard_name");

		      NcAtt *positiveAtt = coordVar->get_att("positive");

		      NcAtt *unitsAtt  = coordVar->get_att("units");
		      
		      char *stdName = standardNameAtt->as_string(0);

		      //
		      // Test for the x coordinate variable
		      //
		      if ( standardNameAtt != 0 &&
			   ( strcmp (stdName, "projection_x_coordinate")== 0 ||
			     strcmp (stdName, "longitude")== 0 ) )
			{
			  xVar = coordVar;
			  xset = true;
			}
		      
		      //
		      // Test for the y coordinate variable
		      //
		      else if ( standardNameAtt != 0 &&
				( strcmp (stdName, "projection_y_coordinate")== 0 ||
				  strcmp (stdName, "latitude")== 0 ) )
			{
			  yVar = coordVar;
			  yset = true;
			}
		      
		      //
		      // Test for time variable
		      //
		      else if (!strcmp(stdName, "time"))
			tVar = coordVar;
		     
		      
		      //
		      // Test for the z coordinate variable 
		      //
		      else if ( positiveAtt != 0 ) 
			zVar = coordVar;
		      
		      //
		      // Need to examine units to ascertain variable type
		      else 
			{
			  //
			  // Define a udunits2 units system
			  //
			  ut_system* unitSys = ut_read_xml(NULL);
			  
			  if (unitSys == NULL )
			    {
			      cerr << "NcfReader::_getGriddedData: ERROR: Trouble opening udunits2 xml unit "
				   << "system file. Cannot resolve coordinate variables!" << endl;
			      
			      return 1;
			    }

			  //
			  // Get the unit corresponding to the unitsAtt from the udunits2 
			  // units system
			  //
			  char *unitStr = unitsAtt->as_string(0);

			  ut_unit* varUnit = ut_parse(unitSys,unitStr, UT_ASCII);

			  //
			  // Cleanup unitStr
			  // 
			  if (unitStr)
			    delete[] unitStr;

			  //
			  // Create a generic time stamp to compare units against (for testing time variable)
			  //
			  ut_unit *timestampUnit = ut_offset_by_time(ut_get_unit_by_name(unitSys, "second"), 0);
			  
			  ut_unit *pressureUnit = ut_get_unit_by_name(unitSys, "Pa");

			  ut_unit *secondUnit =  ut_get_unit_by_name(unitSys, "second");

			  if ( ut_are_convertible(varUnit, pressureUnit) && varUnit != NULL && pressureUnit != NULL)
			    zVar = coordVar;

			  else if( (ut_are_convertible(varUnit, secondUnit)  && varUnit != NULL && secondUnit !=NULL) ||
				   (ut_are_convertible(varUnit, timestampUnit) && varUnit != NULL && timestampUnit != NULL))
			    tVar = coordVar;
			  else
			    {
			      cerr << "NcfReader::_getGriddedData: Cannot resolve coordinate variable!" << endl;
			      
			      if ( standardNameAtt)
				{
				  
				  cerr << "     Coordinate variable standard_name: " 
				       <<  stdName << endl;
				}
			      else
				{
				  NcAtt *longNameAtt = coordVar->get_att("long_name");

				  if (longNameAtt)
				    {
				      char *longNameStr = longNameAtt->as_string(0);
				      
				      cerr << "    Coordinate variable long_name: " 
					   <<  longNameStr << endl;
				      
				      delete[] longNameStr;
				      
				      delete longNameAtt;
				    }
				  
				}  
    
			      return 1;
			    }

			  //
			  // udunits2 cleanup
			  //
			  if (varUnit)
			    ut_free(varUnit);

			  if (timestampUnit)
			    ut_free(timestampUnit);

			  if (pressureUnit)
			    ut_free(pressureUnit);

			  if(secondUnit)
			    ut_free(secondUnit);

			  ut_free_system(unitSys);
			}

		      //
		      // Cleanup: Caller must delete NcAtt* memory created by 
		      // netCDF *as_string calls
		      //
		      if (standardNameAtt)
			delete standardNameAtt;
		      
		      if (positiveAtt)
			delete positiveAtt;
		      		      
		      if (unitsAtt)
			delete unitsAtt;

		      if( stdName)
			delete[] stdName;
		      
		    }// end if dim is valid
		}//  j = 0 to < numdims
	      
	      if (xset && yset)
		{
		  //
		  // Create x CoordinateVar object
		  //	 
		  CoordinateVar *xCoordVar = new CoordinateVar(xVar);
		      
		  //
		  // Keep it if we havent seen it before
		  //
		  bool xIsNew = true;
		  
		  for (int i = 0; i < (int) _xCoordinates.size(); i++)
		    { 
		      if ( *xCoordVar == *_xCoordinates[i])
			{
			  delete xCoordVar;
			 
			  xCoordVar = _xCoordinates[i];
			  
			  i = (int)_xCoordinates.size();
			  
			  xIsNew = false;
			}
		    }
		  
		  if (xIsNew)
		    _xCoordinates.push_back(xCoordVar);

		  
		  //
		  // Create y CoordinateVar object
		  //	 
		  CoordinateVar *yCoordVar = new CoordinateVar(yVar);
		      
		  //
		  // Keep it if we havent seen it before
		  //
		  bool yIsNew = true;
		  
		  for (int i = 0; i < (int) _yCoordinates.size(); i++)
		    { 
		      if ( *yCoordVar == *_yCoordinates[i])
			{
			  delete yCoordVar;
			  
			  yCoordVar = _yCoordinates[i];

			  i = (int)_yCoordinates.size();
			  
			  yIsNew = false;
			}
		    }
		  
		  if (yIsNew)
		    _yCoordinates.push_back(yCoordVar);
		  

		  //
		  // Create z CoordinateVar object
		  // 

		  CoordinateVar *zCoordVar = NULL;

		  if (zVar != NULL )
		    {
		      zCoordVar = new CoordinateVar(zVar);
		      
		      //
		      // Keep it if we havent seen it before
		      //
		      bool zIsNew = true;
		      
		      for (int i = 0; i < (int) _zCoordinates.size(); i++)
			{ 
			  if ( *zCoordVar == *_zCoordinates[i])
			    {
			      delete zCoordVar;
			      
			      zCoordVar = _zCoordinates[i];

			      i = (int)_zCoordinates.size();

			      zIsNew = false;
			    }
			}
		      
		      if (zIsNew)
			_zCoordinates.push_back(zCoordVar);
		    }

		  
		  //
		  // Create t CoordinateVar object
		  // 
		  CoordinateVar *tCoordVar = NULL;
	
		  if (tVar != NULL )
		    {
		      tCoordVar = new CoordinateVar(tVar);
		      
		      //
		      // Keep it if we havent seen it before
		      //
		      bool tIsNew = true;
		      
		      for (int i = 0; i < (int) _tCoordinates.size(); i++)
			{ 
			  if ( *tCoordVar == *_tCoordinates[i])
			    {
			      delete tCoordVar;
			      
			      tCoordVar = _tCoordinates[i];

			      i = (int)_tCoordinates.size();

			      tIsNew = false;
			    }
			}
		      
		      if (tIsNew)
			_tCoordinates.push_back(tCoordVar);
		    }

		  
		  //
		  // Get the NcVar* referenced by the "grid_mapping" attribute
		  // if there is one. Must clean up the attribute after use.
		  //
		  NcAtt *gridMapAtt = var->get_att("grid_mapping");

		  NcVar *projVar;

		  if (gridMapAtt)
		    {
		      char *gridMapAttName = gridMapAtt->as_string(0);

		      projVar = _ncFile->get_var(gridMapAttName);

		      delete gridMapAtt;

		      if (gridMapAttName)
			delete[] gridMapAttName;
		    }
		  else
		    projVar = NULL;

		  //
		  // Create ProjInfo object to handle reprojections of data
		  //
		  
		  ProjInfo *projInfo = new ProjInfo(projVar, yCoordVar, xCoordVar,(bool)_debug ); 
		  
		  if (projInfo->init())
		    {
		      cerr << "NcfReader::_getGriddedData(): Failure for ProjInfo class to initialize." << endl;
		      
		      continue;
		    }
		  

		  //
		  // Keep it if we havent seen it before
		  //
		  bool projIsNew = true;
		  
		  for (int i = 0; i < (int) _projInfo.size(); i++)
			{ 
			  if ( *projInfo == *_projInfo[i])
			    {
			      delete projInfo;

			      projInfo = _projInfo[i];
			      
			      i = (int)_projInfo.size();

			      projIsNew = false;
			    }
			}
		      
		      if (projIsNew)
			{
			  projInfo->createLut(_outputGrid);
			  
			  _projInfo.push_back(projInfo);
			}
		      //
		      // Create GriddedDataset object 
		      //
		      int nx =  _outputGrid->getNx();

		      int ny =  _outputGrid->getNy();

		      //GriddedDataset *griddedData = new GriddedDataset(var, tCoordVar, zCoordVar, projInfo,_debug,  
		      //					       _outputGrid->getNx(), _outputGrid->getNy());
		 
		      GriddedDataset *griddedData = new GriddedDataset(var, tCoordVar, zCoordVar, projInfo,_debug,nx,ny);

		  if ( griddedData->init() )
		    {
		      cerr << "NcfReader::_getGriddedData(): Error initializing GriddedData object from "
			   << var->name() << endl;
		      return 1;
		    } 
		  _griddedDatasets.push_back(griddedData); 
		}

	    }// end if num of dims > 2 and <=4
	}// end if var is valid
    }// end for i
  return 0;
}

int NcfReader::getNumDataDimensions() 
{ 
  if ( _griddedDatasets.size() > 0) 
    return _griddedDatasets[0]-> getNumDimensions();
  else
    return 0;
} 
 
GriddedDataset *NcfReader::getGriddedDataset(int i) 
{ 
  if ( i < (int)_griddedDatasets.size() )
  
    return _griddedDatasets[i]; 
  else
    return NULL;
}

int NcfReader::getNumGriddedDataFields()
{ 
  
  //
  // If a dataset is 4d, we count each time as separate
  // dataset since Mdv format will treat each as a separate field
  //  
  int sum = 0;

  for( int i = 0; i < (int)_griddedDatasets.size(); i++) 
    {
      sum = sum + _griddedDatasets[i]->getNt(); 
    }

  return sum;
}
