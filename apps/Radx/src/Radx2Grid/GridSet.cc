/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/

#include "GridSet.hh"



Grid :: Grid(int i, int j, int k, Cell *** gridCell){
	_ni=i;
	_nj=j;
	_nk=k;
	_gridCell=gridCell;
}

Grid :: ~Grid(){
	//Free any heap allocated memories.
}

GridSet::GridSet(vector<VolumePoint*> vps){

	//More data members to be decided
	_vps=vps;		
}

void GridSet::interpolateEachVolumePoint(){
	//Interporate different volume points.
	//this function will eventually be called for, fill function. 

}

vector<Grid *> fill(){
	//Populate all the grid cell and return vector of such a set.
}


GridSet :: ~GridSet(){
	//Free any heap allocated memories.
}

