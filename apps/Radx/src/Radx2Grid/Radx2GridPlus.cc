
#include "Radx2GridPlus.hh"
#include "Params.hh"
#include "tbb/task_group.h"
#include "tbb/tbb.h"
#include <ctime>

Radx2GridPlus::Radx2GridPlus(std::string pName) { _programName = pName; }

Radx2GridPlus::~Radx2GridPlus() {}

void Radx2GridPlus::processFiles(const vector<string> &filepaths,
                                 const Params &params) {
  _inputDir = params.input_dir;
  _outputDir = params.output_dir;

  auto pds = std::make_shared<std::vector<PolarDataStream>>();
  auto cg = std::make_shared<std::vector<Cartesian2Grid>>();
  auto p2c = std::make_shared<std::vector<Polar2Cartesian>>();

  for (size_t i = 0; i < filepaths.size(); i++) {
    pds->push_back(PolarDataStream(filepaths[i]));
    p2c->push_back(Polar2Cartesian(pds->at(i).getRepository()));
    cg->push_back(Cartesian2Grid(pds->at(i).getRepository()));
  }

  clock_t start = clock();
  for (size_t i = 0; i < filepaths.size(); i++) {
    pds->at(i).LoadDataFromNetCDFFilesIntoRepository();
    pds->at(i).populateOutputValues();

    // step 2 : PolarDataStream to Cartesian coordinates
    p2c->at(i).calculateCartesianCoords();

    // step 3 : Gridding
    cg->at(i).calculateGridSize(params);
    cg->at(i).calculateRefGrid();
  }
  std::cout << "Serial time" << clock() - start << std::endl;

  auto n = filepaths.size();
  start = clock();
  tbb::parallel_for(size_t(0), n, [&](size_t i) {
    pds->at(i).LoadDataFromNetCDFFilesIntoRepository();
    pds->at(i).populateOutputValues();

    // step 2 : PolarDataStream to Cartesian coordinates
    p2c->at(i).calculateCartesianCoords();

    // step 3 : Gridding
    cg->at(i).calculateGridSize(params);
    cg->at(i).calculateRefGrid();

    // std::cout<<filepaths[i]<<" completed!"<<std::endl;
  });

  std::cout << "Parallel time" << clock() - start << std::endl;
}

std::string Radx2GridPlus::getInputDir() { throw "Not Implemented"; }

std::string Radx2GridPlus::getOutputDir() { throw "Not Implemented"; }
