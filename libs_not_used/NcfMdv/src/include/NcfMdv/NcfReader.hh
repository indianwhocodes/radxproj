
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
// file.Some assumptions are made about the file that are not in the CF 
// document including the existence of start_time, stop_time, and 
// forecast_period vars. We assume the attributes "title", "institution", 
// "source", "history", "references", and "comment" exist and are global.
// 
///////////////////////////////////////////////////////////////////////////


#ifndef NCFREADER_HH
#define NCFREADER_HH

#include <string>
#include <vector>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <netcdfcpp.h>
#include <proj_api.h>
#include <udunits2.h>
#include <euclid/Pjg.hh>
#include <NcfMdv/GriddedDataset.hh>
#include <NcfMdv/ProjInfo.hh>
#include <NcfMdv/CoordinateVar.hh>

using namespace std;

class NcfReader {

public:
  //
  // constructor
  //
  NcfReader(char *filename, bool debug, Pjg *outputGrid);

  ~NcfReader();

  int readFile();
  
  int getTime() { return _time.utime(); }

  int getStartTime() { return _startTime.utime(); }

  int getStopTime() {  return _stopTime.utime(); }

  int getForecastPeriod() {  return _forecastPeriod; }  

  int getNumGriddedDatasets() { return _griddedDatasets.size(); }

  int getNumGriddedDataFields();

  GriddedDataset *getGriddedDataset(int i);

  int getNumDataDimensions();
 
  const char* getDatasetName() { return _datasetName;}

  const char* getInstitution() { return _institution; }

  const char* getSource() { return _source; }

  const char* getTitle() { return _title; }

  //
  // Methods to get data from GriddedDataset objects
  //
  int getGridNx(int i) { return _griddedDatasets[i]->getNx();}

  int getGridNy(int i) { return _griddedDatasets[i]->getNy();}

  int getGridNz(int i) { return _griddedDatasets[i]->getNz();}

  int getGridNt(int i) { return _griddedDatasets[i]->getNt();}

  double getGridZ(int i, int level) { return _griddedDatasets[i]->getZ(level);}

  double getGridT(int i, int tstep) { return _griddedDatasets[i]->getT(tstep);}

  bool gridHasVlevelInfo(int i) {  return _griddedDatasets[i]->hasVlevelInfo();}

  const char *getGridStandardName(int i) { return _griddedDatasets[i]->getStandardName();}

  const char *getGridLongName(int i) { return _griddedDatasets[i]->getLongName();}

  const char *getGridVarName(int i) { return _griddedDatasets[i]->getVarName();}

  const char *getGridUnits(int i) { return _griddedDatasets[i]->getUnits();}

  const float *getGridData(int i) { return _griddedDatasets[i]->get(); }

  float getGridFillValue(int i) { return _griddedDatasets[i]->getFillValue(); }

  int getGridDataType(int i) { return _griddedDatasets[i]->getDataType(); }

private:

  void _getDatasetName();

  //
  // set _title, _institution, _source, _history, _references, _comment
  //
  int _getGlobalAttributes();

  //
  // set _time, _startTime, _stopTime, _validTime, _writeTime
  // 
  int _getTimeInfo();

  //
  // Get time variable by name
  // 
  int _getTimeVar(char * timeVarName, long& timeData);

  //
  // Get the gridded data fields
  //
  int _getGriddedData();

  //
  // Members
  //
  string _ncfFileStr;

  Pjg *_outputGrid;

  bool _debug;

  NcFile *_ncFile;

  char * _datasetName;

  int _numVars;

  int _numDims;

  //
  // Description of file contents. 
  // The attributes below are required for CF-1.0 compliance.
  //
  
  //
  // A succinct description of the dataset contents
  //
  char * _title;

  //
  // Specifies where the original data was produced
  // 
  char * _institution;
  
  //
  // Method of production of original data (eg. model/version, observational
  // source like radiosondeetc.
  //
  char * _source;

  //
  // Provides history of modifications to file. Each line should begin with
  // timestamp of dateaand time that program was executed.
  //
  char * _history;
  
  //
  // Published or web based references that describe the data.
  //
  char * _references;

  //
  // Miscellaneous information about the data or methods used to produce it
  //
  char * _comment;
  
  //
  // Conventions (should be CF-1.0 for this code to work)
  //
  char * _conventions;

  //
  // Time variables
  //
  DateTime _time;

  DateTime _startTime;

  DateTime _stopTime;
  
  DateTime _forecastReferenceTime;

  int _forecastPeriod;


  //
  // 
  //
  vector <GriddedDataset* > _griddedDatasets;
 
  vector <ProjInfo *> _projInfo;

  vector <CoordinateVar *> _xCoordinates;

  vector <CoordinateVar *> _yCoordinates;

  vector <CoordinateVar *> _zCoordinates;

  vector <CoordinateVar *> _tCoordinates;

  
};

#endif
