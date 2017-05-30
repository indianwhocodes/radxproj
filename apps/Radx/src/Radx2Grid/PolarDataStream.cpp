#include "PolarDataStream.hh"
#include "Params.hh"
#include "netcdf"
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <radar/BeamHeight.hh>
#include <tbb/tbb.h>
#include <vector>

// constructor
PolarDataStream::PolarDataStream(const std::string &inputFile,
                                 const Params &params)
    : _params(params) {
  _store = std::make_shared<Repository>();
  _store->_inputFile = inputFile;
}

PolarDataStream::~PolarDataStream() /*delete _store;*/
{}

// read dimenssions, variables from NetCDF file and fill the repository
void PolarDataStream::LoadDataFromNetCDFFilesIntoRepository() {
  if (_params.debug) {
    std::cerr << "Function called: LoadDataFromNetCDFFilesIntoRepository"
              << std::endl;
  }

  if (_readFile(_store->_inputFile)) {
    cerr << "ERROR - Radx2Grid::_processFile" << endl;
  }

  // make sure gate geom is constant
  _readVol.remapToFinestGeom();

  netCDF::NcFile dataFile(_store->_inputFile, netCDF::NcFile::read);
  netCDF::NcDim TimeDim = dataFile.getDim("time");
  _store->_timeDim = TimeDim.getSize();
  netCDF::NcDim RangeDim = dataFile.getDim("range");
  _store->_rangeDim = RangeDim.getSize();
  netCDF::NcDim n_points = dataFile.getDim("n_points");
  _store->_nPoints = n_points.getSize();
  netCDF::NcVar lat = dataFile.getVar("latitude");
  lat.getVar(&_store->_latitude);
  netCDF::NcVar lon = dataFile.getVar("longitude");
  lon.getVar(&_store->_longitude);
  netCDF::NcVar alt_agl = dataFile.getVar("altitude_agl");
  alt_agl.getVar(&_store->_altitudeAgl);
  _store->_timeVar.resize(_store->_timeDim);
  float *timeVarPtr = _store->_timeVar.data();
  netCDF::NcVar timeVar = dataFile.getVar("time");
  timeVar.getVar(timeVarPtr);
  _store->_rangeVar.resize(_store->_timeDim);
  float *rangeVarPtr = _store->_rangeVar.data();
  netCDF::NcVar range = dataFile.getVar("range");
  range.getVar(rangeVarPtr);
  _store->_rayNGates.resize(_store->_timeDim);
  int *rayNGatesPtr = _store->_rayNGates.data();
  netCDF::NcVar ray_n_gates = dataFile.getVar("ray_n_gates");
  ray_n_gates.getVar(rayNGatesPtr);
  _store->_gateSize.resize(_store->_timeDim);
  float *gateSizePtr = _store->_gateSize.data();
  netCDF::NcVar gateSize = dataFile.getVar("ray_gate_spacing");
  gateSize.getVar(gateSizePtr);
  _store->_rayStartIndex.resize(_store->_timeDim);
  int *rayStartIndexPtr = _store->_rayStartIndex.data();
  netCDF::NcVar ray_start_index = dataFile.getVar("ray_start_index");
  ray_start_index.getVar(rayStartIndexPtr);
  _store->_rayStartRange.resize(_store->_timeDim);
  float *rayStartRangePtr = _store->_rayStartRange.data();
  netCDF::NcVar ray_start_range = dataFile.getVar("ray_start_range");
  ray_start_range.getVar(rayStartRangePtr);
  _store->_azimuth.resize(_store->_timeDim);
  float *azimuthPtr = _store->_azimuth.data();
  netCDF::NcVar azimuth = dataFile.getVar("azimuth");
  azimuth.getVar(azimuthPtr);
  _store->_elevation.resize(_store->_timeDim);
  float *elevationPtr = _store->_elevation.data();
  netCDF::NcVar elevation = dataFile.getVar("elevation");
  elevation.getVar(elevationPtr);
  // TODO: Must read fields from parameter files
  _store->_raw_reflectivity.resize(_store->_timeDim * _store->_rangeDim);
  float *reflectivityPtr = _store->_raw_reflectivity.data();
  netCDF::NcVar ref = dataFile.getVar("REF");
  ref.getVar(reflectivityPtr);
  netCDF::NcVarAtt scale_factor = ref.getAtt("scale_factor");
  scale_factor.getValues(&_store->_scalingFactor);
  netCDF::NcVarAtt fillValue = ref.getAtt("_FillValue");
  fillValue.getValues(&_store->_fillValue);
  netCDF::NcVarAtt offset = ref.getAtt("add_offset");
  offset.getValues(&_store->_addOffset);
}

