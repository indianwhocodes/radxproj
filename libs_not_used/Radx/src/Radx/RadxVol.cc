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
// RadxVol.cc
//
// RadxVol object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxAngleHist.hh>
#include <Radx/RadxGeoref.hh>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <sys/stat.h>
using namespace std;

const double RadxVol::_searchAngleRes = 360.0 / _searchAngleN;

//////////////
// Constructor

RadxVol::RadxVol()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxVol::RadxVol(const RadxVol &rhs)
     
{
  _init();
  _copy(rhs);
}

////////////////////////////////////
// Copy constructor, one sweep only

RadxVol::RadxVol(const RadxVol &rhs, int sweepNum)
     
{
  _init();
  copy(rhs, sweepNum);
}

/////////////
// destructor

RadxVol::~RadxVol()

{

  clear();

}

/////////////////////////////
// Assignment
//

RadxVol &RadxVol::operator=(const RadxVol &rhs)
  

{
  return _copy(rhs);
}

// set radar and lidar parameters

void RadxVol::setRadarBeamWidthDegH(double val) { 
  _platform.setRadarBeamWidthDegH(val);
}
void RadxVol::setRadarBeamWidthDegV(double val) {
  _platform.setRadarBeamWidthDegV(val);
}
void RadxVol::setRadarAntennaGainDbH(double val) {
  _platform.setRadarAntennaGainDbH(val);
}
void RadxVol::setRadarAntennaGainDbV(double val) {
  _platform.setRadarAntennaGainDbV(val);
}
void RadxVol::setRadarReceiverBandwidthMhz(double val) {
  _platform.setRadarReceiverBandwidthMhz(val);
}

void RadxVol::setIsLidar(bool val) {
  _platform.setIsLidar(val);
}

void RadxVol::setLidarConstant(double val) {
  _platform.setLidarConstant(val);
}
void RadxVol::setLidarPulseEnergyJ(double val) {
  _platform.setLidarPulseEnergyJ(val);
}
void RadxVol::setLidarPeakPowerW(double val) {
  _platform.setLidarPeakPowerW(val);
}
void RadxVol::setLidarApertureDiamCm(double val) {
  _platform.setLidarApertureDiamCm(val);
}
void RadxVol::setLidarApertureEfficiency(double val) {
  _platform.setLidarApertureEfficiency(val);
}
void RadxVol::setLidarFieldOfViewMrad(double val) {
  _platform.setLidarFieldOfViewMrad(val);
}
void RadxVol::setLidarBeamDivergenceMrad(double val) {
  _platform.setLidarBeamDivergenceMrad(val);
}

/////////////////////////////////////////////////////////
// initialize data members

void RadxVol::_init()
  
{

  _debug = false;
  _cfactors = NULL;
  _searchRays.resize(_searchAngleN);

  clear();
  
  for (int ii = 0; ii < _searchAngleN; ii++) {
    _searchRays[ii] = NULL;
  }

}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxVol &RadxVol::_copy(const RadxVol &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  // copy meta data

  copyMeta(rhs);

  // copy the range geometry and data packing

  copyRangeGeom(rhs);
  copyPacking(rhs);
  
  // sweeps
  
  clearSweeps();
  for (size_t ii = 0; ii < rhs._sweeps.size(); ii++) {
    RadxSweep *sweep = new RadxSweep(*rhs._sweeps[ii]);
    _sweeps.push_back(sweep);
  }

  // rays

  clearRays();
  for (size_t ii = 0; ii < rhs._rays.size(); ii++) {
    RadxRay *ray = new RadxRay(*rhs._rays[ii]);
    addRay(ray);
  }

  // fields

  clearFields();
  for (size_t ii = 0; ii < rhs._fields.size(); ii++) {
    RadxField *field = new RadxField(*rhs._fields[ii]);
    _fields.push_back(field);
  }

  return *this;
  
}

//////////////////////////////////////////////////
// copy following meta data only:
//   main members, calibs, sweeps as in file
//
// does not copy sweeps, rays and fields

void RadxVol::copyMeta(const RadxVol &rhs)
  
{
  
  if (&rhs == this) {
    return;
  }
  
  // copy the base class metadata

  _debug = rhs._debug;

  _version = rhs._version;
  _title = rhs._title;
  _institution = rhs._institution;
  _references = rhs._references;
  _source = rhs._source;
  _history = rhs._history;
  _comment = rhs._comment;
  _statusXml = rhs._statusXml;
  
  _scanName = rhs._scanName;
  _scanId = rhs._scanId;

  _platform = rhs._platform;

  _volNum = rhs._volNum;

  _startTimeSecs = rhs._startTimeSecs;
  _endTimeSecs = rhs._endTimeSecs;
  _startNanoSecs = rhs._startNanoSecs;
  _endNanoSecs = rhs._endNanoSecs;

  _rayTimesIncrease = rhs._rayTimesIncrease;
  _transitionFlags = rhs._transitionFlags;
  _pathInUse = rhs._pathInUse;

  // sweeps as in orig file

  clearSweepsAsInFile();
  for (size_t ii = 0; ii < rhs._sweepsAsInFile.size(); ii++) {
    RadxSweep *sweep = new RadxSweep(*rhs._sweepsAsInFile[ii]);
    _sweepsAsInFile.push_back(sweep);
  }

  // radar calibs

  clearRcalibs();
  for (size_t ii = 0; ii < rhs._rcalibs.size(); ii++) {
    RadxRcalib *calib = new RadxRcalib(*rhs._rcalibs[ii]);
    _rcalibs.push_back(calib);
  }

  // correction factors

  clearCfactors();
  if (_cfactors != NULL) {
    _cfactors = new RadxCfactors(*_cfactors);
  } else {
    _cfactors = NULL;
  }

  // ray search

  _searchMaxWidth = rhs._searchMaxWidth;
  _searchRays = rhs._searchRays;

}

//////////////////////////////////////////////////
// copy one sweep, clearing other sweeps as needed
//

void RadxVol::copy(const RadxVol &rhs, int sweepNum)
  
{
  
  if (&rhs == this) {
    return;
  }

  // copy meta data
  
  copyMeta(rhs);
  
  // copy the range geometry and data packing

  copyRangeGeom(rhs);
  copyPacking(rhs);
  
  // find sweep index corresponding to the sweep number
  
  int sweepIndex = -1;
  for (size_t ii = 0; ii < rhs._sweeps.size(); ii++) {
    if (rhs._sweeps[ii]->getSweepNumber() == sweepNum) {
      sweepIndex = ii;
      break;
    }
  }

  // if we did not get an exact match in sweep number, choose
  // the closest one to that requested
  
  if (sweepIndex < 0) {
    int minDiff = 9999;
    sweepIndex = 0;
    for (size_t ii = 0; ii < rhs._sweeps.size(); ii++) {
      int num = rhs._sweeps[ii]->getSweepNumber();
      int diff = abs(num - sweepNum);
      if (diff < minDiff) {
        minDiff = diff;
        sweepIndex = ii;
      }
    }
  }

  // get min and max ray indexes

  int minRayIndex = rhs._sweeps[sweepIndex]->getStartRayIndex();
  int maxRayIndex = rhs._sweeps[sweepIndex]->getEndRayIndex();

  // clear the packing info

  clearPacking();

  // add the rays to this volume

  const vector<RadxRay *> &rhsRays = rhs.getRays();
  for (int ii = minRayIndex; ii <= maxRayIndex; ii++) {
    RadxRay *ray = new RadxRay(*rhsRays[ii]);
    addRay(ray);
  }

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();

  // check for indexed rays, set info on rays
  
  checkForIndexedRays();

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxVol::clear()
  
{

  _version.clear();
  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _statusXml.clear();

  _scanName.clear();
  _scanId = 0;

  _platform.clear();

  _volNum = Radx::missingMetaInt;
  
  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _rayTimesIncrease = true;

  for (int ii = 0; ii < _searchAngleN; ii++) {
    _searchRays[ii] = NULL;
  }

  clearRays();
  clearSweeps();
  clearSweepsAsInFile();
  clearRcalibs();
  clearFields();
  clearCfactors();
  clearFrequency();
  clearPacking();

}

///////////////
// set debug

void RadxVol::setDebug(bool val) {
  _debug = val;
}

////////////////////////////////////////////////////////////////
// add a ray

void RadxVol::addRay(RadxRay *ray)
  
{

  if (!ray->getRangeGeomSet()) {
    cerr << "WARNING - Range geom has not been set on ray" << endl;
  }
  _rays.push_back(ray);
  ray->addClient();  // take responsibility for ray memory
  addToPacking(ray->getNGates());
  copyRangeGeom(*ray);

}

//////////////////////////////////////////////////////////////
// add a sweep object
// Volume takes responsibility for freeing the sweep object.

void RadxVol::addSweep(RadxSweep *sweep)
  
{
  _sweeps.push_back(sweep);
}
 
//////////////////////////////////////////////////////////////
/// Add sweep to 'as-in-file' vector
/// These are the sweeps as originally in the file before selected
/// sweeps were requested.
/// A copy of the object is made, and is managed by this object.

void RadxVol::addSweepAsInFile(RadxSweep *sweep)
  
{
  RadxSweep *sweepf = new RadxSweep(*sweep);
  _sweepsAsInFile.push_back(sweepf);
}
 
//////////////////////////////////////////////////////////////
// add a calibration object
// Volume takes responsibility for freeing the cal object

void RadxVol::addCalib(RadxRcalib *calib)
  
{

  if (calib->getCalibTime() <= 0) {
    calib->setCalibTime(getStartTimeSecs());
  }

  if (calib->getRadarName().size() == 0) {
    calib->setRadarName(getInstrumentName());
  }
  if (calib->getWavelengthCm() < 0) {
    calib->setWavelengthCm(getWavelengthCm());
  }
  if (calib->getBeamWidthDegH() < 0) {
    calib->setBeamWidthDegH(getRadarBeamWidthDegH());
  }
  if (calib->getBeamWidthDegV() < 0) {
    calib->setBeamWidthDegV(getRadarBeamWidthDegV());
  }
  if (calib->getAntennaGainDbH() < 0) {
    calib->setAntennaGainDbH(getRadarAntennaGainDbH());
  }
  if (calib->getAntennaGainDbV() < 0) {
    calib->setAntennaGainDbV(getRadarAntennaGainDbV());
  }
  
  _rcalibs.push_back(calib);
}

//////////////////////////////////////////////////////////////
// add a field object
// Volume takes responsibility for freeing the field object

void RadxVol::addField(RadxField *field)
  
{
  _fields.push_back(field);
  copyRangeGeom(*field);
}

////////////////////////////////////////////////////////////////
// compute the max number of gates

void RadxVol::computeMaxNGates() const
  
{
  _maxNGates = 0;
  _nGatesVary = false;
  size_t prevNGates = 0;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay &ray = *_rays[iray];
    size_t rayNGates = ray.getNGates();
    if (rayNGates > _maxNGates) {
      _maxNGates = rayNGates;
    }
    if (iray > 0) {
      if (rayNGates != prevNGates) {
        _nGatesVary = true;
      }
    }
    prevNGates = rayNGates;
  }
}

//////////////////////////////////////////////////////////////
// set the correction factors

void RadxVol::setCfactors(RadxCfactors &cfac)
  
{
  clearCfactors();
  _cfactors = new RadxCfactors(cfac);
}

/////////////////////////////////////////
// set the target scan rate

void RadxVol::setTargetScanRateDegPerSec(double rate)
{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    _sweeps[ii]->setTargetScanRateDegPerSec(rate);
  }
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setTargetScanRateDegPerSec(rate);
  }
}
  
void RadxVol::setTargetScanRateDegPerSec(int sweepNum, double rate)
{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii]->getSweepNumber() == sweepNum) {
      RadxSweep *sweep = _sweeps[ii];
      sweep->setTargetScanRateDegPerSec(rate);
      for (size_t iray = sweep->getStartRayIndex();
           iray <= sweep->getEndRayIndex(); iray++) {
        _rays[iray]->setTargetScanRateDegPerSec(rate);
      }
      return;
    }
  }
}

//////////////////////////////////////////////////////////////
/// Set the number of gates.
///
/// If more gates are needed, extend the field data out to a set number of
/// gates. The data for extra gates are set to missing values.
///
/// If fewer gates are needed, the data is truncated.

void RadxVol::setNGates(size_t nGates)

{
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setNGates(nGates);
  }
}

//////////////////////////////////////////////////////////////
/// Set to constant number of gates per ray.
/// 
/// First we determine the max number of gates, and also check
/// for a variable number of gates. If the number of gates does
/// vary, the shorter rays are padded out with missing data.

void RadxVol::setNGatesConstant()

{
  
  computeMaxNGates();
  if (_nGatesVary) {
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      _rays[iray]->setNGates(_maxNGates);
    }
  }
  _nGatesVary = false;
  
}
  
/////////////////////////////////////////////////////////////////////////
/// Set the maximum range.
/// Removes excess gates as needed.
/// Does nothing if the current max range is less than that specified.

void RadxVol::setMaxRangeKm(double maxRangeKm)

{
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setMaxRangeKm(maxRangeKm);
  }
  setPackingFromRays();
}
  
//////////////////////////////////////////////////////////////
/// Set the packing from the rays

void RadxVol::setPackingFromRays()

{
  clearPacking();
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    addToPacking(_rays[iray]->getNGates());
  }
}
  
//////////////////////////////////////////////////////////////////
/// get the predominant sweep mode
/// i.e. the most common sweep mode in the volume

Radx::SweepMode_t RadxVol::getPredomSweepMode() const

{

  // set up a map for the various sweep modes

  map<Radx::SweepMode_t, int> modeMap;

  modeMap[Radx::SWEEP_MODE_NOT_SET] = 0;
  modeMap[Radx::SWEEP_MODE_CALIBRATION] = 0;
  modeMap[Radx::SWEEP_MODE_SECTOR] = 0;
  modeMap[Radx::SWEEP_MODE_COPLANE] = 0;
  modeMap[Radx::SWEEP_MODE_RHI] = 0;
  modeMap[Radx::SWEEP_MODE_VERTICAL_POINTING] = 0;
  modeMap[Radx::SWEEP_MODE_IDLE] = 0;
  modeMap[Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE] = 0;
  modeMap[Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE] = 0;
  modeMap[Radx::SWEEP_MODE_SUNSCAN] = 0;
  modeMap[Radx::SWEEP_MODE_POINTING] = 0;
  modeMap[Radx::SWEEP_MODE_FOLLOW_VEHICLE] = 0;
  modeMap[Radx::SWEEP_MODE_EL_SURV] = 0;
  modeMap[Radx::SWEEP_MODE_MANUAL_PPI] = 0;
  modeMap[Radx::SWEEP_MODE_MANUAL_RHI] = 0;
  modeMap[Radx::SWEEP_MODE_SUNSCAN_RHI] = 0;

  // accumulate the number of rays for each mode
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    if (!ray.getAntennaTransition()) {
      Radx::SweepMode_t sweepMode = ray.getSweepMode();
      if (fabs(ray.getElevationDeg() - 90.0) < 2.5) {
	// elev between 87.5 and 92.5
	sweepMode = Radx::SWEEP_MODE_VERTICAL_POINTING;
      }
      modeMap[sweepMode]++;
    }
  }
  
  // now find the one with the max count

  Radx::SweepMode_t predomMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  int maxCount = 0;
  map<Radx::SweepMode_t, int>::iterator ii;
  for (ii = modeMap.begin(); ii != modeMap.end(); ii++) {
    Radx::SweepMode_t mode = ii->first;
    int count = ii->second;
    if (count > maxCount) {
      predomMode = mode;
      maxCount = count;
    }
  }

  return predomMode;

}

