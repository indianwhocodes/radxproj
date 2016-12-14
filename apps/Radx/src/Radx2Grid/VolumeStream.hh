#ifndef VolumeStream_hh
#define VolumeStream_hh

#define MAX_FIELDS 10
#define MAX_FIELD_LEN 2000

#define INVALID_DATA -9999F    // You can change this to a different value Later if needed.

class VolumeStream{

	private:
		float _latitude;
		float _longitude;
		float _altitude;
		float _altitude_agl;
		string _input_file;


		// Change them to Smart Pointers 
		float * _elevation;
		float * _azimuth;
		float * _gate;
		double * _time;
		float * _fields[MAX_FIELDS];

	//Fuctions decide later	

	public:
	    void processFile();
}

#endif