// create 1D array for Elevation, Azimuth, Gate and field values such as Ref
void PolarDataStream::populateOutputValues() {
  // Size of outGate, outRef, outAzi etc = sum(ray_n_gates) = n_points
  // Pre-allocate for speed.
  _store->_outGate.resize(_store->_nPoints);
  _store->_outElevation.resize(_store->_nPoints);
  _store->_outAzimuth.resize(_store->_nPoints);
  _store->_outRef.resize(_store->_nPoints);
  tbb::parallel_for(0, _store->_timeDim, 1, [=](int i) {
    // Start of Expansion function
    float r0 = _store->_rayStartRange[i];
    float g = _store->_gateSize[i];
    int start = _store->_rayStartIndex[i];
    int end = start + _store->_rayNGates[i];
    float replicateElevation = _store->_elevation[i];
    float replicateAz = _store->_azimuth[i];
    std::vector<float> outRangeGate(end - start + 1);

#pragma ivdep
    for (size_t m = 0; m < outRangeGate.size(); m++) {
      outRangeGate[m] = m * g + r0;
    }

#pragma ivdep
    for (int j = start; j < end; j++) {
      _store->_outGate[j] = outRangeGate[j - start];
      _store->_outElevation[j] = replicateElevation;
      _store->_outAzimuth[j] = replicateAz;

      if (_store->_raw_reflectivity[i] == _store->_fillValue) {
        _store->_outRef[j] = INVALID_DATA;
      } else {
        _store->_outRef[j] =
            _store->_raw_reflectivity[j] * _store->_scalingFactor +
            _store->_addOffset;
      }
    }
    // End of Expansion function
  });
}

// getter for output values
std::vector<float> PolarDataStream::getOutElevation() {
  return _store->_outElevation;
}

std::vector<float> PolarDataStream::getOutAzimuth() {
  return _store->_azimuth;
  // return _store->_outAzimuth;
  // which one is rignt?
}

std::vector<float> PolarDataStream::getOutGate() { return _store->_outGate; }

std::vector<float> PolarDataStream::getOutRef() { return _store->_outRef; }

std::shared_ptr<Repository> PolarDataStream::getRepository() { return _store; }

RadxVol &PolarDataStream::getRadxVol() { return _readVol; }

vector<Interp::Field> &PolarDataStream::getInterpFields() {
  return _interpFields;
}

