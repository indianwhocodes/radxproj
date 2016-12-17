/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/

#ifndef GridSet_hh
#define GridSet_hh

#include "Params.hh"

struct Cell{
	vector<float> distance;
	vector <float> field_value;
};

class Grid{
	public:
		int _ni;
		int _nj;
		int _nk;

		//Pair comprises of <distance, value> 
		//Distance is the distance from z,y,z to the cell and value is the 
		//feild value of the cell e.g. reflectivity value or velocity value etc.
		 
	    Cell *** _gridCell; 

		Grid(int i, int j, int k, Cell *** gridCell);
};

class GridSet{

	  //Input VolumePoints
	  vector<VolumePoint*> _vps;
	  //Output grids 
	  vector<Grid *> grid_set;
	  
	  public:
	  		GridSet(vector<VolumePoint*> vps,  grid_z_geom_t xy, grid_z_geom_t z);
			void interpolateEachVolumePoints();
			//function to fill the grid set
			vector<Grid *> fill();
}

#endif
