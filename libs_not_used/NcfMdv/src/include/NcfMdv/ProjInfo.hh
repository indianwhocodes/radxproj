
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
// ProjInfo.hh
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


#ifndef PROJ_INFO_HH
#define PROJ_INFO_HH

#include <euclid/Pjg.hh>
#include <string>
#include <vector>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <netcdfcpp.h>
#include <proj_api.h>
#include <udunits2.h>
#include <NcfMdv/CoordinateVar.hh>

using namespace std;

#define PROJ_INFO_UNKNOWN -9999.0
#define PROJ_INFO_EPSILON .0000001

//
// The following projections are supported by CF-1.0 conventions
//
typedef enum {

  PROJ_ALBERS_CONICAL_EQUAL_AREA = 0,
  PROJ_AZIMUTHAL_EQUIDISTANT = 1,
  PROJ_LAMBERT_AZIMUTHAL_EQUAL_AREA = 2,
  PROJ_LAMBERT_CONFORMAL = 3,
  PR0J_POLAR_STEREOGRAPHIC = 4,
  PROJ_ROTATED_POLE = 5,
  PROJ_STEREOGRAPHIC = 6,
  PROJ_TRANSVERSE_MERCATOR = 7,
  PROJ_LATLON = 8

} proj_type_t;

class ProjInfo {

public:
  
  //
  // Constructor
  //
  ProjInfo(NcVar *projVar, CoordinateVar *yVar, CoordinateVar *xVar, bool debug);

  //
  // Destructor
  //
  ~ProjInfo();
  
  //
  //  init(): get attributes, initialize Proj4 projection object  
  // ( see private member functions )
  //
  int init();

  int getNx() const { return _inputGridX->getDimension(); }

  int getNy() const { return _inputGridY->getDimension(); }

  //
  // Get the input grid index which corresponds to output grid 
  // index i
  //
  int getInputXYIndex( int i) const { return _indexLut[i]; }

  //
  // Create a lookup table: For each element of the output
  // grid we record the index of the corresponding element 
  // of the input grid 
  // 
  int createLut(Pjg *outputGrid);
  
  bool operator==(const ProjInfo &other);
    
private:

  //
  // Get projection attributes of NcVar* projection variable  
  //
  int _getAttributes();

  //
  // We are using the Proj.4 projection library (see http://remotesensing.org/proj
  // to handle non latitude longitude grids. This method initializes a 
  // projPJ object.
  //
  int _initializeProj4();

  //
  // members
  //
  bool _debug;

  CoordinateVar *_inputGridX;

  CoordinateVar *_inputGridY;

  NcVar *_projVar;

  int *_indexLut;

  proj_type_t _projType;

  //
  // Proj.4 
  //
  projPJ _inputProj;

  //
  // projection attributes of supported CF-1.0 projections 
  //
  float _falseEasting;

  float _falseNorthing;

  const char *_gridMappingName;

  float _gridNorthPoleLatitude;

  float _gridNorthPoleLongitude;

  float _latitudeProjectionOrigin;

  float _longitudeProjectionOrigin;

  float _longitudeCentralMeridian;

  float _northPoleGridLongitude;
  
  float _scaleFactorAtCentralMeridian;

  float _scaleFactorAtProjectionOrigin;

  float _standardParallel[2];

  float _straightVerticalLongitudeFromPole;

};

#endif