//////////////////////////////////////
// code below is copied from Radx2Grid for output file
//////////////////////////////////////

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int PolarDataStream::_readFile(const string &filePath) {

  GenericRadxFile inFile;
  _setupRead(inFile);

  // read in file

  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - Radx2Grid::_readFile" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();

  // apply angle corrections as appropriate
  // side effect - forces el between -180 and 180

  _readVol.applyAzimuthOffset(_params.azimuth_correction_deg);
  _readVol.applyElevationOffset(_params.elevation_correction_deg);

  //  check for rhi

  _rhiMode = _isRhi();
  if (_params.interp_mode == Params::INTERP_MODE_CART_SAT) {
    _rhiMode = false;
  }

  // override radar location if requested

  if (_params.override_radar_location) {
    _readVol.overrideLocation(_params.radar_latitude_deg,
                              _params.radar_longitude_deg,
                              _params.radar_altitude_meters / 1000.0);
  }

  // volume number

  if (_params.override_volume_number || _params.autoincrement_volume_number) {
    _readVol.setVolumeNumber(_volNum);
  }
  if (_params.autoincrement_volume_number) {
    _volNum++;
  }

  // override radar name and site name if requested

  if (_params.override_instrument_name) {
    _readVol.setInstrumentName(_params.instrument_name);
  }
  if (_params.override_site_name) {
    _readVol.setSiteName(_params.site_name);
  }

  // override beam width if requested

  if (_params.override_beam_width) {
    _readVol.setRadarBeamWidthDegH(_params.beam_width_deg_h);
    _readVol.setRadarBeamWidthDegV(_params.beam_width_deg_v);
  }

  // override gate geometry if requested

  if (_params.interp_mode == Params::INTERP_MODE_CART_SAT) {
    if (_params.sat_data_set_range_geom_from_fields) {
      _readVol.copyRangeGeomFromFieldsToRays();
    }
  }

  if (_params.override_gate_geometry) {
    _readVol.setRangeGeom(_params.start_range_km, _params.gate_spacing_km);
  }

  // override fixed angle if required

  if (_params.override_fixed_angle_with_mean_measured_angle) {
    _readVol.computeFixedAngleFromRays();
  }

  // trim surveillance sweeps to 360 degrees if requested

  if (_params.trim_surveillance_sweeps_to_360deg) {
    _readVol.trimSurveillanceSweepsTo360Deg();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _startRangeKm: " << _readVol.getStartRangeKm() << endl;
    cerr << "  _gateSpacingKm: " << _readVol.getGateSpacingKm() << endl;
  }

  // set up and fields than need transforming

  _setupTransformFields();

  // add test and coverage fields to input rays as required

  _addTestAndCoverageInputFields();

  // for reorder, add in extra sweep at start and end
  // so that we can require boundedness

  if (_params.interp_mode == Params::INTERP_MODE_CART_REORDER) {
    _addBoundingSweeps();
  }

  // set up interp fields

  _initInterpFields();

  // load up the input ray data vector

  _loadInterpRays();

  // check all fields are present
  // set standard names etc

  _checkFields(filePath);

  return 0;
}

//////////////////////////////////////////////////
// set up read
// RadxFile provides the methods for writing and reading RadxVol.

