#include "WriteOutput.hh"

#include <algorithm>
#include <memory>
#include "netcdf"
//#include "gdal_priv.h"
//#include "cpl_string.h"
//#include "gdal_priv.h"
//#include "ogr_spatialref.h"


#include "netcdf"

<<<<<<< HEAD
WriteOutput::WriteOutput(const shared_ptr<Cart2Grid>& grid, 
			 const shared_ptr<Repository>& store,
=======
WriteOutput::WriteOutput(const shared_ptr<Cart2Grid>& grid,
                         const shared_ptr<Repository>& store,
>>>>>>> 9528541a7cea8260d9a52c52fbdf50b7f8f99e7d
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
    std::cout << outputFileName << std::endl;
    std::string xDim("x0");
    std::string yDim("y0");
    std::string zDim("z0");
    std::string xCoordinateVar("x0");
    std::string yCoordinateVar("y0");
    std::string zCoordinateVar("z0");
    
    std::vector<float> xCoordinates;
    std::vector<float> yCoordinates;
    std::vector<float> zCoordinates;
    //TODO: Remove this hard coded string. std::string reflName("reflectivity");

    // WHY NOT USE std::iota? 
    // It increments only by 1. If we really want, we can implement our own version of
    // iota that takes another parameter called "stride".  
    for (int i = 0; i < _grid->getGridDimX(); i++)
      xCoordinates.push_back(_grid->getDMinX() + i * 0.5);
    for (int i = 0; i < _grid->getGridDimY(); i++)
      yCoordinates.push_back(_grid->getDMinY() + i * 0.5);
    for (int i = 0; i < _grid->getGridDimZ(); i++)
      zCoordinates.push_back(_grid->getDMinZ() + i * 0.5);

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
    netCDF::NcVar z0Var =
      opFile.addVar(zCoordinateVar, netCDF::ncFloat, z0Dim);

    // write coordinate variable data
    x0Var.putVar(xCoordinates.data());
    y0Var.putVar(yCoordinates.data());
    z0Var.putVar(zCoordinates.data());
    
    for (auto const& field : _grid->getOutputFinalGrid())
    {
      //Define field Variable
      std::vector<netCDF::NcDim> fieldDim;
      fieldDim.push_back(x0Dim);
      fieldDim.push_back(y0Dim);
      fieldDim.push_back(z0Dim);
      
      netCDF::NcVar nc_field =
      opFile.addVar(field.first, netCDF::ncFloat, fieldDim);
      
<<<<<<< HEAD
      //Define REF variable
      std::vector<netCDF::NcDim> Refldims;
      Refldims.push_back(x0Dim);
      Refldims.push_back(y0Dim);
      Refldims.push_back(z0Dim); 
      netCDF::NcVar reflectivity = opFile.addVar(reflName, netCDF::ncFloat, Refldims);
      
      //Add attribute to REF variable
      //netCDF::NcVarAtt refl_att = reflectivity.puttAtt("standard_name", "REF");
      //refl_att = reflectivity.puttAtt("units", "dbZ");
      
      //Write REF data
      std::vector<float> reflData;
      auto temp_map = _grid->getOutputFinalGrid()["REF"];
      auto temp = *temp_map;

      
      for (auto i = temp.begin(); i != temp.end(); i++)
	
         for(auto j = i->begin(); j != i->end(); j++)
      		for(auto k = j->begin(); k != j->end(); k++)
        		reflData.push_back( static_cast<float> (*k) );
	

=======
      //Write field data
      std::vector<float> field_data;
      for (auto i = field.second->cbegin(); i != field.second->cend(); i++) {
        for (auto j = i->cbegin(); j != i->cend(); j++) {
          field_data.insert(field_data.end(), j->begin(), j->end());
        }
      }
>>>>>>> 9528541a7cea8260d9a52c52fbdf50b7f8f99e7d

      nc_field.putVar(field_data.data());
    }


    
    /*
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
    */
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
    groupAttr = opFile.putAtt("latitude",
                              netCDF::NcType::nc_FLOAT,
                              static_cast<float>(_store->latitude));
    groupAttr = opFile.putAtt("longitude",
                              netCDF::NcType::nc_FLOAT,
                              static_cast<float>(_store->longitude));
    // groupAttr = opFile.putAtt("altitude_agl", netCDF::NcType::nc_FLOAT,
    // static_cast<float>(_store->altitudeAgl));
    return 0;
    

  } else if (_params.output_format == Params::output_format_t::RASTER) {
<<<<<<< HEAD
      //Write out Raster

      
      std::cerr<< "Raster format block" << std::endl; 
     
      //Driver Initializaion	
      GDALAllRegister();
      const char *pszFormat = "GTiff";
      GDALDriver *poDriver;
      poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
      if( poDriver == NULL )
        exit( 1 );


      //null value initialization
      GDALDataset *poDstDS;
      char **papszOptions = NULL;
      OGRSpatialReference oSRS;
      char *pszSRS_WKT = NULL;
      GDALRasterBand *poBand;

            
      std::string outputFileName("tif_");
      outputFileName += _store->instrumentName;
      outputFileName += "_";
      outputFileName += _store->startDateTime;

      Params::grid_xy_geom_t tempXY = _grid->getStructXYGeom();
      Params::grid_z_geom_t tempZ= _grid->getStructZGeom();

      map<string, ptr_vector3d<double>> temp_OPFinalGrid = 
                                        _grid->getOutputFinalGrid();	
      for (auto const& x : temp_OPFinalGrid) 
      {
        outputFileName += "_";	
	    outputFileName += x.first;
	    std::replace(outputFileName.begin(), outputFileName.end(), ':', '-');
      	const char * pszDstFilename = outputFileName.c_str();	


	   //Create a new dataset
        poDstDS = poDriver->Create( pszDstFilename,
                                    _grid->getGridDimX(),
                                    _grid->getGridDimY(),
                                    _grid->getGridDimZ(),
                                    GDT_Float32,
                                    papszOptions );
	
        //Setting various options
       
       double adfGeoTransform[6] = { tempXY.minx, 
                                     tempXY.dx,
                                     0, 
                                     tempXY.miny+(tempXY.ny * tempXY.dy),
                                     0, 
                                     tempXY.dy };
       poDstDS->SetGeoTransform( adfGeoTransform ); 
       
       oSRS.SetUTM( 11, TRUE );
       oSRS.SetWellKnownGeogCS( "NAD27" );
       oSRS.exportToWkt( &pszSRS_WKT );
       poDstDS->SetProjection( pszSRS_WKT );
       CPLFree( pszSRS_WKT );
	
       

       //Write the metadata
       poDstDS->SetMetadataItem( "instrument_name", _store->instrumentName.c_str(), nullptr );
       poDstDS->SetMetadataItem( "start_datetime", _store->startDateTime.c_str(), nullptr );
       poDstDS->SetMetadataItem( "latitude", to_string(_store->latitude).c_str(), nullptr );
       poDstDS->SetMetadataItem( "longitude", to_string(_store->longitude).c_str(), nullptr );
      


	
       auto temp_grid = x.second;

       //Writing Raster Band
       int nbands = _grid->getGridDimZ();
       std::vector<float> abyRaster;
       
       int z = 0;
       for( int bands = 1; bands <= nbands ; bands++)
       {
         poBand = poDstDS->GetRasterBand(bands);
	     for (int j = _grid->getGridDimY() - 1; j >= 0; --j) {
           for (int i = 0; i < _grid->getGridDimX(); i++) {
              abyRaster.push_back( (*temp_grid)[i][j][z] );
           }
         }
         z++;
  	     
         poBand->RasterIO( GF_Write, 
			               0,
			               0,
               			   _grid->getGridDimX(),
                                       _grid->getGridDimY(),
                                       abyRaster.data(),
			               _grid->getGridDimX(),
			               _grid->getGridDimY(),
			               GDT_Float32,
			               0,
			               0  );

	     abyRaster.clear();
        
       }
       

       std::string temp2("+proj=aeqd +lat_0="+ 
			std::to_string(_store->latitude) + 
                        "lon_0=" + std::to_string(_store->longitude) + 
			"+x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");	
       oSRS.importFromProj4(temp2.c_str());

       GDALClose((GDALDatasetH)poDstDS);
	      
	
      }


=======
    // Write out Raster

      /*
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
	    
    std::cerr << "Raster format block" << std::endl;

    GDALAllRegister();
    const char* pszFormat = "GTiff";
    GDALDriver* poDriver;
    GDALDataset* poDstDS;
    char** papszOptions = NULL;

    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
    if (poDriver == NULL)
      exit(1);

    std::string outputFileName("tif_");
    outputFileName += _store->instrumentName;
    outputFileName += "_";
    outputFileName += _store->startDateTime;
    std::replace(outputFileName.begin(), outputFileName.end(), ':', '-');
    const char* pszDstFilename = outputFileName.c_str();

    poDstDS = poDriver->Create(pszDstFilename,
                               _grid->getGridDimX(),
                               _grid->getGridDimY(),
                               _grid->getGridDimZ(),
                               GDT_Float64,
                               papszOptions);

    std::vector<float> reflData;
    auto temp_map = _grid->getOutputFinalGrid()["REF"];
    auto temp = *temp_map;

    for (auto i = temp.begin(); i != temp.end(); i++)
      for (auto j = i->begin(); j != i->end(); j++) {
        auto k = j->begin();
        reflData.push_back(static_cast<float>(*k));
      }

    GDALRasterBand* rb = poDstDS->GetRasterBand(1);
    CPLErr err = rb->RasterIO(GF_Write,
                              0,
                              0,
                              _grid->getGridDimX(),
                              _grid->getGridDimY(),
                              reflData.data(),
                              _grid->getGridDimX(),
                              _grid->getGridDimY(),
                              GDT_Float32,
                              0,
                              0,
                              NULL);

    OGRSpatialReference sr;
    // WHY USE INFORMATION FROM PARAMS RATHER THAN FROM NETCDF INPUT?
    // NEVER USE?!
    std::string temp2(
      "+proj=aeqd +lat_0=" + std::to_string(_params.radar_latitude_deg) +
      "lon_0=" + std::to_string(_params.radar_longitude_deg) +
      "+x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs");
    sr.importFromProj4(temp2.c_str());

    GDALClose((GDALDatasetH)poDstDS);

  */
>>>>>>> 9528541a7cea8260d9a52c52fbdf50b7f8f99e7d
  } else {
    // Show a warning that outputs format are not supported
    std::cerr << "The output format specified is not supported" << std::endl;
    return -1;
  }

  return 0;
}
