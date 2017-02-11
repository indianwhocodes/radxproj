/*******************************************************
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/

#ifndef Radx2GridPlus_hh
#define Radx2GridPlus_hh

#include <string>
#include <list>

#include "VolumeStream.hh"
#include "VolumePointsSet.hh"
#include "GridSet.hh"

typedef enum {
  REF = 0,
  VEL = 1,
  SW = 2,
  ZDR = 3,
  RHO = 4,
  PHI = 5,
  KDP = 6,
  U = 7,
  V = 8,
  W = 9,
}fields_t;


class Radx2GridPlus{

	private: 
		std::string _program_name;
		std::list<int> _fields;
		int _no_of_fields;


		
	public:
		Radx2GridPlus(std::string pname);
		void processFile(const time_t &startTime, const time_t &endTime, const Params &params );
		void mapping_field_names(Params::select_field_t * selected_fields);
        ~Radx2GridPlus();
};

#endif
