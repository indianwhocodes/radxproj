
#ifndef Cartesian2Grid_h
#define Cartesian2Grid_h

#include "Params.hh"
#include "Polar2Cartesian.hh"

struct Grid // Cartesian2Grid::Cartesian2GridImpl
{
  // domain
  double _domXMin;
  double _domXMax;
  double _domYMin;
  double _domYMax;
  double _domZMin;
  double _domZMax;

  // cell size
  size_t _cellX;
  size_t _cellY;
  size_t _cellZ;

  // grid size
  size_t _gridSizeI;
  size_t _gridSizeJ;
  size_t _gridSizek;

  // weighted sum of values
  float ***_fieldGrid;
  // sum of weights
  float ***_weightGrid;
  // count of affcting gates
  float ***_countGrid;

  // final grid that contains Ref
  float ***_refGrid;
};
;

class Cartesian2Grid {
public:
  // constructor & destructor
  Cartesian2Grid(std::shared_ptr<Repository> store);
  ~Cartesian2Grid();

  // calculate the grid size
  void calculateGridSize(const Params &params);

  // calculate the final grid
  void calculateRefGrid();

  // setter, getter
  float ***getRefGrid();

  void updateFieldGrid(size_t i, size_t j, size_t k, float value);
  void updateWeightGrid(size_t i, size_t j, size_t k, float value);
  void updateCountGrid(size_t i, size_t j, size_t k, float value);
  void updateFinalGrid(size_t i, size_t j, size_t k, float value);

private:
  std::shared_ptr<Repository> _store;

  std::shared_ptr<Grid> _grid;
};

#endif
