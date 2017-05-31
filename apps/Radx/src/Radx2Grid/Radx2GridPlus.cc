
#include "Radx2GridPlus.hh"
#include "Params.hh"
#include <chrono>
#include <memory>
#include <thread>

Radx2GridPlus::Radx2GridPlus(std::string pName) { _programName = pName; }

Radx2GridPlus::~Radx2GridPlus() {}

ThreadQueue<std::shared_ptr<PolarDataStream>>
    Radx2GridPlus::polarDataStreamQueue;

long _currentTimestamp() {
  std::chrono::microseconds start =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch());
  return start.count();
}

void _pushDataintoBuffer(const std::vector<string> &filepaths,
                         const Params &params) {

  // DONE: Make this across threads
  for (size_t i = 0; i < filepaths.size(); i++) {
    long start_clock = _currentTimestamp();
    // Step 1: Read from netCDF
    auto pds = std::make_shared<PolarDataStream>(filepaths[i], params);
    pds->LoadDataFromNetCDFFilesIntoRepository();
    std::cerr << "Loading data: " << (_currentTimestamp() - start_clock) / 1.0E6
              << " sec" << std::endl;
    Radx2GridPlus::polarDataStreamQueue.push(pds);
  }
}

void _popDatafromBuffer(size_t total_size) {

  for (auto i = 0; i < total_size; i++) {
    auto p = Radx2GridPlus::polarDataStreamQueue.pop();
    long start_clock = _currentTimestamp();
    p->populateOutputValues();
    std::cerr << "Expanding data: "
              << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
              << std::endl;
    p.reset();
  }
}

void Radx2GridPlus::processFiles(const std::vector<string> &filepaths,
                                 const Params &params) {
  _inputDir = params.input_dir;
  _outputDir = params.output_dir;

  // Get total number of files to be allocated
  auto n = filepaths.size();

  // Why we need these vectors on heap? Because we may pass them to another
  // thread!
  // std::vector<PolarDataStream> pds{};
  // pds.reserve(n);
  // auto cg = std::make_shared<std::vector<Cartesian2Grid>>();
  // auto p2c = std::make_shard<std::vector<Polar2Cartesian>>();
  // auto wo = std::make_shared<std::vector<WriteOutput>>();

  std::thread thread_read_nc(_pushDataintoBuffer, filepaths, params);
  std::thread thread_process_polarstream(_popDatafromBuffer, n);

  thread_read_nc.join();
  thread_process_polarstream.join();

  // for (size_t i = 0; i < filepaths.size(); i++) {
  // pds.push_back(PolarDataStream(filepaths[i], params));
  // p2c->at(i) = Polar2Cartesian(pds->at(i).getRepository());
  // cg->at(i) = Cartesian2Grid(pds->at(i).getRepository());
  // wo->at(i) = WriteOutput(_programName, pds->at(i).getRepository(),
  // pds->at(i).getRadxVol(), params, pds->at(i).getInterpFields());
  //}

  // step 2 : PolarDataStream to Cartesian coordinates
  // p2c->at(i).calculateCartesianCoords();

  // step 3 : Gridding
  // cg->at(i).calculateGridSize(params);
  // cg->at(i).calculateRefGrid();

  // step 4 : Output file
  // wo->at(i).writeOutputFile();
}

std::string Radx2GridPlus::getInputDir() { throw "Not Implemented"; }

std::string Radx2GridPlus::getOutputDir() { throw "Not Implemented"; }