//////////////////////////////////////////////////////////////////
/// get the predominant range geometry

void RadxVol::getPredomRayGeom(double &startRangeKm,
                               double &gateSpacingKm) const

{
  RayGeom predom = _getPredomGeom();
  startRangeKm = predom.startRange;
  gateSpacingKm = predom.gateSpacing;
}

//////////////////////////////////////////////////////////////////
/// get the finest range geometry

void RadxVol::getFinestRayGeom(double &startRangeKm,
                               double &gateSpacingKm) const

{
  RayGeom finest = _getFinestGeom();
  startRangeKm = finest.startRange;
  gateSpacingKm = finest.gateSpacing;
}

//////////////////////////////////////////////////////////////////
/// Get the list of unique field names, compiled by
/// searching through all rays.
///
/// The order of the field names found is preserved

vector<string> RadxVol::getUniqueFieldNameList() const

{
  
  vector<string> fieldNames;

  // find the set of fields names

  set<string> nameSet;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    for (size_t ifield = 0; ifield < ray.getNFields(); ifield++) {
      string name = ray.getField(ifield)->getName();
      pair<set<string>::const_iterator, bool> ret = nameSet.insert(name);
      if (ret.second == true) {
        // field name not previously in set, so add to vector
        fieldNames.push_back(name);
      }
    }
  }

  return fieldNames;

}

//////////////////////////////////////////////////
/// convert all fields to same data type
/// widening as required

void RadxVol::widenFieldsToUniformType()

{

  // check if fields are owned by rays instead of volume

  bool fieldsOwnedByRays = true;
  if (_fields.size() > 0) {
    fieldsOwnedByRays = false;
  } else {
    loadFieldsFromRays(true);
  }

  // search for the narrowest data type which works for all
  // fields
  
  int dataByteWidth = 1;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    const RadxField &fld = *_fields[ii];
    if (fld.getByteWidth() > dataByteWidth) {
      dataByteWidth = fld.getByteWidth();
    }
  }
  
  if (dataByteWidth == 2) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi16();
    }
  } else if (dataByteWidth == 4) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl32();
    }
  } else if (dataByteWidth == 8) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl64();
    }
  }

  // do we need to put back into ray ownership

  if (fieldsOwnedByRays) {
    loadRaysFromFields();
  } else {
    setRayFieldPointers();
  }
  
}

//////////////////////////////////////////////////
/// set all fields to same data type

void RadxVol::setFieldsToUniformType(Radx::DataType_t dataType)

{

  // check if fields are owned by rays instead of volume

  bool fieldsOwnedByRays = true;
  if (_fields.size() > 0) {
    fieldsOwnedByRays = false;
  } else {
    loadFieldsFromRays(true);
  }

  // convert to requested type

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToType(dataType);
  }
  
  // do we need to put back into ray ownership

  if (fieldsOwnedByRays) {
    loadRaysFromFields();
  } else {
    setRayFieldPointers();
  }
  
}

//////////////////////////////////////////////////////////////////
/// Get a field from a ray, given the name.
/// Find the first available field on a suitable ray.
/// Returns field pointer on success, NULL on failure.

const RadxField *RadxVol::getFieldFromRay(const string &name) const

{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    const vector<RadxField *> &fields = ray.getFields();
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      if (fields[ifield]->getName() == name) {
        return fields[ifield];
      }
    }
  }

  return NULL;

}

//////////////////////////////////////////////////////////////////
/// Load contiguous fields on the volume, from fields in the rays.
///
/// The fields in the rays are then set to point to the contiguous
/// fields.
///
/// The memory for the fields is managed by the fields on the volume.
/// The fields on the rays point into these contiguous arrys.
///
/// nFieldsConstantPerRay:
///   only applicable if not all input fields are present in the
///   input data.
/// If true, each ray will have the same number of fields, even if in
/// the incoming data some rays do not have all of the fields. If
/// false, a field will only be included in a ray if that field
/// also exists in the incoming data.
///
/// See also: loadRaysFromFields()

void RadxVol::loadFieldsFromRays(bool nFieldsConstantPerRay /* = false */)
  
{

  // check we have data
  
  if (_rays.size() < 1) {
    return;
  }

  // has this already been done?

  if (_fields.size() > 0) {
    return;
  }

  // ensure all rays have local data
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setDataLocal();
  }

  // free any existing fields on the volume

  clearFields();
  
  // set vol number and times

  loadVolumeInfoFromRays();
  
  // get the set of unique fields names in this volume

  vector<string> fieldNames = getUniqueFieldNameList();

  // make copies of the fields, and add them to the volume

  for (size_t ii = 0; ii < fieldNames.size(); ii++) {
    RadxField *field = copyField(fieldNames[ii]);
    if (field != NULL) {
      addField(field);
    }
  } // ii
      
  // Free up field data in rays, point them to the contiguous fields.
  // This process also adjusts the scale and bias in the ray fields to
  // the same as in the global fields.

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    RadxField &field = *_fields[ifield];
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      RadxField *rayField = ray.getField(field.getName());
      if (rayField != NULL) {
        size_t nGates;
        const void *data = field.getData(iray, nGates);
        rayField->setDataRemote(field, data, nGates);
      } else {
        if (nFieldsConstantPerRay) {
          // add any missing fields
          rayField = new RadxField(field.getName(), field.getUnits());
          rayField->copyMetaData(field);
          size_t nGates;
          const void *data = field.getData(iray, nGates);
          rayField->setDataRemote(field, data, nGates);
          ray.addField(rayField);
        }
      }
    }      
  } // ifield

  // load the sweep info from rays
  
  loadSweepInfoFromRays();

  // copy packing from first field to the volume

  if (_fields.size() > 0) {
    copyPacking(*_fields[0]);
  }

}

//////////////////////////////////////////////////////////////
/// Load up the ray fields from the contiguous fields in the volume.
/// This is the inverse of loadFieldsFromRays()

void RadxVol::loadRaysFromFields()
  
{

  if (_fields.size() < 1) {
    return;
  }

  // for fields on all rays, set them to manage their own data

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setDataLocal();
  } // iray
  
  // clear fields on the volume

  clearFields();

}

//////////////////////////////////////////////////////////////
/// Set field data pointers in the rays to point into the
/// main contiguous fields on the volume.
///
/// If the rays that have been added to the volume do not hold the
/// data themselves, call this method to set the pointers in the ray
/// fields so that they refer to the main fields on the volume.
  
void RadxVol::setRayFieldPointers()
  
{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay *ray = _rays[iray];
    ray->clearFields();
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

      const RadxField *field = _fields[ifield];
      const string &name = field->getName();
      const string &standardName = field->getStandardName();
      const string &longName = field->getLongName();
      const string &units = field->getUnits();
      const string &thresholdFieldName = field->getThresholdFieldName();
      double thresholdValue = field->getThresholdValue();
      int nGates = field->getRayNGates(iray);
      int index = field->getRayStartIndex(iray);
      
      switch (field->getDataType()) {
        case Radx::FL64: {
          const Radx::fl64 *dptr = (Radx::fl64 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingFl64(),
                                          dptr + index, false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::FL32: {
          const Radx::fl32 *dptr = (Radx::fl32 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingFl32(),
                                          dptr + index, false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::SI32: {
          const Radx::si32 *dptr = (Radx::si32 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingSi32(),
                                          dptr + index,
                                          field->getScale(),
                                          field->getOffset(),
                                          false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::SI16: {
          const Radx::si16 *dptr = (Radx::si16 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingSi16(),
                                          dptr + index,
                                          field->getScale(),
                                          field->getOffset(),
                                          false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::SI08: {
          const Radx::si08 *dptr = (Radx::si08 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingSi08(),
                                          dptr + index,
                                          field->getScale(),
                                          field->getOffset(),
                                          false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        default: {}
      } // switch

    } // ifield

  } // iray

}

//////////////////////////////////////////////////////////////////
/// Make a copy of the field with the specified name.
///
/// This forms a contiguous field from the ray data.
///
/// Returns a pointer to the field on success, NULL on failure.

RadxField *RadxVol::copyField(const string &fieldName) const
  
{

  // check we have data
  
  if (_rays.size() < 1) {
    return NULL;
  }
  
  // create the field
  // use the first available field in any ray as a template
  
  RadxField *copy = NULL;
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    const RadxField *rayField = ray.getField(fieldName);
    if (rayField != NULL) {
      // create new field using name, units and type
      copy = new RadxField(rayField->getName(), rayField->getUnits());
      copy->copyMetaData(*rayField);
      break;
    }
    if (copy != NULL) {
      break;
    }
  } // iray
  
  if (copy == NULL) {
    // no suitable field
    return NULL;
  }

  // check if the fields on the rays are uniform -
  // i.e. all have the same type, scale and offset
  
  bool fieldsAreUniform = true;
  Radx::DataType_t dataType = copy->getDataType();
  double scale = copy->getScale();
  double offset = copy->getOffset();
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    const RadxField *rayField = ray.getField(fieldName);
    if (rayField == NULL) {
      continue;
    }
    if (rayField->getDataType() != dataType) {
      // different field data types
      fieldsAreUniform = false;
      break;
    }
    if (dataType == Radx::FL32 || dataType == Radx::FL64) {
      // for float types don't worry about scale and bias
      continue;
    }
    if (fabs(rayField->getScale() - scale) > 1.0e-5) {
      // different scale
      fieldsAreUniform = false;
      break;
    }
    if (fabs(rayField->getOffset() - offset) > 1.0e-5) {
      // different offset
      fieldsAreUniform = false;
      break;
    }
  } // iray

  // load up data from the rays

  if (fieldsAreUniform) {

    // type, scale and offset constant

    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      size_t nGates = ray.getNGates();
      RadxField *rfld = ray.getField(fieldName);
      if (rfld == NULL) {
        copy->addDataMissing(nGates);
      } else {
        RadxField rcopy(*rfld);
        if (dataType == Radx::FL64) {
          rcopy.setMissingFl64(copy->getMissingFl64());
          copy->addDataFl64(nGates, rcopy.getDataFl64());
        } else if (dataType == Radx::FL32) {
          rcopy.setMissingFl32(copy->getMissingFl32());
          copy->addDataFl32(nGates, rcopy.getDataFl32());
        } else if (dataType == Radx::SI32) {
          rcopy.setMissingSi32(copy->getMissingSi32());
          copy->addDataSi32(nGates, rcopy.getDataSi32());
        } else if (dataType == Radx::SI16) {
          rcopy.setMissingSi16(copy->getMissingSi16());
          copy->addDataSi16(nGates, rcopy.getDataSi16());
        } else if (dataType == Radx::SI08) {
          rcopy.setMissingSi08(copy->getMissingSi08());
          copy->addDataSi08(nGates, rcopy.getDataSi08());
        }
      }
    } // iray

    return copy;

  }

  // fields are not uniform and must be converted to a common type

  if (dataType == Radx::FL64) {
    
    // 64-bit floats
    
    copy->setTypeFl64(Radx::missingFl64);
    
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      size_t nGates = ray.getNGates();
      RadxField *rfld = ray.getField(fieldName);
      if (rfld == NULL) {
        copy->addDataMissing(nGates);
      } else {
        RadxField rcopy(*rfld);
        rcopy.convertToFl64();
        copy->addDataFl64(nGates, rcopy.getDataFl64());
      }
    } // iray
    
    // convert to final type
    copy->convertToType(dataType);

  } else {
    
    // all others
    // convert to fl32 for now
    
    copy->setTypeFl32(Radx::missingFl32);
    
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      size_t nGates = ray.getNGates();
      RadxField *rfld = ray.getField(fieldName);
      if (rfld == NULL) {
        copy->addDataMissing(nGates);
      } else {
        RadxField rcopy(*rfld);
        rcopy.convertToFl32();
        copy->addDataFl32(nGates, rcopy.getDataFl32());
      }
    } // iray
    
    // convert to final type
    copy->convertToType(dataType);

  } // if (dataType == Radx::FL64)

  return copy;

}

/////////////////////////////////////////////////////////////////
/// Rename a field
/// returns 0 on success, -1 if field does not exist in any ray

int RadxVol::renameField(const string &oldName, const string &newName)
  
{

  int iret = -1;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay &ray = *_rays[iray];
    if (ray.renameField(oldName, newName) == 0) {
      iret = 0;
    }
  }

  return iret;
  
}

//////////////////////////////////////////////////////////////
// Load up modes from sweeps to rays
// This assumes the sweeps meta data is filled out, but the
// modes are missing from the rays.

void RadxVol::loadModesFromSweepsToRays()
  
{

  if (_rays.size() < 1) {
    return;
  }

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    const RadxSweep *sweep = _sweeps[isweep];

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {

      RadxRay &ray = *_rays[iray];
      
      ray.setSweepNumber(sweep->getSweepNumber());
      ray.setSweepMode(sweep->getSweepMode());
      ray.setPolarizationMode(sweep->getPolarizationMode());
      ray.setPrtMode(sweep->getPrtMode());
      ray.setFollowMode(sweep->getFollowMode());

    } // iray

  } // isweep

}

//////////////////////////////////////////////////////////////
// Load up fixed angle from sweeps to rays
// This assumes the sweeps meta data is filled out, but the
// fixed angles are missing from the rays.

void RadxVol::loadFixedAnglesFromSweepsToRays()
  
{

  if (_rays.size() < 1) {
    return;
  }

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    const RadxSweep *sweep = _sweeps[isweep];

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {

      RadxRay &ray = *_rays[iray];
      
      ray.setFixedAngleDeg(sweep->getFixedAngleDeg());

    } // iray

  } // isweep

}

//////////////////////////////////////////////////////////////
// Set the calbration index on the rays,
// using pulse width to determine which calibration is relevant

void RadxVol::loadCalibIndexOnRays()
{

  if (_rcalibs.size() < 1) {
    return;
  }

  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay &ray = *_rays[iray];
    double rayPulseWidth = ray.getPulseWidthUsec();

    // find the calibration which minimizes the difference between the
    // ray pulse width and calib pulse width

    int calIndex = 0;
    double minDiff = fabs(rayPulseWidth - _rcalibs[0]->getPulseWidthUsec());
    
    for (size_t icalib = 1; icalib < _rcalibs.size(); icalib++) {
      double calibPulseWidth = _rcalibs[icalib]->getPulseWidthUsec();
      double diff = fabs(rayPulseWidth - calibPulseWidth);
      if (diff < minDiff) {
        calIndex = icalib;
        minDiff = diff;
      }
      
    } // icalib

    ray.setCalibIndex(calIndex);

  } // iray

}

/////////////////////////////////////////////////////////
// clear rays

