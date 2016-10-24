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
// OdimHdf5RadxFile.cc
//
// ODIM HDF5 file data for radar radial data
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2013
//
///////////////////////////////////////////////////////////////

#include <Radx/NcfRadxFile.hh>
#include <Radx/OdimHdf5RadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxComplex.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxReadDir.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>

//////////////
// Constructor

OdimHdf5RadxFile::OdimHdf5RadxFile() : RadxFile()
  
{

  _volumeNumber = 0;
  _ncFormat = NETCDF_CLASSIC;
  _readVol = NULL;
  clear();
  
}

/////////////
// destructor

OdimHdf5RadxFile::~OdimHdf5RadxFile()

{
  clear();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void OdimHdf5RadxFile::clear()
  
{

  clearErrStr();

  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::PLATFORM_TYPE_FIXED;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;
  _nSweeps = 0;

  _conventions.clear();

  _objectStr.clear();
  _version.clear();
  _dateStr.clear();
  _timeStr.clear();
  _source.clear();
  
  _latitudeDeg = Radx::missingMetaDouble;
  _longitudeDeg = Radx::missingMetaDouble;
  _altitudeKm = Radx::missingMetaDouble;

  _task.clear();
  _system.clear();
  _simulated.clear();
  _software.clear();
  _swVersion.clear();

  _wavelengthM = Radx::missingMetaDouble;
  _frequencyHz = Radx::missingMetaDouble;
  _scanRateRpm = Radx::missingMetaDouble;
  _scanRateDegPerSec = Radx::missingMetaDouble;
  
  _pulseWidthUs = Radx::missingMetaDouble;
  _rxBandwidthMhz = Radx::missingMetaDouble;
  
  _lowPrfHz = Radx::missingMetaDouble;
  _highPrfHz = Radx::missingMetaDouble;

  _prtMode = Radx::PRT_MODE_FIXED;
  _prtRatio = Radx::missingMetaDouble;
  
  _txLossDb = Radx::missingMetaDouble;
  _rxLossDb = Radx::missingMetaDouble;
  _radomeLossOneWayDb = Radx::missingMetaDouble;
  _antennaGainDb = Radx::missingMetaDouble;

  _beamWidthHDeg = Radx::missingMetaDouble;
  _beamWidthVDeg = Radx::missingMetaDouble;

  _gasAttenDbPerKm = Radx::missingMetaDouble;

  _radarConstantH = Radx::missingMetaDouble;
  _radarConstantV = Radx::missingMetaDouble;

  _nomTxPowerKw = Radx::missingMetaDouble;

  _unambigVelMps = Radx::missingMetaDouble;
  
  _nSamples = 0;

  _azMethod.clear();
  _binMethod.clear();

  _pointAccEl = Radx::missingMetaDouble;
  _pointAccAz = Radx::missingMetaDouble;

  _malfuncFlag = false;
  _malfuncMsg.clear();

  _maxRangeKm = Radx::missingMetaDouble;

  _dbz0 = Radx::missingMetaDouble;

  _comment.clear();

  _sqiThreshold = Radx::missingMetaDouble;
  _csrThreshold = Radx::missingMetaDouble;
  _logThreshold = Radx::missingMetaDouble;
  _snrThreshold = Radx::missingMetaDouble;

  _peakPowerKw = Radx::missingMetaDouble;
  _avPowerKw = Radx::missingMetaDouble;
  _dynRangeDb = Radx::missingMetaDouble;
  
  _polarization.clear();

  _nGates = 0;
  _nDataFields = 0;
  _nQualityFields = 0;
  _nFields = 0;
  _isRhi = false;
  _fixedAngleDeg = Radx::missingMetaDouble;
  _startRangeKm = Radx::missingMetaDouble;
  _gateSpacingKm = Radx::missingMetaDouble;
  _nRaysSweep = 0;

  _product.clear();
  _fieldName.clear();
  _startDateStr.clear();
  _startTimeStr.clear();
  _endDateStr.clear();
  _endTimeStr.clear();
  _scale = Radx::missingMetaDouble;
  _offset = Radx::missingMetaDouble;
  _missingDataVal = Radx::missingMetaDouble;
  _lowDataVal = Radx::missingMetaDouble;

  _sweepStatusXml.clear();
  _statusXml.clear();

}

/////////////////////////////////////////////////////////
// Check if specified file is CfRadial format
// Returns true if supported, false otherwise

bool OdimHdf5RadxFile::isSupported(const string &path)

{
  
  if (isOdimHdf5(path)) {
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////
// Check if this is a OdimHdf5 file
// Returns true on success, false on failure

bool OdimHdf5RadxFile::isOdimHdf5(const string &path)
  
{

  clear();
  
  if (!H5File::isHdf5(path)) {
    if (_verbose) {
      cerr << "DEBUG - not OdimHdf5 file" << endl;
    }
    return false;
  }

  // suppress automatic exception printing so we can handle
  // errors appropriately

  Exception::dontPrint();

  // open file

  H5File file(path, H5F_ACC_RDONLY);

  // check for how group

  Group *how = NULL;
  try {
    how = new Group(file.openGroup("how"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'how' group, not ODIM file" << endl;
    }
    if (how) delete how;
    return false;
  }
  delete how;

  // check for what group

  Group *what = NULL;
  try {
    what = new Group(file.openGroup("what"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'what' group, not ODIM file" << endl;
    }
    if (what) delete what;
    return false;
  }
  delete what;

  // check for where group

  Group *where = NULL;
  try {
    where = new Group(file.openGroup("where"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'where' group, not ODIM file" << endl;
    }
    if (where) delete where;
    return false;
  }
  delete where;

  // check for dataset1 group

  Group *dataset1 = NULL;
  try {
    dataset1 = new Group(file.openGroup("dataset1"));
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "No 'dataset1' group, not ODIM file" << endl;
    }
    if (dataset1) delete dataset1;
    return false;
  }
  delete dataset1;

  // good

  return true;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified directory
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::writeToDir(const RadxVol &vol,
                                 const string &dir,
                                 bool addDaySubDir,
                                 bool addYearSubDir)
  
{

  // Writing OdimHdf5 files is not supported
  // therefore write in CF Radial format instead
  
  cerr << "WARNING - OdimHdf5RadxFile::writeToDir" << endl;
  cerr << "  Writing OdimHdf5 raw format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;
  
  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToDir(vol, dir, addDaySubDir, addYearSubDir);

  // set return values

  _errStr = ncfFile.getErrStr();
  _dirInUse = ncfFile.getDirInUse();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// Write data from volume to specified path
//
// Writes not supported for this class
// Use NcfRadxFile methods instead.
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::writeToPath(const RadxVol &vol,
                                  const string &path)
  
{

  // Writing OdimHdf5 files is not supported
  // therefore write in CF Radial format instead

  cerr << "WARNING - OdimHdf5RadxFile::writeToPath" << endl;
  cerr << "  Writing OdimHdf5 raw format files not supported" << endl;
  cerr << "  Will write CfRadial file instead" << endl;

  // set up NcfRadxFile object

  NcfRadxFile ncfFile;
  ncfFile.copyWriteDirectives(*this);

  // perform write

  int iret = ncfFile.writeToPath(vol, path);

  // set return values

  _errStr = ncfFile.getErrStr();
  _pathInUse = ncfFile.getPathInUse();
  vol.setPathInUse(_pathInUse);

  return iret;

}

/////////////////////////////////////////////////////////
// print summary after read

void OdimHdf5RadxFile::print(ostream &out) const
  
{
  
  out << "=============== OdimHdf5RadxFile ===============" << endl;
  RadxFile::print(out);

  out << "  volumeNumber: " << _volumeNumber << endl;
  out << "  instrumentType: " 
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  nSweeps: " << _nSweeps << endl;
  out << "  conventions: " << _conventions << endl;
  out << "  objectStr: " << _objectStr << endl;
  out << "  version: " << _version << endl;
  out << "  dateStr: " << _dateStr << endl;
  out << "  timeStr: " << _timeStr << endl;
  out << "  source: " << _source << endl;
  out << "  latitudeDeg: " << _latitudeDeg << endl;
  out << "  longitudeDeg: " << _longitudeDeg << endl;
  out << "  altitudeKm: " << _altitudeKm << endl;
  out << "  task: " << _task << endl;
  out << "  system: " << _system << endl;
  out << "  simulated: " << _simulated << endl;
  out << "  software: " << _software << endl;
  out << "  swVersion: " << _swVersion << endl;
  out << "  wavelengthM: " << _wavelengthM << endl;
  out << "  frequencyHz: " << _frequencyHz << endl;
  out << "  scanRateRpm: " << _scanRateRpm << endl;
  out << "  scanRateDegPerSec: " << _scanRateDegPerSec << endl;
  out << "  pulseWidthUs: " << _pulseWidthUs << endl;
  out << "  rxBandwidthMhz: " << _rxBandwidthMhz << endl;
  out << "  lowPrfHz: " << _lowPrfHz << endl;
  out << "  highPrfHz: " << _highPrfHz << endl;
  out << "  prtMode: " << Radx::prtModeToStr(_prtMode) << endl;
  out << "  prtRatio: " << _prtRatio << endl;
  out << "  txLossDb: " << _txLossDb << endl;
  out << "  rxLossDb: " << _rxLossDb << endl;
  out << "  radomeLossOneWayDb: " << _radomeLossOneWayDb << endl;
  out << "  antennaGainDb: " << _antennaGainDb << endl;
  out << "  beamWidthHDeg: " << _beamWidthHDeg << endl;
  out << "  beamWidthVDeg: " << _beamWidthVDeg << endl;
  out << "  gasAttenDbPerKm: " << _gasAttenDbPerKm << endl;
  out << "  radarConstantH: " << _radarConstantH << endl;
  out << "  radarConstantV: " << _radarConstantV << endl;
  out << "  nomTxPowerKw: " << _nomTxPowerKw << endl;
  out << "  unambigVelMps: " << _unambigVelMps << endl;
  out << "  nSamples: " << _nSamples << endl;
  out << "  azMethod: " << _azMethod << endl;
  out << "  binMethod: " << _binMethod << endl;
  out << "  pointAccEl: " << _pointAccEl << endl;
  out << "  pointAccAz: " << _pointAccAz << endl;
  out << "  malfuncFlag: " << (_malfuncFlag?"Y":"N") << endl;
  out << "  malfuncMsg: " << _malfuncMsg << endl;
  out << "  maxRangeKm: " << _maxRangeKm << endl;
  out << "  dbz0: " << _dbz0 << endl;
  out << "  comment: " << _comment << endl;
  out << "  sqiThreshold: " << _sqiThreshold << endl;
  out << "  csrThreshold: " << _csrThreshold << endl;
  out << "  logThreshold: " << _logThreshold << endl;
  out << "  snrThreshold: " << _snrThreshold << endl;
  out << "  peakPowerKw: " << _peakPowerKw << endl;
  out << "  avPowerKw: " << _avPowerKw << endl;
  out << "  dynRangeDb: " << _dynRangeDb << endl;
  out << "  polarization: " << _polarization << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  nDataFields: " << _nDataFields << endl;
  out << "  nQualityFields: " << _nQualityFields << endl;
  out << "  nFields: " << _nFields << endl;
  out << "  isRhi: " << (_isRhi?"Y":"N") << endl;
  out << "  fixedAngleDeg: " << _fixedAngleDeg << endl;
  out << "  startRangeKm: " << _startRangeKm << endl;
  out << "  gateSpacingKm: " << _gateSpacingKm << endl;
  out << "  nRaysSweep: " << _nRaysSweep << endl;
  out << "  product: " << _product << endl;
  out << "  quantity: " << _fieldName << endl;
  out << "  startDateStr: " << _startDateStr << endl;
  out << "  startTimeStr: " << _startTimeStr << endl;
  out << "  endDateStr: " << _endDateStr << endl;
  out << "  endTimeStr: " << _endTimeStr << endl;
  out << "  scale: " << _scale << endl;
  out << "  offset: " << _offset << endl;
  out << "  missingDataVal: " << _missingDataVal << endl;
  out << "  lowDataVal: " << _lowDataVal << endl;
  out << "  sweepStatusXml: " << _sweepStatusXml << endl;
  out << "  statusXml: " << _statusXml << endl;
  
  out << "===========================================" << endl;

}

////////////////////////////////////////////////////////////
// Print native data in uf file
// Returns 0 on success, -1 on failure
// Use getErrStr() if error occurs

int OdimHdf5RadxFile::printNative(const string &path, ostream &out,
                                  bool printRays, bool printData)
  
{

  if (!H5File::isHdf5(path)) {
    return false;
  }
  
  // open file
  
  H5File file(path, H5F_ACC_RDONLY);
  
  out << "Printing ODIM HDF5 contents" << endl;
  out << "  file path: " << file.getFileName() << endl;
  out << "  file size: " << file.getFileSize() << endl;
  
  try {
    Group root(file.openGroup("/"));
    _utils.printGroup(root, "/", out, printRays, printData);
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - trying to read ODIM HDF5 file");
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure
//
// Use getErrStr() if error occurs

int OdimHdf5RadxFile::readFromPath(const string &path,
                                   RadxVol &vol)
  
{

  if (_debug) {
    cerr << "Reading ODIM HDF5 file, master path: " << path << endl;
  }

  _initForRead(path, vol);
  _volumeNumber++;

  // is this a Gematronik file? - each field is stored in a separate file

  string dateStr, fieldName, volName;
  if (_isGematronikFieldFile(path, dateStr, fieldName, volName)) {
    return _readGemFieldFiles(path, dateStr, fieldName, vol);
  }

  // not gematronik - read single file

  return _readFromPath(path, vol);

}

////////////////////////////////////////////////////////////
// Read in data from specified path, load up volume object.
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_readFromPath(const string &path,
                                    RadxVol &vol)
  
{

  if (_debug) {
    cerr << "_readFromPath, reading ODIM HDF5 file, path: " << path << endl;
  }

  // initialize status XML

  _statusXml.clear();
  _statusXml += RadxXml::writeStartTag("Status", 0);
  
  string errStr("ERROR - OdimHdf5RadxFile::readFromPath");
  if (!H5File::isHdf5(path)) {
    _addErrStr("ERROR - not a ODIM HDF5 file");
    return -1;
  }
  
  // use try block to catch any exceptions
  
  try {

    // open file
    
    H5File file(path, H5F_ACC_RDONLY);
    if (_debug) {
      cerr << "  file size: " << file.getFileSize() << endl;
    }
    
    // get the root group
    
    Group root(file.openGroup("/"));

    // root attributes

    Hdf5Utils::DecodedAttr decodedAttr;
    _utils.loadAttribute(root, "Conventions", "root-attr", decodedAttr);
    _conventions = decodedAttr.getAsString();

    // set the number of sweeps

    if (_getNSweeps(root)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::readFromPath");
      _addErrStr("  path: ", path);
      return -1;
    }

    // read the root how, what and where groups
    
    if (_readRootSubGroups(root)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::readFromPath");
      _addErrStr("  path: ", path);
      return -1;
    }

    // read the sweeps
    
    for (int isweep = 0; isweep < _nSweeps; isweep++) {
      if (_readSweep(root, isweep)) {
        return -1;
      }
      _statusXml += _sweepStatusXml;
    }

  }

  catch (H5::Exception e) {
    _addErrStr("ERROR - reading ODIM HDF5 file");
    return -1;
  }

  // finalize status xml

  _setStatusXml();
  _statusXml += RadxXml::writeEndTag("Status", 0);

  // append to read paths
  
  _readPaths.push_back(path);

  // load the data into the read volume
  
  if (_finalizeReadVolume()) {
    return -1;
  }
  
  // set format as read

  _fileFormat = FILE_FORMAT_DOE_NC;

  return 0;

}

//////////////////////////////////////////////
// get the number of sweeps in the vol

int OdimHdf5RadxFile::_getNSweeps(Group &root)

{
  
  // init

  _nSweeps = 0;

  // look through all objects, counting up the data sets
  
  for (size_t ii = 0; ii <= root.getNumObjs(); ii++) {
    
    char datasetName[128];
    sprintf(datasetName, "dataset%d", (int) ii);
    
    Group *ds = NULL;
    try {
      ds = new Group(root.openGroup(datasetName));
    }
    catch (H5::Exception e) {
      // data set does not exist
      if (ds) delete ds;
      continue;
    }
    _nSweeps = ii;
    delete ds;
    
  }

  if (_nSweeps == 0) {
    _addErrStr("ERROR - no sweeps found");
    return -1;
  }

  if (_debug) {
    cerr << "  _nSweeps: " << _nSweeps << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// get the number of fields in a sweep

int OdimHdf5RadxFile::_getNFields(Group &sweep)
  
{
  
  // init

  _nDataFields = 0;
  _nQualityFields = 0;
  _nFields = 0;

  // look through all objects, counting up the data sets
  
  for (size_t ii = 0; ii <= sweep.getNumObjs(); ii++) {
    char dataName[128];
    sprintf(dataName, "data%d", (int) ii);
    Group *data = NULL;
    try {
      data = new Group(sweep.openGroup(dataName));
    }
    catch (H5::Exception e) {
      // data set does not exist
      if (data) delete data;
      continue;
    }
    _nDataFields = ii;
    delete data;
  }

  for (size_t ii = 0; ii <= sweep.getNumObjs(); ii++) {
    char qualityName[128];
    sprintf(qualityName, "quality%d", (int) ii);
    Group *quality = NULL;
    try {
      quality = new Group(sweep.openGroup(qualityName));
    }
    catch (H5::Exception e) {
      // quality set does not exist
      if (quality) delete quality;
      continue;
    }
    _nQualityFields = ii;
    delete quality;
  }

  _nFields = _nDataFields + _nQualityFields;

  if (_nFields == 0) {
    _addErrStr("ERROR - no fields found");
    return -1;
  }
  
  if (_debug) {
    cerr << "  _nFields: " << _nFields << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////
// clear the sweep variables

void OdimHdf5RadxFile::_clearSweepVars()

{

  _sweepStatusXml.clear();
  _fixedAngleDeg = Radx::missingMetaDouble;
  _malfuncFlag = false;
  _malfuncMsg.clear();
  _nGates = 0;
  _nRaysSweep = 0;
  _nFields = 0;

}

//////////////////////////////////////////////
// read sweep

int OdimHdf5RadxFile::_readSweep(Group &root, int sweepNumber)

{
  
  // clear
  
  _clearSweepVars();
  
  // compute dataset name: dataset1, dataset2 etc ...

  char sweepName[128];
  sprintf(sweepName, "dataset%d", sweepNumber + 1);

  if (_debug) {
    cerr << "===== reading sweep " << sweepNumber << " "
         << "group: " << sweepName
         << " =====" << endl;
  }
  
  // open scan group
  
  Group sweep(root.openGroup(sweepName));
  
  // read sweep what group
  
  Group *what = NULL;
  try {
    char label[128];
    sprintf(label, "%s what", sweepName);
    what = new Group(sweep.openGroup("what"));
    if (_readSweepWhat(*what, label)) {
      delete what;
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_debug) {
      cerr << "NOTE - no 'what' group for sweep: " << sweepName << endl;
    }
  }
  if (what) delete what;
  
  // read sweep where group
  
  Group *where = NULL;
  try {
    char label[128];
    sprintf(label, "%s where", sweepName);
    where = new Group(sweep.openGroup("where"));
    if (_readSweepWhere(*where, label)) {
      delete where;
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_debug) {
      cerr << "NOTE - no 'where' group for sweep: " << sweepName << endl;
    }
  }
  if (where) delete where;

  // read sweep how group
  
  Group *how = NULL;
  try {
    how = new Group(sweep.openGroup("how"));
    char label[128];
    sprintf(label, "%s how", sweepName);
    if (_readSweepHow(*how, label)) {
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "NOTE - no 'how' group for sweep: " << sweepName << endl;
    }
  }
  if (how) delete how;
  
  // check if this sweep is required
  
  if (_readFixedAngleLimitsSet && _readStrictAngleLimits) {
    if (_fixedAngleDeg < _readMinFixedAngle ||
        _fixedAngleDeg > _readMaxFixedAngle) {
      _clearSweepVars();
      return 0;
    }
  } else if (_readSweepNumLimitsSet && _readStrictAngleLimits) {
    if (sweepNumber < _readMinSweepNum ||
        sweepNumber > _readMaxSweepNum) {
      _clearSweepVars();
      return 0;
    }
  }

  // create rays for this sweep

  _createRaysForSweep(sweepNumber);
  
  // determine number of fields
  
  if (_getNFields(sweep)) {
    _addErrInt("ERROR - no fields in sweep: ", sweepNumber);
    return -1;
  }

  // add data fields to rays
  
  for (int ifield = 0; ifield < _nDataFields; ifield++) {
    if (_addFieldToRays("data", sweep, _sweepRays, ifield)) {
      // free up rays from array
      for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
        delete _sweepRays[ii];
      }
      _sweepRays.clear();
      return -1;
    }
  } // ifield
  
  // add quality fields to rays
  
  for (int ifield = 0; ifield < _nQualityFields; ifield++) {
    if (_addFieldToRays("quality", sweep, _sweepRays, ifield)) {
      // free up rays from array
      for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
        delete _sweepRays[ii];
      }
      _sweepRays.clear();
      return -1;
    }
  } // ifield
  
  // add rays to vol in time order
  
  for (size_t ii = 0; ii < _sweepRays.size(); ii++) {
    // first ray index is a1gate
    int jj = (ii + _a1Gate) % _nRaysSweep;
    _readVol->addRay(_sweepRays[jj]);
  }
  _sweepRays.clear();

  return 0;

}

//////////////////////////////////////////////
// set status xml

void OdimHdf5RadxFile::_setStatusXml()

{
  
  _statusXml += RadxXml::writeDouble("pointAccEl", 1, _pointAccEl);
  _statusXml += RadxXml::writeDouble("pointAccAz", 1, _pointAccAz);
  _statusXml += RadxXml::writeString("azMethod", 1, _azMethod);
  _statusXml += RadxXml::writeString("binMethod", 1, _binMethod);
  _statusXml += RadxXml::writeString("polarization", 1, _polarization);
  _statusXml += RadxXml::writeDouble("sqiThreshold", 1, _sqiThreshold);
  _statusXml += RadxXml::writeDouble("csrThreshold", 1, _csrThreshold);
  _statusXml += RadxXml::writeDouble("logThreshold", 1, _logThreshold);
  _statusXml += RadxXml::writeDouble("snrThreshold", 1, _snrThreshold);
  _statusXml += RadxXml::writeDouble("peakPowerKw", 1, _peakPowerKw);
  _statusXml += RadxXml::writeDouble("avPowerKw", 1, _avPowerKw);
  _statusXml += RadxXml::writeDouble("dynRangeDb", 1, _dynRangeDb);
  
}
  
//////////////////////////////////////////////
// set sweep status xml

void OdimHdf5RadxFile::_setSweepStatusXml(int sweepNum)

{
  
  // initialize

  char tag[128];
  sprintf(tag, "SweepStatus_%d", sweepNum);
  _sweepStatusXml += RadxXml::writeStartTag(tag, 1);

  _sweepStatusXml += RadxXml::writeInt("a1Gate", 2, _a1Gate);
  _sweepStatusXml += RadxXml::writeDouble("maxRangeKm", 2, _maxRangeKm);
  if (_simulated.size() > 0) {
    _sweepStatusXml += RadxXml::writeString("simulated", 2, _simulated);
  }
  _sweepStatusXml += RadxXml::writeString("startTime", 2, _startDateStr + _startTimeStr);
  _sweepStatusXml += RadxXml::writeString("endTime", 2, _endDateStr + _endTimeStr);
  
  // finalize XML
  
  _sweepStatusXml += RadxXml::writeEndTag(tag, 1);

  if (_debug) {
    cerr << "========= sweep status XML ===============" << endl;
    cerr << _sweepStatusXml;
    cerr << "==========================================" << endl;
  }

}
  
//////////////////////////////////////////////
// read the root how, where and what groups

int OdimHdf5RadxFile::_readRootSubGroups(Group &root)

{

  Group what(root.openGroup("what"));
  if (_readRootWhat(what)) {
    return -1;
  }

  Group where(root.openGroup("where"));
  if (_readRootWhere(where)) {
    return -1;
  }

  Group how(root.openGroup("how"));
  _readHow(how, "root how");

  return 0;

}

//////////////////////////////////////////////
// read the root what group

int OdimHdf5RadxFile::_readRootWhat(Group &what)

{

  Hdf5Utils::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(what, "object", "root-what", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _objectStr = decodedAttr.getAsString();

  if (_objectStr != "PVOL" && _objectStr != "SCAN") {
    _addErrStr("Bad object type: ", _objectStr);
    _addErrStr("  Must be 'PVOL' or 'SCAN'");
    return -1;
  }

  _utils.loadAttribute(what, "version", "root-what", decodedAttr);
  _version = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "date", "root-what", decodedAttr);
  _dateStr = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "time", "root-what", decodedAttr);
  _timeStr = decodedAttr.getAsString();
  
  _utils.loadAttribute(what, "source", "root-what", decodedAttr);
  _source = decodedAttr.getAsString();
  
  if (_debug) {
    cerr  << "  root what _objectStr: " << _objectStr << endl;
    cerr  << "  root what _version: " << _version << endl;
    cerr  << "  root what _dateStr: " << _dateStr << endl;
    cerr  << "  root what _timeStr: " << _timeStr << endl;
    cerr  << "  root what _source: " << _source << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the root where group

int OdimHdf5RadxFile::_readRootWhere(Group &where)

{

  Hdf5Utils::DecodedAttr decodedAttr;
  
  if (_utils.loadAttribute(where, "height", "root-where", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _altitudeKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_utils.loadAttribute(where, "lat", "root-where", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _latitudeDeg = decodedAttr.getAsDouble();
  
  if (_utils.loadAttribute(where, "lon", "root-where", decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _longitudeDeg = decodedAttr.getAsDouble();
  
  if (_debug) {
    cerr  << "  root where _altitudeKm: " << _altitudeKm << endl;
    cerr  << "  root where _latitudeDeg: " << _latitudeDeg << endl;
    cerr  << "  root where _longitudeDeg: " << _longitudeDeg << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// read the how group

void OdimHdf5RadxFile::_readHow(Group &how, const string &label)

{

  Hdf5Utils::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(how, "task", label, decodedAttr) == 0) {
    _task = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _task: "
           << _task << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "system", label, decodedAttr) == 0) {
    _system = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _system: "
           << _system << endl;
    }
  }

  if (_utils.loadAttribute(how, "simulated", label, decodedAttr) == 0) {
    _simulated = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _simulated: "
           << _simulated << endl;
    }
  }

  if(_utils.loadAttribute(how, "software", label, decodedAttr) == 0) {
    _software = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _software: "
           << _software << endl;
    }
  }

  if (_utils.loadAttribute(how, "sw_version", label, decodedAttr) == 0) {
    _swVersion = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _swVersion: "
           << _swVersion << endl;
    }
  }

  if (_utils.loadAttribute(how, "beamwidth", label, decodedAttr) == 0) {
    _beamWidthHDeg = decodedAttr.getAsDouble();
    _beamWidthVDeg = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _beamWidthHDeg: "
           << _beamWidthHDeg << endl;
      cerr << "  " << label << " _beamWidthVDeg: "
           << _beamWidthVDeg << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "beamwH", label, decodedAttr) == 0) {
    _beamWidthHDeg = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _beamWidthHDeg: "
           << _beamWidthHDeg << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "beamwV", label, decodedAttr) == 0) {
    _beamWidthVDeg = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _beamWidthVDeg: " << _beamWidthVDeg << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "rpm", label, decodedAttr) == 0) {
    _scanRateRpm = decodedAttr.getAsDouble();
    _scanRateDegPerSec = _scanRateRpm * 6.0;
    if (_debug) {
      cerr << "  " << label << " _scanRateRpm: " << _scanRateRpm << endl;
    }
  }

  if (_utils.loadAttribute(how, "wavelength", label, decodedAttr) == 0) {
    _wavelengthM = decodedAttr.getAsDouble() / 100.0;
    _frequencyHz = Radx::LIGHT_SPEED / _wavelengthM;
    if (_debug) {
      cerr << "  " << label << " _wavelengthM: " << _wavelengthM << endl;
      cerr << "  " << label << " _frequencyHz: " << _frequencyHz << endl;
    }
  }

  if (_utils.loadAttribute(how, "pulsewidth", label, decodedAttr) == 0) {
    _pulseWidthUs = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _pulseWidthUs: " << _pulseWidthUs << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "RXbandwidth", label, decodedAttr) == 0) {
    _rxBandwidthMhz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _rxBandwidthMhz: "
           << _rxBandwidthMhz << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "lowprf", label, decodedAttr) == 0) {
    _lowPrfHz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _lowPrfHz: "
           << _lowPrfHz << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "highprf", label, decodedAttr) == 0) {
    _highPrfHz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _highPrfHz: "
           << _highPrfHz << endl;
    }
  }

  if (_lowPrfHz != _highPrfHz) {
    _prtMode = Radx::PRT_MODE_STAGGERED;
  } else {
    _prtMode = Radx::PRT_MODE_FIXED;
  }

  if (_utils.loadAttribute(how, "TXloss", label, decodedAttr) == 0) {
    _txLossDb = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _txLossDb: "
           << _txLossDb << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "RXloss", label, decodedAttr) == 0) {
    _rxLossDb = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _rxLossDb: "
           << _rxLossDb << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "radomeloss", label, decodedAttr) == 0) {
    _radomeLossOneWayDb = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radomeLossOneWayDb: "
           << _radomeLossOneWayDb << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "antgain", label, decodedAttr) == 0) {
    _antennaGainDb = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _antennaGainDb: "
           << _antennaGainDb << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "gasattn", label, decodedAttr) == 0) {
    _gasAttenDbPerKm = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _gasAttenDbPerKm: "
           << _gasAttenDbPerKm << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "radConstH", label, decodedAttr) == 0) {
    _radarConstantH = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radarConstantH: "
           << _radarConstantH << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "radConstV", label, decodedAttr) == 0) {
    _radarConstantV = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _radarConstantV: "
           << _radarConstantV << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "nomTXpower", label, decodedAttr) == 0) {
    _nomTxPowerKw = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _nomTxPowerKw: "
           << _nomTxPowerKw << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "NI", label, decodedAttr) == 0) {
    _unambigVelMps = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _unambigVelMps: "
           << _unambigVelMps << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "Vsamples", label, decodedAttr) == 0) {
    _nSamples = decodedAttr.getAsInt();
    if (_debug) {
      cerr << "  " << label << " _nSamples: "
           << _nSamples << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "azmethod", label, decodedAttr) == 0) {
    _azMethod = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _azMethod: "
           << _azMethod << endl;
    }
  }

  if (_utils.loadAttribute(how, "binmethod", label, decodedAttr) == 0) {
    _binMethod = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _binMethod: "
           << _binMethod << endl;
    }
  }

  if (_utils.loadAttribute(how, "pointaccEl", label, decodedAttr) == 0) {
    _pointAccEl = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _pointAccEl: "
           << _pointAccEl << endl;
    }
  }
  
  if (_utils.loadAttribute(how, "pointaccAz", label, decodedAttr) == 0) {
    _pointAccAz = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _pointAccAz: "
           << _pointAccAz << endl;
    }
  }
  
  if(_utils.loadAttribute(how, "malfunc", label, decodedAttr)== 0) {
    _malfuncFlag = false;
    string sval = decodedAttr.getAsString();
    if (sval == "True") {
      _malfuncFlag = true;
    }
    if (_debug) {
      cerr << "  " << label << " _malfuncFlag: "
           << _malfuncFlag << endl;
    }
  }

  if(_utils.loadAttribute(how, "radar_msg", label, decodedAttr)== 0) {
    _malfuncMsg = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _malfuncMsg: "
           << _malfuncMsg << endl;
    }
  }
  
  if(_utils.loadAttribute(how, "radhoriz", label, decodedAttr)== 0) {
    _maxRangeKm = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _maxRangeKm: "
           << _maxRangeKm << endl;
    }
  }

  if(_utils.loadAttribute(how, "NEZ", label, decodedAttr)== 0) {
    _dbz0 = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _dbz0: "
           << _dbz0 << endl;
    }
  }

  if(_utils.loadAttribute(how, "comment", label, decodedAttr)== 0) {
    _comment = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _comment: "
           << _comment << endl;
    }
  }
  
  if(_utils.loadAttribute(how, "SQI", label, decodedAttr)== 0) {
    _sqiThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _sqiThreshold: "
           << _sqiThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "CSR", label, decodedAttr)== 0) {
    _csrThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _csrThreshold: "
           << _csrThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "LOG", label, decodedAttr)== 0) {
    _logThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _logThreshold: "
           << _logThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "S2N", label, decodedAttr)== 0) {
    _snrThreshold = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _snrThreshold: "
           << _snrThreshold << endl;
    }
  }

  if(_utils.loadAttribute(how, "peakpwr", label, decodedAttr)== 0) {
    _peakPowerKw = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _peakPowerKw: "
           << _peakPowerKw << endl;
    }
  }

  if(_utils.loadAttribute(how, "avgpwr", label, decodedAttr)== 0) {
    _avPowerKw = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _avPowerKm: "
           << _avPowerKw << endl;
    }
  }

  if(_utils.loadAttribute(how, "dynrange", label, decodedAttr)== 0) {
    _dynRangeDb = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _dynRangeDb: "
           << _dynRangeDb << endl;
    }
  }

  if(_utils.loadAttribute(how, "polarization", label, decodedAttr)== 0) {
    _polarization = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _polarization: "
           << _polarization << endl;
    }
  }

}

