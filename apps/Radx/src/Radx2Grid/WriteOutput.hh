#ifndef WriteOutput_h
#define WriteOutput_h

#include "Interp.hh"
#include "PolarDataStream.hh"
#include <Radx/Radx.hh>
#include <memory>
#include <vector>

const fl32 missingFl32 = -9999.0;

class WriteOutput
{
public:
  // constructor & destructor
  WriteOutput(std::string progN,
              std::shared_ptr<Repository> store,
              RadxVol& readVol,
              const Params& params,
              vector<Interp::Field>& interpFields);
  ~WriteOutput();

  int writeOutputFile();
  void _createTestFields();
  void _createConvStratFields();
  void _initGrid();
  void _initZLevels();
  void _initProjection();
  void _computeGridRelative();
  int _setRadarParams();

  int testWriteOutputFile(fl32** outputFields);

protected:
  // class for debug fields
  class DerivedField
  {
  public:
    string name;
    string longName;
    string units;
    vector<double> vertLevels;
    fl32* data;
    bool writeToFile;
    DerivedField(const string& nameStr,
                 const string& longNameStr,
                 const string& unitsStr,
                 bool writeOut)
      : name(nameStr)
      , longName(longNameStr)
      , units(unitsStr)
      , data(NULL)
      , writeToFile(writeOut)
      , _nGrid(0)
    {
    }
    ~DerivedField()
    {
      if (data) {
        delete[] data;
      }
    }
    void alloc(size_t nGrid, const vector<double>& zLevels)
    {
      vertLevels = zLevels;
      if (nGrid == _nGrid) {
        return;
      }
      if (data) {
        delete[] data;
      }
      data = new fl32[nGrid];
      _nGrid = nGrid;
      for (size_t ii = 0; ii < _nGrid; ii++) {
        data[ii] = missingFl32;
      }
    }
    void setToZero()
    {
      for (size_t ii = 0; ii < _nGrid; ii++) {
        data[ii] = 0.0;
      }
    }

  private:
    size_t _nGrid;
  };

private:
  std::string _progName;
  std::shared_ptr<Repository> _store;
  RadxVol& _readVol;
  const Params& _params;
  MdvxProj _proj;
  vector<double> _gridZLevels;

  vector<Interp::Field>& _interpFields;

  DerivedField* _nContribDebug;
  DerivedField* _gridAzDebug;
  DerivedField* _gridElDebug;
  DerivedField* _gridRangeDebug;
  DerivedField* _llElDebug;
  DerivedField* _llAzDebug;
  DerivedField* _lrElDebug;
  DerivedField* _lrAzDebug;
  DerivedField* _ulElDebug;
  DerivedField* _ulAzDebug;
  DerivedField* _urElDebug;
  DerivedField* _urAzDebug;

  vector<DerivedField*> _derived3DFields;
  vector<DerivedField*> _derived2DFields;

  DerivedField* _convStratDbzMax;
  DerivedField* _convStratDbzCount;
  DerivedField* _convStratDbzSum;
  DerivedField* _convStratDbzSqSum;
  DerivedField* _convStratDbzSqSqSum;
  DerivedField* _convStratDbzTexture;
  DerivedField* _convStratFilledTexture;
  DerivedField* _convStratDbzSqTexture;
  DerivedField* _convStratFilledSqTexture;

  DerivedField* _convStratDbzColMax;
  DerivedField* _convStratMeanTexture;
  DerivedField* _convStratMeanSqTexture;
  DerivedField* _convStratCategory;

  // radar location

  double _radarLat, _radarLon, _radarAltKm;
  double _prevRadarLat, _prevRadarLon, _prevRadarAltKm;

  // beam width

  double _beamWidthDegH;
  double _beamWidthDegV;

  // gate geometry

  int _maxNGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _maxRangeKm;

  // output projection and grid

  double _gridOriginLat, _gridOriginLon;
  int _gridNx, _gridNy, _gridNz;
  int _nPointsVol, _nPointsPlane;
  double _gridMinx, _gridMiny;
  double _gridDx, _gridDy;
  double _radarX, _radarY;
};

#endif
