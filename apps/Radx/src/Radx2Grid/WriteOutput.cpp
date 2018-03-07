#include "WriteOutput.hh"

#include <algorithm>
#include <memory>
#include <sstream>
#include <fstream>
#include "netcdf"

using namespace std;


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

			//Write field data
			std::vector<float> field_data;
			for (auto i = field.second->cbegin(); i != field.second->cend(); i++) {
				for (auto j = i->cbegin(); j != i->cend(); j++) {
					field_data.insert(field_data.end(), j->begin(), j->end());
				}
			}

			nc_field.putVar(field_data.data());
		}


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

		// Construct projection WKT;
		stringstream prj_writer;
		prj_writer << "PROJCS[\"World_Azimuthal_Equidistant\",";
		prj_writer << "GEOGCS[\"GCS_WGS_1984\",";
		prj_writer << "DATUM[\"D_WGS_1984\",";
		prj_writer << "SPHEROID[\"WGS_1984\",6378137,298.257223563]],";
		prj_writer << "PRIMEM[\"Greenwich\",0],";
		prj_writer << "UNIT[\"Degree\",0.017453292519943295]],";
		prj_writer << "PROJECTION[\"Azimuthal_Equidistant\"],";
		prj_writer << "PARAMETER[\"False_Easting\",0],";
		prj_writer << "PARAMETER[\"False_Northing\",0],";
		prj_writer << "PARAMETER[\"Central_Meridian\"," << _store->longitude << "],";
		prj_writer << "PARAMETER[\"Latitude_Of_Origin\"," << _store->latitude << "],";
		prj_writer << "UNIT[\"Meter\",1]]";
		prj_writer.flush();

		std::string outputFileName("grd_");
		outputFileName += _store->instrumentName;
		outputFileName += "_";
		outputFileName += _store->startDateTime;
		std::replace(outputFileName.begin(), outputFileName.end(), ':', '_');

		for (const auto& kv : _grid->getOutputFinalGrid()) {
			string field_name = kv.first;
			double z0 = _params.grid_z_geom.minz;
			for (int z = 0; z < _grid->getGridDimZ(); z++) {
				double h =
					(z0 + z * _params.grid_z_geom.dz) * 1000.0; // Convert to meters
				char h_cstr[6];
				sprintf(h_cstr, "%05.0f", h);
				string h_str(h_cstr);
				// Write projection file
				ofstream prj_file;
				prj_file.open(outputFileName + "_" + h_str + "_" + field_name + ".prj",
						std::ios::trunc);
				prj_file << prj_writer.str();
				prj_file.flush();
				prj_file.close();

				// Write grid file
				ofstream grd_file;
				grd_file.open(outputFileName + "_" + h_str + "_" + field_name + ".asc",
						std::ios::trunc);
				auto ref_grid = kv.second;
				grd_file << "NCOLS " << _params.grid_xy_geom.nx << std::endl;
				grd_file << "NROWS " << _params.grid_xy_geom.ny << std::endl;
				grd_file << "XLLCENTER " << _params.grid_xy_geom.minx * 1000.0
					<< std::endl;
				grd_file << "YLLCENTER " << _params.grid_xy_geom.miny * 1000.0
					<< std::endl;
				grd_file << "CELLSIZE " << _params.grid_xy_geom.dx * 1000.0 << std::endl;
				grd_file << "NODATA_VALUE " << INVALID_DATA << std::endl;

				for (int j = _grid->getGridDimY() - 1; j >= 0; --j) {
					for (int i = 0; i < _grid->getGridDimX(); i++) {
						grd_file << (*ref_grid)[i][j][z] << " ";
					}
					grd_file << std::endl;
				}
				grd_file.close();
			}
		}

	} else {
		// Show a warning that outputs format are not supported
		std::cerr << "The output format specified is not supported" << std::endl;
		return -1;
	}

	return 0;
}
