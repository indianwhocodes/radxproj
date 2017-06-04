#ifndef RADX_RADX2GRID_POLARDATASTREAM_H_
#define RADX_RADX2GRID_POLARDATASTREAM_H_

#define INVALID_DATA_F -9999.0F
#define INVALID_DATA -9999.0

#include <map>
#include <vector>

#include <iostream>

#include <string>
#include <memory>

#include <toolsa/TaArray.hh>
#include <Radx/RadxVol.hh>

#include "Interp.hh"
#include "Params.hh"

class RadxFile;
class RadxRay;
class RadxField;

struct RepositoryField
{
  std::vector<float> fieldValues;
  float scaleFactor = 1.0;
  float addOffset = 0.0;
  float fillValue = INVALID_DATA;
};

struct Repository
{
  // dimenssions
  size_t nPoints;
  size_t timeDim;
  size_t rangeDim;

  // variables
  float latitude;
  float longitude;
  float altitude;
  float altitudeAgl;

  float scalingFactor;
  short fillValue;
  short padding0;
  float addOffset;

  std::vector<float> gateSize;
  std::vector<int> rayNGates;
  std::vector<int> rayStartIndex;
  std::vector<float> rayStartRange;

  std::vector<float> azimuth;
  std::vector<float> elevation;
  std::vector<float> timeVar;
  std::vector<float> rangeVar;

  // name change. Accomodate multiple values.
  std::vector<float> rawReflectivity;
  std::map<std::string, shared_ptr<RepositoryField>> inFields;

  // input file name;
  std::string inputFile;

  // output values
  std::vector<double> outElevation;
  std::vector<double> outAzimuth;
  std::vector<double> outGate;
  std::map<std::string, shared_ptr<std::vector<double>>> outFields;

  // Cartesian Coords
  std::vector<double> gateGroundDistance;
  std::vector<double> gateZr;
  std::vector<double> gateX;
  std::vector<double> gateY;
  std::vector<double> gateZ;
  std::vector<double> gateRoI;
};

class PolarDataStream
{
public:
  // constructor & destructor
  PolarDataStream(const std::string& inputFile, const Params& params);
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
  RadxVol& getRadxVol();
  vector<Interp::Field>& getInterpFields();

  int readFile(const string& filePath);     
  void setupRead(RadxFile& file);           

  bool isRhi();     //TODO: Copy isRhi method definition from Radx2Grid.cc                              

private:
  std::shared_ptr<Repository> _store;
  const Params& _params;

  RadxVol _readVol;
  vector<string> _readPaths;
  bool _rhiMode;
  vector<Interp::Field> _interpFields;
  vector<Interp::Ray*> _interpRays;

  int _volNum;
};

#endif //RADX_RADX2GRID_POLARDATASTREAM_H_