void RadxVol::clearRays()
  
{

  // delete rays

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay::deleteIfUnused(_rays[ii]);
  }
  _rays.clear();

}

/////////////////////////////////////////////////////////
// clear ray field data
// retain the ray meta-data

void RadxVol::clearRayFields()
  
{

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->clearFields();
  }

}

/////////////////////////////////////////////////////////
// clear sweep info - for this object

void RadxVol::clearSweeps()
  
{

  // delete sweeps

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();

}

/////////////////////////////////////////////////////////
// clear sweep info - as they were in file

void RadxVol::clearSweepsAsInFile()
  
{
  for (size_t ii = 0; ii < _sweepsAsInFile.size(); ii++) {
    delete _sweepsAsInFile[ii];
  }
  _sweepsAsInFile.clear();
}

/////////////////////////////////////////////////////////
// clear radar calib info

void RadxVol::clearRcalibs()
  
{

  // delete calibs

  for (size_t ii = 0; ii < _rcalibs.size(); ii++) {
    delete _rcalibs[ii];
  }
  _rcalibs.clear();

}

/////////////////////////////////////////////////////////
// clear the field data in the object, and the
// field vector

void RadxVol::clearFields()
  
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// clear the correction factors

void RadxVol::clearCfactors()
  
{
  if (_cfactors) {
    delete _cfactors;
  }
  _cfactors = NULL;
}

/////////////////////////////////////////////////////////
// clear the frequency list

void RadxVol::clearFrequency()
  
{
  _platform.clearFrequency();
}

/////////////////////////////////////////////////////////
// print

void RadxVol::print(ostream &out) const
  
{
  
  out << "=============== RadxVol ===============" << endl;
  out << "  version: " << _version << endl;
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId(VCP): " << _scanId << endl;

  _platform.print(out);

  out << "  volNum: " << _volNum << endl;
  if (checkIsRhi()) {
    out << "  rhiMode? Y" << endl;
  } else {
    out << "  rhiMode? N" << endl;
  }
  out << "  startTimeSecs: " << RadxTime::strm(_startTimeSecs) << endl;
  out << "  startNanoSecs: " << _startNanoSecs << endl;
  out << "  endTimeSecs: " << RadxTime::strm(_endTimeSecs) << endl;
  out << "  endNanoSecs: " << _endNanoSecs << endl;
  if (_rayTimesIncrease) {
    out << "  ray times are in increasing order" << endl;
  } else {
    out << "  NOTE: ray times are NOT in increasing order" << endl;
  }
  out << "  n sweeps: " << _sweeps.size() << endl;
  out << "  n rays: " << _rays.size() << endl;
  out << "  n calibs: " << _rcalibs.size() << endl;
  vector<string> fieldNames = getUniqueFieldNameList();
  if (_fields.size() > 0) {
    out << "  n fields: " << _fields.size() << endl;
  } else {
    out << "  n fields: " << fieldNames.size() << endl;
    for (size_t ii = 0; ii < fieldNames.size(); ii++) {
      out << "    field[" << ii << "]: " << fieldNames[ii] << endl;
    }
  }
  RadxRangeGeom::print(out);
  RadxPacking::printSummary(out);
  out << "===========================================" << endl;

  if (_sweepsAsInFile.size() != _sweeps.size()) {
    out << "===========>> SWEEPS AS IN FILE <<===============" << endl;
    for (size_t ii = 0; ii < _sweepsAsInFile.size(); ii++) {
      _sweepsAsInFile[ii]->print(out);
    }
    out << "=========>> END SWEEPS AS IN FILE <<=============" << endl;
    out << "=========>>  SWEEPS AS IN OBJECT  <<=============" << endl;
  }

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    _sweeps[ii]->print(out);
  }

  if (_sweepsAsInFile.size() != _sweeps.size()) {
    out << "=========>> END SWEEPS AS IN OBJECT <<===========" << endl;
  }

  if (_fields.size() > 0) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->print(cout);
    }
  } else {
    for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {
      string fieldName = fieldNames[ifield];
      const RadxField *fld = getFieldFromRay(fieldName);
      if (fld != NULL) {
        out << "===== NOTE: Field is from first ray =====" << endl;
        fld->print(out);
        out << "=========================================" << endl;
      }
    }
  }

  for (size_t ii = 0; ii < _rcalibs.size(); ii++) {
    _rcalibs[ii]->print(out);
  }

  if (_cfactors) {
    _cfactors->print(out);
  }

  out << "=========== statusXml ===================" << endl;
  out << _statusXml << endl;
  out << "=========================================" << endl;

}

///////////////////////////////////
// print with ray meta data

void RadxVol::printWithRayMetaData(ostream &out) const

{
  
  print(out);
  
  // print rays
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->print(cout);
  }

}

///////////////////////////////////
// print with ray summary

void RadxVol::printRaySummary(ostream &out) const

{
  
  print(out);
  
  // print rays

  out << "================ RAY SUMMARY =================" << endl;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->printSummary(cout);
  }

}

//////////////////////////  
// print with field data

void RadxVol::printWithFieldData(ostream &out) const

{
  
  print(out);

  // check if rays have fields

  bool raysHaveFields = false;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_rays[ii]->getFields().size() > 0) {
      raysHaveFields = true;
      break;
    }
  }

  // print rays

  if (raysHaveFields) {
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->printWithFieldData(cout);
    }
  } else {
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->print(cout);
    }
  }

}
  
///////////////////////////////////////////
// nested ray geometry class - constructor

RadxVol::RayGeom::RayGeom() :
        startRange(0.0),
        gateSpacing(0.0),
        rayCount(0)
{
  
}

RadxVol::RayGeom::RayGeom(double start_range,
                          double gate_spacing) :
        startRange(start_range),
        gateSpacing(gate_spacing),
        rayCount(1)
{
  
}

///////////////////////////////////////////
// nested ray geometry class - print

void RadxVol::RayGeom::print(ostream &out) const
  
{
  
  out << "== RAY GEOM ==" << endl;
  out << "  startRange: " << startRange << endl;
  out << "  gateSpacing: " << gateSpacing << endl;
  out << "  rayCount: " << rayCount << endl;

}

///////////////////////////////////////////////////////////
/// Remap all fields and rays onto the specified geometry.
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.
///
/// If interp is true, uses interpolation if appropriate.
/// Otherwise uses nearest neighbor.

void RadxVol::remapRangeGeom(double startRangeKm,
                             double gateSpacingKm,
                             bool interp /* = false */)
  
{

  // set the ray fields to manage their own data

  loadRaysFromFields();

  // loop through rays, remapping geometry
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->remapRangeGeom(startRangeKm, gateSpacingKm, interp);
  }
  
  // save geometry members
  
  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

}

////////////////////////////////////////////////////////////
/// Remap data in all rays to the predominant range geometry.
///
/// A search is made through the rays, to identify which is the
/// predominant range geometry.  All rays which do not match this
/// are then remapped to this predominant geometry.
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.

void RadxVol::remapToPredomGeom()

{

  if (_rays.size() < 3) {
    return;
  }

  // remap rays to predominant geom
  
  RayGeom predom = _getPredomGeom();
  remapRangeGeom(predom.startRange, predom.gateSpacing);
  
}

////////////////////////////////////////////////////////////
/// Remap data in all rays to the finest range geometry.
///
/// A search is made through the rays, to identify which has the
/// finest gate spacing.  All rays which do not match this
/// are then remapped to this predominant geometry.
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.

void RadxVol::remapToFinestGeom()

{

  if (_rays.size() < 2) {
    return;
  }

  // remap rays to finest geom
  
  RayGeom finest = _getFinestGeom();
  remapRangeGeom(finest.startRange, finest.gateSpacing);
  
}

///////////////////////////////////////////////////////////
/// filter on the predominant geometry
///
/// Remove rays which do not conform to the 
/// predominant geometry
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.

void RadxVol::filterOnPredomGeom()

{

  // set the ray fields to manage their own data

  loadRaysFromFields();

  // find predominant geom
  
  RayGeom predom = _getPredomGeom();

  // create vector with good rays, deleting bad ones

  vector<RadxRay *> good;
  double smallDiff = 0.0001;
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay *ray = _rays[iray];

    // check the geometry
    if (fabs(ray->getStartRangeKm() - predom.startRange) < smallDiff &&
        fabs(ray->getGateSpacingKm() - predom.gateSpacing) < smallDiff) {
      good.push_back(ray);
    } else {
      RadxRay::deleteIfUnused(ray);
    }

  }

  // replace rays vector

  _rays = good;

  // set geometry

  _startRangeKm = predom.startRange;
  _gateSpacingKm = predom.gateSpacing;

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();

}

///////////////////////////////////////////
// find predominant geometry

RadxVol::RayGeom RadxVol::_getPredomGeom() const

{

  double smallDiff = 0.0001;
  vector<RayGeom> geoms;

  // loop through rays, accumulating geometry information

  for (size_t iray = 0; iray < _rays.size(); iray++) {

    const RadxRay &ray = *_rays[iray];

    // check if we already have a matching geometry
    
    bool found = false;
    for (size_t ii = 0; ii < geoms.size(); ii++) {
      RayGeom &geom = geoms[ii];
      // check the geometry
      if (fabs(ray.getStartRangeKm() - geom.startRange) < smallDiff &&
          fabs(ray.getGateSpacingKm() - geom.gateSpacing) < smallDiff) {
        // matches
        geom.rayCount++;
        found = true;
        break;
      }
    } // ii 
    
    if (!found) {
      // no match, add one
      RayGeom geom(ray.getStartRangeKm(),
                   ray.getGateSpacingKm());
      geoms.push_back(geom);
    }
    
  } // iray

  // find the predominant geometry - i.e with highest ray count

  RayGeom predom(0.0, 0.0);
  for (size_t ii = 0; ii < geoms.size(); ii++) {
    RayGeom &geom = geoms[ii];
    if (geom.rayCount > predom.rayCount) {
      predom = geom;
    }
  } // ii 

  return predom;

}

///////////////////////////////////////////
// find geometry with finest resolytion

RadxVol::RayGeom RadxVol::_getFinestGeom() const

{
  
  RayGeom finest;
  finest.gateSpacing = 1.0e99;
  finest.startRange = 0.0;
  
  // loop through rays, accumulating geometry information
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    const RadxRay &ray = *_rays[iray];

    if (ray.getGateSpacingKm() < finest.gateSpacing) {
      finest.gateSpacing = ray.getGateSpacingKm();
      finest.startRange = ray.getStartRangeKm();
    }

  }

  return finest;

}

//////////////////////////////////////////////////////////////
/// Copy the range geom from the fields to the rays, provided
/// the fields have consistent in geometry

void RadxVol::copyRangeGeomFromFieldsToRays()
  
{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    ray->copyRangeGeomFromFields();
    copyRangeGeom(*ray);
  }

}
  
//////////////////////////////////////////////////////////////
/// Copy the range geom from the rays to the fields

void RadxVol::copyRangeGeomFromRaysToFields()
  
{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    ray->copyRangeGeomToFields();
  }

}
  
///////////////////////////////////////////////////////////
/// remove rays with utility flag set

void RadxVol::removeFlaggedRays()

{
  
  vector<RadxRay *> goodRays;
  vector<RadxRay *> badRays;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_rays[ii]->getUtilityFlag()) {
      badRays.push_back(_rays[ii]);
    } else {
      goodRays.push_back(_rays[ii]);
    }
  }

  removeBadRays(goodRays, badRays);

}

///////////////////////////////////////////////////////////
/// filter based on ray vectors
/// Keep the good rays, remove the bad rays

void RadxVol::removeBadRays(vector<RadxRay *> &goodRays,
                            vector<RadxRay *> &badRays)

{

  // free up the bad rays

  for (size_t ii = 0; ii < badRays.size(); ii++) {
    RadxRay::deleteIfUnused(badRays[ii]);
  }
  _rays = goodRays;

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();

}

///////////////////////////////////////////////////////////
/// Remove rays with the antenna transition flag set.

void RadxVol::removeTransitionRays()

{

  vector<RadxRay *> goodRays;

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    if (ray->getAntennaTransition()) {
      // free up transition ray
      RadxRay::deleteIfUnused(ray);
    } else {
      // add to good list
      goodRays.push_back(ray);
    }
  }

  _rays = goodRays;

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();
  
}

////////////////////////////////////////////////////////////
// Remove rays with transitions, with the specified margin.
//
// Sometimes the transition flag is turned on too early in
// a transition, on not turned off quickly enough after a transition.
// If you set this to a number greater than 0, that number of rays
// will be included at each end of the transition, i.e. the
// transition will effectively be shorter at each end by this
// number of rays

void RadxVol::removeTransitionRays(int nRaysMargin)

{

  // find any marked transitions

  _findTransitions(nRaysMargin);
  bool transitionsExist = false;
  for (size_t ii = 0; ii < _transitionFlags.size(); ii++) {
    if (_transitionFlags[ii]) {
      transitionsExist = true;
      break;
    }
  }

  if (!transitionsExist) {
    return;
  }

  // load up good rays with non-transitions

  vector<RadxRay *> goodRays;
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    if (_transitionFlags[iray]) {
      // free up transition ray
      RadxRay::deleteIfUnused(ray);
    } else {
      // add to good list
      ray->setAntennaTransition(false);
      goodRays.push_back(ray);
    }
  }

  // save to _rays

  _rays = goodRays;

  // load up current information

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

}

////////////////////////////////////////////////////////////
// Find the transitions in the rays

void RadxVol::_findTransitions(int nRaysMargin)

{

  _transitionFlags.clear();

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _transitionFlags.push_back(_rays[iray]->getAntennaTransition());
  } // iray

  // widen good data by the specified margin

  if (nRaysMargin > 0) {
    
    // widen at start of transitions
    
    for (int ii = _transitionFlags.size() - 1; ii > 0; ii--) {
      if (_transitionFlags[ii] && !_transitionFlags[ii-1]) {
        for (int jj = ii; jj < ii + nRaysMargin; jj++) {
          if (jj < (int) _transitionFlags.size()) {
            _transitionFlags[jj] = false;
          }
        }
      }
    } // ii
    
    // widen at end of transitions
    
    for (int ii = 1; ii < (int) _transitionFlags.size(); ii++) {
      if (_transitionFlags[ii-1] && !_transitionFlags[ii]) {
        for (int jj = ii - nRaysMargin; jj < ii; jj++) {
          if (jj >= 0) {
            _transitionFlags[jj] = false;
          }
        }
      }
    } // ii
    
  }

}

/////////////////////////////////////////////////////////////
/// Check transitions in surveillance mode, ensuring that the
/// pointing angle error is within the  given margin and that
/// the ray belongs to the correct sweep.

void RadxVol::optimizeSurveillanceTransitions(double maxFixedAngleErrorDeg)

