/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/

#include "GridSet.hh"

Grid :: Grid(int i, int j, int k, Cell *** gridCell){
    std::cout << "Grid constructor" << std::endl;
	_ni=i;
	_nj=j;
	_nk=k;
	_gridCell=gridCell;
}

Grid :: ~Grid(){
	//Free any heap allocated memories.
}

GridSet::GridSet(std::vector<VolumePoint*> vps, Params::grid_xy_geom_t xy, Params::grid_z_geom_t z){

    std::cout << "GridSet constructor" << std::endl;
	//More data members to be decided
//	_vps=vps;
}


void GridSet::interpolateEachVolumePoints(){
    std::cout << "GridSet interpolateEachVolumePoints function" << std::endl;
	//Interporate different volume points.
	//this function will eventually be called for, fill function. 

}

std::vector<Grid *> GridSet::fill() {
    std::cout << "GridSet fill function" << std::endl;
	//Populate all the grid cell and return vector of such a set.
}


GridSet :: ~GridSet(){
	//Free any heap allocated memories.
}