void PolarDataStream::_setupRead(RadxFile &file) {

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_params.remove_rays_with_antenna_transitions &&
      !_params.trim_surveillance_sweeps_to_360deg) {
    file.setReadIgnoreTransitions(true);
    file.setReadTransitionNraysMargin(_params.transition_nrays_margin);
  }

  if (_params.remove_long_range_rays) {
    file.setReadRemoveLongRange(true);
  } else {
    file.setReadRemoveLongRange(false);
  }

  if (_params.remove_short_range_rays) {
    file.setReadRemoveShortRange(true);
  } else {
    file.setReadRemoveShortRange(false);
  }

  if (_params.interp_mode == Params::INTERP_MODE_POLAR ||
      _params.interp_mode == Params::INTERP_MODE_PPI) {
    if (_params.set_elevation_angle_limits) {
      file.setReadFixedAngleLimits(_params.lower_elevation_angle_limit,
                                   _params.upper_elevation_angle_limit);
      file.setReadStrictAngleLimits(true);
    }
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.select_fields) {
    for (int ii = 0; ii < _params.selected_fields_n; ii++) {
      if (_params._selected_fields[ii].process_this_field) {
        file.addReadField(_params._selected_fields[ii].input_name);
      }
    }
    if (_params.apply_censoring) {
      for (int ii = 0; ii < _params.censoring_fields_n; ii++) {
        file.addReadField(_params._censoring_fields[ii].name);
      }
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
}

//////////////////////////////////////////////////
// set up the transform fields, as needed

void PolarDataStream::_setupTransformFields() {

  if (!_params.transform_fields_for_interpolation) {
    return;
  }

  // loop through rays

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];

    // loop through fields to be transformed

    for (int jfield = 0; jfield < _params.transform_fields_n; jfield++) {

      bool makeCopy = true;
      const Params::transform_field_t &transform =
          _params._transform_fields[jfield];
      if (strcmp(transform.input_name, transform.output_name) == 0) {
        makeCopy = false;
      }

      // find field on ray

      RadxField *rfield = ray->getField(transform.input_name);
      if (rfield == NULL) {
        continue;
      }

      // get working field

      RadxField *xfield = rfield;
      if (makeCopy) {
        // copy field
        xfield = new RadxField(*rfield);
        xfield->setName(transform.output_name);
      }

      // set units

      xfield->setUnits(transform.output_units);

      // transform

      xfield->convertToFl32();
      if (transform.transform == Params::TRANSFORM_DB_TO_LINEAR ||
          transform.transform == Params::TRANSFORM_DB_TO_LINEAR_AND_BACK) {
        xfield->transformDbToLinear();
      } else if (transform.transform == Params::TRANSFORM_LINEAR_TO_DB ||
                 transform.transform ==
                     Params::TRANSFORM_LINEAR_TO_DB_AND_BACK) {
        xfield->transformLinearToDb();
      }

      if (makeCopy) {
        // add to ray
        ray->addField(xfield);
      }

    } // jfield

  } // iray
}

//////////////////////////////////////////////////
// add test and coverage fields as required
// to the input data, so that they are available
// for interpolation as needed.

