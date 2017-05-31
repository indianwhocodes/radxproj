
#ifndef PolarDataStream_h
#define PolarDataStream_h

#define INVALID_DATA -9999.0F

#include "Interp.hh"
#include "Params.hh"
#include <Radx/RadxVol.hh>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <toolsa/TaArray.hh>
#include <vector>

class RadxFile;
class RadxRay;
class RadxField;

struct RepositoryField {
  std::vector<float> fieldValues;
  float scale_factor = 1.0;
  float add_offset = 0.0;
  float fill_value = INVALID_DATA;
};

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
  std::map<std::string, shared_ptr<RepositoryField>> _inFields;

  // input file name;
  std::string _inputFile;

  // output values
  std::vector<float> _outElevation;
  std::vector<float> _outAzimuth;
  std::vector<float> _outGate;
  std::map<std::string, shared_ptr<std::vector<float>>> _outFields;

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

  std::shared_ptr<Repository> getRepository();
  RadxVol &getRadxVol();
  vector<Interp::Field> &getInterpFields();

  int _readFile(const string &filePath);
  void _setupRead(RadxFile &file);

  void _setupTransformFields();
  void _addTestAndCoverageInputFields();
  void _addBoundingSweeps();
  void _initInterpFields();
  void _loadInterpRays();
  void _checkFields(const string &filePath);
  bool _isRhi();
  void _censorInterpRay(Interp::Ray *interpRay);

private:
  std::shared_ptr<Repository> _store;
  const Params &_params;

  RadxVol _readVol;
  vector<string> _readPaths;
  bool _rhiMode;
  vector<Interp::Field> _interpFields;
  vector<Interp::Ray *> _interpRays;

  int _volNum;

  // censoring
  int _nWarnCensorPrint;

  // class PolarDataStreamImpl;
  // PolarDataStreamImpl* _dataStreamHandle;
};

#endif
