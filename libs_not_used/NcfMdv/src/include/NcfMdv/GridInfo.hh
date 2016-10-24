/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 2008, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/////////////////////////////////////////////////////////////
// GridInfo.hh
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008 
//
///////////////////////////////////////////////////////////////
//
//  GridInfo object will bridge the Mdv projection info with the
//  netCDF dimensions, coordinate variables,  and auxilliary variables 
//  (if relevant). 
//
///////////////////////////////////////////////////////////////////////

#ifndef GRID_INFO_H
#define GRID_INFO_H

#include <string>
#include <math.h>
#include <netcdfcpp.h>
#include <Mdv/MdvxField.hh> 
#include <Mdv/DsMdvx.hh>
#include <euclid/Pjg.hh>
#include <toolsa/pmu.h>

using namespace std;

////////////////////////
// This class

class GridInfo {
  
public:

  /// constructor

  GridInfo (Mdvx::field_header_t fHdr);

  /// destructor
  
  ~GridInfo();
  
  ///  Compute projection x and y arrays. If data is NOT in lat lon projection
  ///  compute the auxiliary 2D coordinate variables. See Section 5 of CF-1.0 
  ///  Coordnate Systems for discussion on auxiliary coordinate variables
  
  int computeCoordinateVars();
  
  /// For an Xsection, set the CoordinateVars from the sample points

  int setCoordinateVarsFromSamplePoints(vector<Mdvx::vsect_samplept_t> pts);

  /// add xy dimension for this grid
  /// returns 0 on success, -1 on failure
  
  int addXyDim(int gridNum, NcFile *ncFile, string &errStr);

  /// add projection variable for this grid
  
  int addProjVar(int projNum, NcFile *ncFile, string &errStr);

  /// add coordinate variables for this grid
  
  int addCoordVars(int gridNum, bool outputLatlonArrays,
                   NcFile *ncFile, string &errStr);

  /// add vert section coordinate variables for this grid
  
  int addVsectCoordVars(int gridNum,
                        NcFile *ncFile, string &errStr);
    
  /// Write the coordinate data to file
  /// Returns 0 on success, -1 on error
  
  int writeCoordDataToFile(NcFile *ncFile, string &errStr);
  
  // Get methods
  
  NcDim *getNcXdim() const { return _xDim; }
  NcDim *getNcYdim() const { return _yDim; }
  NcVar *getNcXvar() const { return _xVar; }
  NcVar *getNcYvar() const { return _yVar; }
  NcVar *getNcLatVar() const { return _latVar; }
  NcVar *getNcLonVar() const { return _lonVar; }
  
  Mdvx::projection_type_t getProjType() const { return _proj.getProjType();}
  const NcVar *getNcProjVar()  const{ return _projVar;}

  bool getOutputLatlonArrays() const { return _outputLatlonArrays; }

  /// equality  -- Assume the objects are equal if their coordinate structures
  /// are equal.  This requires that the structure be initialized to all zeros
  /// before setting any of the structure's fields.

  bool operator==(const GridInfo &other) const;
  
protected:
  
private:
 
  Mdvx::field_header_t _fHdr;
  MdvxProj _proj;
  Mdvx::coord_t _coord;
  NcVar *_projVar;
  bool _outputLatlonArrays;
  bool _isXSect;
  
  NcDim *_xDim;
  NcDim *_yDim;

  NcVar *_xVar;
  NcVar *_yVar;
  NcVar *_latVar;
  NcVar *_lonVar;
  NcVar *_altVar;

  float *_xData;
  float *_yData;
  float *_lonData;
  float *_latData;
  
  void _clear();

};

#endif