void PolarDataStream::_addTestAndCoverageInputFields() {

  if (!_params.output_test_fields && !_params.output_coverage_field &&
      !_params.output_range_field && !_params.output_time_field) {
    return;
  }

  string mode = "_nearest";
  if (_params.interp_test_fields) {
    mode = "_interp";
  }

  // start times

  time_t startTimeSecs = _readVol.getStartTimeSecs();
  double startNanoSecs = _readVol.getStartNanoSecs();

  // beamHeight

  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_readVol.getAltitudeKm());
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }

  // loop through rays

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(nGates);

    if (_params.output_test_fields) {

      // add elevation field

      RadxField *elevFld = new RadxField("el" + mode, "deg");
      elevFld->setLongName("diagnostic_field_elevation_angle" + mode);
      elevFld->setStandardName("elevation_angle" + mode);
      elevFld->setTypeFl32(-9999.0);
      double elev = fmod(ray->getElevationDeg(), _params.modulus_for_elevation);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = elev;
      }
      elevFld->addDataFl32(nGates, data);
      if (!_params.interp_test_fields) {
        elevFld->setIsDiscrete(true);
      }
      ray->addField(elevFld);

      // add azimuth field

      RadxField *azFld = new RadxField("az" + mode, "deg");
      azFld->setLongName("diagnostic_field_azimuth_angle" + mode);
      azFld->setStandardName("azimuth_angle" + mode);
      azFld->setTypeFl32(-9999.0);
      double az = fmod(ray->getAzimuthDeg(), _params.modulus_for_azimuth);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = az;
      }
      azFld->addDataFl32(nGates, data);
      if (!_params.interp_test_fields) {
        azFld->setIsDiscrete(true);
      }
      ray->addField(azFld);

      // add ht field

      RadxField *htFld = new RadxField("hgt" + mode, "km");
      htFld->setLongName("diagnostic_field_height_MSL" + mode);
      htFld->setStandardName("height_MSL" + mode);
      htFld->setTypeFl32(-9999.0);
      double elevDeg = ray->getElevationDeg();
      double range = ray->getStartRangeKm();
      for (int ii = 0; ii < nGates; ii++) {
        double ht = beamHt.computeHtKm(elevDeg, range);
        data[ii] = ht;
        range += ray->getGateSpacingKm();
      }
      htFld->addDataFl32(nGates, data);
      if (!_params.interp_test_fields) {
        htFld->setIsDiscrete(true);
      }
      ray->addField(htFld);

    } // if (_params.output_test_fields)

    // range field

    RadxField *rangeFld = new RadxField("range", "km");
    rangeFld->setLongName("diagnostic_field_range_from_radar" + mode);
    rangeFld->setStandardName("slant_range" + mode);
    rangeFld->setTypeFl32(-9999.0);
    double range = ray->getStartRangeKm();
    for (int ii = 0; ii < nGates; ii++) {
      data[ii] = fmod(range, _params.modulus_for_range);
      range += ray->getGateSpacingKm();
    }
    rangeFld->addDataFl32(nGates, data);
    if (!_params.interp_range_field) {
      rangeFld->setIsDiscrete(true);
    }
    ray->addField(rangeFld);

    // time field

    if (_params.output_time_field) {
      RadxField *timeFld = new RadxField("time_elapsed", "secs");
      timeFld->setLongName("diagnostic_field_time_since_volume_start");
      timeFld->setStandardName("time_since_volume_start");
      timeFld->setTypeFl32(-9999.0);
      time_t timeSecs = ray->getTimeSecs();
      double nanoSecs = ray->getNanoSecs();
      double secsSinceStart = (double)(timeSecs - startTimeSecs) -
                              ((nanoSecs - startNanoSecs) * 1.0e-9);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = secsSinceStart;
      }
      timeFld->addDataFl32(nGates, data);
      if (!_params.interp_time_field) {
        timeFld->setIsDiscrete(true);
      }
      ray->addField(timeFld);
    }

    // coverage field

    if (_params.output_coverage_field) {
      RadxField *covFld = new RadxField(_params.coverage_field_name, "");
      covFld->setLongName("diagnostic_field_radar_coverage_flag");
      covFld->setStandardName("radar_coverage_flag");
      covFld->setTypeFl32(-9999.0);
      for (int ii = 0; ii < nGates; ii++) {
        data[ii] = 1.0;
      }
      covFld->addDataFl32(nGates, data);
      ray->addField(covFld);
    }

  } // iray
}

////////////////////////////////////////////////////////////
// Add in sweeps to provide boundedness

void PolarDataStream::_addBoundingSweeps()

{

  if (_rhiMode) {
    return;
  }

  size_t nSweeps = _readVol.getNSweeps();
  if (nSweeps < 2) {
    return;
  }

  // find sweep number for lowest and highest angles

  int maxSweepNum = -99;
  int lowSweepNum = 0, highSweepNum = 0;
  double minFixedAngle = 360;
  double maxFixedAngle = -360;
  const vector<RadxSweep *> &sweeps = _readVol.getSweeps();
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    const RadxSweep *sweep = sweeps[ii];
    double fixedAngle = sweep->getFixedAngleDeg();
    int sweepNum = sweep->getSweepNumber();
    if (fixedAngle < minFixedAngle) {
      lowSweepNum = sweepNum;
      minFixedAngle = fixedAngle;
    }
    if (fixedAngle > maxFixedAngle) {
      highSweepNum = sweepNum;
      maxFixedAngle = fixedAngle;
    }
    if (sweepNum > maxSweepNum) {
      maxSweepNum = sweepNum;
    }
  }

  // loop through rays, adding rays below the lowest sweep

  double beamWidthDegV = _readVol.getRadarBeamWidthDegV();
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    if (ray->getSweepNumber() == lowSweepNum) {
      // copy the ray
      RadxRay *copy = new RadxRay(*ray);
      // set the fixed angle half a beam width below
      copy->setFixedAngleDeg(ray->getFixedAngleDeg() - beamWidthDegV / 2.0);
      // set the elevation angle half a beam width below
      copy->setElevationDeg(ray->getElevationDeg() - beamWidthDegV / 2.0);
      // set the sweep number to 1 above the max
      copy->setSweepNumber(maxSweepNum + 1);
      // add to vol
      _readVol.addRay(copy);
    }
  } // ii

  // loop through rays, adding rays above the highest sweep

  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    if (ray->getSweepNumber() == highSweepNum) {
      // copy the ray
      RadxRay *copy = new RadxRay(*ray);
      // set the fixed angle half a beam width above
      copy->setFixedAngleDeg(ray->getFixedAngleDeg() + beamWidthDegV / 2.0);
      // set the elevation angle half a beam width above
      copy->setElevationDeg(ray->getElevationDeg() + beamWidthDegV / 2.0);
      // set the sweep number to 2 above the max
      copy->setSweepNumber(maxSweepNum + 2);
      // add to vol
      _readVol.addRay(copy);
    }
  } // ii

  _readVol.loadSweepInfoFromRays();
}

