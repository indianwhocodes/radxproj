/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 2008, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/////////////////////////////////////////////////////////////
// VlevelInfo.hh
//
// Vertical level information object for gridded dataset
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008 
//
///////////////////////////////////////////////////////////////
//
//  VlevelInfo object will bridge the Mdv vlevel info with the
//  netCDF dimensions and vertical coordinate variables. 
//
///////////////////////////////////////////////////////////////////////

#ifndef VLEVEL_INFO_H
#define VLEVEL_INFO_H

#include <string>
#include <math.h>
#include <netcdfcpp.h>
#include <Mdv/MdvxField.hh> 
#include <Mdv/DsMdvx.hh>
#include <euclid/Pjg.hh>
#include <toolsa/pmu.h>

using namespace std;

#define VLEVEL_EPSILON .0000001

////////////////////////
// This class

class VlevelInfo {
  
public:

  /// constructor

  VlevelInfo (Mdvx::vlevel_header_t &vlevHdr, int vlevelType, int nz);

  /// destructor
  
  ~VlevelInfo();
  
  /// add dimension for this object

  int addDim(int vlevelNum, NcFile *ncFile, string &errStr);

  /// add vert level variable to NcFile object
  
  int addVlevelVar(int vlevelNum, NcFile *ncFile, string &errStr);

  /// Write the vlevel data
  /// Returns 0 on success, -1 on error

  int writeDataToFile(NcFile *ncFile, string &errStr);

  /// Get methods

  NcDim *getNcZdim() const { return _zDim; }
  NcVar *getNcZvar() const { return _zVar; }

  /// equality

  bool operator==(const VlevelInfo &other);
  
protected:
  
private:
  
  Mdvx::vlevel_header_t _vlevHdr;

  string _units;

  string _longName;

  string _standardName;

  string _positive;

  int _vlevelType;

  int _nz;

  NcDim *_zDim;

  NcVar *_zVar;

  float _zData[MDV_MAX_VLEVELS];

};

#endif

