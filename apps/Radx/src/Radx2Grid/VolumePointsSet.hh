/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/

#ifndef VolumePointSet_hh
#define VolumePointSet_hh

#include <vector>
#include <list>
#include <string>

#include "VolumeStream.hh"

class VolumePoint{



	public:
		float * _x;
		float * _y;
		float * _z;
		double * _ts;
		std::string _field_name;
		float * _field;
		float * _roi;  //radius of influence
		
		//This is filled by Volume Point Set class for initializing multiple VolumePoints
		VolumePoint(float * x, float * y, float * z, double * ts, std::string name, float * field, float * roi);
	    ~VolumePoint();

};


class VolumePointsSet{
	
	  std::vector<VolumePoint *> vp_set;
	  VolumeStream * vs;
	  std::list<int> _fields;
	  int _no_of_fields;


	  
	  public:
	  		VolumePointsSet(VolumeStream vs, std::list<int> fields, int no_of_fields);
			//This populates volume points from the incoming VolumeStream.
			std::vector<VolumePoint *> populate_volume_points();
			float * getROI(float * ele, float * azimuth, float * gate);
			~VolumePointsSet();
};

#endif

