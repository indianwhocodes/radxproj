
#include "Cartesian2Grid.hh"
#include "memory"

struct Grid;

// constructor
Cartesian2Grid::Cartesian2Grid(std::shared_ptr<Repository> store) {
  _store = store;
  _grid = std::make_shared<Grid>();
}

Cartesian2Grid::~Cartesian2Grid() {}

// calculate the grid size
void Cartesian2Grid::calculateGridSize(const Params &params) {}

// calculate the final grid
void Cartesian2Grid::calculateRefGrid() {}

// getter
float ***Cartesian2Grid::getRefGrid() { return _grid->_refGrid; }

void Cartesian2Grid::updateFieldGrid(size_t i, size_t j, size_t k,
                                     float value) {
  _grid->_fieldGrid[i][j][k] = value;
}
void Cartesian2Grid::updateWeightGrid(size_t i, size_t j, size_t k,
                                      float value) {
  _grid->_weightGrid[i][j][k] = value;
}
void Cartesian2Grid::updateCountGrid(size_t i, size_t j, size_t k,
                                     float value) {
  _grid->_countGrid[i][j][k] = value;
}
void Cartesian2Grid::updateFinalGrid(size_t i, size_t j, size_t k,
                                     float value) {
  _grid->_fieldGrid[i][j][k] = value;
}
