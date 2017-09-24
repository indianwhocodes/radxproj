
#include "Radx2GridPlus.hh"
#include "Cart2Grid.hh"
#include "Params.hh"
#include "tbb/task_scheduler_init.h"
#include <chrono>
#include <memory>
#include <thread>

/*Define the static variable to solve undefined reference errors.*/
int Radx2GridPlus::numberOfCores;

Radx2GridPlus::Radx2GridPlus(std::string pName)
  : _programName(pName) 
{
  SetupThreadControl();
}

Radx2GridPlus::~Radx2GridPlus()
{}

ThreadQueue<std::shared_ptr<PolarDataStream>>
  Radx2GridPlus::polarDataStreamQueue;

ThreadQueue<std::shared_ptr<Cart2Grid>> Radx2GridPlus::gridQueue;

void
Radx2GridPlus::SetupThreadControl()
{
  // If the user wants to control the number of threads, s/he will set the
  // TBB_NUM_THREADS environment variable. 
  bool flag = false;
  
  if(const char* tbb_num_thread = std::getenv("TBB_NUM_THREADS"))
  {
    //set the number of threads to the value of this env_var after validating value
    for(int i = 0; i < strlen(tbb_num_thread); i++){
      //ASCII value of 0 = 48, 9 = 57. So if value is outside of numeric range then fail
      if (tbb_num_thread[i] < 48 || tbb_num_thread[i] > 57){
        flag = true;
        break;
      }
    }
    if(!flag){
      try{
        std::string temp(tbb_num_thread);
        numberOfCores = std::stoi(temp);
      }
      catch(std::exception const & e){
        std::cerr<<"error reading TBB_NUM_THREADS: " << e.what() <<std::endl;
        numberOfCores = std::thread::hardware_concurrency();
      } 
    }
    else
    {
      numberOfCores = std::thread::hardware_concurrency();
    }
  }
  else
  {
    numberOfCores = std::thread::hardware_concurrency();
  }
  std::cerr << "No. of threads: " << numberOfCores << std::endl;
}


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
    p->populateOutputValues(Radx2GridPlus::numberOfCores);
    if (_debug) {
      std::cerr << "Expanding data: "
                << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
                << std::endl;
    }

    // Calculate Cartesian Coords.
    start_clock = _currentTimestamp();
    auto p2c = std::make_shared<Polar2Cartesian>(p->getRepository());
    p2c->calculateXYZ(Radx2GridPlus::numberOfCores);
    if (_debug) {
      std::cerr << "Append coordinates: "
                << (_currentTimestamp() - start_clock) / 1.0E6 << " sec"
                << std::endl;
    }

    start_clock = _currentTimestamp();
    auto c2g = std::make_shared<Cart2Grid>(p->getRepository(), params, Radx2GridPlus::numberOfCores);
    c2g->interpGrid(Radx2GridPlus::numberOfCores);
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

  //set up number of threads to be created
  if(numberOfCores > 0) 
  {
    tbb::task_scheduler_init init(numberOfCores); 
  }
  else
  {
    std::cout << "1" << std::endl;
    tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);
  }
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