{
  
  if (_sweeps.size() < 2) {
    return;
  }
  
  /// ensure we are in surveillance mode
  
  int nSur = 0;
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    if (sweep->getSweepMode() == Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      nSur++;
    }
  }
  if (nSur < ((int) _sweeps.size() / 2)) {
    // not predominantly surveillance
    return;
  }

  // ensure sweep info is up to date

  loadSweepInfoFromRays();

  // loop through sweeps, working on pairs of sweeps
  // we consider the transition from one sweep to the next

  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {

    // check modes
    
    const RadxSweep *sweep1 = _sweeps[isweep];
    if (sweep1->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      continue;
    }
    int sweepNum1 = sweep1->getSweepNumber();

    const RadxSweep *sweep2 = _sweeps[isweep + 1];
    if (sweep2->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      continue;
    }
    int sweepNum2 = sweep2->getSweepNumber();

    // check the sweeps are not primarily in transition
    
    if (computeSweepFractionInTransition(isweep) > 0.8) {
      continue;
    }
    if (computeSweepFractionInTransition(isweep + 1) > 0.8) {
      continue;
    }

    double fixedAngle1 = sweep1->getFixedAngleDeg();
    double fixedAngle2 = sweep2->getFixedAngleDeg();

    int index1 = sweep1->getEndRayIndex();
    int index2 = sweep2->getStartRayIndex();
    
    RadxRay *ray1 = _rays[index1];
    RadxRay *ray2 = _rays[index2];

    double elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
    double elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);

    if (elError1 < maxFixedAngleErrorDeg &&
        elError2 < maxFixedAngleErrorDeg) {
      // no problem - do nothing
      continue;
    }
    
    if (elError1 < elError2) {
      
      // transition is early, search ahead for correct transition point

      while (elError1 < elError2) {
        
        // ray2 belongs to this sweep, not the next
        
        ray2->setSweepNumber(sweepNum1);
        ray2->setFixedAngleDeg(fixedAngle1);
        
        // update transition flag
        
        double elError = fabs(ray2->getElevationDeg() - fixedAngle1);
        if (elError < maxFixedAngleErrorDeg) {
          ray2->setAntennaTransition(false);
        } else {
          ray2->setAntennaTransition(true);
        }
        
        // move one ray forward

        index1++;
        index2++;
        if (index2 >= (int) sweep2->getEndRayIndex()) {
          break;
        }
        ray1 = _rays[index1];
        ray2 = _rays[index2];
        elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
        elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);

      }

      // update transition flags in next sweep
      
      while (elError2 > maxFixedAngleErrorDeg || ray2->getAntennaTransition()) {
        if (elError2 > maxFixedAngleErrorDeg) {
          ray2->setAntennaTransition(true);
        } else {
          ray2->setAntennaTransition(false);
        }
        index2++;
        if (index2 > (int) sweep2->getEndRayIndex()) {
          break;
        }
        ray2 = _rays[index2];
        elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);
      }

    } else { // if (elError1 < elError2) {

      // transition is late, search back for correct transition point

      while (elError2 < elError1) {

        // ray1 belongs to next sweep, not this one
        
        ray1->setSweepNumber(sweepNum2);
        ray1->setFixedAngleDeg(fixedAngle2);
       
        // update transition flags
        
        double elError = fabs(ray1->getElevationDeg() - fixedAngle2);
        if (elError < maxFixedAngleErrorDeg) {
          ray1->setAntennaTransition(false);
        } else {
          ray1->setAntennaTransition(true);
        }

        // move one ray back

        index1--;
        index2--;
        if (index1 <= (int) sweep1->getStartRayIndex()) {
          break;
        }
        ray1 = _rays[index1];
        ray2 = _rays[index2];
        elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
        elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);

      }

      // update transition flags in this sweep

      while (elError1 > maxFixedAngleErrorDeg || ray1->getAntennaTransition()) {
        if (elError1 > maxFixedAngleErrorDeg) {
          ray1->setAntennaTransition(true);
        } else {
          ray1->setAntennaTransition(false);
        }
        index1--;
        if (index1 < (int) sweep1->getStartRayIndex()) {
          break;
        }
        ray1 = _rays[index1];
        elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
      }

    } // if (elError1 < elError2) {

  } // isweep
  
  // reload sweep info, since sweep numbers on rays have changed
  
  loadSweepInfoFromRays();
  
}

///////////////////////////////////////////////////////////
/// Trim surveillance sweeps to 360 deg
///
/// Remove extra rays in each surveillance sweep

void RadxVol::trimSurveillanceSweepsTo360Deg()

{

  // clear utility flags

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->setUtilityFlag(false);
  }

  // loop through sweeps
  // If the sweep covers more than 360 degrees, set transition flag
  // on extra rays. Use the elevation angle to determine which 
  // rays to remove.

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    const RadxSweep *sweep = _sweeps[isweep];
    if (sweep->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      continue;
    }

    double azCovered = computeAzCovered(sweep);
    if (azCovered <= 360.0) {
      continue;
    }

    // initialize

    int startIndex = (int) sweep->getStartRayIndex();
    int endIndex = (int) sweep->getEndRayIndex();
    int midIndex = (startIndex + endIndex) / 2;

    // sanity check

    if (endIndex - startIndex < 10) {
      continue;
    }

    // check rotation sense - clockwise or not?
    
    bool clockwise = true;
    double azm0 = _rays[midIndex]->getAzimuthDeg();
    double azm1 = _rays[midIndex+1]->getAzimuthDeg();
    double dAz0 = azm1 - azm0;
    if (dAz0 < -180.0) {
      dAz0 += 360.0;
    } else if (dAz0 > 180.0) {
      dAz0 -= 360.0;
    } else if (dAz0 == 0) {
      continue;
    }
    if (dAz0 < 0) {
      clockwise = false;
    }

    // compute median elevation

    vector<double> elevs;
    for (int iray = startIndex; iray <= endIndex; iray++) {
      elevs.push_back(_rays[iray]->getElevationDeg());
    }
    sort(elevs.begin(), elevs.end());
    double medianElev = elevs[elevs.size()/2];
    
    // iterate, looking for 360 wraps, and setting
    // flags until there is no wrap
    
    size_t lowIndex = sweep->getStartRayIndex();
    size_t highIndex = sweep->getEndRayIndex();
    
    while (lowIndex < highIndex) {

      double lowAz = _rays[lowIndex]->getAzimuthDeg();
      double highAz = _rays[highIndex]->getAzimuthDeg();

      double dAz = highAz - lowAz;
      if (dAz < -180.0) {
        dAz += 360.0;
      } else if (dAz > 180.0) {
        dAz -= 360.0;
      }
      if (!clockwise) {
        dAz *= -1.0;
      }

      if (dAz < 0) {
        // no wrap;
        break;
      }

      // we have wrapped, so set one transition flag

      double elErrorLow = 
        fabs(medianElev - _rays[lowIndex]->getElevationDeg());
      double elErrorHigh =
        fabs(medianElev - _rays[highIndex]->getElevationDeg());
      if (elErrorLow > elErrorHigh) {
        _rays[lowIndex]->setUtilityFlag(true);
        lowIndex++;
      } else {
        _rays[highIndex]->setUtilityFlag(true);
        highIndex--;
      }
      
    } // while (lowIndex < highIndex)

    azCovered = computeAzCovered(sweep);

  } // isweep

  // remove rays which we have flagged with utility flag

  removeFlaggedRays();
  
}

////////////////////////////////////////////////////////////
/// Remove sweeps with fewer that the given number of rays
  
void RadxVol::removeSweepsWithTooFewRays(size_t minNRays)

{

  vector<RadxRay *> goodRays, badRays;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep = _sweeps[ii];
    size_t istart = sweep->getStartRayIndex();
    size_t iend = sweep->getEndRayIndex();
    if (sweep->getNRays() < minNRays) {
      for (size_t ii = istart; ii <= iend; ii++) {
        badRays.push_back(_rays[ii]);
      }
    } else {
      for (size_t ii = istart; ii <= iend; ii++) {
        goodRays.push_back(_rays[ii]);
      }
    }
  }

  removeBadRays(goodRays, badRays);

}

////////////////////////////////////////////////////////////
// Reorder the sweeps into ascending angle order
//
// If the sweeps are reordered, this means that the rays times
// will no longer be monotonically increasing

void RadxVol::reorderSweepsAscendingAngle()
{

  if (_sweeps.size() < 1) {
    return;
  }

  // check sweeps are in ascending order, do nothing if they are

  bool ascending = true;
  for (size_t ii = 1; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep0 = _sweeps[ii-1];
    RadxSweep *sweep1 = _sweeps[ii];
    if (sweep0->getFixedAngleDeg() > sweep1->getFixedAngleDeg()) {
      ascending = false;
      break;
    }
  }
  if (ascending) {
    return;
  }
  
  // create a map of sweeps with angles in ascending order

  multimap< double, RadxSweep* > sortedSweeps;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep = _sweeps[ii];
    double angle = sweep->getFixedAngleDeg();
    sortedSweeps.insert(pair< double, RadxSweep* >(angle, sweep));
  }

  if (_rays.size() < 1) {

    // if only metadata was read, just reorder the sweeps

    _sweeps.clear();
    multimap< double, RadxSweep* >::iterator iter;
    for (iter = sortedSweeps.begin(); iter != sortedSweeps.end(); iter++) {
      RadxSweep *sweep = iter->second;
      _sweeps.push_back(sweep);
    }

  } else {

    // load up sorted array vector with elevation angles in ascending order
    
    vector<RadxRay *> sortedRays;
    multimap< double, RadxSweep* >::iterator iter;
    for (iter = sortedSweeps.begin(); iter != sortedSweeps.end(); iter++) {
      RadxSweep *sweep = iter->second;
      for (size_t iray = sweep->getStartRayIndex();
           iray <= sweep->getEndRayIndex(); iray++) {
        RadxRay *ray = _rays[iray];
        sortedRays.push_back(ray);
      }
    }
    
    // copy sorted rays to main array
    
    _rays = sortedRays;
    checkRayTimesIncrease();
    
    // load up the sweep information from the rays
    
    loadSweepInfoFromRays();
    loadVolumeInfoFromRays();

  }

}

////////////////////////////////////////////////////////////
// Reorder the sweeps as in file into ascending angle order

void RadxVol::reorderSweepsAsInFileAscendingAngle()
{

  if (_sweepsAsInFile.size() < 1) {
    return;
  }

  // check sweeps are in ascending order, do nothing if they are

  bool ascending = true;
  for (size_t ii = 1; ii < _sweepsAsInFile.size(); ii++) {
    RadxSweep *sweep0 = _sweepsAsInFile[ii-1];
    RadxSweep *sweep1 = _sweepsAsInFile[ii];
    if (sweep0->getFixedAngleDeg() > sweep1->getFixedAngleDeg()) {
      ascending = false;
      break;
    }
  }
  if (ascending) {
    return;
  }
  
  // create a map of sweeps with angles in ascending order

  multimap< double, RadxSweep* > sortedSweeps;
  for (size_t ii = 0; ii < _sweepsAsInFile.size(); ii++) {
    RadxSweep *sweep = _sweepsAsInFile[ii];
    double angle = sweep->getFixedAngleDeg();
    sortedSweeps.insert(pair< double, RadxSweep* >(angle, sweep));
  }

  _sweepsAsInFile.clear();
  multimap< double, RadxSweep* >::iterator iter;
  for (iter = sortedSweeps.begin(); iter != sortedSweeps.end(); iter++) {
    RadxSweep *sweep = iter->second;
    _sweepsAsInFile.push_back(sweep);
  }

}

//////////////////////////////////////////////////////////////////
/// Apply an azimuth offset to all rays in the volume
/// This applies to the rays currently in the volume, not to
/// any future reads

void RadxVol::applyAzimuthOffset(double offset)

{

  bool isRhi = checkIsRhi();
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay *ray = _rays[ii];
    double az = ray->getAzimuthDeg() + offset;
    while (az > 360.0) {
      az -= 360.0;
    }
    while (az < 0) {
      az += 360.0;
    }
    ray->setAzimuthDeg(az);
    if (isRhi) {
      double fixedAng = ray->getFixedAngleDeg() + offset;
      while (fixedAng > 360.0) {
        fixedAng -= 360.0;
      }
      while (fixedAng < 0) {
        fixedAng += 360.0;
      }
      ray->setFixedAngleDeg(fixedAng);
    }
  } // ii

  // reload sweep info, since sweep numbers on rays have changed
  
  loadSweepInfoFromRays();

  // update history

  time_t now = time(NULL);
  char note[1024];
  sprintf(note, "Applying azimuth offset: %g, time %s\n",
          offset, RadxTime::strm(now).c_str());
  _history += note;

}

//////////////////////////////////////////////////////////////////
/// Apply an elevation offset to all rays in the volume
/// This applies to the rays currently in the volume, not to
/// any future reads

void RadxVol::applyElevationOffset(double offset)

{

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay *ray = _rays[ii];
    double el = ray->getElevationDeg() + offset;
    while (el > 180.0) {
      el -= 360.0;
    }
    while (el < -180) {
      el += 360.0;
    }
    ray->setElevationDeg(el);
  } // ii

  // update history

  time_t now = time(NULL);
  char note[1024];
  sprintf(note, "Applying elevation offset: %g, time %s\n",
          offset, RadxTime::strm(now).c_str());
  _history += note;

}

////////////////////////////////////////////  
/// Set the fixed angle for a sweep
/// Also sets the fixed angle for the rays in the sweep

void RadxVol::setFixedAngleDeg(int sweepNum, double fixedAngle)

{

  // check sweep num is valid
  
  assert(sweepNum < (int) _sweeps.size());
  
  // get sweep

  RadxSweep *sweep = _sweeps[sweepNum];

  // set the fixed angle on the rays

  for (size_t ii = sweep->getStartRayIndex();
       ii <= sweep->getEndRayIndex(); ii++) {
    _rays[ii]->setFixedAngleDeg(fixedAngle);
  }

  // set the fixed angle on the sweep

  sweep->setFixedAngleDeg(fixedAngle);

}

//////////////////////////////////////////////////////////  
/// Compute the fixed angle from the rays
/// Also sets the fixed angle on rays and sweeps
/// Uses the mean pointing angle to estimate the fixed angle

void RadxVol::computeFixedAngleFromRays()

{

  // load sweep info if needed
  
  if (_sweeps.size() < 1) {
    loadSweepInfoFromRays();
  }

  // loop through sweeps

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    
    RadxSweep *sweep = _sweeps[isweep];
    Radx::SweepMode_t sweepMode = sweep->getSweepMode();
    
    // sum up (x,y) coords of measured angles

    double sumx = 0.0;
    double sumy = 0.0;
    
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      
      const RadxRay *ray = _rays[iray];
      double angle;
      if (sweepMode == Radx::SWEEP_MODE_RHI ||
          sweepMode == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
        angle = ray->getAzimuthDeg();
      } else {
        angle = ray->getElevationDeg();
      }
      
      double sinVal, cosVal;
      Radx::sincos(angle * Radx::DegToRad, sinVal, cosVal);
      sumy += sinVal;
      sumx += cosVal;
      
    } // iray

    // compute mean angle, to use as fixed angle

    double meanAngleDeg = atan2(sumy, sumx) * Radx::RadToDeg;
    sweep->setFixedAngleDeg(meanAngleDeg);

    // set on rays in this sweep

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RadxRay *ray = _rays[iray];
      ray->setFixedAngleDeg(meanAngleDeg);
    }
    
  } // isweep

}

