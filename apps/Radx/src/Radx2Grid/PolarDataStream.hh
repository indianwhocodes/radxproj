
#ifndef PolarDataStream_h
#define PolarDataStream_h

#define INVALID_DATA -99999.0F

#include "Params.hh"
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct Repository {
  // dimenssions
  int _nPoints;
  int _timeDim;
  int _rangeDim;

  // variables
  float _latitude;
  float _longitude;
  float _altitude;
  float _altitudeAgl;

  float _scalingFactor;
  short _fillValue;
  short __padding0;
  float _addOffset;

  std::vector<float> _gateSize;
  std::vector<int> _rayNGates;
  std::vector<int> _rayStartIndex;
  std::vector<float> _rayStartRange;

  std::vector<float> _azimuth;
  std::vector<float> _elevation;
  std::vector<float> _timeVar;
  std::vector<float> _rangeVar;

  // name change. Accomodate multiple values.
  std::vector<float> _raw_reflectivity;
  std::map<std::string, std::vector<float>> _fields;

  // input file name;
  std::string _inputFile;

  // output values
  std::vector<float> _outElevation;
  std::vector<float> _outAzimuth;
  std::vector<float> _outGate;
  std::vector<float> _outRef;

  // Cartesian Coords
  std::vector<float> _gateDistance;
  std::vector<float> _gateZr;
  std::vector<float> _gateX;
  std::vector<float> _gateY;
  std::vector<float> _gateZ;
  std::vector<float> _gateRoI;
};

class PolarDataStream {
public:
  // constructor & destructor
  PolarDataStream(const std::string &inputFile, const Params &params);
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
  Params _params;

  // class PolarDataStreamImpl;
  // PolarDataStreamImpl* _dataStreamHandle;
};

#endif
