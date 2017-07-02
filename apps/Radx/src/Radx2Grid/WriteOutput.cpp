#include "WriteOutput.hh"
#include <Radx/RadxRay.hh>

#include <algorithm>
#include <memory>

#include "cpl_string.h"
#include "gdal_priv.h"
#include "netcdf"
#include <ogr_spatialref.h>

WriteOutput::WriteOutput(const shared_ptr<Cart2Grid> &grid,
                         const shared_ptr<Repository> &store,
                         const Params &params)
    : _grid(grid), _store(store), _params(params)

{}

WriteOutput::~WriteOutput() {}

/////////////////////////////////////////////////////
// write out data

int WriteOutput::writeOutputFile() {

  if (_params.output_format == Params::output_format_t::CF_NETCDF) {
    // Write out netcdf

    std::string outputFileName("ncf_");
    outputFileName += _store->instrumentName;
    outputFileName += "_";
    outputFileName += _store->startDateTime;
    std::replace(outputFileName.begin(), outputFileName.end(), ':', '-');
    outputFileName += ".ncf";
    std::string xDim("x0");
    std::string yDim("y0");
    std::string zDim("z0");
    std::string xCoordinateVar("x0");
    std::string yCoordinateVar("y0");
    // WHERE IS z0
    std::vector<float> xCoordinates;
    std::vector<float> yCoordinates;
    std::string reflName("reflectivity");

    // WHY NOT USE std::iota
    for (int i = 0; i < _grid->getGridDimX(); i++)
      xCoordinates.push_back(_grid->getDMinX() + i * 0.5);
    for (int i = 0; i < _grid->getGridDimY(); i++)
      yCoordinates.push_back(_grid->getDMinY() + i * 0.5);

    // WHY EXPCETION
    try {
      // Open file for writing.
      netCDF::NcFile opFile(outputFileName, netCDF::NcFile::replace);
      // Add dimensions to the file.
      netCDF::NcDim x0Dim = opFile.addDim(xDim, _grid->getGridDimX());
      netCDF::NcDim y0Dim = opFile.addDim(yDim, _grid->getGridDimY());
      netCDF::NcDim z0Dim = opFile.addDim(zDim, _grid->getGridDimZ());

      // define coordinate variables
      netCDF::NcVar x0Var =
          opFile.addVar(xCoordinateVar, netCDF::ncFloat, x0Dim);
      netCDF::NcVar y0Var =
          opFile.addVar(yCoordinateVar, netCDF::ncFloat, y0Dim);

      // write coordinate variable data
      x0Var.putVar(xCoordinates.data());
      y0Var.putVar(yCoordinates.data());

      // Define REF variable
      std::vector<netCDF::NcDim> Refldims;
      Refldims.push_back(x0Dim);
      Refldims.push_back(y0Dim);
      Refldims.push_back(z0Dim);
      netCDF::NcVar nc_field =
          opFile.addVar(reflName, netCDF::ncFloat, Refldims);

      // Add attribute to REF variable
      // netCDF::NcVarAtt refl_att = reflectivity.puttAtt("standard_name",
      // "REF");  refl_att = reflectivity.puttAtt("units", "dbZ");

      // Write REF data
      std::vector<float> field_data;
      auto temp_map = _grid->getOutputFinalGrid()["REF"];
      // WHY COPY IT HERE?
      //      auto temp = *temp_map;

      //      for (auto i = temp.begin(); i != temp.end(); i++)
      //         for(auto j = i->begin(); j != i->end(); j++)
      //      		for(auto k = j->begin(); k != j->end(); k++)
      //        		reflData.push_back( static_cast<float> (*k) );

      // WHY USE push_back RATHER THAN insert
      for (auto i = temp_map->cbegin(); i != temp_map->cend(); i++) {
        for (auto j = i->cbegin(); j != i->cend(); j++) {
          field_data.insert(field_data.end(), j->begin(), j->end());
        }
      }

      nc_field.putVar(field_data.data());

      // Add global Attributes
      netCDF::NcGroupAtt groupAttr =
          opFile.putAtt("instrument_name", _store->instrumentName);
      groupAttr = opFile.putAtt("start_datetime", _store->startDateTime);
      // groupAttr = opFile.putAtt("time_dimension", netCDF::NcType::nc_INT,
      // static_cast<int>(_store->timeDim));  groupAttr =
      // opFile.putAtt("range_dimension", netCDF::NcType::nc_INT,
      // static_cast<int>(_store->rangeDim));  groupAttr =
      // opFile.putAtt("n_points", netCDF::NcType::nc_INT,
      // static_cast<int>(_store->nPoints));
      groupAttr = opFile.putAtt("latitude", netCDF::NcType::nc_FLOAT,
                                static_cast<float>(_store->latitude));
      groupAttr = opFile.putAtt("longitude", netCDF::NcType::nc_FLOAT,
                                static_cast<float>(_store->longitude));
      // groupAttr = opFile.putAtt("altitude_agl", netCDF::NcType::nc_FLOAT,
      // static_cast<float>(_store->altitudeAgl));
      return 0;
    } catch (netCDF::exceptions::NcException &e) {
      e.what();
      return -1;
    }

  } else if (_params.output_format == Params::output_format_t::RASTER) {
    // Write out Raster
    std::cerr << "Raster format block" << std::endl;

    GDALAllRegister();
    const char *pszFormat = "GTiff";
    GDALDriver *poDriver;
    GDALDataset *poDstDS;
    char **papszOptions = NULL;

    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
    if (poDriver == NULL)
      exit(1);

    std::string outputFileName("tif_");
    outputFileName += _store->instrumentName;
    outputFileName += "_";
    outputFileName += _store->startDateTime;
    std::replace(outputFileName.begin(), outputFileName.end(), ':', '-');
    const char *pszDstFilename = outputFileName.c_str();

    poDstDS = poDriver->Create(pszDstFilename, _grid->getGridDimX(),
                               _grid->getGridDimY(), _grid->getGridDimZ(),
                               GDT_Float64, papszOptions);

    std::vector<float> reflData;
    auto temp_map = _grid->getOutputFinalGrid()["REF"];
    auto temp = *temp_map;

    for (auto i = temp.begin(); i != temp.end(); i++)
      for (auto j = i->begin(); j != i->end(); j++) {
        auto k = j->begin();
        reflData.push_back(static_cast<float>(*k));
      }

    GDALRasterBand *rb = poDstDS->GetRasterBand(1);
    CPLErr err =
        rb->RasterIO(GF_Write, 0, 0, _grid->getGridDimX(), _grid->getGridDimY(),
                     reflData.data(), _grid->getGridDimX(),
                     _grid->getGridDimY(), GDT_Float32, 0, 0, NULL);

    OGRSpatialReference::OGRSpatialReference sr;
    // WHY USE INFORMATION FROM PARAMS RATHER THAN FROM NETCDF INPUT?
    // NEVER USE?!
    std::string temp2(
        "+proj=aeqd +lat_0=" + std::to_string(_params.radar_latitude_deg) +
        "lon_0=" + std::to_string(_params.radar_longitude_deg) +
        "+x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
    sr.importFromProj4(temp2.c_str());

    GDALClose((GDALDatasetH)poDstDS);

  } else {
    // Show a warning that outputs format are not supported
    std::cerr << "The output format specified is not supported" << std::endl;
    return -1;
  }

  return 0;
}