//////////////////////////////////////////////////////////  
/// Set the sweep scan modes from ray angles
///
/// Deduce the antenna scan mode from the ray angles in each
/// sweep, and set the scan mode on the rays and sweep objects.

void RadxVol::setSweepScanModeFromRayAngles()

{

  // load sweep info if needed
  
  if (_sweeps.size() < 1) {
    loadSweepInfoFromRays();
  }

  // loop through sweeps

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    
    RadxSweep *sweep = _sweeps[isweep];
    Radx::SweepMode_t sweepMode = sweep->getSweepMode();
    size_t startRayIndex = sweep->getStartRayIndex();
    size_t endRayIndex = sweep->getEndRayIndex();
    
    if (checkIsRhi(startRayIndex, endRayIndex)) {
      sweepMode = Radx::SWEEP_MODE_RHI;
    } else if (checkIsSurveillance(startRayIndex, endRayIndex)) {
      sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    } else {
      sweepMode = Radx::SWEEP_MODE_SECTOR;
    }
    
    for (size_t iray = startRayIndex; iray <= endRayIndex; iray++) {
      RadxRay *ray = _rays[iray];
      ray->setSweepMode(sweepMode);
    }
    sweep->setSweepMode(sweepMode);

  } // isweep

}

//////////////////////////////////////////////////////////////////
/// Set the volume number.
/// This increments with every volume, and may wrap.

void RadxVol::setVolumeNumber(int volNum)

{

  _volNum = volNum; 

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    _sweeps[isweep]->setVolumeNumber(_volNum);
  }

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setVolumeNumber(_volNum);
  }

}
  
//////////////////////////////////////////////////////////////////
/// Load volume information from the rays.
///
/// This sets the volume number and the start and end times.

void RadxVol::loadVolumeInfoFromRays()
  
{
  
  // check we have data
  
  if (_rays.size() < 1) {
    return;
  }
  
  // set vol number from first ray

  _volNum = _rays[_rays.size()/2]->getVolumeNumber();

  // set start and end times

  double startTime = 1.0e99;
  double endTime = -1.0e99;

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    const RadxRay *ray = _rays[ii];
    double dtime = ray->getTimeDouble();
    if (dtime < startTime) {
      startTime = dtime;
      _startTimeSecs = ray->getTimeSecs();
      _startNanoSecs = ray->getNanoSecs();
    }
    if (dtime > endTime) {
      endTime = dtime;
      _endTimeSecs = ray->getTimeSecs();
      _endNanoSecs = ray->getNanoSecs();
    }
  }

  // check that ray times increase
  
  checkRayTimesIncrease();

  // set geom from predominant

  RayGeom predom = _getPredomGeom();
  setRangeGeom(predom.startRange, predom.gateSpacing);                                     

}

///////////////////////////////////////////////////////////////
// check that the ray times increase, set flag accordingly

void RadxVol::checkRayTimesIncrease()

{

  _rayTimesIncrease = true;

  if (_rays.size() < 1) {
    return;
  }

  double prevTime = _rays[0]->getTimeDouble();

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    const RadxRay *ray = _rays[ii];
    double rayTime = ray->getTimeDouble();
    if (rayTime < prevTime) {
      _rayTimesIncrease = false;
      return;
    }
    prevTime = rayTime;
  }

}

///////////////////////////////////////////////////////////////
/// Check through the rays, and increment the sweep number
/// if the polarization mode changes in the middle of a sweep

void RadxVol::incrementSweepOnPolModeChange()

{

  if (_rays.size() < 2) {
    return;
  }

  int nIncr = 0;

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    
    Radx::PolarizationMode_t prevPolMode = _rays[ii-1]->getPolarizationMode();
    Radx::PolarizationMode_t polMode = _rays[ii]->getPolarizationMode();

    int prevSweepNum = _rays[ii-1]->getSweepNumber();
    int sweepNum = _rays[ii]->getSweepNumber();

    if ((polMode != prevPolMode) && (sweepNum == prevSweepNum)) {
      nIncr++;
    }

    if (nIncr > 0) {
      _rays[ii]->setSweepNumber(_rays[ii]->getSweepNumber() + nIncr);
    }

  } // ii

}
  
///////////////////////////////////////////////////////////////
/// Check through the rays, and increment the sweep number
/// if the prt mode changes in the middle of a sweep

void RadxVol::incrementSweepOnPrtModeChange()

{

  if (_rays.size() < 2) {
    return;
  }

  int nIncr = 0;

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    
    Radx::PrtMode_t prevPrtMode = _rays[ii-1]->getPrtMode();
    Radx::PrtMode_t prtMode = _rays[ii]->getPrtMode();

    int prevSweepNum = _rays[ii-1]->getSweepNumber();
    int sweepNum = _rays[ii]->getSweepNumber();

    if ((prtMode != prevPrtMode) && (sweepNum == prevSweepNum)) {
      nIncr++;
    }

    if (nIncr > 0) {
      _rays[ii]->setSweepNumber(_rays[ii]->getSweepNumber() + nIncr);
    }

  } // ii

}
  
///////////////////////////////////////////////////////////////
/// Load the sweep information from the rays.
///
/// This loops through all of the rays, and determines the sweep
/// information from them. The resulting information is stored
/// in the sweeps array on the volume.
///
/// Also sets the start/end of sweep/volume flags

void RadxVol::loadSweepInfoFromRays()
  
{

  clearSweeps();
  int prevSweepNum = -9999;
  RadxSweep *sweep = NULL;
  int rayIndex = 0;

  // do we need to fill in sweep numbers?

  RadxAngleHist hist;
  if (hist.checkSweepsNumbersAllMissing(_rays)) {
    hist.fillInSweepNumbers(_rays);
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    
    RadxRay &ray = *_rays[ii];
    int sweepNum = ray.getSweepNumber();
    rayIndex = ii;

    if (ii == 0) {
      ray.setStartOfSweepFlag(true);
      ray.setStartOfVolumeFlag(true);
    } else if (ii == _rays.size() - 1) {
      ray.setEndOfSweepFlag(true);
      ray.setEndOfVolumeFlag(true);
    }

    if (sweepNum != prevSweepNum) {

      ray.setStartOfSweepFlag(true);
      if (ii > 0) {
        _rays[ii-1]->setEndOfSweepFlag(true);
      }

      sweep = new RadxSweep();

      sweep->setVolumeNumber(ray.getVolumeNumber());
      sweep->setSweepNumber(sweepNum);

      sweep->setStartRayIndex(rayIndex);
      sweep->setEndRayIndex(rayIndex);

      sweep->setSweepMode(ray.getSweepMode());
      sweep->setPolarizationMode(ray.getPolarizationMode());
      sweep->setPrtMode(ray.getPrtMode());
      sweep->setFollowMode(ray.getFollowMode());
      
      sweep->setFixedAngleDeg(ray.getFixedAngleDeg());
      sweep->setTargetScanRateDegPerSec(ray.getTargetScanRateDegPerSec());
      
      sweep->setRaysAreIndexed(ray.getIsIndexed());
      sweep->setAngleResDeg(ray.getAngleResDeg());

      _sweeps.push_back(sweep);
      
      prevSweepNum = sweepNum;
      
    } else {
      
      if (sweep) {
        sweep->setEndRayIndex(rayIndex);
      }

    }

  }

  // last sweep ends at last ray
  
  if (_sweeps.size() > 0) {
    _sweeps[_sweeps.size()-1]->setEndRayIndex(rayIndex);
  }

  // set sweep flags using median value for rays in sweep
  // also check long range

  vector<Radx::SweepMode_t> sweepModes;
  vector<Radx::PolarizationMode_t> polModes;
  vector<Radx::PrtMode_t> prtModes;
  vector<Radx::FollowMode_t> followModes;
  vector<double> fixedAngles;
  vector<double> scanRates;
  vector<bool> raysAreIndexed;
  vector<double> angleRes;
  vector<bool> isLongRange;
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    // find sweep

    RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();
    if (startIndex == endIndex) {
      continue;
    }

    // initialize sweep counters and vectors

    sweepModes.clear();
    polModes.clear();
    prtModes.clear();
    followModes.clear();
    fixedAngles.clear();
    scanRates.clear();
    raysAreIndexed.clear();
    angleRes.clear();
    isLongRange.clear();

    // accum data for sweep

    for (size_t iray = startIndex; iray <= endIndex; iray++) {
      const RadxRay &ray = *_rays[iray];
      sweepModes.push_back(ray.getSweepMode());
      polModes.push_back(ray.getPolarizationMode());
      prtModes.push_back(ray.getPrtMode());
      followModes.push_back(ray.getFollowMode());
      fixedAngles.push_back(ray.getFixedAngleDeg());
      scanRates.push_back(ray.getTargetScanRateDegPerSec());
      raysAreIndexed.push_back(ray.getIsIndexed());
      angleRes.push_back(ray.getAngleResDeg());
      isLongRange.push_back(ray.getIsLongRange());
    } // iray

    // sort vectors to prepare for median
    
    sort(sweepModes.begin(), sweepModes.end());
    sort(polModes.begin(), polModes.end());
    sort(prtModes.begin(), prtModes.end());
    sort(followModes.begin(), followModes.end());
    sort(fixedAngles.begin(), fixedAngles.end());
    sort(scanRates.begin(), scanRates.end());
    sort(raysAreIndexed.begin(), raysAreIndexed.end());
    sort(angleRes.begin(), angleRes.end());
    sort(isLongRange.begin(), isLongRange.end());

    // set sweep info

    int nRaysSweep = endIndex - startIndex + 1;
    int nRaysHalf = nRaysSweep / 2;
    
    sweep->setSweepMode(sweepModes[nRaysHalf]);
    sweep->setPolarizationMode(polModes[nRaysHalf]);
    sweep->setPrtMode(prtModes[nRaysHalf]);
    sweep->setFollowMode(followModes[nRaysHalf]);
    sweep->setFixedAngleDeg(fixedAngles[nRaysHalf]);
    sweep->setTargetScanRateDegPerSec(scanRates[nRaysHalf]);
    sweep->setRaysAreIndexed(raysAreIndexed[nRaysHalf]);
    sweep->setAngleResDeg(angleRes[nRaysHalf]);
    sweep->setIsLongRange(isLongRange[nRaysHalf]);

  } // isweep

  // copy into sweep array as in file, if not previously done
  
  if (_sweepsAsInFile.size() < _sweeps.size()) {
    clearSweepsAsInFile();
    for (size_t ii = 0; ii < _sweeps.size(); ii++) {
      addSweepAsInFile(_sweeps[ii]);
    }
  }

}

///////////////////////////////////////////////////////////////
/// Adjust the limits of sweeps, by comparing the measured angles
/// to the fixed angles.
///
/// Sometimes the transitions from one fixed angle to another are
/// not accurately described by the scan flags, and as a result rays
/// are not correctly assigned to the sweeps.
///
/// This routine goes through the volume in ray order, and adjusts
/// the way rays are associated with each sweep. It does this by
/// comparing the actual angle with the fixed angle, and minimizes
/// the angular difference.
///
/// Before calling this routine you need to ensure that the fixed
/// sweep angle and measured ray angle has been set on the rays.

void RadxVol::adjustSweepLimitsUsingAngles()
  
{

  // load up the sweep info from the rays
  
  loadSweepInfoFromRays();
  
  if (_sweeps.size() < 2) {
    // no action required
    return;
  }

  if (checkIsRhi()) {
    _adjustSweepLimitsRhi();
  } else {
    _adjustSweepLimitsPpi();
  }

}

void RadxVol::_adjustSweepLimitsPpi()

{
    
  // loop through sweep pairs
  
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];

    // get fixed angles, and compute the change between them
    
    double fixedAngleThis = sweepThis->getFixedAngleDeg();
    double fixedAngleNext = sweepNext->getFixedAngleDeg();
    double delta = fabs(fixedAngleNext - fixedAngleThis);
    if (delta > 180.0) {
      delta = fabs(delta - 360.0);  // correct for north crossing in PPI
    }
    if (delta < 0.01) {
      continue; // fixed angles essentially the same
    }
    double halfDelta = delta / 2.0;

    // find transition location
    
    size_t iray0 = sweepThis->getStartRayIndex();
    size_t iray1 = sweepThis->getEndRayIndex();
    size_t iray2 = sweepNext->getStartRayIndex();
    size_t iray3 = sweepNext->getEndRayIndex();
    
    size_t irayTrans = 0;
    for (size_t iray = iray0; iray < iray3; iray++) {
      RadxRay *ray = _rays[iray];
      double fixedAngle = ray->getElevationDeg();
      double diff = fabs(fixedAngleNext - fixedAngle);
      if (diff > 180.0) {
        diff = fabs(diff - 360.0); // correct for north crossing
      }
      if (diff < halfDelta) {
        // have moved to next sweep
        irayTrans = iray;
        break;
      }
    }
    
    if (irayTrans == 0) {
      // no transition found, do nothing
      continue;
    }
    
    // adjust sweep info to match transition
    
    if (irayTrans <= iray1) {
      for (size_t iray = irayTrans; iray <= iray1; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepNext->getSweepNumber());
        ray->setFixedAngleDeg(sweepNext->getFixedAngleDeg());
      }
    } else if (irayTrans >= iray2) {
      for (size_t iray = iray2; iray < irayTrans; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepThis->getSweepNumber());
        ray->setFixedAngleDeg(sweepThis->getFixedAngleDeg());
      }
    }
    
    // adjust start/end indices
    
    sweepThis->setEndRayIndex(irayTrans - 1);
    sweepNext->setStartRayIndex(irayTrans);
    
  } // isweep

}

void RadxVol::_adjustSweepLimitsRhi()

{
    
  // loop through sweep pairs
  
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];

    // check motion direction
    
    size_t iray0 = sweepThis->getStartRayIndex();
    size_t iray1 = sweepThis->getEndRayIndex();
    size_t iray2 = sweepNext->getStartRayIndex();
    size_t iray3 = sweepNext->getEndRayIndex();

    RadxRay *ray0 = _rays[iray0];
    RadxRay *ray1 = _rays[iray1];
    RadxRay *ray2 = _rays[iray2];
    RadxRay *ray3 = _rays[iray3];
    
    double elev0 = ray0->getElevationDeg();
    double elev1 = ray1->getElevationDeg();
    double elev2 = ray2->getElevationDeg();
    double elev3 = ray3->getElevationDeg();

    double deltaElevThis = elev1 - elev0;
    double deltaElevNext = elev3 - elev2;

    double goingUpThis = true;
    if (deltaElevThis < 0.0 && deltaElevNext > 0) {
      goingUpThis = false;
    }

    // find transition location
    // find min or max elevation depending on whether this sweep is going up or down
    // we will use this inflection point as the transition to the next sweep

    size_t irayTrans = 0;
    
    if (goingUpThis) {
      
      double maxElev = -999.0;
      for (size_t iray = iray0; iray < iray3; iray++) {
        RadxRay *ray = _rays[iray];
        double elev = ray->getElevationDeg();
        if (elev > maxElev) {
          irayTrans = iray;
          maxElev = elev;
        }
      }

    } else {

      double minElev = 999.0;
      for (size_t iray = iray0; iray < iray3; iray++) {
        RadxRay *ray = _rays[iray];
        double elev = ray->getElevationDeg();
        if (elev < minElev) {
          irayTrans = iray;
          minElev = elev;
        }
      }

    }

    if (irayTrans == 0) {
      // no transition found, do nothing
      continue;
    }
    
    // adjust sweep info to match transition
    
    if (irayTrans <= iray1) {
      for (size_t iray = irayTrans; iray <= iray1; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepNext->getSweepNumber());
        ray->setFixedAngleDeg(sweepNext->getFixedAngleDeg());
      }
    } else if (irayTrans >= iray2) {
      for (size_t iray = iray2; iray < irayTrans; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepThis->getSweepNumber());
        ray->setFixedAngleDeg(sweepThis->getFixedAngleDeg());
      }
    }
    
    // adjust start/end indices
    
    sweepThis->setEndRayIndex(irayTrans - 1);
    sweepNext->setStartRayIndex(irayTrans);
    
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Adjust surveillance sweep limits based on azimuth.
///
/// Some radars (e.g. DOWs) always change elevation angle at a
/// fixed theorerical azimuth.
/// 
/// This function sets the transitions between sweeps based on a
/// designated azimuth.

