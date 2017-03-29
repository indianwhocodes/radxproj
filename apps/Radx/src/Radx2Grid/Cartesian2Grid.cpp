
#include "Cartesian2Grid.h"

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
    float*** _fieldGrid;
    // sum of weights
    float*** _weightGrid;
    // count of affcting gates
    float*** _countGrid;

    // final grid that contains Ref
    float*** _refGrid;
};

// constructor 
Cartesian2Grid::Cartesian2Grid(Repository* store)
{
    _store = store; 
    _grid = new Grid();
}

Cartesian2Grid::~Cartesian2Grid() 
{

}

// calculate the grid size
void Cartesian2Grid::calculateGridSize(const Params& params)
{
    
}

// calculate the final grid
void Cartesian2Grid::calculateRefGrid()
{
    
}

// getter
float*** Cartesian2Grid::getRefGrid()
{
    return _grid->_refGrid;
}


void Cartesian2Grid::updateFieldGrid(size_t i,size_t j,size_t k, float value)
{
    _grid->_fieldGrid[i][j][k] = value;
}
void Cartesian2Grid::updateWeightGrid(size_t i,size_t j,size_t k, float value)
{
    _grid->_weightGrid[i][j][k] = value;
}
void Cartesian2Grid::updateCountGrid(size_t i,size_t j,size_t k, float value)
{
    _grid->_countGrid[i][j][k] = value;
}
void Cartesian2Grid::updateFinalGrid(size_t i,size_t j,size_t k, float value)
{
    _grid->_fieldGrid[i][j][k] = value;
}
