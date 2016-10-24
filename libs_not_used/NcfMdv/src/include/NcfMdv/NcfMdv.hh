/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 2008, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/////////////////////////////////////////////////////////////
// NcfMdv.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008 
//
///////////////////////////////////////////////////////////////
//
// Utilities for Ncf / MDV conversions
// 
///////////////////////////////////////////////////////////////////////

#ifndef NCF_MDV_HH
#define NCF_MDV_HH

#include <string>
#include <netcdfcpp.h>

using namespace std;

////////////////////////
// 
// Mdv2NcfTrans object extracts data from DsMdvx object and stores
// in objects including GridInfo, FieldData, VlevelInfo.
//
// GridInfo stores projection meta data for a netCDF projection information
// variable, as well as 2D grid information for field datasets.
// and is used to define netCDF dimensions,  coordinate variables, and auxiliary 
// coordinate variables for field datasets.
//
// VlevelInfo objects contain vertical 
// level information and are used to define dimensions and coordinate variables 
// for vertical coordinates.
//
// FieldData objects contain the mdv field data and associate 
// the data to relevant grid,vertical level, and projection information. 

class NcfMdv {
  
public:
  
  // standard strings

  const static char *mdv_master_header;
  const static char *mdv_revision_number;
  const static char *mdv_epoch;
  const static char *mdv_time_centroid;
  const static char *mdv_time_gen;
  const static char *mdv_time_begin;
  const static char *mdv_time_end;
  const static char *mdv_user_time;
  const static char *mdv_time_expire;
  const static char *mdv_time_written;
  const static char *mdv_data_collection_type;
  const static char *mdv_forecast_time;
  const static char *mdv_forecast_delta;
  const static char *mdv_user_data;
  const static char *mdv_user_data_si32_0;
  const static char *mdv_user_data_si32_1;
  const static char *mdv_user_data_si32_2;
  const static char *mdv_user_data_si32_3;
  const static char *mdv_user_data_si32_4;
  const static char *mdv_user_data_si32_5;
  const static char *mdv_user_data_si32_6;
  const static char *mdv_user_data_si32_7;
  const static char *mdv_user_data_si32_8;
  const static char *mdv_user_data_si32_9;
  const static char *mdv_user_data_fl32_0;
  const static char *mdv_user_data_fl32_1;
  const static char *mdv_user_data_fl32_2;
  const static char *mdv_user_data_fl32_3;
  const static char *mdv_user_data_fl32_4;
  const static char *mdv_user_data_fl32_5;
  const static char *mdv_sensor_lon;
  const static char *mdv_sensor_lat;
  const static char *mdv_sensor_alt;
  const static char *mdv_field_code;
  const static char *mdv_user_time_1;
  const static char *mdv_user_time_2;
  const static char *mdv_user_time_3;
  const static char *mdv_user_time_4;
  const static char *mdv_transform_type;
  const static char *mdv_vlevel_type;
  const static char *mdv_native_vlevel_type;
  const static char *mdv_transform;
  const static char *mdv_proj_type;
  const static char *mdv_proj_origin_lat;
  const static char *mdv_proj_origin_lon;
  const static char *vertical_section;
  const static char *id;
  const static char *size;
  const static char *info;
  const static char *mdv_chunk;
  const static char *nbytes_mdv_chunk;

protected:  

private:

};

#endif