void RadxVol::adjustSurSweepLimitsToFixedAzimuth(double azimuth)
  
{

  // load up the sweep info from the rays
  
  loadSweepInfoFromRays();
  
  if (_sweeps.size() < 2) {
    // no action required
    return;
  }

  if (checkIsRhi()) {
    // not applicable in RHI mode
    return;
  }
  
  if (!checkIsSurveillance()) {
    // not applicable unless in surveillance mode
    return;
  }

  // loop through sweep pairs, looking for the transitions across
  // the specified azimuth

  vector<int> azTransIndex;
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];

    // check for surveillance

    if (sweepThis->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE ||
        sweepNext->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      return;
    }

    // get point for each sweep where it crosses the azimuth
    
    int azIndexThis = _getTransIndex(sweepThis, azimuth);
    int azIndexNext = _getTransIndex(sweepNext, azimuth);

    if (azIndexThis < 0 && azIndexNext < 0) {
      // no transition found
      azTransIndex.push_back(-1);
      continue;
    }

    if (azIndexThis < 0 && azIndexNext >= 0) {
      // no azimuth transition this sweep, but got one for next sweep
      int nFwd = azIndexNext - sweepNext->getStartRayIndex();
      if (nFwd < (int) sweepNext->getNRays() / 2) {
        // use this transition
        azTransIndex.push_back(azIndexNext);
      } else {
        azTransIndex.push_back(-1);
      }
      continue;
    }

    if (azIndexThis >= 0 && azIndexNext < 0) {
      // no azimuth transition next sweep, but got one for this sweep
      int nBack = sweepThis->getEndRayIndex() - azIndexThis;
      if (nBack < (int) sweepThis->getNRays() / 2) {
        // use this transition
        azTransIndex.push_back(azIndexThis);
      } else {
        azTransIndex.push_back(-1);
      }
      continue;
    }

    int nBack = sweepThis->getEndRayIndex() - azIndexThis;
    int nFwd = azIndexNext - sweepNext->getStartRayIndex();
    if (nBack < nFwd) {
      azTransIndex.push_back(azIndexThis);
    } else {
      azTransIndex.push_back(azIndexNext);
    }
    
  } // isweep
    
  // loop through sweep pairs, adjusting the transitions
  
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {

    if (azTransIndex[isweep] < 0) {
      // do not adjust this one
      continue;
    }
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];
    int transIndex = azTransIndex[isweep];

    sweepThis->setEndRayIndex(transIndex);
    sweepNext->setStartRayIndex(transIndex + 1);

  } // isweep

  // set the ray metadata to match
 
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    RadxSweep *sweep = _sweeps[isweep];
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RadxRay &ray = *_rays[iray];
      ray.setSweepNumber(sweep->getSweepNumber());
      ray.setFixedAngleDeg(sweep->getFixedAngleDeg());
    } // iray
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Compute sweep fixed angles from ray data
///
/// Normally the sweep angles are set using the scan strategy angles -
/// i.e., the theoretically perfect angles. This option allows you to
/// recompute the sweep angles using the measured elevation angles (in
/// PPI mode) or azimuth angles (in RHI mode).

void RadxVol::computeSweepFixedAnglesFromRays()
  
{

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();

    // sweep mode

    bool isRhi = checkIsRhi();

    // compute median angle

    vector<double> angles;
    for (size_t iray = startIndex; iray <= endIndex; iray++) {
      const RadxRay &ray = *_rays[iray];
      if (isRhi) {
        angles.push_back(ray.getAzimuthDeg());
      } else {
        angles.push_back(ray.getElevationDeg());
      }
    } // iray
    if (angles.size() > 2) {
      sort(angles.begin(), angles.end());
      double medianAngle = angles[angles.size() / 2];
      sweep->setFixedAngleDeg(medianAngle);
      for (size_t iray = startIndex; iray <= endIndex; iray++) {
        RadxRay &ray = *_rays[iray];
        ray.setFixedAngleDeg(medianAngle);
      } // iray
    }
    
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Get fraction of sweep with transition rays.
/// Returns the fraction.

double RadxVol::computeSweepFractionInTransition(int sweepIndex)
  
{
  
  if (sweepIndex < 0 || sweepIndex > (int) _sweeps.size()) {
    return 0.0;
  }

  RadxSweep *sweep = _sweeps[sweepIndex];
  size_t startIndex = sweep->getStartRayIndex();
  size_t endIndex = sweep->getEndRayIndex();

  int nTrans = 0;
  int nTotal = 0;

  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    const RadxRay *ray = _rays[iray];
    if (ray->getAntennaTransition()) {
      nTrans++;
    }
    nTotal++;
  }

  double fraction = (double) nTrans / (double) nTotal;
  return fraction;

}

////////////////////////////////////////////////////////////
// Constrain the data by specifying fixedAngle limits
//
// This operation will remove unwanted rays from the
// data set, remap the field arrays for the remaining
// rays and set the field pointers in the rays to
// the remapped fields.
///
/// If strictChecking is TRUE, we only get rays within the specified limits.
/// If strictChecking is FALSE, we are guaranteed to get at least 1 sweep.
///
/// Returns 0 on success, -1 on failure

int RadxVol::constrainByFixedAngle(double minFixedAngleDeg,
                                   double maxFixedAngleDeg,
                                   bool strictChecking /* = false */)
{

  // find sweep indexes which lie within the fixedAngle limits

  vector<int> sweepIndexes;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    double angle = _sweeps[ii]->getFixedAngleDeg();
    if (angle > (minFixedAngleDeg - 0.01) &&
        angle < (maxFixedAngleDeg + 0.01)) {
      sweepIndexes.push_back(ii);
    }
  }

  // make sure we have at least one index

  if (sweepIndexes.size() == 0) {
    if (strictChecking) {
      // require at least 1 sweep within limits
      return -1;
    }
    double minDiff = 1.0e99;
    double meanAngle = (minFixedAngleDeg + maxFixedAngleDeg) / 2.0;
    if (maxFixedAngleDeg - minFixedAngleDeg < 0) {
      meanAngle -= 180.0;
    }
    if (meanAngle < 0) {
      meanAngle += 360.0;
    }
    int index = 0;
    for (size_t ii = 0; ii < _sweeps.size(); ii++) {
      double angle = _sweeps[ii]->getFixedAngleDeg();
      double diff = fabs(angle - meanAngle);
      if (diff < minDiff) {
        minDiff = diff;
        index = ii;
      }
    }
    sweepIndexes.push_back(index);
  }

  // constrain based on sweep indexes

  _constrainBySweepIndex(sweepIndexes);

  return 0;

}

////////////////////////////////////////////////////////////
// constrain the data by specifying sweep number limits.
//
// This operation will remove unwanted rays from the
// data set, remap the field arrays for the remaining
// rays and set the field pointers in the rays to
// the remapped fields.
///
/// If strictChecking is TRUE, we only get rays within the specified limits.
/// If strictChecking is FALSE, we are guaranteed to get at least 1 sweep.
///
/// Returns 0 on success, -1 on failure

int RadxVol::constrainBySweepNum(int minSweepNum,
                                 int maxSweepNum,
                                 bool strictChecking /* = false */)

{

  // find sweep indexes which lie within the sweep number limits

  vector<int> sweepIndexes;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    int num = _sweeps[ii]->getSweepNumber();
    if (num >= minSweepNum && num <= maxSweepNum) {
      sweepIndexes.push_back(ii);
    }
  }

  // make sure we have at least one index

  if (sweepIndexes.size() == 0) {
    if (strictChecking) {
      // require at least 1 sweep within limits
      return -1;
    }
    double minDiff = 1.0e99;
    double meanSweepNum = (minSweepNum + maxSweepNum) / 2.0;
    int index = 0;
    for (size_t ii = 0; ii < _sweeps.size(); ii++) {
      double num = _sweeps[ii]->getSweepNumber();
      double diff = fabs(num - meanSweepNum);
      if (diff < minDiff) {
        minDiff = diff;
        index = ii;
      }
    }
    sweepIndexes.push_back(index);
  }

  // constrain based on sweep indexes

  _constrainBySweepIndex(sweepIndexes);

  return 0;

}

/////////////////////////////////////////////////////////////////
// Ensure ray times are monotonically increasing by
// interpolating the times if there are duplicates

void RadxVol::interpRayTimes()

{

  if (_rays.size() < 3) {
    return;
  }
  
  time_t prevSecs =  _rays[0]->getTimeSecs();
  int prevNano =  (int) (_rays[0]->getNanoSecs() + 0.5);
  size_t prevIndex = 0;

  for (size_t iray = 1; iray < _rays.size(); iray++) {
    
    RadxRay &ray = *_rays[iray];
    time_t secs =  ray.getTimeSecs();
    int nano =  (int) (ray.getNanoSecs() + 0.5);

    if (secs != prevSecs || nano != prevNano) {

      int nRaysSame = iray - prevIndex;
      
      if (nRaysSame > 0) {
        // interpolate
        double prevTime = (double) prevSecs + prevNano / 1.0e9;
        double thisTime = (double) secs + nano / 1.0e9;
        double delta = (thisTime - prevTime) / (double) nRaysSame;
        for (size_t jj = prevIndex; jj < iray; jj++) {
          RadxRay &jray = *_rays[jj];
          jray.setTime(jray.getTimeDouble() + delta * (jj - prevIndex));
        }
      }

      prevSecs = secs;
      prevNano = nano;
      prevIndex = iray;
      
    } // if (secs != prevSecs || nano != prevNano)

  } // iray
    
}

/////////////////////////////////////////////////////////////////
// Sort rays by time

bool RadxVol::SortByRayTime::operator()
  (const RayPtr &lhs, const RayPtr &rhs) const
{
  return lhs.ptr->getRadxTime() < rhs.ptr->getRadxTime();
}

void RadxVol::sortRaysByTime()

{

  // sanity check

  if (_rays.size() < 2) {
    return;
  }

  // create set with sorted ray pointers

  set<RayPtr, SortByRayTime> sortedRayPtrs;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RayPtr rptr(_rays[iray]);
    sortedRayPtrs.insert(rptr);
  }

  // reload _rays array in time-sorted order

  _rays.clear();
  for (set<RayPtr, SortByRayTime>::iterator ii = sortedRayPtrs.begin();
       ii != sortedRayPtrs.end(); ii++) {
    _rays.push_back(ii->ptr);
  }
    
}

////////////////////////////////////////////////////////////
/// set or add frequency or wavelength
/// The set methods clear the list first, and then add the value
/// The add methods do not clear the list first

void RadxVol::setFrequencyHz(double val)
{
  _platform.setFrequencyHz(val);
}

void RadxVol::setWavelengthM(double val)
{
  _platform.setWavelengthM(val);
}

void RadxVol::setWavelengthCm(double val)
{
  _platform.setWavelengthCm(val);
}

void RadxVol::addFrequencyHz(double val)
{
  _platform.addFrequencyHz(val);
}

void RadxVol::addWavelengthM(double val)
{
  _platform.addWavelengthM(val);
}

void RadxVol::addWavelengthCm(double val)
{
  _platform.addWavelengthCm(val);
}

////////////////////////////////////////////////////////////
// get wavelength

double RadxVol::getWavelengthM() const
{
  return _platform.getWavelengthM();
}

double RadxVol::getWavelengthCm() const
{
  return _platform.getWavelengthCm();
}

////////////////////////////////////////////////////////////
// get sweep by sweep number (not the index)
// returns NULL on failure

const RadxSweep *RadxVol::getSweepByNumber(int sweepNum) const

{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii]->getSweepNumber() == sweepNum) {
      return _sweeps[ii];
    }
  } // ii
  return NULL;
}

RadxSweep *RadxVol::getSweepByNumber(int sweepNum)
{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii]->getSweepNumber() == sweepNum) {
      return _sweeps[ii];
    }
  } // ii
  return NULL;
}

//////////////////////////////////////////////////////////////////////////
/// check if all rays in a sweep are in an antenna transition

bool RadxVol::checkAllSweepRaysInTransition(const RadxSweep *sweep) const

{
  int startIndex = sweep->getStartRayIndex();
  int endIndex = sweep->getEndRayIndex();
  for (int ii = startIndex; ii <= endIndex; ii++) {
    const RadxRay &ray = *_rays[ii];
    if (!ray.getAntennaTransition()) {
      return false;
    }
  }
  return true;
}
 
bool RadxVol::checkAllSweepRaysInTransition(int sweepNum) const

{
  if (sweepNum < 0 || sweepNum > (int) _sweeps.size() - 1) {
    return false;
  }
  return checkAllSweepRaysInTransition(_sweeps[sweepNum]);
}

///////////////////////////////////////////////////////
// internal implementation of sweep constraint

void RadxVol::_constrainBySweepIndex(vector<int> &sweepIndexes)

{

  if (sweepIndexes.size() < 1) {
    return;
  }

  // ensure we always return 1

  if (_sweeps.size() < 2) {
    return;
  }
  
  // sort the vector
  
  if (sweepIndexes.size() > 1) {
    sort(sweepIndexes.begin(), sweepIndexes.end());
  }

  // get min and max sweep indexes

  int minSweepIndex = sweepIndexes[0];
  int maxSweepIndex = sweepIndexes[sweepIndexes.size()-1];

  if (minSweepIndex == 0 &&
      maxSweepIndex == (int) (_sweeps.size() - 1)) {
    // nothing to do - all sweeps are needed
    return;
  }

  // get min and max ray indexes

  size_t minRayIndex = _sweeps[minSweepIndex]->getStartRayIndex();
  size_t maxRayIndex = _sweeps[maxSweepIndex]->getEndRayIndex();

  // remap field data

  //   for (int ii = 0; ii < (int) _fields.size(); ii++) {
  //     _fields[ii]->remapRays(minRayIndex, maxRayIndex);
  //   }
  
  // remap ray vector

  vector<RadxRay *> goodRays;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (ii >= minRayIndex && ii <= maxRayIndex) {
      goodRays.push_back(_rays[ii]);
    } else {
      RadxRay::deleteIfUnused(_rays[ii]);
    }
  }
  _rays = goodRays;

  // load up sweep info from the revised list of rays

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

}

