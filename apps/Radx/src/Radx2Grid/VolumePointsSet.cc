/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/


#include "VolumePointsSet.hh"



VolumePoint :: VolumePoint(float* x,float* y,float* z,double* ts, std::string name, float * field, float * roi){
	std::cout << "VolumePoint constructor" << std::endl;
	_x=x;
	_y=y;
	_z=z;
	_ts=ts;
	_field_name=name;
	_field=field;
	_roi=roi;
}


VolumePoint :: ~VolumePoint(){
	//Free if anything allocated on heap
}


VolumePointsSet :: VolumePointsSet(VolumeStream vs, std::list<int> fields, int no_of_fields){

	std::cout << "VolumePointsSet constructor" << std::endl;

	_fields = fields;
	_no_of_fields = no_of_fields;
	
	//Allocate volumepoint array of objects on heap with size _no_of_fields 
}

//This populates volume points from the incoming VolumeStream.
std::vector<VolumePoint *> VolumePointsSet :: populate_volume_points(){
	std::cout << "VolumePointsSet populate_volume_points function" << std::endl;
	//populate vp_set
	return vp_set;
}

float * VolumePointsSet :: getROI(float * ele, float * azimuth, float * gate){
	std::cout << "VolumePointsSet getROI function" << std::endl;
	//Calculate ROI from VS receieved in constructor.
	return ;
}

VolumePointsSet :: ~VolumePointsSet(){
	//Free if anything allocated on heap
}



