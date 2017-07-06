
#include "Radx2GridPlus.hh"
#include "Cart2Grid.hh"
#include "Params.hh"
#include "tbb/task_scheduler_init.h"
#include <chrono>
#include <memory>
#include <thread>

Radx2GridPlus::Radx2GridPlus(std::string pName)
  : _programName(pName)
{
}

Radx2GridPlus::~Radx2GridPlus()
{
}

ThreadQueue<std::shared_ptr<PolarDataStream>>
  Radx2GridPlus::polarDataStreamQueue;

ThreadQueue<std::shared_ptr<Cart2Grid>> Radx2GridPlus::gridQueue;

inline long
_currentTimestamp()
{
  std::chrono::microseconds start =
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch());
  return start.count();
}

void
_pushDataintoBuffer(const std::vector<string>& filepaths, const Params& params)
{

  // DONE: Make this across threads
  for (size_t i = 0; i < filepaths.size(); i++) {
    long start_clock = _currentTimestamp();
    // Step 1: Read from netCDF
    auto pds = std::make_shared<PolarDataStream>(filepaths[i], params);
    pds->LoadDataFromNetCDFFilesIntoRepository();
    if (params.debug) {
      std::cerr << "Loading data: "
                << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
                << std::endl;
    }
    Radx2GridPlus::polarDataStreamQueue.push(pds);
  }
}

void
_popDatafromBuffer(int total_size, bool _debug, const Params& params)
{

  for (auto i = 0; i < total_size; i++) {

    auto p = Radx2GridPlus::polarDataStreamQueue.pop();
    // Expand data

    long start_clock = _currentTimestamp();
    p->populateOutputValues();
    if (_debug) {
      std::cerr << "Expanding data: "
                << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
                << std::endl;
    }

    // Calculate Cartesian Coords.
    start_clock = _currentTimestamp();
    auto p2c = std::make_shared<Polar2Cartesian>(p->getRepository());
    p2c->calculateXYZ();
    if (_debug) {
      std::cerr << "Append coordinates: "
                << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
                << std::endl;
    }

    start_clock = _currentTimestamp();
    auto c2g = std::make_shared<Cart2Grid>(p->getRepository(), params);
    c2g->interpGrid();
    if (_debug) {
      std::cerr << "Interp coordinates: "
                << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
                << std::endl;
    }
    Radx2GridPlus::gridQueue.push(c2g);
  }
}

void
_writeToDisk(int total_size, const Params& params)
{
  // TODO: You task
  for (auto i = 0; i < total_size; i++) {
    auto c2g = Radx2GridPlus::gridQueue.pop();
    auto wo = std::make_shared<WriteOutput>(c2g, c2g->getRepository(), params);
    wo->writeOutputFile();
  }
  return;
}

void
Radx2GridPlus::processFiles(const std::vector<string>& filepaths,
                            const Params& params)
{
  _inputDir = params.input_dir;
  _outputDir = params.output_dir;

  // Get total number of files to be allocated
  auto n = filepaths.size();

  std::thread thread_read_nc(_pushDataintoBuffer, filepaths, params);
  std::thread thread_process_polarstream(
    _popDatafromBuffer, n, params.debug > 0, params);
  std::thread thread_write_out(_writeToDisk, n, params);

  thread_read_nc.join();
  thread_process_polarstream.join();
  thread_write_out.join();
}
