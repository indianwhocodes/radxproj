/******************************************************* 
*	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *		
*	Date :  12/15/2016 								   *
********************************************************/


#include "VolumeStream.hh"


VolumeStream:: VolumeStream(const std::string input_file){

    std::cout << "VolumeStream constructor" << std::endl;
	//Read n_points dimension from input netcdf file and the create arrays for _elevation, _azimuth, _azimuth, _gate, _time, _fields array
	//Intialize all to INVALID_DATA	(macro above)
}

void VolumeStream::fillFromFile()
{
    std::cout << "VolumeStream fillFromFile Function" << std::endl;
	//Fill VolumeStream member variables from the input file
}


VolumeStream::~VolumeStream()
{
	//Free memory allocates in constructor of VolumeStream

}






