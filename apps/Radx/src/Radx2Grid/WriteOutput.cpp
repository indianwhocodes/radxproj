#include "WriteOutput.hh"
#include "OutputMdv.hh"
#include <Radx/RadxRay.hh>
#include <algorithm>
#include <memory>

WriteOutput::WriteOutput(std::string progN, std::shared_ptr<Repository> store,
                         RadxVol &readVol, const Params &params,
                         vector<Interp::Field> &interpFields)
    : _readVol(readVol), _params(params), _interpFields(interpFields) {
  _progName = progN, _store = store;

  if (_params.output_test_fields) {
    _createTestFields();
  }

  // create convective/stratiform fields if needed

  if (_params.identify_convective_stratiform_split) {
    _createConvStratFields();
  }
}
WriteOutput::~WriteOutput() {}

/////////////////////////////////////////////////////
// write out data

int WriteOutput::writeOutputFile() {
  if (_params.debug) {
    cerr << "  Writing output file ... " << endl;
  }
  // cedric is a special case
  if (_params.output_format == Params::CEDRIC) {
    //        return _writeCedricFile(false);  do we need this?
  }

  // initialize the output grid dimensions
  _initGrid();

  if (_setRadarParams()) {
    cerr << "ERROR - CartInterp::interpVol()" << endl;
  }

  OutputMdv out(_progName, _params);
  out.setMasterHeader(_readVol);
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    const Interp::Field &ifld = _interpFields[ifield];
    const fl32 *_outputFields = _store->_outFields["REF"]->data();
    out.addField(_readVol, _proj, _gridZLevels, ifld.outputName, ifld.longName,
                 ifld.units, Radx::DataType_t::FL32, 1.0, 0.0, missingFl32,
                 _outputFields);
  } // ifield

  // debug (test) fields

  for (size_t ii = 0; ii < _derived3DFields.size(); ii++) {
    const DerivedField *dfld = _derived3DFields[ii];
    if (dfld->writeToFile) {
      out.addField(_readVol, _proj, dfld->vertLevels, dfld->name,
                   dfld->longName, dfld->units, Radx::FL32, 1.0, 0.0,
                   missingFl32, dfld->data);
    }
  }

  for (size_t ii = 0; ii < _derived2DFields.size(); ii++) {
    const DerivedField *dfld = _derived2DFields[ii];
    if (dfld->writeToFile) {
      out.addField(_readVol, _proj, dfld->vertLevels, dfld->name,
                   dfld->longName, dfld->units, Radx::FL32, 1.0, 0.0,
                   missingFl32, dfld->data);
    }
  }

  // chunks

  out.addChunks(_readVol, _interpFields.size());

  // write out file

  if (out.writeVol()) {
    cerr << "ERROR - Interp::processFile" << endl;
    cerr << "  Cannot write file to output_dir: " << _params.output_dir << endl;
    return -1;
  }

  return 0;
}

//////////////////////////////////////////////////
// create the test fields

void WriteOutput::_createTestFields()

{

  _nContribDebug =
      new DerivedField("nContrib", "n_points_contrib", "count", true);
  _derived3DFields.push_back(_nContribDebug);

  _gridAzDebug = new DerivedField("gridAz", "grid_azimiuth", "deg", true);
  _derived3DFields.push_back(_gridAzDebug);

  _gridElDebug = new DerivedField("gridEl", "grid_elevation", "deg", true);
  _derived3DFields.push_back(_gridElDebug);

  _gridRangeDebug =
      new DerivedField("gridRange", "grid_slant_range", "km", true);
  _derived3DFields.push_back(_gridRangeDebug);

  _llElDebug = new DerivedField("llEl", "lower_left_el", "deg", true);
  _derived3DFields.push_back(_llElDebug);
  _llAzDebug = new DerivedField("llAz", "lower_left_az", "deg", true);
  _derived3DFields.push_back(_llAzDebug);

  _lrElDebug = new DerivedField("lrEl", "lower_right_el", "deg", true);
  _derived3DFields.push_back(_lrElDebug);
  _lrAzDebug = new DerivedField("lrAz", "lower_right_az", "deg", true);
  _derived3DFields.push_back(_lrAzDebug);

  _ulElDebug = new DerivedField("ulEl", "upper_left_el", "deg", true);
  _derived3DFields.push_back(_ulElDebug);
  _ulAzDebug = new DerivedField("ulAz", "upper_left_az", "deg", true);
  _derived3DFields.push_back(_ulAzDebug);

  _urElDebug = new DerivedField("urEl", "upper_right_el", "deg", true);
  _derived3DFields.push_back(_urElDebug);
  _urAzDebug = new DerivedField("urAz", "upper_right_az", "deg", true);
  _derived3DFields.push_back(_urAzDebug);
}

