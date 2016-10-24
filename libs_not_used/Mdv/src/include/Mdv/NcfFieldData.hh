// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

/////////////////////////////////////////////////////////////
// NcfFieldData.hh
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008 
//
///////////////////////////////////////////////////////////////////////////////////
//
//  NcfFieldData class holds:
//    Mdv field and data
//    NcfGridInfo object( access to 2D grid info and relevant netCDF variables,
//                     plus projection information )
//    NcfVlevelInfo object (access to vertical level information and
//                       relevant netCDF variables), 
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef FIELDDATA_H
#define FIELDDATA_H

#include <string>
#include <math.h>
#include <netcdfcpp.h>
#include <Mdv/MdvxField.hh> 
#include <Mdv/DsMdvx.hh>
#include <euclid/Pjg.hh>
#include <toolsa/pmu.h>
#include <Mdv/NcfMdv.hh>
#include <Mdv/NcfGridInfo.hh>
#include <Mdv/NcfVlevelInfo.hh>

#define FIELD_DATA_EPSILON .0000001

class NcfFieldData {
  
public:
  
  /// constructor

  NcfFieldData(bool debug,
            const MdvxField *mdvField,
            const NcfGridInfo *gridInfo,
            const NcfVlevelInfo *vlevelInfo,
            const string &mdvFieldName,
            const string &ncfFieldName,
            const string &ncfStandardName,
            const string &ncfLongName,
            const string &ncfUnits,
            bool doLinearTransform,
            double linearMult,
            double linearOffset,
            DsMdvx::ncf_pack_t packing,
            bool output_latlon_arrays,
            bool compress,
            int compression_level,
            NcFile::FileFormat format);

  /// destructor

  ~NcfFieldData();
  
  /// add to NetCDF file object
  /// returns 0 on success, -1 on error
  
  int addToNc(NcFile *ncFile, NcDim *timeDim, bool outputMdvAttr, string &errStr);
  
  /// Write the data to the NcFile
  /// Returns 0 on success, -1 on error

  int writeToFile(NcFile *ncFile, string &errStr);

protected:
    
private:

  bool _debug;

  MdvxField _mdvField;
  Mdvx::field_header_t _fhdrIn;
  Mdvx::field_header_t _fhdrFl32;

  string _name;
  string _nameLong;
  string _units;

  float _maxOut;
  float _minOut;
  
  fl32 _missing;
  
  int _forecastTime;
  int _forecastDelta;

  const NcfGridInfo *_gridInfo;
  const NcfVlevelInfo *_vlevelInfo;

  string _mdvName;
  string _ncfName;
  string _ncfStandardName;
  string _ncfLongName;
  string _ncfUnits;
  bool _doLinearTransform;
  double _linearMult;
  double _linearOffset;
  DsMdvx::ncf_pack_t _packingRequested;
  DsMdvx::ncf_pack_t _packingUsed;
  NcType _ncType;
  
  float _addOffset;
  float _scaleFactor;

  bool _outputLatlonArrays;
  bool _compress;
  int _compressionLevel;
  NcFile::FileFormat _ncFormat;

  NcVar *_ncVar;

  // set compression

  int _setCompression(NcFile *ncFile, string &errStr);

};

#endif

