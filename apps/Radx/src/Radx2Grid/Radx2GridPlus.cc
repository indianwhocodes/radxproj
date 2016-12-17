
#include "Radx2GridPlus.hh"
#include "Params.hh"

Radx2GridPlus:: Radx2GridPlus(string pname){
	_program_name = pname;
}


int Radx2GridPlus:: processFile(const string filename, const Params &params ){

	VolumeStream vs(filename);
	vs.fillFromFile();

	_no_of_fields = params.selected_fields_n;

	//Map input selected fields and fill _fields using feilds enum
	this->mapping_field_names(params._selected_fields);

	// VolumePointSets are multiple points populated using different feilds from VolumeStream
	VolumePointsSet vpset(vs,_no_of_fields,_fields);
	vector<VolumePoint *> vp_res = vp_set.populate_volume_points();

	//vp_set is then used to form grid set where each VolumePoint corresponds to each Grid in a GridSet.
	GridSet g(vp_res,params.grid_z_geom, params.grid_xy_geom);
	vector<Grid *> gcell_res = g.fill();	

	//gcell_res need to be written into final output file.


	
}

void Radx2GridPlus::mapping_field_names(select_field_t * selected_fields){

	//Fill fields based on _selected_fields and fields_t. fields member variable to contain enum elements from fields_t.
	//e.g if(selected_fields.input_name=='reflectivty') fields.push(REF);
}


Radx2GridPlus::~Radx2GridPlus(){
   //Free any allocated memory.
}