//////////////////////////////////////////////////
// initialize the fields for interpolation

void PolarDataStream::_initInterpFields() {

  _interpFields.clear();

  // get field name list

  vector<string> fieldNames = _readVol.getUniqueFieldNameList();

  // find an example of each field, by search through the rays
  // use that field as the template

  for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {

    string radxName = fieldNames[ifield];

    vector<RadxRay *> &rays = _readVol.getRays();
    for (size_t iray = 0; iray < rays.size(); iray++) {
      const RadxRay *ray = rays[iray];
      const RadxField *field = ray->getField(radxName);
      if (field != NULL) {
        Interp::Field interpField;
        interpField.radxName = field->getName();
        interpField.outputName = field->getName();
        interpField.longName = field->getLongName();
        interpField.standardName = field->getStandardName();
        interpField.units = field->getUnits();
        interpField.inputDataType = field->getDataType();
        interpField.inputScale = field->getScale();
        interpField.inputOffset = field->getOffset();
        if (field->getFieldFolds()) {
          interpField.fieldFolds = true;
          interpField.foldLimitLower = field->getFoldLimitLower();
          interpField.foldLimitUpper = field->getFoldLimitUpper();
          interpField.foldRange =
              interpField.foldLimitUpper - interpField.foldLimitLower;
        }
        if (field->getIsDiscrete()) {
          interpField.isDiscrete = true;
        }
        _interpFields.push_back(interpField);
        break;
      }
    }

  } // ifield

  // override fold limits from the parameters

  double nyquist = 0.0;
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {
    const RadxRay *ray = rays[iray];
    if (fabs(ray->getNyquistMps() - Radx::missingFl64) > 1.0e-6) {
      nyquist = ray->getNyquistMps();
      break;
    }
  }

  if (_params.set_fold_limits) {
    for (int ii = 0; ii < _params.folded_fields_n; ii++) {
      string radxName = _params._folded_fields[ii].input_name;
      bool fieldFolds = _params._folded_fields[ii].field_folds;
      bool useNyquist = _params._folded_fields[ii].use_global_nyquist;
      double foldLimitLower = _params._folded_fields[ii].fold_limit_lower;
      double foldLimitUpper = _params._folded_fields[ii].fold_limit_upper;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == radxName) {
          _interpFields[ifld].fieldFolds = fieldFolds;
          if (useNyquist && nyquist > 0) {
            _interpFields[ifld].foldLimitLower = -nyquist;
            _interpFields[ifld].foldLimitUpper = nyquist;
          } else {
            _interpFields[ifld].foldLimitLower = foldLimitLower;
            _interpFields[ifld].foldLimitUpper = foldLimitUpper;
          }
          _interpFields[ifld].foldRange = _interpFields[ifld].foldLimitUpper -
                                          _interpFields[ifld].foldLimitLower;
          break;
        }
      } // ifld
    }   // ii
  }

  // override discrete flag from the parameters

  if (_params.set_discrete_fields) {
    for (int ii = 0; ii < _params.discrete_fields_n; ii++) {
      string radxName = _params._discrete_fields[ii].input_name;
      bool isDiscrete = _params._discrete_fields[ii].is_discrete;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == radxName) {
          _interpFields[ifld].isDiscrete = isDiscrete;
          break;
        }
      } // ifld
    }   // ii
  }

  // set bounded fields from the parameters (probably not in the radx data
  // anyway)

  if (_params.bound_fields) {
    for (int ii = 0; ii < _params.bounded_fields_n; ii++) {
      string radxName = _params._bounded_fields[ii].input_name;
      double v0 = _params._bounded_fields[ii].min_value;
      double v1 = _params._bounded_fields[ii].max_value;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == radxName) {
          _interpFields[ifld].isBounded = true;
          _interpFields[ifld].boundLimitLower = v0;
          _interpFields[ifld].boundLimitUpper = v1;
          break;
        }
      } // ifld
    }   // ii
  }

  // rename fields

  if (_params.rename_fields) {
    for (int ii = 0; ii < _params.renamed_fields_n; ii++) {
      string inputName = _params._renamed_fields[ii].input_name;
      string outputName = _params._renamed_fields[ii].output_name;
      for (size_t ifld = 0; ifld < _interpFields.size(); ifld++) {
        if (_interpFields[ifld].radxName == inputName) {
          _interpFields[ifld].outputName = outputName;
          break;
        }
      } // ifld
    }   // ii
  }     // if (_params.specify_field_names)
}

