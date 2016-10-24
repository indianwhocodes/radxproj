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
// GriddedDataset.hh
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


#ifndef GRIDDED_DATASET_HH
#define GRIDDED_DATASET_HH

#include <string>
#include <vector>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <netcdfcpp.h>
#include <proj_api.h>
#include <NcfMdv/CoordinateVar.hh>
#include <NcfMdv/ProjInfo.hh>

using namespace std;

#define GRIDDED_DATASET_UNKNOWN -9999.0

class GriddedDataset {

public:
  
  //
  // Constructor
  //
  GriddedDataset(NcVar *var, CoordinateVar *tCoordVar, 
		 CoordinateVar *zCoordVar, ProjInfo *projInfo, 
		 bool debug, int outputGridNx, int outputGridNy);
  //
  // Destructor
  //
  ~GriddedDataset();

  int getNumDimensions() const { return _numDimensions; }

  int getNx() const { return _projInfo->getNx(); }

  int getNy() const { return _projInfo->getNy(); }

  int getNz() const;

  int getNt() const;

  double getZ(int i) const;

  double getT(int i) const;

  bool hasVlevelInfo() const;

  bool hasTimeInfo() const;
  
  const char *getStandardName() const;

  const char *getLongName()  const{ return _longName; }

  const char *getVarName()  const{ return _varName; }

  const char *getUnits()  const{ return _units; }

  int init();

  const float *get() const { return _outputData; }
  
  float getFillValue()  const { return _fillValue; }

  int getDataType() const { return (int)_dataType; }

private:

  //
  // member methods
  //
  void _getAttributes();

  int _getDimensionInfo();

  int _getGriddedDataArray();

  int _projectData();
  
  int _getFlagValues(NcAtt *att);

  //
  // Constructor args
  //
  NcVar *_var;

  ProjInfo *_projInfo;

  CoordinateVar *_zCoordVar;

  CoordinateVar *_tCoordVar;

  bool _debug;
  
  int _outNx;
  
  int _outNy;
  

  //
  // NcVar attributes
  //
  NcType _dataType;

  const char *_varName;

  float _fillValue;

  float _addOffset;

  float _scaleFactor;

  float _validMin;

  float _validMax;

  char * _standardName;

  char * _longName;

  char *_units;

  char * _ancillaryVariables;

  float *_flagValues;

  char *_flagMeanings;

  int _numFlags;
 
  bool _validMinSet;

  bool _validMaxSet;

  bool _fillValueSet;

  //
  // input output data array related members
  //
  float *_data;
  
  float *_outputData;

  int _numDimensions;

  long *_dataDimensions;

  bool _isFlagData;
 
  int _inputDataArraySize;

};

#endif
