
#include "Radx2GridPlus.hh"
#include "Params.hh"
#include <ctime>
#include <memory>

Radx2GridPlus::Radx2GridPlus(std::string pName) { _programName = pName; }

Radx2GridPlus::~Radx2GridPlus() {}

ThreadQueue<PolarDataStream> Radx2GridPlus::polarDataStreamQueue;

// ThreadQueue<PolarDataStream> Radx2GridPlus::polarDataStreamQueue();

void Radx2GridPlus::processFiles(const vector<string> &filepaths,
                                 const Params &params) {
  _inputDir = params.input_dir;
  _outputDir = params.output_dir;

  // Get total number of files to be allocated
  auto n = filepaths.size();

  // Why we need these vectors on heap? Because we may pass them to another
  // thread!
  auto pds = std::make_shared<std::vector<PolarDataStream>>();
  pds->reserve(n);
  // auto cg = std::make_shared<std::vector<Cartesian2Grid>>();
  // auto p2c = std::make_shared<std::vector<Polar2Cartesian>>();

  for (size_t i = 0; i < filepaths.size(); i++) {
    pds->push_back(PolarDataStream(filepaths[i], params));
    // p2c->at(i) = Polar2Cartesian(pds->at(i).getRepository());
    // cg->at(i) = Cartesian2Grid(pds->at(i).getRepository());
  }

  clock_t start;
  // TODO: Make this across threads
  for (size_t i = 0; i < filepaths.size(); i++) {
    start = clock();
    // Step 1: Read from netCDF
    pds->at(i).LoadDataFromNetCDFFilesIntoRepository();
    polarDataStreamQueue.push(pds->at(i));
    std::cerr << "Loading data: " << 1.0 * (clock() - start) / CLOCKS_PER_SEC
              << std::endl;

    // Step 2: Expand tables. It should be in another threads
    start = clock();
    // pds->at(i).populateOutputValues();
    auto p = polarDataStreamQueue.pop();
    p.populateOutputValues();
    std::cerr << "Expanding data: " << 1.0 * (clock() - start) / CLOCKS_PER_SEC
              << std::endl;

    // step 2 : PolarDataStream to Cartesian coordinates
    // p2c->at(i).calculateCartesianCoords();

    // step 3 : Gridding
    // cg->at(i).calculateGridSize(params);
    // cg->at(i).calculateRefGrid();
  }
}

std::string Radx2GridPlus::getInputDir() { throw "Not Implemented"; }

std::string Radx2GridPlus::getOutputDir() { throw "Not Implemented"; }
