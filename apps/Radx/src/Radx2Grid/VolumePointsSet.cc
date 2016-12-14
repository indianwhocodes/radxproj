

#include "VolumePointsSet.hh"



VolumePoint :: VolumePoint(int x,int y,int z,double ts,string name){
	_nx=x;
	_ny=y;
	_nz=z;
	_ts=ts;
	_feild=name;
}


VolumePoint :: VolumePoint(){
	//Free if anything allocated on heap
}


VolumePointsSet :: VolumePointsSet(VolumeStream vs, List<int> fields, int no_of_fields){
	
	_fields = fields;
	_no_of_fields = no_of_fields;
	
	//Allocate volumepoint array of objects on heap with size _no_of_fields 
}

vector<VolumePoint *> VolumePointsSet :: populate_volume_points(){
	//populate vp_set
}

VolumePointsSet :: ~VolumePointsSet(){
	//Free if anything allocated on heap
}



