
#ifndef PolarDataStream_h
#define PolarDataStream_h

#define INVALID_DATA -99999.0F

#include <iostream>
#include <string>
#include <vector>

struct Repository;
class PolarDataStream {
public:
  // constructor & destructor
  PolarDataStream(const std::string &inputFile);
  ~PolarDataStream();

  // read variables from NetCDF files
  void LoadDataFromNetCDFFilesIntoRepository();

  // populate the output values
  void populateOutputValues();

  // setter, getter
  std::vector<float> getOutElevation();
  std::vector<float> getOutAzimuth();
  std::vector<float> getOutGate();
  std::vector<float> getOutRef();

  Repository *getRepository();

private:
  Repository *_store;

  // class PolarDataStreamImpl;
  // PolarDataStreamImpl* _dataStreamHandle;
};

#endif
