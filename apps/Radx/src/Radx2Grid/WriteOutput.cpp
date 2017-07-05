#include "WriteOutput.hh"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>

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
  return 0;
}
