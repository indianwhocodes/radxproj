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
PolarDataStream::PolarDataStream(const std::string& inputFile,
                                 const Params& params)
  : _params(params)
{
  _store = std::make_shared<Repository>();
  _store->_inputFile = inputFile;
}

PolarDataStream::~PolarDataStream() /*delete _store;*/
{
}

// read dimenssions, variables from NetCDF file and fill the repository
void
PolarDataStream::LoadDataFromNetCDFFilesIntoRepository()
{

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
  float* timeVarPtr = _store->_timeVar.data();
  netCDF::NcVar timeVar = dataFile.getVar("time");
  timeVar.getVar(timeVarPtr);
  _store->_rangeVar.resize(_store->_timeDim);
  float* rangeVarPtr = _store->_rangeVar.data();
  netCDF::NcVar range = dataFile.getVar("range");
  range.getVar(rangeVarPtr);
  _store->_rayNGates.resize(_store->_timeDim);
  int* rayNGatesPtr = _store->_rayNGates.data();
  netCDF::NcVar ray_n_gates = dataFile.getVar("ray_n_gates");
  ray_n_gates.getVar(rayNGatesPtr);
  _store->_gateSize.resize(_store->_timeDim);
  float* gateSizePtr = _store->_gateSize.data();
  netCDF::NcVar gateSize = dataFile.getVar("ray_gate_spacing");
  gateSize.getVar(gateSizePtr);
  _store->_rayStartIndex.resize(_store->_timeDim);
  int* rayStartIndexPtr = _store->_rayStartIndex.data();
  netCDF::NcVar ray_start_index = dataFile.getVar("ray_start_index");
  ray_start_index.getVar(rayStartIndexPtr);
  _store->_rayStartRange.resize(_store->_timeDim);
  float* rayStartRangePtr = _store->_rayStartRange.data();
  netCDF::NcVar ray_start_range = dataFile.getVar("ray_start_range");
  ray_start_range.getVar(rayStartRangePtr);
  _store->_azimuth.resize(_store->_timeDim);
  float* azimuthPtr = _store->_azimuth.data();
  netCDF::NcVar azimuth = dataFile.getVar("azimuth");
  azimuth.getVar(azimuthPtr);
  _store->_elevation.resize(_store->_timeDim);
  float* elevationPtr = _store->_elevation.data();
  netCDF::NcVar elevation = dataFile.getVar("elevation");
  elevation.getVar(elevationPtr);

  // TODO: Must read fields from parameter files
  auto f1 = make_shared<RepositoryField>();
  f1->fieldValues.resize(_store->_nPoints);
  float* reflectivityPtr = f1->fieldValues.data();
  netCDF::NcVar ref = dataFile.getVar("REF");
  ref.getVar(reflectivityPtr);
  netCDF::NcVarAtt scaleFactor = ref.getAtt("scale_factor");
  if (!scaleFactor.isNull()) {
    scaleFactor.getValues(&(f1->scale_factor));
  }
  netCDF::NcVarAtt fillValue = ref.getAtt("_FillValue");
  if (!fillValue.isNull()) {
    fillValue.getValues(&(f1->fill_value));
  }
  netCDF::NcVarAtt offset = ref.getAtt("add_offset");
  if (!offset.isNull()) {
    offset.getValues(&(f1->add_offset));
  }
  _store->_inFields.insert(
    std::pair<std::string, shared_ptr<RepositoryField>>("REF", f1));
}

// create 1D array for Elevation, Azimuth, Gate and field values such as Ref
void
PolarDataStream::populateOutputValues()
{
  // Size of outGate, outRef, outAzi etc = sum(ray_n_gates) = n_points

  // Pre-allocate for speed.
  _store->_outGate.resize(_store->_nPoints);
  _store->_outElevation.resize(_store->_nPoints);
  _store->_outAzimuth.resize(_store->_nPoints);
  for (auto it = _store->_inFields.cbegin(); it != _store->_inFields.cend();
       ++it) {
    string name = (*it).first;
    auto fin = (*it).second;
    auto fout = make_shared<std::vector<double>>();
    fout->resize(_store->_nPoints);

    tbb::parallel_for(size_t(0), _store->_timeDim, [=](size_t i) {
      // Start of Expansion function
      const float& r0 = _store->_rayStartRange[i];
      const float& g = _store->_gateSize[i];
      const size_t& start = size_t(_store->_rayStartIndex[i]);
      const size_t& end = start + size_t(_store->_rayNGates[i]);
      const float& replicateElevation = _store->_elevation[i];
      const float& replicateAz = _store->_azimuth[i];
      std::vector<double> outRangeGate(end - start + 1);

#pragma ivdep
      for (size_t m = 0; m < outRangeGate.size(); m++) {
        outRangeGate[m] = m * g + r0;
      }

#pragma ivdep
      for (auto j = start; j < end; j++) {
        _store->_outGate[j] = outRangeGate[j - start];
        _store->_outElevation[j] = replicateElevation;
        _store->_outAzimuth[j] = replicateAz;

        if (fin->fieldValues[j] == fin->fill_value) {
          fout->at(j) = INVALID_DATA;
        } else {
          fout->at(j) =
            fin->fieldValues[j] * fin->scale_factor + fin->add_offset;
        }
      }
      // End of Expansion function
    });
    _store->_outFields.insert(std::make_pair(name, fout));
  }
}

std::shared_ptr<Repository>
PolarDataStream::getRepository()
{
  return _store;
}

RadxVol&
PolarDataStream::getRadxVol()
{
  return _readVol;
}

vector<Interp::Field>&
PolarDataStream::getInterpFields()
{
  return _interpFields;
}

//////////////////////////////////////
// code below is copied from Radx2Grid for output file
//////////////////////////////////////

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int
PolarDataStream::_readFile(const string& filePath)
{

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

  _rhiMode = false;

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

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _startRangeKm: " << _readVol.getStartRangeKm() << endl;
    cerr << "  _gateSpacingKm: " << _readVol.getGateSpacingKm() << endl;
  }

  return 0;
}

//////////////////////////////////////////////////
// set up read
// RadxFile provides the methods for writing and reading RadxVol.

void
PolarDataStream::_setupRead(RadxFile& file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }

  if (_params.select_fields) {
    for (int ii = 0; ii < _params.selected_fields_n; ii++) {
      if (_params._selected_fields[ii].process_this_field) {
        file.addReadField(_params._selected_fields[ii].input_name);
      }
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
}