//////////////////////////////////////////////////
// create the conv-strat fields

void WriteOutput::_createConvStratFields()

{

  bool writeDebug = _params.conv_strat_write_debug_fields;
  bool writePartition = _params.conv_strat_write_partition;

  _convStratDbzMax =
      new DerivedField("DbzMax", "max_dbz_for_conv_strat", "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzMax);

  _convStratDbzCount = new DerivedField(
      "DbzCount", "n_points_dbz_for_conv_strat", "count", writeDebug);
  _derived3DFields.push_back(_convStratDbzCount);

  _convStratDbzSum =
      new DerivedField("DbzSum", "sum_dbz_for_conv_strat", "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzSum);

  _convStratDbzSqSum = new DerivedField("DbzSqSum", "sum_dbz_sq_for_conv_strat",
                                        "dbz^2", writeDebug);
  _derived3DFields.push_back(_convStratDbzSqSum);

  _convStratDbzSqSqSum = new DerivedField(
      "DbzSqSqSum", "sum_dbz_sq_sq_for_conv_strat", "dbz^4", writeDebug);
  _derived3DFields.push_back(_convStratDbzSqSqSum);

  _convStratDbzTexture = new DerivedField(
      "DbzTexture", "dbz_texture_for_conv_strat", "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzTexture);

  _convStratFilledTexture = new DerivedField(
      "FilledTexture", "filled_dbz_texture_for_conv_strat", "dbz", writeDebug);
  _derived3DFields.push_back(_convStratFilledTexture);

  _convStratDbzSqTexture = new DerivedField(
      "DbzSqTexture", "dbz_sq_texture_for_conv_strat", "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzSqTexture);

  _convStratFilledSqTexture = new DerivedField(
      "FilledSqTexture", "filled_dbz_sq_texture_for_conv_strat", "dbz",
      writeDebug);
  _derived3DFields.push_back(_convStratFilledSqTexture);

  _convStratDbzColMax = new DerivedField(
      "DbzColMax", "col_max_dbz_for_conv_strat", "dbz", writeDebug);
  _derived2DFields.push_back(_convStratDbzColMax);

  _convStratMeanTexture = new DerivedField(
      "MeanTexture", "mean_dbz_texture_for_conv_strat", "dbz", writeDebug);
  _derived2DFields.push_back(_convStratMeanTexture);

  _convStratMeanSqTexture = new DerivedField(
      "MeanSqTexture", "mean_dbz_sq_texture_for_conv_strat", "dbz", writeDebug);
  _derived2DFields.push_back(_convStratMeanSqTexture);

  _convStratCategory = new DerivedField("ConvStrat", "category_for_conv_strat",
                                        "", writePartition);
  _derived2DFields.push_back(_convStratCategory);
}

////////////////////////////////////////////////////////////
// Initialize output grid

void WriteOutput::_initGrid()

{

  // initialize the Z levels

  _initZLevels();

  // init the xy grid

  _gridNx = _params.grid_xy_geom.nx;
  _gridMinx = _params.grid_xy_geom.minx;
  _gridDx = _params.grid_xy_geom.dx;

  _gridNy = _params.grid_xy_geom.ny;
  _gridMiny = _params.grid_xy_geom.miny;
  _gridDy = _params.grid_xy_geom.dy;

  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _gridNz;

  for (size_t ii = 0; ii < _derived3DFields.size(); ii++) {
    _derived3DFields[ii]->alloc(_nPointsVol, _gridZLevels);
  }

  vector<double> singleLevel;
  singleLevel.push_back(0.0);
  for (size_t ii = 0; ii < _derived2DFields.size(); ii++) {
    _derived2DFields[ii]->alloc(_nPointsPlane, singleLevel);
  }

  if (_params.identify_convective_stratiform_split) {
    _convStratDbzCount->setToZero();
    _convStratDbzSum->setToZero();
    _convStratDbzSqSum->setToZero();
    _convStratDbzSqSqSum->setToZero();
  }
}

////////////////////////////////////////////////////////////
// Initialize Z levels

void WriteOutput::_initZLevels() {
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

void WriteOutput::_initProjection()

{

  _proj.setGrid(_gridNx, _gridNy, _gridDx, _gridDy, _gridMinx, _gridMiny);

  if (_params.grid_projection == Params::PROJ_LATLON) {
    _proj.initLatlon();
  } else if (_params.grid_projection == Params::PROJ_FLAT) {
    _proj.initFlat(_gridOriginLat, _gridOriginLon, _params.grid_rotation);
  } else if (_params.grid_projection == Params::PROJ_LAMBERT_CONF) {
    _proj.initLambertConf(_gridOriginLat, _gridOriginLon, _params.grid_lat1,
                          _params.grid_lat2);
  } else if (_params.grid_projection == Params::PROJ_POLAR_STEREO) {
    Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
    if (!_params.grid_pole_is_north) {
      poleType = Mdvx::POLE_SOUTH;
    }
    _proj.initPolarStereo(_gridOriginLat, _gridOriginLon,
                          _params.grid_tangent_lon, poleType,
                          _params.grid_central_scale);
  } else if (_params.grid_projection == Params::PROJ_OBLIQUE_STEREO) {
    _proj.initObliqueStereo(_gridOriginLat, _gridOriginLon,
                            _params.grid_tangent_lat, _params.grid_tangent_lon,
                            _params.grid_central_scale);
  } else if (_params.grid_projection == Params::PROJ_MERCATOR) {
    _proj.initMercator(_gridOriginLat, _gridOriginLon);
  } else if (_params.grid_projection == Params::PROJ_TRANS_MERCATOR) {
    _proj.initTransMercator(_gridOriginLat, _gridOriginLon,
                            _params.grid_central_scale);
  } else if (_params.grid_projection == Params::PROJ_ALBERS) {
    _proj.initAlbers(_gridOriginLat, _gridOriginLon, _params.grid_lat1,
                     _params.grid_lat2);
  } else if (_params.grid_projection == Params::PROJ_LAMBERT_AZIM) {
    _proj.initLambertAzim(_gridOriginLat, _gridOriginLon);
  } else if (_params.grid_projection == Params::PROJ_VERT_PERSP) {
    _proj.initVertPersp(_gridOriginLat, _gridOriginLon,
                        _params.grid_persp_radius);
  }

  if (_params.grid_set_offset_origin) {
    _proj.setOffsetOrigin(_params.grid_offset_origin_latitude,
                          _params.grid_offset_origin_longitude);
  } else {
    _proj.setOffsetCoords(_params.grid_false_northing,
                          _params.grid_false_easting);
  }

  _proj.latlon2xy(_readVol.getLatitudeDeg(), _readVol.getLongitudeDeg(),
                  _radarX, _radarY);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "============ Interp::_initProjection() ===============" << endl;
    _proj.print(cerr, false);
    cerr << "  radarX: " << _radarX << endl;
    cerr << "  radarY: " << _radarY << endl;
    cerr << "======================================================" << endl;
  }
}

////////////////////////////////////////////////////////////
// Compute grid locations relative to radar

void WriteOutput::_computeGridRelative()

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

int WriteOutput::_setRadarParams()

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
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    int nGates = ray->getNGates();
    if (nGates > _maxNGates) {
      _maxNGates = nGates;
    }
  }
  _maxRangeKm = _startRangeKm + _maxNGates * _gateSpacingKm;

  return 0;
}
