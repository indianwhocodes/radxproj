#ifndef VolumePointSet_hh
#define VolumePointSet_hh


class VolumePoint{

	public:
		float * _x;
		float * _y;
		float * _z;
		double * _ts;
		string _field_name;
		float * _field;
		float * _roi;  //radius of influence
		
		//This is filled by Volume Point Set class for initializing multiple VolumePoints
		VolumePoint(float * x, float * y, float * z, double * ts, string name, float * feild, float * roi);
}


class VolumePointSet{
	
	  vector<VolumePoint *> vp_set;
	  VolumeStream * vs;
	  List<int> _fields;
	  int _no_of_fields;
	  
	  public:
	  		VolumePointsSet(VolumeStream vs, List<int> _fields, int no_of_fields);
			//This populates volume points from the incoming VolumeStream.
			vector<VolumePoint *> populate_volume_points();
			float * getROI(float * ele, float * azimuth, float * gate);
}

#endif

