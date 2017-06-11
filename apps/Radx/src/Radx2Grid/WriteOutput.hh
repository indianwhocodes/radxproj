#ifndef WriteOutput_h
#define WriteOutput_h

#include "Interp.hh"
#include "PolarDataStream.hh"
#include <Radx/Radx.hh>
#include <memory>
#include <vector>

#define INVALID_DATA -9999.0f

class WriteOutput
{
public:
  // constructor & destructor
  WriteOutput(shared_ptr<Repository> store,
              RadxVol& readVol,
              const Params& params);
  ~WriteOutput();

  int writeOutputFile();

private:
  std::shared_ptr<Repository> _store;
  RadxVol& _readVol;
  const Params& _params;

  // output projection and grid

  double _gridOriginLat, _gridOriginLon;
  int _gridNx, _gridNy, _gridNz;
  int _nPointsVol, _nPointsPlane;
  double _gridMinx, _gridMiny;
  double _gridDx, _gridDy;
  double _radarX, _radarY;
};

#endif
