/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
** Copyright (c) 2008, UCAR
** University Corporation for Atmospheric Research(UCAR)
** National Center for Atmospheric Research(NCAR)
** Research Applications Program(RAP)
** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/////////////////////////////////////////////////////////////
// Ncf.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
///////////////////////////////////////////////////////////////
//
// Utility class for CF netCDF
//
///////////////////////////////////////////////////////////////////////

#include <NcfMdv/Ncf.hh>

// standard strings used by CF NetCDF

const char* Ncf::_360_day = "360_day";
const char* Ncf::_365_day = "365_day";
const char* Ncf::_366_day = "366_day";
const char* Ncf::FillValue = "_FillValue";
const char* Ncf::add_offset = "add_offset";
const char* Ncf::albers_conical_equal_area = "albers_conical_equal_area";
const char* Ncf::all_leap = "all_leap";
const char* Ncf::ancillary_variables = "ancillary_variables";
const char* Ncf::area = "area";
const char* Ncf::axis = "axis";
const char* Ncf::azimuthal_equidistant = "azimuthal_equidistant";
const char* Ncf::bounds = "bounds";
const char* Ncf::cell_measures = "cell_measures";
const char* Ncf::cell_methods = "cell_methods";
const char* Ncf::comment = "comment";
const char* Ncf::compress = "compress";
const char* Ncf::conventions = "conventions";
const char* Ncf::coordinates = "coordinates";
const char* Ncf::degrees = "degrees";
const char* Ncf::degrees_east = "degrees_east";
const char* Ncf::degrees_north = "degrees_north";
const char* Ncf::detection_minimum = "detection_minimum";
const char* Ncf::down = "down";
const char* Ncf::earth_radius = "earth_radius";
const char* Ncf::false_easting = "false_easting";
const char* Ncf::false_northing = "false_northing";
const char* Ncf::flag_meanings = "flag_meanings";
const char* Ncf::flag_values = "flag_values";
const char* Ncf::forecast_period = "forecast_period";
const char* Ncf::forecast_reference_time = "forecast_reference_time";
const char* Ncf::formula_terms = "formula_terms";
const char* Ncf::gregorian = "gregorian";
const char* Ncf::grid_latitude = "grid_latitude";
const char* Ncf::grid_longitude = "grid_longitude";
const char* Ncf::grid_mapping = "grid_mapping";
const char* Ncf::grid_mapping_attribute = "grid_mapping_attribute";
const char* Ncf::grid_mapping_name = "grid_mapping_name";
const char* Ncf::grid_north_pole_latitude = "grid_north_pole_latitude";
const char* Ncf::grid_north_pole_longitude = "grid_north_pole_longitude";
const char* Ncf::history = "history";
const char* Ncf::institution = "institution";
const char* Ncf::inverse_flattening = "inverse_flattening";
const char* Ncf::julian = "julian";
const char* Ncf::lambert_azimuthal_equal_area = "lambert_azimuthal_equal_area";
const char* Ncf::lambert_conformal_conic = "lambert_conformal_conic";
const char* Ncf::latitude = "latitude";
const char* Ncf::latitude_longitude = "latitude_longitude";
const char* Ncf::latitude_of_projection_origin = "latitude_of_projection_origin";
const char* Ncf::layer = "layer";
const char* Ncf::leap_month = "leap_month";
const char* Ncf::leap_year = "leap_year";
const char* Ncf::level = "level";
const char* Ncf::long_name = "long_name";
const char* Ncf::longitude = "longitude";
const char* Ncf::longitude_of_central_meridian = "longitude_of_central_meridian";
const char* Ncf::longitude_of_prime_meridian = "longitude_of_prime_meridian";
const char* Ncf::longitude_of_projection_origin = "longitude_of_projection_origin";
const char* Ncf::maximum = "maximum";
const char* Ncf::mean = "mean";
const char* Ncf::median = "median";
const char* Ncf::mercator = "mercator";
const char* Ncf::mid_range = "mid_range";
const char* Ncf::minimum = "minimum";
const char* Ncf::missing_value = "missing_value";
const char* Ncf::mode = "mode";
const char* Ncf::month_lengths = "month_lengths";
const char* Ncf::noleap = "noleap";
const char* Ncf::none = "none";
const char* Ncf::number_of_observations = "number_of_observations";
const char* Ncf::perspective_point_height = "perspective_point_height";
const char* Ncf::point = "point";
const char* Ncf::polar_radar = "polar_radar";
const char* Ncf::polar_stereographic = "polar_stereographic";
const char* Ncf::positive = "positive";
const char* Ncf::projection_x_coordinate = "projection_x_coordinate";
const char* Ncf::projection_y_coordinate = "projection_y_coordinate";
const char* Ncf::proleptic_gregorian = "proleptic_gregorian";
const char* Ncf::references = "references";
const char* Ncf::region = "region";
const char* Ncf::rotated_latitude_longitude = "rotated_latitude_longitude";
const char* Ncf::scale_factor = "scale_factor";
const char* Ncf::scale_factor_at_central_meridian = "scale_factor_at_central_meridian";
const char* Ncf::scale_factor_at_projection_origin = "scale_factor_at_projection_origin";
const char* Ncf::seconds = "seconds";
const char* Ncf::secs_since_jan1_1970 = "seconds since 1970-01-01T00:00:00Z";
const char* Ncf::semi_major_axis = "semi_major_axis";
const char* Ncf::semi_minor_axis = "semi_minor_axis";
const char* Ncf::sigma_level = "sigma_level";
const char* Ncf::source = "source";
const char* Ncf::standard = "standard";
const char* Ncf::standard_deviation = "standard_deviation";
const char* Ncf::standard_error = "standard_error";
const char* Ncf::standard_name = "standard_name";
const char* Ncf::standard_parallel = "standard_parallel";
const char* Ncf::start_time = "start_time";
const char* Ncf::status_flag = "status_flag";
const char* Ncf::stereographic = "stereographic";
const char* Ncf::stop_time = "stop_time";
const char* Ncf::straight_vertical_longitude_from_pole = "straight_vertical_longitude_from_pole";
const char* Ncf::sum = "sum";
const char* Ncf::time = "time";
const char* Ncf::time_bounds = "time_bounds";
const char* Ncf::title = "title";
const char* Ncf::transverse_mercator = "transverse_mercator";
const char* Ncf::units = "units";
const char* Ncf::up = "up";
const char* Ncf::valid_max = "valid_max";
const char* Ncf::valid_min = "valid_min";
const char* Ncf::valid_range = "valid_range";
const char* Ncf::variance = "variance";
const char* Ncf::vertical = "vertical";
const char* Ncf::vertical_perspective = "vertical_perspective";
const char* Ncf::volume = "volume";