//////////////////////////////////////////////
// read the required how group for a sweep

int OdimHdf5RadxFile::_readSweepHow(Group &how, const string &label)
  
{

  // read the scalar how attributes

  _readHow(how, label);

  double sweepStartSecs = _sweepStartSecs;
  double sweepEndSecs = _sweepEndSecs;
  double sweepDeltaSecs = sweepEndSecs - sweepStartSecs;
  double rayDeltaSecs = sweepDeltaSecs / (double) (_nRaysSweep - 1);

  for (int ii = 0; ii < _nRaysSweep; ii++) {

    if (_isRhi) {
      _rayEl[ii] = fmod(_a1Gate + ii * 1.0, 360.0);
      _rayAz[ii] = _fixedAngleDeg;
    } else {
      _rayAz[ii] = fmod(_a1Gate + ii * 1.0, 360.0);
      if (_rayAz[ii] < 0) {
        _rayAz[ii] += 360.0;
      }
      _rayEl[ii] = _fixedAngleDeg;
    }

    // the first ray in the sweep is at index _a1Gate

    int jj = (ii + _a1Gate) % _nRaysSweep;
    _rayTime[jj] = sweepStartSecs + ii * rayDeltaSecs;

  } // ii

  // read in array attributes

  if (_isRhi) {

    // ray elevation angles

    Hdf5Utils::ArrayAttr rayAz;
    if (_utils.loadArrayAttribute(how, "azangles", label, rayAz) == 0) {
      if ((int) rayAz.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of azangles");
        _addErrInt("  Found array len: ", rayAz.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *azangles = rayAz.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayAz[ii] = azangles[ii];
      }
    }

    // ray elevation angles

    Hdf5Utils::ArrayAttr startEl, stopEl;
    if ((_utils.loadArrayAttribute(how, "startelA", label, startEl) == 0) &&
        (_utils.loadArrayAttribute(how, "stopelA", label, stopEl)) == 0) {
      
      if ((int) startEl.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startelA");
        _addErrInt("  Found array len: ", startEl.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopEl.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopelA");
        _addErrInt("  Found array len: ", stopEl.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startEls = startEl.getAsDoubles();
      const double *stopEls = stopEl.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayEl[ii] = RadxComplex::computeMeanDeg(startEls[ii], stopEls[ii]);
      }
    }

    // ray times
    
    Hdf5Utils::ArrayAttr startTime, stopTime;
    if ((_utils.loadArrayAttribute(how, "startelT", label, startTime) == 0) &&
        (_utils.loadArrayAttribute(how, "stopelT", label, stopTime)) == 0) {
      
      if ((int) startTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startelT");
        _addErrInt("  Found array len: ", startTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopelT");
        _addErrInt("  Found array len: ", stopTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startTimes = startTime.getAsDoubles();
      const double *stopTimes = stopTime.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayTime[ii] = (startTimes[ii] + stopTimes[ii]) / 2.0;
      }
    }

  } else {

    // ray elevation angles

    Hdf5Utils::ArrayAttr rayEl;
    if (_utils.loadArrayAttribute(how, "elangles", label, rayEl) == 0) {
      if ((int) rayEl.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of elangles");
        _addErrInt("  Found array len: ", rayEl.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *elangles = rayEl.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayEl[ii] = elangles[ii];
      }
    }

    // ray azimuth angles

    Hdf5Utils::ArrayAttr startAz, stopAz;
    if ((_utils.loadArrayAttribute(how, "startazA", label, startAz) == 0) &&
        (_utils.loadArrayAttribute(how, "stopazA", label, stopAz)) == 0) {
      
      if ((int) startAz.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startazA");
        _addErrInt("  Found array len: ", startAz.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopAz.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopazA");
        _addErrInt("  Found array len: ", stopAz.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startAzs = startAz.getAsDoubles();
      const double *stopAzs = stopAz.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayAz[ii] = RadxComplex::computeMeanDeg(startAzs[ii], stopAzs[ii]);
        if (_rayAz[ii] < 0) {
          _rayAz[ii] += 360.0;
        }
      }
    }

    // ray times
    
    Hdf5Utils::ArrayAttr startTime, stopTime;
    if ((_utils.loadArrayAttribute(how, "startazT", label, startTime) == 0) &&
        (_utils.loadArrayAttribute(how, "stopazT", label, stopTime)) == 0) {
      
      if ((int) startTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of startazT");
        _addErrInt("  Found array len: ", startTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      if ((int) stopTime.getLen() != _nRaysSweep) {
        _addErrStr("ERROR - In reading PPI sweep, incorrect number of stopazT");
        _addErrInt("  Found array len: ", stopTime.getLen());
        _addErrInt("  Expected nRays: ", _nRaysSweep);
        return -1;
      }
      const double *startTimes = startTime.getAsDoubles();
      const double *stopTimes = stopTime.getAsDoubles();
      for (int ii = 0; ii < _nRaysSweep; ii++) {
        _rayTime[ii] = (startTimes[ii] + stopTimes[ii]) / 2.0;
      }
    }
    
  }

  return 0;

}

//////////////////////////////////////////////
// read what group for sweep

int OdimHdf5RadxFile::_readSweepWhat(Group &what, const string &label)

{
  
  Hdf5Utils::DecodedAttr decodedAttr;

  // product scan type
  
  _product = "SCAN";
  _isRhi = false;
  if (_utils.loadAttribute(what, "product", label, decodedAttr) == 0) {
    _product = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _product: " << _product << endl;
    }
    if (_product == "RHI") {
      _isRhi = true;
    } else {
      _isRhi = false;
    }
  }

  // date / time

  if (_utils.loadAttribute(what, "startdate", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _startDateStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _startDateStr: " << _startDateStr << endl;
  }

  if (_utils.loadAttribute(what, "enddate", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _endDateStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _endDateStr: " << _endDateStr << endl;
  }

  if (_utils.loadAttribute(what, "starttime", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _startTimeStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _startTimeStr: " << _startTimeStr << endl;
  }

  if (_utils.loadAttribute(what, "endtime", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _endTimeStr = decodedAttr.getAsString();
  if (_debug) {
    cerr << "  " << label << " _endTimeStr: " << _endTimeStr << endl;
  }

  RadxTime sweepStartTime(_startDateStr + _startTimeStr);
  RadxTime sweepEndTime(_endDateStr + _endTimeStr);

  _sweepStartSecs = sweepStartTime.utime();
  _sweepEndSecs = sweepEndTime.utime();

  return 0;

}

//////////////////////////////////////////////
// read what group for data field

int OdimHdf5RadxFile::_readDataWhat(Group &what, const string &label)

{

  Hdf5Utils::DecodedAttr decodedAttr;

  // field quantity

  if (_utils.loadAttribute(what, "quantity", label, decodedAttr) == 0) {
    _fieldName = decodedAttr.getAsString();
    if (_debug) {
      cerr << "  " << label << " _fieldName: " << _fieldName << endl;
    }
  }

  if (_utils.loadAttribute(what, "gain", label, decodedAttr) == 0) {
    _scale = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _scale: " << _scale << endl;
    }
  }
  
  if (_utils.loadAttribute(what, "offset", label, decodedAttr) == 0) {
    _offset = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _offset: " << _offset << endl;
    }
  }
  
  if (_utils.loadAttribute(what, "nodata", label, decodedAttr) == 0) {
    _missingDataVal = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _missingDataVal: " << _missingDataVal << endl;
    }
  }
  
  if (_utils.loadAttribute(what, "undetect", label, decodedAttr) == 0) {
    _lowDataVal = decodedAttr.getAsDouble();
    if (_debug) {
      cerr << "  " << label << " _lowDataVal: " << _lowDataVal << endl;
    }
  }
  
  return 0;

}

//////////////////////////////////////////////
// read a sweep where group

int OdimHdf5RadxFile::_readSweepWhere(Group &where, const string& label)

{

  Hdf5Utils::DecodedAttr decodedAttr;

  if (_utils.loadAttribute(where, "a1gate", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _a1Gate = decodedAttr.getAsInt();
  
  if (_utils.loadAttribute(where, "elangle", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _fixedAngleDeg = decodedAttr.getAsDouble();
  
  if (_utils.loadAttribute(where, "nbins", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _nGates = decodedAttr.getAsInt();
  
  if (_utils.loadAttribute(where, "nrays", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _nRaysSweep = decodedAttr.getAsInt();

  // initialize ray vectors
  
  _rayAz.resize(_nRaysSweep);
  _rayEl.resize(_nRaysSweep);
  _rayTime.resize(_nRaysSweep);
  
  if (_utils.loadAttribute(where, "rscale", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _gateSpacingKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_utils.loadAttribute(where, "rstart", label, decodedAttr)) {
    _addErrStr(_utils.getErrStr());
    return -1;
  }
  _startRangeKm = decodedAttr.getAsDouble() / 1000.0;
  
  if (_debug) {
    cerr << "  " << label << " _a1Gate: " << _a1Gate << endl;
    cerr << "  " << label << " _fixedAngleDeg: " << _fixedAngleDeg << endl;
    cerr << "  " << label << " _nGates: " << _nGates << endl;
    cerr << "  " << label << " _nRaysSweep: " << _nRaysSweep << endl;
    cerr << "  " << label << " _gateSpacingKm: " << _gateSpacingKm << endl;
    cerr << "  " << label << " _startRangeKm: " << _startRangeKm << endl;
  }
  return 0;

}

////////////////////////////////////////
// create the rays for this sweep

void OdimHdf5RadxFile::_createRaysForSweep(int sweepNumber)
{
  
  // create rays for sweep

  for (int  iray = 0; iray < _nRaysSweep; iray++) {

    // time

    time_t raySecs = (time_t) _rayTime[iray];
    int msecs = (int) ((_rayTime[iray] - raySecs) * 1000.0 + 0.5);

    if (_verbose) {
      char rayTimeStr[128];
      sprintf(rayTimeStr, "%s.%.3d", RadxTime::strm(raySecs).c_str(), msecs);
      cerr << "ray iray, time, el, az: " << iray << ", "
           << rayTimeStr << ", "
           << _rayEl[iray] << ", "
           << _rayAz[iray] << endl;
    }

    RadxRay *ray = new RadxRay;
    ray->setTime(_rayTime[iray]);
    ray->setAzimuthDeg(_rayAz[iray]);
    ray->setElevationDeg(_rayEl[iray]);
    ray->setVolumeNumber(_volumeNumber);
    ray->setSweepNumber(sweepNumber);
    ray->setNSamples(_nSamples);
    ray->setPulseWidthUsec(_pulseWidthUs);
    ray->setPrtSec(1.0 / _highPrfHz);
    ray->setPrtRatio(_lowPrfHz / _highPrfHz);
    ray->setNyquistMps(_unambigVelMps);
    ray->setUnambigRangeKm(_maxRangeKm);
    ray->setTargetScanRateDegPerSec(_scanRateDegPerSec);
    ray->setFixedAngleDeg(_fixedAngleDeg);

    if (_isRhi) {
      ray->setSweepMode(Radx::SWEEP_MODE_RHI);
    } else {
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
    }
    ray->setPrtMode(_prtMode);
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    _sweepRays.push_back(ray);

  } // iray
  
}

//////////////////////////////////////////////////
// add field to rays

int OdimHdf5RadxFile::_addFieldToRays(const char *label,
                                      Group &sweep,
                                      vector<RadxRay *> &rays, 
                                      int fieldNum)

{
  // compute field name
  
  char dataGroupName[1024];
  sprintf(dataGroupName, "%s%d", label, fieldNum + 1);
  
  // get data group
  
  Group *dg = NULL;
  try {
    dg = new Group(sweep.openGroup(dataGroupName));
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Cannot open data grop");
    _addErrStr("  Data group name: ", dataGroupName);
    if (dg) delete dg;
    return -1;
  }

  // read what group
  
  Group *what = NULL;
  try {
    char label[128];
    sprintf(label, "%s what", dataGroupName);
    what = new Group(dg->openGroup("what"));
    if (_readDataWhat(*what, label)) {
      delete what;
      delete dg;
      return -1;
    }
  }
  catch (H5::Exception e) {
    if (_debug) {
      cerr << "NOTE - no 'what' group for data field: " << dataGroupName << endl;
    }
  }
  if (what) delete what;
  
  // read sweep how group
  
  Group *how = NULL;
  try {
    how = new Group(dg->openGroup("how"));
    char label[128];
    sprintf(label, "%s how", dataGroupName);
    _readHow(*how, label);
  }
  catch (H5::Exception e) {
    if (_verbose) {
      cerr << "NOTE - no 'how' group for data field: " << dataGroupName << endl;
    }
  }
  if (how) delete how;
  
  // check that we need this field
  
  if (!isFieldRequiredOnRead(_fieldName)) {
    if (_verbose) {
      cerr << "DEBUG - OdimHdf5RadxFile::_addFieldToRays" << endl;
      cerr << "  -->> rejecting field: " << _fieldName << endl;
    }
    delete dg;
    return 0;
  }

  // get units, standard name and long name

  string units, standardName, longName;
  _lookupUnitsAndNames(_fieldName, units, standardName, longName);

  // get data set
  
  DataSet *ds = NULL;
  try {
    ds = new DataSet(dg->openDataSet("data"));
  }
  catch (H5::Exception e) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Cannot open data set for field: ", _fieldName);
    if (ds) delete ds;
    delete dg;
    return -1;
  }

  // get data size
  
  DataSpace dataspace = ds->getSpace();
  int nPoints = dataspace.getSimpleExtentNpoints();

  // get dimensions

  int ndims = dataspace.getSimpleExtentNdims();
  if (ndims != 2) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  Data is not 2-D array");
    _addErrStr("  Should be [nrays][ngates]");
    delete ds;
    delete dg;
    return -1;

  }

  hsize_t dims[2];
  dataspace.getSimpleExtentDims(dims);
  int nRays = dims[0];
  int nGates = dims[1];
  
  if (nRays != (int) rays.size()) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data name: ", dataGroupName);
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  nRays incorrect, should match nRaysSweep");
    _addErrInt("  Found: ", nRays);
    _addErrInt("  Expected: ", rays.size());
    delete ds;
    delete dg;
    return -1;
  }

  if (nGates != _nGates) {
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data name: ", dataGroupName);
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  nGates incorrect, should match bin_count");
    _addErrInt("  nGates: ", nGates);
    _addErrInt("  bin_count: ", _nGates);
    delete ds;
    delete dg;
    return -1;
  }
  
  // get data type and size

  DataType dtype = ds->getDataType();
  H5T_class_t aclass = dtype.getClass();
  size_t tsize = dtype.getSize();
  
  if (aclass == H5T_INTEGER) {

    if (tsize == 1) {

      _loadSi08Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     _scale, _offset, rays);
  
    } else if (tsize == 2) {

      _loadSi16Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     _scale, _offset, rays);
  
    } else if (tsize == 4) {

      _loadSi32Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints,
                     _scale, _offset, rays);
  
    } else {
      
      _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
      _addErrStr("  Data name: ", dataGroupName);
      _addErrStr("  Field name: ", _fieldName);
      _addErrInt("  integer data size not supported: ", tsize);
      delete ds;
      delete dg;
      return -1;
      
    }

  } else if (aclass == H5T_FLOAT) {

    if (tsize == 4) {

      _loadFl32Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints, rays);
  
    } else if (tsize == 8) {
      
      _loadFl64Field(*ds, _fieldName, units, standardName, longName,
                     nRays, nGates, nPoints, rays);
  
    } else {

      _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
      _addErrStr("  Data name: ", dataGroupName);
      _addErrStr("  Field name: ", _fieldName);
      _addErrInt("  float data size not supported: ", tsize);
      delete ds;
      delete dg;
      return -1;
      
    }

  } else {
    
    _addErrStr("ERROR - OdimHdf5RadxFile::_addFieldToRays");
    _addErrStr("  Data name: ", dataGroupName);
    _addErrStr("  Field name: ", _fieldName);
    _addErrStr("  data type not supported: ", dtype.fromClass());
    delete ds;
    delete dg;
    return -1;
      
  }

  // clean up

  delete ds;
  delete dg;
  return 0;

}

///////////////////////////////////////////////////////////////////
// Load float array for given data set, using the type passed in

int OdimHdf5RadxFile::_loadFloatArray(DataSet &ds,
                                      const string dsname,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      Radx::fl32 *floatVals)
  
{

  DataType dtype = ds.getDataType();
  H5T_class_t aclass = dtype.getClass();
  
  if (aclass == H5T_INTEGER) {

    IntType intType = ds.getIntType();
    H5T_order_t order = intType.getOrder();
    H5T_sign_t sign = intType.getSign();
    size_t tsize = intType.getSize();
    
    if (sign == H5T_SGN_NONE) {
      
      // unsigned
      
      if (tsize == 1) {

        Radx::ui08 *ivals = new Radx::ui08[nPoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::ui16 *ivals = new Radx::ui16[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::ui16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::ui16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::ui32 *ivals = new Radx::ui32[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::ui32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::ui32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::ui64 *ivals = new Radx::ui64[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::ui64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::ui64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;
      }

    } else {

      // signed

      if (tsize == 1) {

        Radx::si08 *ivals = new Radx::si08[nPoints];
        ds.read(ivals, dtype);
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 2) {

        Radx::si16 *ivals = new Radx::si16[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::si16), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap16(ivals, nPoints * sizeof(Radx::si16), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 4) {

        Radx::si32 *ivals = new Radx::si32[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::si32), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap32(ivals, nPoints * sizeof(Radx::si32), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;

      } else if (tsize == 8) {

        Radx::si64 *ivals = new Radx::si64[nPoints];
        ds.read(ivals, dtype);
        if (ByteOrder::hostIsBigEndian()) {
          if (order == H5T_ORDER_LE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::si64), true);
          }
        } else {
          if (order == H5T_ORDER_BE) {
            ByteOrder::swap64(ivals, nPoints * sizeof(Radx::si64), true);
          }
        }
        for (int ii = 0; ii < nPoints; ii++) {
          if (ivals[ii] == 0) {
            floatVals[ii] = Radx::missingFl32;
          } else {
            floatVals[ii] = offset + ivals[ii] * scale;
          }
        }
        delete[] ivals;
      }

    }

  } else if (aclass == H5T_FLOAT) {

    FloatType flType = ds.getFloatType();
    H5T_order_t order = flType.getOrder();
    size_t tsize = flType.getSize();
    
    if (tsize == 4) {

      Radx::fl32 *fvals = new Radx::fl32[nPoints];
      ds.read(fvals, dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap32(fvals, nPoints * sizeof(Radx::fl32), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap32(fvals, nPoints * sizeof(Radx::fl32), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        floatVals[ii] = fvals[ii];
      }
      delete[] fvals;

    } else if (tsize == 8) {

      Radx::fl64 *fvals = new Radx::fl64[nPoints];
      ds.read(fvals, dtype);
      if (ByteOrder::hostIsBigEndian()) {
        if (order == H5T_ORDER_LE) {
          ByteOrder::swap64(fvals, nPoints * sizeof(Radx::fl64), true);
        }
      } else {
        if (order == H5T_ORDER_BE) {
          ByteOrder::swap64(fvals, nPoints * sizeof(Radx::fl64), true);
        }
      }
      for (int ii = 0; ii < nPoints; ii++) {
        floatVals[ii] = fvals[ii];
      }
      delete[] fvals;

    }

  } else {

    return -1;

  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Load si08 array for given data set, using the type passed in

void OdimHdf5RadxFile::_loadSi08Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_sign_t sign = intType.getSign();

  int imin = -128;

  Radx::si08 *ivals = new Radx::si08[nPoints];
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    Radx::ui08 *uvals = new Radx::ui08[nPoints];
    ds.read(uvals, dtype);
    for (int ii = 0; ii < nPoints; ii++) {
      int ival = (int) uvals[ii] + imin;
      ivals[ii] = (Radx::si08) ival;
    }
    delete[] uvals;
    offset -= (double) imin * scale;
    
  } else {
    
    // signed
    
    ds.read(ivals, dtype);

  }
  
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeSi08(Radx::missingSi08, scale, offset);
    field->addDataSi08(nGates, ivals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] ivals;

}

///////////////////////////////////////////////////////////////////
// Load si16 array for given data set, using the type passed in

void OdimHdf5RadxFile::_loadSi16Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();

  int imin = -32768;

  Radx::si16 *vals = new Radx::si16[nPoints];
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    Radx::ui16 *uvals = new Radx::ui16[nPoints];
    ds.read(uvals, dtype);
    
    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap16(uvals, nPoints * sizeof(Radx::ui16), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap16(uvals, nPoints * sizeof(Radx::ui16), true);
      }
    }
    
    for (int ii = 0; ii < nPoints; ii++) {
      int ival = (int) uvals[ii] + imin;
      vals[ii] = (Radx::si16) ival;
    }
    delete[] uvals;
    offset -= (double) imin * scale;
  
  } else {
    
    // signed
    
    ds.read(vals, dtype);

    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap16(vals, nPoints * sizeof(Radx::ui16), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap16(vals, nPoints * sizeof(Radx::ui16), true);
      }
    }
    
  }
  
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeSi16(Radx::missingSi16, scale, offset);
    field->addDataSi16(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

///////////////////////////////////////////////////////////////////
// Load si32 array for given data set, using the type passed in

void OdimHdf5RadxFile::_loadSi32Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      double scale,
                                      double offset,
                                      vector<RadxRay *> &rays)
  
{

  DataType dtype = ds.getDataType();
  IntType intType = ds.getIntType();
  H5T_order_t order = intType.getOrder();
  H5T_sign_t sign = intType.getSign();
  
  Radx::si64 imin = -2147483648;

  Radx::si32 *vals = new Radx::si32[nPoints];
  
  if (sign == H5T_SGN_NONE) {
    
    // unsigned
    
    Radx::ui32 *uvals = new Radx::ui32[nPoints];
    ds.read(uvals, dtype);
    
    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap32(uvals, nPoints * sizeof(Radx::ui32), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap32(uvals, nPoints * sizeof(Radx::ui32), true);
      }
    }
    
    for (int ii = 0; ii < nPoints; ii++) {
      Radx::si64 ival = (Radx::si64) uvals[ii] + imin;
      vals[ii] = (Radx::si32) ival;
    }
    delete[] uvals;
    offset -= (double) imin * scale;
  
  } else {
    
    // signed
    
    ds.read(vals, dtype);

    if (ByteOrder::hostIsBigEndian()) {
      if (order == H5T_ORDER_LE) {
        ByteOrder::swap32(vals, nPoints * sizeof(Radx::ui32), true);
      }
    } else {
      if (order == H5T_ORDER_BE) {
        ByteOrder::swap32(vals, nPoints * sizeof(Radx::ui32), true);
      }
    }
    
  }
  
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeSi32(Radx::missingSi32, scale, offset);
    field->addDataSi32(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

///////////////////////////////////////////////////////////////////
// Load fl32 array for given data set, using the type passed in

void OdimHdf5RadxFile::_loadFl32Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  FloatType floatType = ds.getFloatType();
  H5T_order_t order = floatType.getOrder();
  
  Radx::fl32 *vals = new Radx::fl32[nPoints];
  ds.read(vals, dtype);
  
  if (ByteOrder::hostIsBigEndian()) {
    if (order == H5T_ORDER_LE) {
      ByteOrder::swap32(vals, nPoints * sizeof(Radx::fl32), true);
    }
  } else {
    if (order == H5T_ORDER_BE) {
      ByteOrder::swap32(vals, nPoints * sizeof(Radx::fl32), true);
    }
  }
    
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

///////////////////////////////////////////////////////////////////
// Load fl64 array for given data set, using the type passed in

void OdimHdf5RadxFile::_loadFl64Field(DataSet &ds,
                                      const string &fieldName,
                                      const string &units,
                                      const string &standardName,
                                      const string &longName,
                                      int nRays,
                                      int nGates,
                                      int nPoints,
                                      vector<RadxRay *> &rays)
  
{
  
  DataType dtype = ds.getDataType();
  FloatType floatType = ds.getFloatType();
  H5T_order_t order = floatType.getOrder();
  
  Radx::fl64 *vals = new Radx::fl64[nPoints];
  ds.read(vals, dtype);
  
  if (ByteOrder::hostIsBigEndian()) {
    if (order == H5T_ORDER_LE) {
      ByteOrder::swap64(vals, nPoints * sizeof(Radx::fl64), true);
    }
  } else {
    if (order == H5T_ORDER_BE) {
      ByteOrder::swap64(vals, nPoints * sizeof(Radx::fl64), true);
    }
  }
    
  // add data to rays
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    
    // load field data
    
    int dataOffset = iray * nGates;
    RadxField *field = new RadxField(fieldName, units);
    field->setStandardName(standardName);
    field->setLongName(longName);
    field->setTypeFl64(Radx::missingFl64);
    field->addDataFl64(nGates, vals + dataOffset);
    field->setRangeGeom(_startRangeKm, _gateSpacingKm);
    
    // add to ray

    rays[iray]->addField(field);

  } // iray

  // clean up

  delete[] vals;

}

//////////////////////////////////////////////////////////
// lookup units and names appropriate to field name

void OdimHdf5RadxFile::_lookupUnitsAndNames(const string &fieldName, 
                                            string &units,
                                            string &standardName,
                                            string &longName)
  
{

  if (fieldName == "TH") {
    units = "dBZ";
    longName = "Logged horizontally-polarized total (uncorrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "TV") {
    units = "dBZ";
    longName = "Logged vertically-polarized total (uncorrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "DBZH") {
    units = "dBZ";
    longName = "Logged horizontally-polarized (corrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "DBZV") {
    units = "dBZ";
    longName = "Logged vertically-polarized (corrected) reflectivity factor";
    standardName = "equivalent_reflectivity_factor";
  }

  if (fieldName == "ZDR") {
    units = "dB";
    longName = "Logged differential reflectivity";
    standardName = "corrected_log_differential_reflectivity_hv";
  }

  if (fieldName == "RHOHV") {
    units = "0-1 ";
    longName = "Correlation between Zh and Zv";
    standardName = "cross_correlation_ratio_hv";
  }

  if (fieldName == "LDR") {
    units = "dB";
    longName = "Linear depolarization ratio";
    standardName = "log_linear_depolarization_ratio_hv";
  }

  if (fieldName == "PHIDP") {
    units = "deg";
    longName = "Differential phase";
    standardName = "differential_phase_hv";
  }

  if (fieldName == "KDP") {
    units = "deg/km";
    longName = "Specific differential phase";
    standardName = "specific_differential_phase_hv";
  }

  if (fieldName == "SQI") {
    units = "0-1";
    longName = "Signal quality index";
    longName = "normalized_coherent_power";
  }

  if (fieldName == "SNR") {
    units = "0-1";
    longName = "Normalized signal-to-noise ratio";
    standardName = "signal_to_noise_ratio";
  }

  if (fieldName == "VRAD") {
    units = "m/s";
    longName = "Radial velocity";
    standardName = "radial_velocity_of_scatterers_away_from_instrument";
  }

  if (fieldName == "WRAD") {
    units = "m/s";
    longName = "Spectral width of radial velocity";
    standardName = "doppler_spectrum_width";
  }
  
  if (fieldName == "QIND") {
    units = "0-1";
    longName = "Spatially analayzed quality indicator";
  }

}


//////////////////////////////////////////////////////////
// load up the read volume with the data from this object

int OdimHdf5RadxFile::_finalizeReadVolume()
  
{

  // set metadata

  _readVol->setOrigFormat("ODIMHDF5");
  _readVol->setVolumeNumber(_volumeNumber);
  _readVol->setInstrumentType(_instrumentType);
  _readVol->setPlatformType(_platformType);
  _readVol->setPrimaryAxis(_primaryAxis);

  _readVol->addFrequencyHz(_frequencyHz);
  
  _readVol->setTitle("ODIM radar data, version: " + _version);
  _readVol->setSource(_source);
  
  string hist;
  if (_system.size() > 0) {
    hist += "system:" + _system;
  }
  if (_software.size() > 0) {
    if (hist.size() > 0) hist += ", ";
    hist += " software:" + _software;
  }
  if (_swVersion.size() > 0) {
    if (hist.size() > 0) hist += ", ";
    hist += " version:" + _swVersion;
  }
  if (_conventions.size() > 0) {
    if (hist.size() > 0) hist += ", ";
    hist += " hdf5-conventions:" + _conventions;
  }
  _readVol->setHistory(hist);

  _readVol->setInstitution("");

  string ref;
  if (_polarization.size() > 0) {
    ref += "polarization:" + _polarization;
  }
  if (_azMethod.size() > 0) {
    if (ref.size() > 0) ref += ", ";
    ref += "azMethod:" + _azMethod;
  }
  if (_binMethod.size() > 0) {
    if (ref.size() > 0) ref += ", ";
    ref += "binMethod:" + _binMethod;
  }
  if (_malfuncFlag && _malfuncMsg.size() > 0) {
    if (ref.size() > 0) ref += ", ";
    ref += "malfunc:" + _malfuncMsg;
  }
  _readVol->setReferences(ref);

  _readVol->setComment(_comment);
  _readVol->setStatusXml(_statusXml);
  // _readVol->setSiteName(_siteName);

  string scanName;
  if (_objectStr.size() > 0) {
    scanName += "object:" + _objectStr;
  }
  if (_task.size() > 0) {
    if (scanName.size() > 0) scanName += ", ";
    scanName += "task:" + _task;
  }
  _readVol->setScanName(scanName);

  _readVol->setLatitudeDeg(_latitudeDeg);
  _readVol->setLongitudeDeg(_longitudeDeg);
  _readVol->setAltitudeKm(_altitudeKm);

  _readVol->setRadarBeamWidthDegH(_beamWidthHDeg);
  _readVol->setRadarBeamWidthDegV(_beamWidthVDeg);
  _readVol->setRadarAntennaGainDbH(_antennaGainDb);
  _readVol->setRadarAntennaGainDbV(_antennaGainDb);
  _readVol->setRadarReceiverBandwidthMhz(_rxBandwidthMhz);

  if (_readSetMaxRange) {
    _readVol->setMaxRangeKm(_readMaxRangeKm);
  }

  // calibration

  RadxRcalib *cal = new RadxRcalib;
  cal->setPulseWidthUsec(_pulseWidthUs);
  cal->setXmitPowerDbmH(10.0 * log10(_peakPowerKw * 1.0e6));
  cal->setXmitPowerDbmV(10.0 * log10(_peakPowerKw * 1.0e6));
  cal->setCouplerForwardLossDbH(_txLossDb);
  cal->setCouplerForwardLossDbV(_txLossDb);
  cal->setTwoWayWaveguideLossDbH(_rxLossDb);
  cal->setTwoWayWaveguideLossDbV(_rxLossDb);
  cal->setTwoWayRadomeLossDbH(_radomeLossOneWayDb * 2.0);
  cal->setTwoWayRadomeLossDbV(_radomeLossOneWayDb * 2.0);
  cal->setRadarConstantH(_radarConstantH);
  cal->setRadarConstantV(_radarConstantV);
  cal->setAntennaGainDbH(_antennaGainDb);
  cal->setAntennaGainDbV(_antennaGainDb);
  cal->setBaseDbz1kmHc(_dbz0);
  cal->setBaseDbz1kmVc(_dbz0);
  _readVol->addCalib(cal);

  // load the sweep information from the rays

  _readVol->loadSweepInfoFromRays();
  
  // constrain the sweep data as appropriate
  
  if (_readFixedAngleLimitsSet) {
    if (_readVol->constrainByFixedAngle(_readMinFixedAngle, _readMaxFixedAngle,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within fixed angle limits");
      _addErrDbl("  min fixed angle: ", _readMinFixedAngle);
      _addErrDbl("  max fixed angle: ", _readMaxFixedAngle);
      return -1;
    }
  } else if (_readSweepNumLimitsSet) {
    if (_readVol->constrainBySweepNum(_readMinSweepNum, _readMaxSweepNum,
                                        _readStrictAngleLimits)) {
      _addErrStr("ERROR - OdimHdf5RadxFile::_finalizeReadVolume");
      _addErrStr("  No data found within sweep num limits");
      _addErrInt("  min sweep num: ", _readMinSweepNum);
      _addErrInt("  max sweep num: ", _readMaxSweepNum);
      return -1;
    }
  }

  // load the volume information from the rays

  _readVol->loadVolumeInfoFromRays();
  
  // check for indexed rays, set info on rays

  _readVol->checkForIndexedRays();

  return 0;

}

////////////////////////////////////////////////////////////
// Is this a Gematronik field file?
// Each field is stored in a separate file

bool OdimHdf5RadxFile::_isGematronikFieldFile(const string &path,
                                              string &dateStr,
                                              string &fieldName,
                                              string &volName)
  
{

  RadxPath rpath(path);
  string fileName = rpath.getFile();

  // find location of first digit
  
  size_t firstDigitLoc = string::npos;
  for (size_t ii = 0; ii < fileName.size(); ii++) {
    if (isdigit(fileName[ii])) {
      firstDigitLoc = ii;
      break;
    }
  }

  // read in date / time

  int year, month, day, hour, min, sec, id;
  if (sscanf(fileName.c_str() + firstDigitLoc,
             "%4d%2d%2d%2d%2d%2d%2d",
             &year, &month, &day, &hour, &min, &sec, &id) != 7) {
    return false;
  }

  RadxTime ptime(year, month, day, hour, min, sec);
  dateStr = fileName.substr(firstDigitLoc, 16);

  // get field name

  size_t startFieldNamePos = firstDigitLoc + 16;
  size_t endFieldNamePos = fileName.find(".", startFieldNamePos);
  if (endFieldNamePos == string::npos) {
    return false;
  }
  size_t endVolPos = fileName.find(".", endFieldNamePos + 1);
  if (endVolPos == string::npos) {
    return false;
  }

  fieldName = fileName.substr(startFieldNamePos,
                              endFieldNamePos - startFieldNamePos);
  
  volName = fileName.substr(endFieldNamePos + 1,
                            endVolPos - endFieldNamePos - 1);
  
  return true;

}

////////////////////////////////////////////////////////////
// Read in data from Gematronik field files
// Each field is stored in a separate file
//
// Returns 0 on success, -1 on failure

int OdimHdf5RadxFile::_readGemFieldFiles(const string &path,
                                         const string &dateStr,
                                         const string &fieldName,
                                         RadxVol &vol)
  
{

  // get dir

  RadxPath rpath(path);
  string dirPath = rpath.getDirectory();

  // load up array of file names that match the dateStr
  
  vector<string> fileNames;
  RadxReadDir rdir;
  if (rdir.open(dirPath.c_str()) == 0) {
    
    // Loop thru directory looking for the data file names
    // or forecast directories
    
    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }

      // make sure we have .vol files

      if (strstr(dp->d_name, ".vol") == NULL) {
	continue;
      }

      string dName(dp->d_name);
      if (dName.find(dateStr) != string::npos) {
        fileNames.push_back(dName);
      }
      
    } // dp
    
    rdir.close();

  } // if (rdir ...

  // sort the file names
  
  sort(fileNames.begin(), fileNames.end());
  
  // read in files
  
  for (size_t ifile = 0; ifile < fileNames.size(); ifile++) {
    
    const string &fileName = fileNames[ifile];
    string filePath = dirPath + RadxPath::RADX_PATH_DELIM + fileName;

    if (ifile == 0) {

      // primary file
      // read into final vol

      if (_readFromPath(filePath, vol)) {
        return -1;
      }
      
    } else {

      // secondary file
      // read into temporary volume
      
      RadxVol tmpVol;
      if (_readFromPath(filePath, tmpVol)) {
        return -1;
      }

      // copy fields across to final volume if the geometry is the same

      vector<RadxRay *> &volRays = vol.getRays();
      vector<RadxRay *> &tmpRays = tmpVol.getRays();

      if (volRays.size() == tmpRays.size()) {

        for (size_t iray = 0; iray < volRays.size(); iray++) {
          
          RadxRay *volRay = volRays[iray];
          RadxRay *tmpRay = tmpRays[iray];
          
          vector<RadxField *> tmpFields = tmpRay->getFields();
          for (size_t ifield = 0; ifield < tmpFields.size(); ifield++) {
            RadxField *newField = new RadxField(*tmpFields[ifield]);
            volRay->addField(newField);
          } // ifield
          
        } // iray

      } // if (volRays.size() == tmpRays.size()
      
    } // if (ifile == 0)
    
  } // ifile
  
  return 0;

}

  
