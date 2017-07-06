#include <vector>

#include "netcdf"
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <radar/BeamHeight.hh>
#include <tbb/tbb.h>

#include "Params.hh"
#include "PolarDataStream.hh"

// constructor
PolarDataStream::PolarDataStream(const std::string& inputFile,
                                 const Params& params)
  : _params(params)
{
  _store = std::make_shared<Repository>();
  _store->inputFile = inputFile;
}

PolarDataStream::~PolarDataStream()
{
}

// read dimenssions, variables from NetCDF file and fill the repository
void
PolarDataStream::LoadDataFromNetCDFFilesIntoRepository()
{

  netCDF::NcFile dataFile(_store->inputFile, netCDF::NcFile::read);
  netCDF::NcDim TimeDim = dataFile.getDim("time");
  _store->timeDim = TimeDim.getSize();
  netCDF::NcDim RangeDim = dataFile.getDim("range");
  _store->rangeDim = RangeDim.getSize();
  netCDF::NcDim n_points = dataFile.getDim("n_points");
  _store->nPoints = n_points.getSize();
  netCDF::NcVar lat = dataFile.getVar("latitude");
  lat.getVar(&_store->latitude);
  netCDF::NcVar lon = dataFile.getVar("longitude");
  lon.getVar(&_store->longitude);
  netCDF::NcVar alt_agl = dataFile.getVar("altitude_agl");
  alt_agl.getVar(&_store->altitudeAgl);
  _store->timeVar.resize(_store->timeDim);
  float* timeVarPtr = _store->timeVar.data();
  netCDF::NcGroupAtt stationName = dataFile.getAtt("instrument_name");
  stationName.getValues(_store->instrumentName);
  netCDF::NcGroupAtt startDateTime = dataFile.getAtt("start_datetime");
  startDateTime.getValues(_store->startDateTime);  
  netCDF::NcVar timeVar = dataFile.getVar("time");
  timeVar.getVar(timeVarPtr);
  _store->rangeVar.resize(_store->timeDim);
  float* rangeVarPtr = _store->rangeVar.data();
  netCDF::NcVar range = dataFile.getVar("range");
  range.getVar(rangeVarPtr);
  _store->rayNGates.resize(_store->timeDim);
  int* rayNGatesPtr = _store->rayNGates.data();
  netCDF::NcVar ray_n_gates = dataFile.getVar("ray_n_gates");
  ray_n_gates.getVar(rayNGatesPtr);
  _store->gateSize.resize(_store->timeDim);
  float* gateSizePtr = _store->gateSize.data();
  netCDF::NcVar gateSize = dataFile.getVar("ray_gate_spacing");
  gateSize.getVar(gateSizePtr);
  _store->rayStartIndex.resize(_store->timeDim);
  int* rayStartIndexPtr = _store->rayStartIndex.data();
  netCDF::NcVar ray_start_index = dataFile.getVar("ray_start_index");
  ray_start_index.getVar(rayStartIndexPtr);
  _store->rayStartRange.resize(_store->timeDim);
  float* rayStartRangePtr = _store->rayStartRange.data();
  netCDF::NcVar ray_start_range = dataFile.getVar("ray_start_range");
  ray_start_range.getVar(rayStartRangePtr);
  _store->azimuth.resize(_store->timeDim);
  float* azimuthPtr = _store->azimuth.data();
  netCDF::NcVar azimuth = dataFile.getVar("azimuth");
  azimuth.getVar(azimuthPtr);
  _store->elevation.resize(_store->timeDim);
  float* elevationPtr = _store->elevation.data();
  netCDF::NcVar elevation = dataFile.getVar("elevation");
  elevation.getVar(elevationPtr);

  // TODO: Must read fields from parameter files
  auto field1 = make_shared<RepositoryField>();
  field1->fieldValues.resize(_store->nPoints);
  float* reflectivityPtr = field1->fieldValues.data();
  netCDF::NcVar ref = dataFile.getVar("REF");
  ref.getVar(reflectivityPtr);
  netCDF::NcVarAtt scaleFactor = ref.getAtt("scale_factor");
  if (!scaleFactor.isNull()) {
    scaleFactor.getValues(&(field1->scaleFactor));
  }
  netCDF::NcVarAtt fillValue = ref.getAtt("_FillValue");
  if (!fillValue.isNull()) {
    fillValue.getValues(&(field1->fillValue));
  }
  netCDF::NcVarAtt offset = ref.getAtt("add_offset");
  if (!offset.isNull()) {
    offset.getValues(&(field1->addOffset));
  }
  _store->inFields.insert(
    std::pair<std::string, shared_ptr<RepositoryField>>("REF", field1));
}

// create 1D array for Elevation, Azimuth, Gate and field values such as Ref
void
PolarDataStream::populateOutputValues(int nthread)
{
  // Size of outGate, outRef, outAzi etc = sum(ray_n_gates) = n_points

  // Pre-allocate for speed.
  _store->outGate.resize(_store->nPoints);
  _store->outElevation.resize(_store->nPoints);
  _store->outAzimuth.resize(_store->nPoints);
  
  
  tbb::task_scheduler_init init(nthread);
  for (auto it = _store->inFields.cbegin(); it != _store->inFields.cend();
       ++it) {
    string name = (*it).first;
    auto fin = (*it).second;
    auto fout = make_shared<std::vector<double>>();
    fout->resize(_store->nPoints);

    tbb::parallel_for(size_t(0), _store->timeDim, [=](size_t i) {
      // Start of Expansion function
      const float& r0 = _store->rayStartRange[i];
      const float& g = _store->gateSize[i];
      const size_t& start = size_t(_store->rayStartIndex[i]);
      const size_t& end = start + size_t(_store->rayNGates[i]);
      const float& replicateElevation = _store->elevation[i];
      const float& replicateAz = _store->azimuth[i];
      std::vector<double> outRangeGate(end - start + 1);

#pragma ivdep
      for (size_t m = 0; m < outRangeGate.size(); m++) {
        outRangeGate[m] = m * g + r0;
      }

#pragma ivdep
      for (auto j = start; j < end; j++) {
        _store->outGate[j] = outRangeGate[j - start];
        _store->outElevation[j] = replicateElevation;
        _store->outAzimuth[j] = replicateAz;

        if (fin->fieldValues[j] == fin->fillValue) {
          fout->at(j) = INVALID_DATA;
        } else {
          fout->at(j) = fin->fieldValues[j] * fin->scaleFactor + fin->addOffset;
        }
      }
      // End of Expansion function
    });
    _store->outFields.insert(std::make_pair(name, fout));
  }
}

std::shared_ptr<Repository>
PolarDataStream::getRepository()
{
  return _store;
}