///////////////////////////////////////////////////////
/// remove rays with all missing data

void RadxVol::removeRaysWithDataAllMissing()

{

  vector<RadxRay *> goodRays;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_rays[ii]->checkDataAllMissing()) {
      RadxRay::deleteIfUnused(_rays[ii]);
    } else {
      goodRays.push_back(_rays[ii]);
    }
  }
  _rays = goodRays;

  // load up sweep info from the revised list of rays

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

}

////////////////////////////////////////////////////////////
// Determine whether rays are indexed in angle, and what
// the predominant angular resolution is.
//
// This is performed by sweep.

bool RadxVol::checkForIndexedRays() const

{
  bool allIndexed = true;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    _checkForIndexedRays(_sweeps[ii]);
    if (!_sweeps[ii]->getRaysAreIndexed()) {
      allIndexed = false;
    }
  } // ii
  return allIndexed;
}

void RadxVol::_checkForIndexedRays(const RadxSweep *sweep) const

{

  // get sweep mode

  Radx::SweepMode_t mode = sweep->getSweepMode();
  bool isRhi = false;
  if (mode == Radx::SWEEP_MODE_RHI ||
      mode == Radx::SWEEP_MODE_MANUAL_RHI) {
    isRhi = true;
  }
  
  // compute histogram of angular differences
  // from 0 to 10 degrees, at 0.01 degree resolution

  double res = 0.005;
  int nn = 2000;
  int startIndex = sweep->getStartRayIndex();
  int endIndex = sweep->getEndRayIndex();
  int nRays = endIndex - startIndex + 1;

  int *hist = new int[nn];
  memset(hist, 0, nn * sizeof(int));
  int count = 0;

  for (int ii = startIndex; ii < endIndex; ii++) {
    const RadxRay &ray0 = *_rays[ii];
    const RadxRay &ray1 = *_rays[ii+1];
    double diff = 0;
    if (isRhi) {
      diff = fabs(ray1.getElevationDeg() - ray0.getElevationDeg());
    } else {
      diff = fabs(ray1.getAzimuthDeg() - ray0.getAzimuthDeg());
    }
    if (diff > 180) {
      diff = fabs(diff - 360);
    }
    int index = (int) (diff / res + 0.5);
    if (index >= 0 && index < nn) {
      hist[index]++;
      count++;
    }
  }

  // find index for mode

  int modeIndex = -1;
  int maxHist = 0;
  for (int ii = 0; ii < nn; ii++) {
    int histVal = hist[ii];
    if (histVal > maxHist) {
      modeIndex = ii;
      maxHist = histVal;
    }
  }
  
  // calculate the angular resolution of the mode

  double modeRes = modeIndex * res;
  
  // count up number of ray pairs with diffs within 0.05 degrees of
  // of the mode

  int modeCount = 0;
  for (int ii = modeIndex - 10; ii < modeIndex + 10; ii++) {
    if (ii >= 0 && ii < nn) {
      modeCount += hist[ii];
    }
  }

  // are more than 90% within 0.05 degrees of the mode

  double fraction = (double) modeCount / ((double) nRays - 1);
  bool isIndexed = false;
  if (fraction > 0.9) {
    isIndexed = true;
  }

  // set ray meta-data
  
  double roundedRes = _computeRoundedAngleRes(modeRes);

  for (int ii = startIndex; ii <= endIndex; ii++) {
    RadxRay &ray = *_rays[ii];
    ray.setIsIndexed(isIndexed);
    ray.setAngleResDeg(roundedRes);
  }

  // clean up

  delete[] hist;

}

/////////////////////////////////////////////////
// data type conversions

void RadxVol::convertToFl64()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl64();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToFl64();
    }
  }
}

void RadxVol::convertToFl32()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl32();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToFl32();
    }
  }
}

void RadxVol::convertToSi32()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi32();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi32();
    }
  }
}

void RadxVol::convertToSi32(double scale, double offset)
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi32(scale, offset);
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi32(scale, offset);
    }
  }
}

void RadxVol::convertToSi16()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi16();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi16();
    }
  }
}

void RadxVol::convertToSi16(double scale, double offset)
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi16(scale, offset);
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi16(scale, offset);
    }
  }
}

void RadxVol::convertToSi08()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi08();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi08();
    }
  }
}

void RadxVol::convertToSi08(double scale, double offset)
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi08(scale, offset);
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi08(scale, offset);
    }
  }
}

void RadxVol::convertToType(Radx::DataType_t targetType)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToType(targetType);
  }
  setRayFieldPointers();
}

////////////////////////////////////////////////////////////////
/// Apply a linear transformation to the data values in a field.
/// Transforms x to y as follows:
///   y = x * scale + offset
/// After operation, field type is unchanged.
/// Nothing is done if field does not exist.

void RadxVol::applyLinearTransform(const string &name,
                                   double scale, double offset)

{
  
  if (_fields.size() > 0) {

    // fields are contiguous - i.e. not in rays

    RadxField *field = getField(name);
    if (field) {
      field->applyLinearTransform(scale, offset);
    }
    
  } else {

    // fields are on rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->applyLinearTransform(name, scale, offset);
    }
    
  } // if (_fields.size() > 0) {

}

//////////////////////////////////////////////////////
/// Converts field type, and optionally changes the
/// names.
///
/// If the data type is an integer type, dynamic scaling
/// is used - i.e. the min and max value is computed and
/// the scale and offset are set to values which maximize the
/// dynamic range.
///
/// If targetType is Radx::ASIS, no conversion is performed.
///
/// If a string is empty, the value on the field will
/// be left unchanged.

void RadxVol::convertField(const string &name,
                           Radx::DataType_t type,
                           const string &newName,
                           const string &units,
                           const string &standardName,
                           const string &longName)

{

  if (_fields.size() > 0) {

    // fields are contiguous

    for (size_t ii = 0; ii < _fields.size(); ii++) {
      RadxField &field = *_fields[ii];
      if (field.getName() == name) {
        field.convert(type, newName, units, standardName, longName);
      }
    }
  
  } else {

    // fields are on rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertField(name, type, 
                              newName, units,
                              standardName, longName);
    }
    
  } // if (_fields.size() > 0) {

}

//////////////////////////////////////////////////////
/// Converts field type, and optionally changes the
/// names.
///
/// For integer types, the specified scale and offset
/// are used.
///
/// If targetType is Radx::ASIS, no conversion is performed.
///
/// If a string is empty, the value on the field will
/// be left unchanged.

void RadxVol::convertField(const string &name,
                           Radx::DataType_t type,
                           double scale,
                           double offset,
                           const string &newName,
                           const string &units,
                           const string &standardName,
                           const string &longName)

{

  if (_fields.size() > 0) {

    // fields are contiguous

    for (size_t ii = 0; ii < _fields.size(); ii++) {
      RadxField &field = *_fields[ii];
      if (field.getName() == name) {
        field.convert(type, scale, offset,
                      newName, units, standardName, longName);
      }
    }
  
  } else {

    // fields are on rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertField(name, type, scale, offset,
                              newName, units,
                              standardName, longName);
    }
    
  } // if (_fields.size() > 0) {

}

///////////////////////////////////////////////////////////////////
/// compute field stats for rays currently in the volume
///
/// Pass in a stats method type, and a vector of fields
///
/// The requested stats on computed for each field,
/// and on a point-by-point basis.
///
/// If the geometry is not constant, remap to the predominant geom.
///
/// Returns NULL if no rays are present in the volume.
/// Otherwise, returns ray containing results.

RadxRay *RadxVol::computeFieldStats(RadxField::StatsMethod_t method)

{

  // check we have some data

  if (_rays.size() == 0) {
    return NULL;
  }

  // remap rays to predominant geometry

  remapToPredomGeom();

  // find middle ray, copy the metadata

  size_t iMid = _rays.size() / 2;
  RadxRay *result = new RadxRay;
  result->copyMetaData(*_rays[iMid]);

  // compute and set the number of samples

  int nSamplesSum = 0;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    nSamplesSum += _rays[iray]->getNSamples();
  }
  result->setNSamples(nSamplesSum);
  
  // get the field name list, and loop through them
  
  vector<string> fieldNames = getUniqueFieldNameList();
  for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {

    // assemble vector of this field on the ray

    vector<const RadxField *> rayFields;
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxField *rayField = _rays[iray]->getField(fieldNames[ifield]);
      if (rayField != NULL) {
        rayFields.push_back(rayField);
      }
    }

    // compute the stats for this field
    // add field to ray

    RadxField *statsField = RadxField::computeStats(method, rayFields);
    if (statsField != NULL) {
      result->addField(statsField);
    }

  } // ifield

  // return resulting ray

  return result;
  
}

//////////////////////////////////////////////////////
// if possible, find a rounded angle resolution evenly
// divisible into 360.0

double RadxVol::_computeRoundedAngleRes(double res) const
{
  static double canonAngles[] =
    {
      0.05, 0.1, 0.12, 0.125, 0.15, 0.18, 0.2, 0.25, 0.3, 0.36,
      0.4, 0.45, 0.5, 0.6, 0.75, 0.8, 0.9, 1.0, 1.2, 1.25,
      1.5, 1.5, 1.8, 2.0, 2.5, 3.6, 4.0, 4.5, 5.0, 6.0
    };

  for (int ii = 0; ii < 30; ii++) {
    double ratio = res / canonAngles[ii];
    if (ratio > 0.9 && ratio < 1.1) {
      return canonAngles[ii];
    }
  }
  if (res < 0.05) {
    return 0.05;
  } else if (res > 6.0) {
    return 6.0;
  } else {
    return res;
  }
}

/////////////////////////////////////////////////////
// check whether volume is predominantly in RHI mode
//
// Returns true if RHI, false otherwise

bool RadxVol::checkIsRhi() const
{

  return RadxAngleHist::checkIsRhi(_rays);

#ifdef NOTNOW
  if (_sweeps.size() < 1) {
    return RadxAngleHist::checkIsRhi(_rays);
  }
  Radx::SweepMode_t sweepMode = _sweeps[0]->getSweepMode();
  if (sweepMode == Radx::SWEEP_MODE_RHI ||
      sweepMode == Radx::SWEEP_MODE_MANUAL_RHI ||
      sweepMode == Radx::SWEEP_MODE_SUNSCAN_RHI ||
      sweepMode == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    return true;
  } else {
    return false;
  }
#endif

}

//////////////////////////////////////////////////////////////
// check whether a series of rays is predominantly in RHI mode
//
// Returns true if RHI, false otherwise

bool RadxVol::checkIsRhi(size_t startRayIndex,
                         size_t endRayIndex)
{

  vector<RadxRay *> rayList;
  for (size_t ii = startRayIndex; ii <= endRayIndex; ii++) {
    rayList.push_back(_rays[ii]);
  }
         
  return RadxAngleHist::checkIsRhi(rayList);

}

///////////////////////////////////////////////////////
/// check if rays are predominantly in
/// SUVEILLANCE mode i.e. 360's azimuth rotation
/// Side effect - sets sweep info
/// Returns true if surveillance, false otherwise

bool RadxVol::checkIsSurveillance()
{
  
  // sanity check

  if (_rays.size() < 2) {
    return false;
  }

  loadSweepInfoFromRays();

  // check for any sweep with an azimuth covered of 345 or more

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    double azCovered = computeAzCovered(sweep);
    if (azCovered >= 345.0) {
      return true;
    }
  }

  return false;

}

///////////////////////////////////////////////////////
/// check if sequence of rays are predominantly in
/// SUVEILLANCE mode i.e. 360's azimuth rotation
/// Returns true if surveillance, false otherwise

bool RadxVol::checkIsSurveillance(size_t startRayIndex,
                                  size_t endRayIndex)
{
  
  double azCovered = computeAzCovered(startRayIndex, endRayIndex);
  if (azCovered >= 345.0) {
    return true;
  }
  
  return false;

}

///////////////////////////////////////////////////////
// Compute the azimuth swept out by a sweep
// Returns the azimuth covered.

double RadxVol::computeAzCovered(const RadxSweep *sweep) const
{
  size_t startIndex = sweep->getStartRayIndex();
  size_t endIndex = sweep->getEndRayIndex();
  return computeAzCovered(startIndex, endIndex);
}
  
///////////////////////////////////////////////////////
// Compute the azimuth swept out by sequence of rays
// Returns the azimuth covered.

double RadxVol::computeAzCovered(size_t startRayIndex,
                                 size_t endRayIndex) const
{

  if (endRayIndex <= startRayIndex) {
    return 0.0;
  }
  
  // sum up azimuth covered by the sweep
  
  double count = 0.0;
  double sumDeltaAz = 0.0;
  double prevAz = _rays[startRayIndex]->getAzimuthDeg();

  for (size_t ii = startRayIndex + 1; ii <= endRayIndex; ii++) {
    double az = _rays[ii]->getAzimuthDeg();
    double deltaAz = az - prevAz;
    if (deltaAz < -180) {
      deltaAz += 360.0;
    } else if (deltaAz > 180) {
      deltaAz -= 360.0;
    }
    sumDeltaAz += fabs(deltaAz);
    count++;
    prevAz = az;
  } // ii

  // account for the width of the end rays

  double meanDeltaAz = sumDeltaAz / count;
  sumDeltaAz += meanDeltaAz;

  // return

  return sumDeltaAz;

}

/////////////////////////////////////////////////////
/// check whether start_range and gate_spacing varies per ray
///
/// Returns true if gate geom varies by ray, false otherwise

bool RadxVol::gateGeomVariesByRay() const

{

  if (_rays.size() < 2) {
    return false;
  }

  double startRange0 = _rays[0]->getStartRangeKm();
  double gateSpacing0 = _rays[0]->getGateSpacingKm();

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    
    if (_rays[ii]->getStartRangeKm() != startRange0) {
      return true;
    }
    
    if (_rays[ii]->getGateSpacingKm() != gateSpacing0) {
      return true;
    }

  }

  return false;

}

/////////////////////////////////////////////////////////////
// combine rays from sweeps with common fixed angle and
// gate geometry, but with different fields

void RadxVol::combineSweepsAtSameFixedAngleAndGeom
  (bool keepLongRange /* = false */)
  
