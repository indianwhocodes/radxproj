#ifndef VolumePointSet_hh
#define VolumePointSet_hh

class VolumePoint{

	private:
		int _nx;
		int _ny;
		int _nz;
		double _ts;
		string _feild;
		
	public:
		VolumePoint(int x, int y, int z, double ts, string name);
}


class VolumePointSet{
	
	  vector<VolumePoint *> vp_set;
	  VolumeStream * vs;
	  List<int> _fields;
	  int _no_of_fields;
	  
	  public:
	  		VolumePointsSet(VolumeStream vs, List<int> _fields, int no_of_fields);
			vector<VolumePoint *> populate_volume_points();
}

#endif

