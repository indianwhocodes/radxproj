#ifndef GridSet_hh
#define GridSet_hh

#include "Params.hh"
class GridSet{

	  vector<VolumePoint*> _vps;
	  
	  public:
	  		GridSet(vector<VolumePoint*> vps,  grid_z_geom_t xy, grid_z_geom_t z);
			void interpolateEachPoint();
}

#endif
