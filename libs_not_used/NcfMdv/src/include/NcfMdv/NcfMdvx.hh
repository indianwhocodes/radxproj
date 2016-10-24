/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1992, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1993/3/3 22:0:32
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

///////////////////////////////////////////////////////////////
// NcfMdvx.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
///////////////////////////////////////////////////////////////
//
// The NcfMdvx extends the DsMdvx class, adding the capability
// to locally convert to/from netCDF CF.
//
///////////////////////////////////////////////////////////////

#ifndef NcfMdvx_HH
#define NcfMdvx_HH

#include <Mdv/DsMdvx.hh>
using namespace std;

class DsMdvxMsg;
class Mdv2NcfTrans;

///////////////////////////////////////////////////////////////
// class definition

class NcfMdvx : public DsMdvx

{

  friend class DsMdvxMsg;
  friend class Mdv2NcfTrans;

public:
  
  // constructor

  NcfMdvx();

  // copy constructor
  
  NcfMdvx(const NcfMdvx &rhs);
  
  // destructor
  
  virtual ~NcfMdvx();

  // assignment
  
  NcfMdvx & operator=(const NcfMdvx &rhs);

  // overload convert to/from NetCDF CF
  // return 0 on success, -1 on failure
  
  virtual int convertMdv2Ncf(const string &url);
  virtual int convertNcf2Mdv(const string &url);
  virtual int constrainNcf(const string &url);

  // read NCF and RADX type files
  // returns 0 on success, -1 on failure
  
  virtual int readAllHeadersNcf(const string &url);
  virtual int readNcf(const string &url);
  virtual int readAllHeadersRadx(const string &url);
  virtual int readRadx(const string &url);

  // write to directory
  // returns 0 on success, -1 on failure
  
  virtual int writeToDir(const string &output_url);

protected:

  // functions

  // overload copy method

  virtual NcfMdvx &_copy(const NcfMdvx &rhs);

private:

  int _convertNcfToMdvAndWrite(const string &url);
  int _convertMdvToNcfAndWrite(const string &url);
  int _constrainNcfAndWrite(const string &url);
  bool _getWriteAsForecast();
  string _computeNcfOutputPath(const string &outputDir);
  void _writeLdataInfo(const string &outputDir,
                       const string &outputPath,
                       const string &dataType);

};

#endif


