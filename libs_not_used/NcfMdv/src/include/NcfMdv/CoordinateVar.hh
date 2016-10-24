
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
// CoordinateVar.hh
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

#ifndef _COORDINATE_VAR_HH
#define _COORDINATE_VAR_HH

#include <string>
#include <vector>
#include <toolsa/pmu.h>
#include <netcdfcpp.h>

using namespace std;

#define COORDINATE_VAR_UNKNOWN -9999.0
#define COORDINATE_VAR_EPSILON .0000001

class CoordinateVar {

public:

  //
  // Constructor
  //
  CoordinateVar(NcVar *var);

  //
  // Destructor
  //
  ~CoordinateVar();
  
  const char *getStandardName() const { return _standardName; }

  const char *getLongName() const { return _longName; }

  const char *getUnits() const { return _units; }

  int getDimension() const { return _dimension; }

  int getDataType() const { return _dataType;}
  
  void extractData() const;

  double get(int i) const;

  bool operator==(const CoordinateVar &other);

private:

  NcVar *_var;

  char *_units;

  char *_standardName;

  char *_longName;

  int _dimension;

  int _dataType;

  void *_data;

};

#endif
