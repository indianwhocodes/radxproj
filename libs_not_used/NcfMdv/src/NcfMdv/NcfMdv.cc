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
// Utility class for NcfMdv conversions.
//
///////////////////////////////////////////////////////////////////////

#include <NcfMdv/NcfMdv.hh>

// standard strings used by NcfMdv transformations

const char* NcfMdv::mdv_master_header = "mdv_master_header";
const char* NcfMdv::mdv_revision_number = "mdv_revision_number";
const char* NcfMdv::mdv_epoch = "mdv_epoch";
const char* NcfMdv::mdv_time_centroid = "mdv_time_centroid";
const char* NcfMdv::mdv_time_gen = "mdv_time_gen";
const char* NcfMdv::mdv_time_begin = "mdv_time_begin";
const char* NcfMdv::mdv_time_end = "mdv_time_end";
const char* NcfMdv::mdv_user_time = "mdv_user_time";
const char* NcfMdv::mdv_time_expire = "mdv_time_expire";
const char* NcfMdv::mdv_time_written = "mdv_time_written";
const char* NcfMdv::mdv_data_collection_type = "mdv_data_collection_type";
const char* NcfMdv::mdv_forecast_time = "mdv_forecast_time";
const char* NcfMdv::mdv_forecast_delta = "mdv_forecast_delta";
const char* NcfMdv::mdv_user_data = "mdv_user_data";
const char* NcfMdv::mdv_user_data_si32_0 = "mdv_user_data_si32_0";
const char* NcfMdv::mdv_user_data_si32_1 = "mdv_user_data_si32_1";
const char* NcfMdv::mdv_user_data_si32_2 = "mdv_user_data_si32_2";
const char* NcfMdv::mdv_user_data_si32_3 = "mdv_user_data_si32_3";
const char* NcfMdv::mdv_user_data_si32_4 = "mdv_user_data_si32_4";
const char* NcfMdv::mdv_user_data_si32_5 = "mdv_user_data_si32_5";
const char* NcfMdv::mdv_user_data_si32_6 = "mdv_user_data_si32_6";
const char* NcfMdv::mdv_user_data_si32_7 = "mdv_user_data_si32_7";
const char* NcfMdv::mdv_user_data_si32_8 = "mdv_user_data_si32_8";
const char* NcfMdv::mdv_user_data_si32_9 = "mdv_user_data_si32_9";
const char* NcfMdv::mdv_user_data_fl32_0 = "mdv_user_data_fl32_0";
const char* NcfMdv::mdv_user_data_fl32_1 = "mdv_user_data_fl32_1";
const char* NcfMdv::mdv_user_data_fl32_2 = "mdv_user_data_fl32_2";
const char* NcfMdv::mdv_user_data_fl32_3 = "mdv_user_data_fl32_3";
const char* NcfMdv::mdv_user_data_fl32_4 = "mdv_user_data_fl32_4";
const char* NcfMdv::mdv_user_data_fl32_5 = "mdv_user_data_fl32_5";
const char* NcfMdv::mdv_sensor_lon = "mdv_sensor_lon";
const char* NcfMdv::mdv_sensor_lat = "mdv_sensor_lat";
const char* NcfMdv::mdv_sensor_alt = "mdv_sensor_alt";
const char* NcfMdv::mdv_field_code = "mdv_field_code";
const char* NcfMdv::mdv_user_time_1 = "mdv_user_time_1";
const char* NcfMdv::mdv_user_time_2 = "mdv_user_time_2";
const char* NcfMdv::mdv_user_time_3 = "mdv_user_time_3";
const char* NcfMdv::mdv_user_time_4 = "mdv_user_time_4";
const char* NcfMdv::mdv_transform_type = "mdv_transform_type";
const char* NcfMdv::mdv_vlevel_type = "mdv_vlevel_type";
const char* NcfMdv::mdv_native_vlevel_type = "mdv_native_vlevel_type";
const char* NcfMdv::mdv_transform = "mdv_transform";
const char* NcfMdv::mdv_proj_type = "mdv_proj_type";
const char* NcfMdv::mdv_proj_origin_lat = "mdv_proj_origin_lat";
const char* NcfMdv::mdv_proj_origin_lon = "mdv_proj_origin_lon";
const char* NcfMdv::vertical_section = "vertical_section";
const char* NcfMdv::id = "id";
const char* NcfMdv::size = "size";
const char* NcfMdv::info = "info";
const char* NcfMdv::mdv_chunk = "mdv_chunk";
const char* NcfMdv::nbytes_mdv_chunk = "nbytes_mdv_chunk";