{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  // make sure sweep info is up to date

  loadSweepInfoFromRays();

  // find sweeps that should be combined

  vector<Combo> combos;
  set<int> sources;

  for (int ii = (int) _sweeps.size() - 1; ii > 0; ii--) {
    
    RadxSweep *sweep1 = _sweeps[ii];
    RadxRay *ray1 = _rays[sweep1->getStartRayIndex()];
    Combo combo(ii);
    
    for (int jj = ii - 1; jj >= 0; jj--) {
      
      RadxSweep *sweep0 = _sweeps[jj];
      RadxRay *ray0 = _rays[sweep0->getStartRayIndex()];
      
      if ((fabs(sweep0->getFixedAngleDeg() -
                sweep1->getFixedAngleDeg()) < 0.001) &&
          (fabs(ray0->getStartRangeKm() -
                ray1->getStartRangeKm()) < 0.001) &&
          (fabs(ray0->getGateSpacingKm() -
                ray1->getGateSpacingKm()) < 0.001)) {
        
        if (sources.find(jj) == sources.end()) {
          // source sweep not previously used
          sources.insert(jj);
          combo.sources.push_back(jj);
        }

      }
      
    } // jj
    
    combos.push_back(combo);
    
  } // ii
  
  // combine the data from the sweeps

  for (size_t ii = 0; ii < combos.size(); ii++) {
    const Combo &combo = combos[ii];
    for (size_t jj = 0; jj < combo.sources.size(); jj++) {
      _augmentSweepFields(combo.target, combo.sources[jj]);
    }
  }

  // select the rays to keep
  
  vector<RadxRay *> keepRays;
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    if (sources.find(ii) == sources.end()) {
      // not a source, so keep these rays
      RadxSweep *sweepTarget = _sweeps[ii];
      for (size_t kk = sweepTarget->getStartRayIndex();
           kk <= sweepTarget->getEndRayIndex(); kk++) {
        keepRays.push_back(_rays[kk]);
      }
    } else {
      // discard the source sweeps if no longer needed
      RadxSweep *sweepSource = _sweeps[ii];
      for (size_t kk = sweepSource->getStartRayIndex();
           kk <= sweepSource->getEndRayIndex(); kk++) {
        if (keepLongRange && _rays[kk]->getIsLongRange()) {
          keepRays.push_back(_rays[kk]);
        } else {
          RadxRay::deleteIfUnused(_rays[kk]);
        }
      }
    }
  } // ii

  _rays = keepRays;

  // reload the sweep info

  loadSweepInfoFromRays();

}

////////////////////////////////////////////////////////
/// Augment fields in a sweep, by copying in fields from
/// another sweep

void RadxVol::_augmentSweepFields(size_t targetIndex, size_t sourceIndex)
{

  RadxSweep *sweepTarget = _sweeps[targetIndex];
  RadxSweep *sweepSource = _sweeps[sourceIndex];

  _setupAngleSearch(sourceIndex);

  // check sweep mode

  Radx::SweepMode_t sweepMode = sweepTarget->getSweepMode();
  if (sweepMode != sweepSource->getSweepMode()) {
    // sweep modes do not match
    return;
  }
  
  // loop through the target rays
  
  for (size_t iray = sweepTarget->getStartRayIndex();
       iray <= sweepTarget->getEndRayIndex(); iray++) {
    
    RadxRay *rayTarget = _rays[iray];
    double angle = rayTarget->getAzimuthDeg();
    if (sweepMode == Radx::SWEEP_MODE_RHI) {
      angle = rayTarget->getElevationDeg();
    }

    // get the source ray

    int angleIndex = _getSearchAngleIndex(angle);
    const RadxRay *raySource = _searchRays[angleIndex];
    if (raySource != NULL) {

      // got a valid ray in source
      // loop through the fields in the source
      
      for (size_t ifield = 0; ifield < raySource->getNFields(); ifield++) {
        
        const RadxField *fldSource = raySource->getField(ifield);
        
        // make a copy of the field
        
        RadxField *copy = new RadxField(*fldSource);

        // does this field exist in the target?
        
        RadxField *fldTarget = rayTarget->getField(fldSource->getName());
        
        if (fldTarget != NULL) {
          // field already exists on the target, so modify its name
          // based on the sweep number
          char newName[128];
          sprintf(newName, "%s-s%d",
                  fldTarget->getName().c_str(), (int) targetIndex);
          fldTarget->setName(newName);
        }
        
        // add field to target ray
        
        rayTarget->addField(copy);
        
      } // ifield
      
    } // if (raySource != NULL)

  } // iray


}

////////////////////////////////////////////////////////
/// Make fields uniform in the volume.
/// This ensures that all rays in the volume have the same fields
/// and that they are in the same order in each ray.
/// If fields a missing from a ray, a suitable field is added
/// containing missing data.

void RadxVol::makeFieldsUniform()

{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  // make uniform

  _makeFieldsUniform(0, _rays.size() - 1);

}

////////////////////////////////////////////////////////
/// Make fields uniform for each sweep.
/// This ensures that all rays in a sweep have the same fields.
/// and that they are in the same order in each ray.
/// If fields a missing from a ray, a suitable field is added
/// containing missing data.

void RadxVol::makeFieldsUniformPerSweep()

{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    const RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();
    
    _makeFieldsUniform(startIndex, endIndex);
    
  } // isweep

}

void RadxVol::_makeFieldsUniform(size_t startIndex, size_t endIndex)

{

  // create field template by searching through the rays
  // for examples of all the field names
  
  set<string> nameSet;
  vector<const RadxField *> tplate;
  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    const RadxRay &ray = *_rays[iray];
    for (size_t ifield = 0; ifield < ray.getNFields(); ifield++) {
      const RadxField *fld = ray.getField(ifield);
      string name = fld->getName();
      pair<set<string>::const_iterator, bool> ret = nameSet.insert(name);
      if (ret.second == true) {
        // field name not previously in set, so add field to template
        tplate.push_back(fld);
      }
    }
  }
  
  // make rays match the field template
  
  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    _rays[iray]->makeFieldsMatchTemplate(tplate);
  }
  
}

////////////////////////////////////////////////////////
/// Reorder the fields, by name, removing any extra fields.

void RadxVol::reorderFieldsByName(const vector<string> &names)

{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->reorderFieldsByName(names);
  }

}

////////////////////////////////////////////////////////
/// Remove a specifed field
/// Returns 0 on success, -1 on failure

int RadxVol::removeField(const string &name)
  
{
  
  int iret = 0;

  if (_fields.size()) {
    
    // fields are contiguous

    // copy good fields to temp array

    vector<RadxField *> temp;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      RadxField *field = _fields[ii];
      if (field->getName() == name) {
        delete field;
      } else {
        temp.push_back(field);
      }
    } // ii

    if (_fields.size() == temp.size()) {
      // field not found
      iret = -1;
    }

    _fields = temp;

  } else {

    // fields are managed by rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      if (_rays[ii]->removeField(name)) {
        return iret;
      }
    }

  }

  return iret;

}

////////////////////////////////////////////////////////
/// Set the latitude of the platform in degrees.
///
/// Used for non-mobile platforms.

void RadxVol::setLatitudeDeg(double val) 
{

  _platform.setLatitudeDeg(val);

}

////////////////////////////////////////////////////////
/// Set the longitude of the platform in degrees.
///
/// Used for non-mobile platforms.

void RadxVol::setLongitudeDeg(double val) 
{

  _platform.setLongitudeDeg(val);

}

////////////////////////////////////////////////////////
/// Set the altitude of the platform in km.
///
/// Used for non-mobile platforms.

void RadxVol::setAltitudeKm(double val) 
{

  _platform.setAltitudeKm(val);

}

////////////////////////////////////////////////////////
/// Set the sensor ht above the surface

void RadxVol::setSensorHtAglM(double val) 
{

  _platform.setSensorHtAglM(val);

}

////////////////////////////////////////////////
/// set the radar location

void RadxVol::setLocation(double latitudeDeg,
                          double longitudeDeg,
                          double altitudeKm)

{

  _platform.setLatitudeDeg(latitudeDeg);
  _platform.setLongitudeDeg(longitudeDeg);
  _platform.setAltitudeKm(altitudeKm);

}

////////////////////////////////////////////////
/// override the radar location
/// this also sets the location in any georeference objects
/// attached to the rays

void RadxVol::overrideLocation(double latitudeDeg,
                               double longitudeDeg,
                               double altitudeKm)

{
  
  setLocation(latitudeDeg, longitudeDeg, altitudeKm);
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxGeoref *georef = _rays[ii]->getGeoreference();
    if (georef) {
      georef->setLatitude(latitudeDeg);
      georef->setLongitude(longitudeDeg);
      georef->setAltitudeKmMsl(altitudeKm);
    }
  }

}

////////////////////////////////////////////////
/// override the radar ht AGL
/// this also sets the location in any georeference objects
/// attached to the rays

void RadxVol::overrideSensorHtAglM(double val)

{
  
  setSensorHtAglM(val);
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxGeoref *georef = _rays[ii]->getGeoreference();
    if (georef) {
      georef->setAltitudeKmAgl(val / 1000.0);
    }
  }

}

////////////////////////////////////////////////////////
/// apply the georeference corrections for moving platforms
/// to compute the earth-relative azimuth and elevation
///
/// If force is true, the georefs are always applied.
/// If force is false, the georefs are applied only if
/// they have not been applied previously.

void RadxVol::applyGeorefs(bool force /* = true */)

{

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_cfactors) {
      _rays[ii]->setCfactors(*_cfactors);
    }
    _rays[ii]->applyGeoref(_platform.getPrimaryAxis());
  }

}

//////////////////////////////////////////////////////////
/// count the number of rays in which each georef element
/// is not missing

void RadxVol::countGeorefsNotMissing(RadxGeoref &count) const

{

  count.setToZero();
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->incrementGeorefNotMissingCount(count);
  }

}
////////////////////////////////////////////  
/// Set up angle search, for a given sweep
/// Return 0 on success, -1 on failure

int RadxVol::_setupAngleSearch(size_t sweepNum)
  
{

  // check sweep num is valid

  assert(sweepNum < _sweeps.size());

  // init rays to NULL - i.e. no match
  
  for (int ii = 0; ii < _searchAngleN; ii++) {
    _searchRays[ii] = NULL;
  }

  // set the sweep pointers

  const RadxSweep *sweep = _sweeps[sweepNum];
  size_t startRayIndex = sweep->getStartRayIndex();
  size_t endRayIndex = sweep->getEndRayIndex();
  
  double prevAngle = _rays[startRayIndex]->getAzimuthDeg();
  if (sweep->getSweepMode() == Radx::SWEEP_MODE_RHI) {
    prevAngle = _rays[startRayIndex]->getElevationDeg();
  }
  double sumDeltaAngle = 0.0;
  double count = 0.0;
  
  for (size_t ii = startRayIndex; ii <= endRayIndex; ii++) {
    
    RadxRay *ray = _rays[ii];

    // get angle depending on sweep mode

    double angle = ray->getAzimuthDeg();
    if (sweep->getSweepMode() == Radx::SWEEP_MODE_RHI) {
      angle = ray->getElevationDeg();
    }

    // condition angle

    while (angle < 0) {
      angle += 360.0;
    }
    while (angle >= 360.0) {
      angle -= 360.0;
    }

    // compute angle change
    
    double deltaAngle = fabs(angle - prevAngle);
    if (deltaAngle > 180) {
      deltaAngle = fabs(deltaAngle - 360.0);
    }
    prevAngle = angle;
    sumDeltaAngle += deltaAngle;
    count++;
    
    // get angle index
    
    int iangle = _getSearchAngleIndex(angle);
    
    // set ray at index location
    
    _searchRays[iangle] = ray;
    
  } // iray
  
  // compute mean delta angle, and search width
  
  double meanDeltaAngle = sumDeltaAngle / count;
  _searchMaxWidth = (int) ((meanDeltaAngle / _searchAngleRes) * 1.5);

  // populate the search array for rays in sweep

  int firstIndex = -1;
  int lastIndex = -1;
  for (int ii = 0; ii < _searchAngleN; ii++) {
    // find active index
    const RadxRay *rayii = _searchRays[ii];
    if (rayii != NULL) {
      if (firstIndex < 0) {
        firstIndex = ii;
      }
      // find next active index
      for (int jj = ii + 1; jj < _searchAngleN; jj++) {
        const RadxRay *rayjj = _searchRays[jj];
        if (rayjj != NULL) {
          lastIndex = jj;
          _populateSearchRays(ii, jj);
          ii = jj - 1;
          break;
        }
      } // jj
    }
  } // ii

  // populate search across 360 line
  
  _populateSearchAcross360(firstIndex, lastIndex);

  return 0;

}
  
/////////////////////////////////////////////////////////
// get the angle index for the search matrix, given
// the angle angle
//
// Returns -1 if out of bounds

int RadxVol::_getSearchAngleIndex(double angle) 
{
  int iangle = (int) (angle / _searchAngleRes + 0.5);
  if (iangle < 0) {
    iangle = 0;
  } else if (iangle > _searchAngleN - 1) {
    iangle = _searchAngleN - 1;
  }
  return iangle;
}
  
/////////////////////////////////////////////////////////
// get the angle given the search index

double RadxVol::_getSearchAngle(int index) 
{
  return index * _searchAngleRes;
}
  
/////////////////////////////////////////////////////////
// populate the search matrix between given indices

void RadxVol::_populateSearchRays(int start, int end)
{
  
  int len = end - start;
  int extend = len / 2;
  if (extend > _searchMaxWidth) {
    extend = _searchMaxWidth;
  }

  const RadxRay *rayStart = _searchRays[start];
  for (int ii = start + 1; ii <= start + extend; ii++) {
    _searchRays[ii] = rayStart;
  }

  const RadxRay *rayEnd = _searchRays[end];
  for (int ii = end - 1; ii >= end - extend; ii--) {
    _searchRays[ii] = rayEnd;
  }

}

/////////////////////////////////////////////////////////
// populate the search across the 360 line

void RadxVol::_populateSearchAcross360(int first, int last)
{
  
  int len = first + _searchAngleN - last;
  int extend = len / 2;
  if (extend > _searchMaxWidth) {
    extend = _searchMaxWidth;
  }
  
  const RadxRay *rayLast = _searchRays[last];
  for (int ii = last + 1; ii <= last + extend; ii++) {
    int jj = ii;
    if (jj >= _searchAngleN) jj -= _searchAngleN;
    _searchRays[jj] = rayLast;
  }

  const RadxRay *rayFirst = _searchRays[first];
  for (int ii = first - 1; ii >= first - extend; ii--) {
    int jj = ii;
    if (jj < 0) jj += _searchAngleN;
    _searchRays[jj] = rayFirst;
  }

}

/////////////////////////////////////////////////////////
// Get the transition index in a sweep, where it crosses
// a specified azimuth
//
// Returns -1 if no transition found.

int RadxVol::_getTransIndex(const RadxSweep *sweep, double azimuth)
{

  for (size_t rayIndex = sweep->getStartRayIndex();
       rayIndex < sweep->getEndRayIndex(); rayIndex++) {

    RadxRay *ray1 = _rays[rayIndex];
    RadxRay *ray2 = _rays[rayIndex + 1];

    double az1 = ray1->getAzimuthDeg();
    double az2 = ray2->getAzimuthDeg();

    double delta1 = az1 - azimuth;
    if (delta1 < -180.0) {
      delta1 += 360.0;
    } else if (delta1 > 180.0) {
      delta1 -= 360.0;
    }

    double delta2 = az2 - azimuth;
    if (delta2 < -180.0) {
      delta2 += 360.0;
    } else if (delta2 > 180.0) {
      delta2 -= 360.0;
    }

    if (delta1 <= 0.0 && delta2 >= 0.0) {
      return (int) rayIndex;
    }
    
    if (delta2 <= 0.0 && delta1 >= 0.0) {
      return (int) rayIndex;
    }

  } // rayIndex

  // no transition found

  return -1;

}

