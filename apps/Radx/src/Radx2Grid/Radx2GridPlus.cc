
#include "Radx2GridPlus.hh"
#include "Params.hh"
#include "tbb/task_group.h"

Radx2GridPlus::Radx2GridPlus(std::string pName) { _programName = pName; }

Radx2GridPlus::~Radx2GridPlus() {}

void Radx2GridPlus::processFiles(const vector<string> &filepaths,
                                 const Params &params) {
  _inputDir = params.input_dir;
  _outputDir = params.output_dir;
  std::string tempFile = "test file name";

  // TODO: use vectors to make a tbb::parallel_for.
  //    std::vector<PolarDataStream> pds(filepaths.size());
  //    std::vector<Cartesian2Grid> cg(filepaths.size())

  // step 1 : Convert netcdf file to PolarDataStream
  // loop the number of files

  //  PolarDataStream pds(fileNames[0]);
  PolarDataStream pds(tempFile);
  pds.LoadDataFromNetCDFFilesIntoRepository();
  pds.populateOutputValues();

  // step 2 : PolarDataStream to Cartesian coordinates
  Polar2Cartesian pc(pds.getRepository());
  pc.calculateCartesianCoords();

  // step 3 : Gridding
  Cartesian2Grid cg(pds.getRepository());
  cg.calculateGridSize(params);
  cg.calculateRefGrid();

  // step 4 :  Writing the grids to MDV File
}

std::string Radx2GridPlus::getInputDir() { throw "Not Implemented"; }

std::string Radx2GridPlus::getOutputDir() { throw "Not Implemented"; }
