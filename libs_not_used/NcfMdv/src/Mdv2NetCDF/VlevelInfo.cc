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

#include <cstring>
#include <NcfMdv/VlevelInfo.hh>
#include <cstring>
#include <NcfMdv/Ncf.hh>
#include <cstring>
#include <toolsa/TaStr.hh>
#include <toolsa/mem.h>

//
// Extract NetCDF vlevel meta data from the Mdv vlevel_header_t including
// standard_name, long_name, units, and positive.
//
VlevelInfo::VlevelInfo (Mdvx::vlevel_header_t &vlevHdr, int vlevelType,
                        int nz) :
        _vlevHdr(vlevHdr)
{

  _nz = nz;

  MEM_zero(_zData);
  memcpy(_zData,  vlevHdr.level, _nz* sizeof(fl32));

  _vlevelType = vlevelType;
  
  switch( _vlevelType)
    {
    case Mdvx::VERT_TYPE_SURFACE:

      _standardName = "";

      _longName = "surface";

      _units = Ncf::level;

      _positive = Ncf::up;

      break;

    case Mdvx::VERT_TYPE_SIGMA_P:
      
      _standardName = "atmosphere_sigma_coordinate";

      _units = Ncf::sigma_level;

      _longName = "sigma p levels";

      _positive = Ncf::up;
      
      break;	

    case  Mdvx::VERT_TYPE_PRESSURE:
      
      _standardName = "air_pressure";

      _units = "mb";

      _longName = "pressure levels";

      //
      // This is an optional attribute since pressure levels
      // can be identified by pressure units.
      //
      _positive = Ncf::down;
 
      break;	     

    case  Mdvx::VERT_TYPE_Z:
      
      _standardName = "altitude";

      _units = "km";

      _longName = "constant altitude levels";

      _positive = Ncf::up;

      break;	         
      
    case  Mdvx::VERT_TYPE_SIGMA_Z:
      
      _standardName = "atmosphere_sigma_coordinate";

      _units = Ncf::level;

      _longName = "sigma z levels";

      _positive = Ncf::up;
      
      break;	
   
    case  Mdvx::VERT_TYPE_ETA:
      
      _standardName = "";

      _units = Ncf::level;

      _longName = "model eta levels";

      _positive = Ncf::up;

      break;
      
    case  Mdvx::VERT_TYPE_THETA:
      
      _standardName = "";

      _units = "degree_Kelvin";

      _longName = "isentropic surface";

      _positive = Ncf::up;

      break;	 
         
    case  Mdvx::VERT_TYPE_ELEV:
      
      _standardName = Ncf::degrees;

      _units = "degree";

      _longName = "elevation angles";

      _positive = Ncf::up;

      break;	 

    case Mdvx::VERT_FLIGHT_LEVEL:

      _standardName = "";

      _units = "100 feet";

      _longName = "Flight levels in 100s of feet";

      _positive = Ncf::up;

      break;	 

    default:
      
      _standardName = "";
      
      _units = Ncf::level;
      
      _longName = "vertical level type unknown";
      
      //
      //  this is a best guess, maybe should be a field_param
      //
      _positive = Ncf::up;
      
      break;
    }
}

VlevelInfo::~VlevelInfo()
{
 

}

//////////////////////////////////////////////////////////
// Operator equals()
// Two VelvelInfo objects are equal if the number of levels,
// vertical types are equal and all the vlevel data agrees.
//

bool VlevelInfo::operator==(const VlevelInfo &other)

{

  //
  // check nz  and vlevel type
  //
  if (_nz != other._nz || _vlevelType != other._vlevelType) {
    return false;
  }

  //
  // Check the vlevel data 
  //
  for (int i = 0; i < _nz; i++) {
    if (fabs( _zData[i] - other._zData[i]) > VLEVEL_EPSILON) {
      return false;
    }
  }
  
  return true;

}


////////////////////////////////////////////////////////////
// add dimension for this vlevel object
// returns 0 on success, -1 on failure

int VlevelInfo::addDim(int vlevelNum, NcFile *ncFile, string &errStr)

{

  char zDimName[4];
  sprintf(zDimName, "z%d", vlevelNum);

  _zDim = ncFile->add_dim(zDimName, _nz);
  if (_zDim == NULL) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// add vert level variable

int VlevelInfo::addVlevelVar(int vlevelNum, NcFile *ncFile, string &errStr)

{

  // Add variable to NcFile
  
  char zVarName[32];
  sprintf(zVarName, "z%d", vlevelNum);
  
  if ((_zVar = ncFile->add_var(zVarName, ncFloat, _zDim)) == NULL) {
    TaStr::AddStr(errStr, "Mdv2NcfTrans::VlevelInfo::addVlevelVar");
    TaStr::AddStr(errStr, "  Cannot add zVar");
    return -1;
  }
  
  // Add Attributes

  int iret = 0;

  if (_standardName.size() > 0) {
    iret |= !_zVar->add_att(Ncf::standard_name, _standardName.c_str());
  }

  // for vert section ??
  // iret |= !_zVar->add_att("standard_name", "z");
  
  iret |= !_zVar->add_att(Ncf::long_name, _longName.c_str());

  // for vert section ??
  // iret |= !_zVar->add_att("long_name",
  //                         "distance from trajectory in vertical dimension");

  iret |= !_zVar->add_att(Ncf::units, _units.c_str());
  
  if (_positive.size() > 0) {
    iret |= !_zVar->add_att(Ncf::positive, _positive.c_str());
  }

  iret |= !_zVar->add_att(Ncf::axis, "Z");

  return (iret? -1 : 0);
  
}

///////////////////////////////////////////////////////////
// Write the vlevel data
//
// Returns 0 on success, -1 on error

int VlevelInfo::writeDataToFile(NcFile *ncFile, string &errStr)
  
{
  
  if (_zVar != NULL && _zData != NULL) {
    if (!_zVar->put( _zData, _nz)) {
      TaStr::AddStr(errStr, "ERROR - VlevelInfo::writeVlevelDataToFile");
      TaStr::AddStr(errStr, "  Cannot put vlevel data");
      return -1;
    }
  }

  return 0;

}
