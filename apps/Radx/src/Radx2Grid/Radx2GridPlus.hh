
#ifndef Radx2GridPlus_h
#define Radx2GridPlus_h

#include "Cartesian2Grid.hh"
#include "Polar2Cartesian.hh"
#include "PolarDataStream.hh"
#include "ThreadQueue.hh"
#include <string>

class Radx2GridPlus {
public:
  Radx2GridPlus(std::string pName);
  ~Radx2GridPlus();
  void processFiles(const vector<string> &filepaths, const Params &params);

  // setter, getter
  std::string getInputDir();
  std::string getOutputDir();

  // TODO: make it a vector of string... for parallel processing.
private:
  std::string _programName;
  std::string _inputDir;
  std::string _outputDir;

public:
  static ThreadQueue<std::shared_ptr<PolarDataStream>> polarDataStreamQueue;
};

#endif