//////////////////////////////////////////////////
// load up the input ray data vector

void PolarDataStream::_loadInterpRays() {

  // loop through the rays in the read volume,
  // making some checks and then adding the rays
  // to the interp rays array as appropriate

  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t isweep = 0; isweep < _readVol.getNSweeps(); isweep++) {

    const RadxSweep *sweep = _readVol.getSweeps()[isweep];

    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {

      const RadxRay *ray = rays[iray];

      // check elevation limits if required

      if (_params.interp_mode != Params::INTERP_MODE_POLAR &&
          _params.interp_mode != Params::INTERP_MODE_PPI) {
        if (_params.set_elevation_angle_limits) {
          double el = ray->getElevationDeg();
          if (el < _params.lower_elevation_angle_limit ||
              el > _params.upper_elevation_angle_limit) {
            continue;
          }
        }
      }

      // check azimith limits if required

      if (_params.set_azimuth_angle_limits) {
        double az = ray->getAzimuthDeg();
        double minAz = _params.lower_azimuth_angle_limit;
        double maxAz = _params.upper_azimuth_angle_limit;
        if (minAz <= maxAz) {
          // valid sector does not cross north
          if (az < minAz || az > maxAz) {
            continue;
          }
        } else {
          // valid sector does cross north
          if (az < minAz && az > maxAz) {
            continue;
          }
        }
      }

      // check fixed angle error?

      if (_params.check_fixed_angle_error) {
        double fixedAngle = ray->getFixedAngleDeg();
        double maxError = _params.max_fixed_angle_error;
        if (_rhiMode) {
          // RHI
          double error = fabs(fixedAngle - ray->getAzimuthDeg());
          if (error > 180) {
            error = fabs(error - 360.0);
          }
          if (error > maxError) {
            continue;
          }
        } else {
          // PPI
          double error = fabs(fixedAngle - ray->getElevationDeg());
          if (error > maxError) {
            continue;
          }
        }
      }

      // accept ray

      Interp::Ray *interpRay =
          new Interp::Ray(rays[iray], isweep, _interpFields,
                          _params.use_fixed_angle_for_interpolation,
                          _params.use_fixed_angle_for_data_limits);
      if (_params.apply_censoring) {
        _censorInterpRay(interpRay);
      }
      _interpRays.push_back(interpRay);

    } // iray

  } // isweep
}

