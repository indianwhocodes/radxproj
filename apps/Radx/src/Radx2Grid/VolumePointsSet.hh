/*******************************************************
 *	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *
 *	Date :  12/15/2016
 **
 ********************************************************/

#ifndef VolumePointSet_hh
#define VolumePointSet_hh

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "VolumeStream.hh"

class VolumePoint {

public:
  std::vector<float> _x;
  std::vector<float> _y;
  std::vector<float> _z;
  std::vector<double> _ts;

  std::vector<float> _field;
  std::vector<float> _roi; // radius of influence

  std::string _field_name;

  // This is filled by Volume Point Set class for initializing multiple
  // VolumePoints
  VolumePoint(std::vector<float> x, std::vector<float> y, std::vector<float> z,
              std::vector<double> ts, std::string name,
              std::vector<float> field, std::vector<float> roi);
  ~VolumePoint();
};

class VolumePointsSet {

private:
  int _no_of_fields;
  std::vector<VolumePoint> _vp_set;
  std::shared_ptr<VolumeStream> _vs;
  std::list<int> _fields;

public:
  VolumePointsSet(std::shared_ptr<VolumeStream> vs, std::list<int> fields,
                  int no_of_fields);
  // This populates volume points from the incoming VolumeStream.
  std::vector<VolumePoint> populate_volume_points();
  std::vector<float> getRoI(const std::vector<float> &ele,
                            const std::vector<float> &azimuth,
                            const std::vector<float> &gate);
  ~VolumePointsSet();
};

#endif
