#include "WriteOutput.hh"
#include <Radx/RadxRay.hh>
#include <algorithm>
#include <memory>
#include "netcdf"
#include "gdal_priv.h"
#include "cpl_string.h"

WriteOutput::WriteOutput(const shared_ptr<Cart2Grid>& grid, 
						 const shared_ptr<Repository>& store,
                         const Params& params)
  : _grid(grid)
  , _store(store)
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
  
  if (_params.output_format == Params::output_format_t::CF_NETCDF) {
    // Write out netcdf
    
    std::string outputFileName("ncf_");
    outputFileName += _store->instrumentName;
    outputFileName += "_";
    outputFileName += _store->startDateTime;
    std::replace(outputFileName.begin(), outputFileName.end(), ':', '-');
    outputFileName += ".ncf";
    try
    {
      netCDF::NcFile opFile(outputFileName, netCDF::NcFile::replace);
      netCDF::NcGroupAtt groupAttr = opFile.putAtt("instrument_name", _store->instrumentName);
	  groupAttr = opFile.putAtt("start_datetime", _store->startDateTime);
      groupAttr = opFile.putAtt("time_dimension", netCDF::NcType::nc_INT, static_cast<int>(_store->timeDim));
      groupAttr = opFile.putAtt("range_dimension", netCDF::NcType::nc_INT, static_cast<int>(_store->rangeDim));
      groupAttr = opFile.putAtt("n_points", netCDF::NcType::nc_INT, static_cast<int>(_store->nPoints));
      groupAttr = opFile.putAtt("latitude", netCDF::NcType::nc_FLOAT, static_cast<float>(_store->latitude));
      groupAttr = opFile.putAtt("longitude", netCDF::NcType::nc_FLOAT, static_cast<float>(_store->longitude));
      groupAttr = opFile.putAtt("altitude_agl", netCDF::NcType::nc_FLOAT, static_cast<float>(_store->altitudeAgl));
      
      return 0;
    }
    catch(netCDF::exceptions::NcException& e)
    {
      e.what();
      return -1;
    }
    
    
  } else if (_params.output_format == Params::output_format_t::RASTER) {
    // Write out Raster
      GDALDataset  *poDataset;
      GDALAllRegister();
      const char *pszFormat = "GTiff";
      GDALDriver *poDriver;
      char **papszMetadata;
      poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
      if( poDriver == NULL )
        exit( 1 );
      papszMetadata = poDriver->GetMetadata();
      if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) )
        printf( "Driver %s supports Create() method.\n", pszFormat );
      if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) )
        printf( "Driver %s supports CreateCopy() method.\n", pszFormat );
	


  } else {
    // Show a warning that outputs format are not supported
    std::cerr<< "The output format specified is not supported" << std::endl; 
    return -1;
  }

  return 0;
}
