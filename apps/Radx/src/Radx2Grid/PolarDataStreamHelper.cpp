#include <Mdv/GenericRadxFile.hh>

#include "PolarDataStream.hh"
#include "Params.hh"


//////////////////////////////////////
// code below is copied from Radx2Grid for output file
//////////////////////////////////////

//////////////////////////////////////////////////
// Read in a RADX file
// Returns 0 on success, -1 on failure

int PolarDataStream::readFile(const string& filePath)
{

  GenericRadxFile inFile;
  setupRead(inFile);

  // read in file

  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - Radx2Grid::readFile" << endl;
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

void PolarDataStream::setupRead(RadxFile& file)
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
