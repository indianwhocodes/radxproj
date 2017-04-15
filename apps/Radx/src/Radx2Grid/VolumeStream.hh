/*******************************************************
 *	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *
 *	Date :  12/15/2016
 **
 ********************************************************/

#ifndef VolumeStream_hh
#define VolumeStream_hh

#include <iostream>
#include <string>
#include <vector>

#define MAX_FIELDS 10
// You can change this to a different value Later if needed.
#define INVALID_DATA -9999F

class VolumeStream {

private:
  float _latitude;
  float _longitude;
  float _altitude;
  float _altitude_agl;
  std::string _input_file;

  // Change them to Smart Pointers
  std::vector<float> _elevation;
  std::vector<float> _azimuth;
  std::vector<float> _gate;
  std::vector<double> _time;
  std::vector<double> _fields[MAX_FIELDS];

  // Fuctions decide later

public:
  VolumeStream(const std::string input_file);
  void fillFromFile();
  ~VolumeStream();
};

#endif
