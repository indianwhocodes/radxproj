
#ifndef Cartesian2Grid_h
#define Cartesian2Grid_h

#include "Polar2Cartesian.h"
#include "Params.hh"

struct Grid;

class Cartesian2Grid
{
public:
    // constructor & destructor
    Cartesian2Grid(Repository* store);
    ~Cartesian2Grid();

    // calculate the grid size
    void calculateGridSize(const Params& params);

    // calculate the final grid
    void calculateRefGrid();


    // setter, getter
    float*** getRefGrid();

    void updateFieldGrid(size_t i,size_t j,size_t k, float value);
    void updateWeightGrid(size_t i,size_t j,size_t k, float value);
    void updateCountGrid(size_t i,size_t j,size_t k, float value);
    void updateFinalGrid(size_t i,size_t j,size_t k, float value);

private:

    Repository* _store;


    Grid* _grid;

};

#endif