//////////////////////////////////////////////////
// check all fields are present
// set standard names etc

void PolarDataStream::_checkFields(const string &filePath) {

  vector<RadxRay *> &rays = _readVol.getRays();

  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    bool found = false;
    string fieldName = _interpFields[ifield].radxName;
    for (size_t ii = 0; ii < rays.size(); ii++) {
      const RadxRay *ray = rays[ii];
      const RadxField *fld = ray->getField(fieldName);
      if (fld != NULL) {
        found = true;
        _interpFields[ifield].longName = fld->getLongName();
        _interpFields[ifield].standardName = fld->getStandardName();
        _interpFields[ifield].units = fld->getUnits();
        break;
      }
    }
    if (!found) {
      cerr << "WARNING - field not found: " << fieldName << endl;
      cerr << "  File: " << filePath << endl;
    }
  } // ifield
}

/////////////////////////////////////////////////////
// check whether volume is predominantly in RHI mode

bool PolarDataStream::_isRhi() {

  // check to see if we are in RHI mode, set flag accordingly

  int nRaysRhi = 0;
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    if (rays[ii]->getSweepMode() == Radx::SWEEP_MODE_RHI ||
        rays[ii]->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
      nRaysRhi++;
    }
  }
  double fractionRhi = (double)nRaysRhi / (double)rays.size();
  if (fractionRhi > 0.5) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
// censor an interp ray

void PolarDataStream::_censorInterpRay(Interp::Ray *interpRay)

{

  RadxRay *ray = interpRay->inputRay;

  if (!_params.apply_censoring) {
    return;
  }

  // initialize censoring flags to true to
  // turn censoring ON everywhere

  vector<int> censorFlag;
  size_t nGates = ray->getNGates();
  for (size_t igate = 0; igate < nGates; igate++) {
    censorFlag.push_back(1);
  }

  // check OR fields
  // if any of these have VALID data, we turn censoring OFF

  int orFieldCount = 0;

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {

    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_OR) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      // field missing, do not censor
      if (_nWarnCensorPrint % 360 == 0) {
        cerr << "WARNING - censoring field missing: " << cfld.name << endl;
        cerr << "  Censoring will not be applied for this field." << endl;
      }
      _nWarnCensorPrint++;
      for (size_t igate = 0; igate < nGates; igate++) {
        censorFlag[igate] = 0;
      }
      continue;
    }

    orFieldCount++;

    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *)field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val >= minValidVal && val <= maxValidVal) {
        censorFlag[igate] = 0;
      }
    }

  } // ifield

  // if no OR fields were found, turn off ALL censoring at this stage

  if (orFieldCount == 0) {
    for (size_t igate = 0; igate < nGates; igate++) {
      censorFlag[igate] = 0;
    }
  }

  // check AND fields
  // if any of these have INVALID data, we turn censoring ON

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {

    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_AND) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      continue;
    }

    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *)field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val < minValidVal || val > maxValidVal) {
        censorFlag[igate] = 1;
      }
    }

  } // ifield

  // check that uncensored runs meet the minimum length
  // those which do not are censored

  int minValidRun = _params.censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < (int)nGates; igate++) {
      if (censorFlag[igate] == 0) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == (int)nGates - 1)
        doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            censorFlag[jgate] = 1;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    }   // igate
  }

  // apply censoring by setting censored gates to missing for all fields

  vector<RadxField *> fields = ray->getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    RadxField *field = fields[ifield];
    if (field->getLongName().find("diagnostic_field_") != string::npos) {
      // do not censor diagnostic fields
      continue;
    }
    Radx::fl32 *fdata = (Radx::fl32 *)field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      if (censorFlag[igate] == 1) {
        fdata[igate] = Radx::missingFl32;
      }
    } // igate
  }   // ifield
}
