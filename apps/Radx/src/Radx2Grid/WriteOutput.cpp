#include "WriteOutput.hh"
#include "OutputMdv.hh"
#include <Radx/RadxRay.hh>
#include <algorithm>
#include <memory>

WriteOutput::WriteOutput(shared_ptr<Repository> store,
                         RadxVol& readVol,
                         const Params& params)
  : _store(store)
  , _readVol(readVol)
  , _params(params)
{
}

WriteOutput::~WriteOutput()
{
}

/////////////////////////////////////////////////////
// write out data

int
WriteOutput::writeOutputFile()
{
  return 0;
}

////////////////////////////////////////////////////////////
// Initialize output grid


////////////////////////////////////////////////////////////
// Initialize Z levels

void
WriteOutput::_initZLevels()
{
  if (_params.specify_individual_z_levels) {
    _gridNz = _params.z_level_array_n;
  } else {
    _gridNz = _params.grid_z_geom.nz;
  }

  if (_params.specify_individual_z_levels) {

    for (int ii = 0; ii < _gridNz; ii++) {
      _gridZLevels.push_back(_params._z_level_array[ii]);
    }

  } else {

    for (int ii = 0; ii < _gridNz; ii++) {
      _gridZLevels.push_back(_params.grid_z_geom.minz +
                             ii * _params.grid_z_geom.dz);
    }
  }
}

////////////////////////////////////////////////////////////
// Initialize projection

void
WriteOutput::_initProjection()
{
  _radarX = 0.0;
  _radarY = 0.0;
}

////////////////////////////////////////////////////////////
// Compute grid locations relative to radar

void
WriteOutput::_computeGridRelative()

{

  // check if radar has moved

  if (fabs(_prevRadarLat - _radarLat) < 0.00001 &&
      fabs(_prevRadarLon - _radarLon) < 0.00001 &&
      fabs(_prevRadarAltKm - _radarAltKm) < 0.00001) {
    return;
  }

  _prevRadarLat = _radarLat;
  _prevRadarLon = _radarLon;
  _prevRadarAltKm = _radarAltKm;

  if (_params.center_grid_on_radar) {
    _gridOriginLat = _radarLat;
    _gridOriginLon = _radarLon;
  } else {
    _gridOriginLat = _params.grid_origin_lat;
    _gridOriginLon = _params.grid_origin_lon;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _radarLat: " << _radarLat << endl;
    cerr << "  _radarLon: " << _radarLon << endl;
    cerr << "  _radarAltKm: " << _radarAltKm << endl;
    cerr << "  _gridOriginLat: " << _gridOriginLat << endl;
    cerr << "  _gridOriginLon: " << _gridOriginLon << endl;
  }

  // initialize the projection

  _initProjection();
}

//////////////////////////////////////////////////
// set the radar details

int
WriteOutput::_setRadarParams()

{

  // set radar location

  _radarLat = _readVol.getLatitudeDeg();
  _radarLon = _readVol.getLongitudeDeg();
  _radarAltKm = _readVol.getAltitudeKm();

  // set beam width

  _beamWidthDegH = _readVol.getRadarBeamWidthDegH();
  _beamWidthDegV = _readVol.getRadarBeamWidthDegV();

  if (_params.interp_mode != Params::INTERP_MODE_CART_SAT) {
    if (_beamWidthDegH <= 0 || _beamWidthDegV <= 0) {
      cerr << "ERROR - Interp::_setRadarParams()" << endl;
      cerr << "  Radar beam width not set" << endl;
      cerr << "  beamWidthDegH: " << _beamWidthDegH << endl;
      cerr << "  beamWidthDegV: " << _beamWidthDegV << endl;
      cerr << "  You must use the option to override the beam width" << endl;
      cerr << "  in the param file" << endl;
      return -1;
    }
  }

  // set gate geometry

  _startRangeKm = _readVol.getStartRangeKm();
  _gateSpacingKm = _readVol.getGateSpacingKm();

  if (_startRangeKm <= -9990) {
    cerr << "ERROR - Interp::_setRadarParams()" << endl;
    cerr << "  Ray start range not set" << endl;
    cerr << "  startRangeKm: " << _startRangeKm << endl;
    cerr << "  You must use the option to override the gate geometry" << endl;
    cerr << "  in the param file" << endl;
    return -1;
  }

  if (_gateSpacingKm <= 0) {
    cerr << "ERROR - Interp::_setRadarParams()" << endl;
    cerr << "  Ray gate spacing not set" << endl;
    cerr << "  gateSpacingKm: " << _gateSpacingKm << endl;
    cerr << "  You must use the option to override the gate geometry" << endl;
    cerr << "  in the param file" << endl;
    return -1;
  }

  _maxNGates = 0;
  const vector<RadxRay*>& rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay* ray = rays[ii];
    int nGates = ray->getNGates();
    if (nGates > _maxNGates) {
      _maxNGates = nGates;
    }
  }
  _maxRangeKm = _startRangeKm + _maxNGates * _gateSpacingKm;

  return 0;
}
