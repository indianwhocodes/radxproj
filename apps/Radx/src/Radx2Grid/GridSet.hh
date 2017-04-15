/*******************************************************
 *	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *
 *	Date :  12/15/2016
 **
 ********************************************************/

#ifndef GridSet_hh
#define GridSet_hh

#include "Params.hh"
#include "VolumePointsSet.hh"
#include <vector>

struct Cell {
  std::vector<float> distance;
  std::vector<float> field_value;
};

class Grid {

public:
  int _ni;
  int _nj;
  int _nk;

  // Pair comprises of <distance, value>
  // Distance is the distance from z,y,z to the cell and value is the
  // feild value of the cell e.g. reflectivity value or velocity value etc.

  Cell ***_gridCell;

  Grid(int i, int j, int k, Cell ***gridCell);
  ~Grid();
};

class GridSet {

private:
  // Input VolumePoints
  std::vector<VolumePoint> _vps;
  // Output grids
  std::vector<Grid> grid_set;

  Params::grid_xy_geom_t _xy_geom;
  Params::grid_z_geom_t _z_geom;

public:
  GridSet(const std::vector<VolumePoint> &vps, Params::grid_xy_geom_t xy,
          Params::grid_z_geom_t z);
  void interpolateEachVolumePoints();

  // function to fill the grid set
  std::vector<Grid> fill();

  ~GridSet();
};

#endif
