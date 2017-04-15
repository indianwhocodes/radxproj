/*******************************************************
 *	Author: Amarjit Kumar Singh - amarnitdgp@gmail.com *
 *	Date :  12/15/2016
 **
 ********************************************************/

#include "VolumePointsSet.hh"
#include <memory>

VolumePoint::VolumePoint(std::vector<float> x, std::vector<float> y,
                         std::vector<float> z, std::vector<double> ts,
                         std::string name, std::vector<float> field,
                         std::vector<float> roi) {
  std::cout << "VolumePoint constructor" << std::endl;
  this->_x = x;
  this->_y = y;
  this->_z = z;
  this->_ts = ts;
  this->_field_name = name;
  this->_field = field;
  this->_roi = roi;
}

VolumePoint::~VolumePoint() {
  // Free if anything allocated on heap
}

VolumePointsSet::VolumePointsSet(std::shared_ptr<VolumeStream> vs,
                                 std::list<int> fields, int no_of_fields) {

  std::cout << "VolumePointsSet constructor" << std::endl;
  this->_vs = vs;
  _fields = fields;
  _no_of_fields = no_of_fields;

  // Allocate volumepoint array of objects on heap with size _no_of_fields
}

// This populates volume points from the incoming VolumeStream.
std::vector<VolumePoint> VolumePointsSet::populate_volume_points() {
  std::cout << "VolumePointsSet populate_volume_points function" << std::endl;
  // populate vp_set
  return this->_vp_set;
}

std::vector<float> VolumePointsSet::getRoI(const std::vector<float> &ele,
                                           const std::vector<float> &azimuth,
                                           const std::vector<float> &gate) {
  std::cout << "VolumePointsSet getROI function" << std::endl;

  // Calculate ROI from VS receieved in constructor.
  throw "Not Implemented";
}

VolumePointsSet::~VolumePointsSet() {
  // Free if anything allocated on heap
}
