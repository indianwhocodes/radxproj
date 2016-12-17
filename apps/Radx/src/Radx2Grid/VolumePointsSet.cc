/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/


#include "VolumePointsSet.hh"



VolumePoint :: VolumePoint(int* x,int* y,int* z,double* ts,string name, float * field){
	_x=x;
	_y=y;
	_z=z;
	_s=ts;
	_field_name=name;
	_field=field;
	_roi=roi;
}


VolumePoint :: VolumePoint(){
	//Free if anything allocated on heap
}


VolumePointsSet :: VolumePointsSet(VolumeStream vs, List<int> fields, int no_of_fields){
	
	_fields = fields;
	_no_of_fields = no_of_fields;
	
	//Allocate volumepoint array of objects on heap with size _no_of_fields 
}

//This populates volume points from the incoming VolumeStream.
vector<VolumePoint *> VolumePointsSet :: populate_volume_points(){
	//populate vp_set
}

float * VolumePointsSet :: getROI(float * ele, float * azimuth, float * gate){
	//Calculate ROI from VS receieved in constructor.
}

VolumePointsSet :: ~VolumePointsSet(){
	//Free if anything allocated on heap
}



