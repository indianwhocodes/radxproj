// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:09:40 $
//   $Id: MinStatCalc.cc,v 1.3 2016/03/03 18:09:40 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MinStatCalc: Class for calculating the minimum statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/climo/MinStatCalc.hh>
#include <toolsa/str.h>

using namespace std;


/**********************************************************************
 * Constructor
 */

MinStatCalc::MinStatCalc(const bool debug_flag, const bool check_z_levels) :
  StatCalc(debug_flag, check_z_levels)
{
}


/**********************************************************************
 * Destructor
 */

MinStatCalc::~MinStatCalc(void)
{
}
  

/**********************************************************************
 * calcStatistic() - Calculate the statistic field using the given
 *                   information.
 */

MdvxField *MinStatCalc::calcStatistic(const DsMdvx &climo_file,
				      const MdvxField &data_field,
				      const DateTime &climo_time)
{
  // Construct the statistic field name

  string stat_field_name = data_field.getFieldHeader().field_name_long;
  
  // Extract the existing minimum field from the climatolog file, if
  // there  is one.  If there is an existing field, we will update
  // the running miniumu.  If there isn't an existing field, we will
  // create a new one.

  string climo_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MIN,
				stat_field_name).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *curr_min_field =
    climo_file.getField(climo_field_name.c_str());
  MdvxField *new_min_field = 0;
  
  if (curr_min_field == 0)
  {
    if (_debug)
      cerr << "   Creating " << climo_field_name << " statistic" << endl;
    
    new_min_field = _createField(data_field, climo_time);
  }
  else
  {
    if (_debug)
      cerr << "   Updating " << climo_field_name << " statistic" << endl;
    
    new_min_field = _updateField(climo_file, data_field,
				 *curr_min_field);
  }
  
  return new_min_field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createField() - Create a new minimum climatology field from the
 *                  given data field.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *MinStatCalc::_createField(const MdvxField &data_field,
				     const DateTime &climo_time) const
{
  static const string method_name = "MinStatCalc::_createField()";

  // Create the field header

  Mdvx::field_header_t field_hdr = data_field.getFieldHeader();;
  
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = climo_time.utime();
  
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;

  STRcopy(field_hdr.field_name_long,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MIN,
				      field_hdr.field_name_long).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MIN,
				      field_hdr.field_name).c_str(),
	  MDV_SHORT_FIELD_LEN);
  
  return new MdvxField(field_hdr,
		       data_field.getVlevelHeader(),
		       data_field.getVol());
}


/*********************************************************************
 * _updateField() - Create a new minimum climatology field that is an
 *                  update of the current minimum field using the given
 *                  information.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *MinStatCalc::_updateField(const DsMdvx &climo_file,
				     const MdvxField &data_field,
				     const MdvxField &curr_min_field) const
{
  static const string method_name = "MinStatCalc::_updateField()";

  // Make sure the fields match so we can update the climos

  if (!_fieldsMatch(data_field, curr_min_field))
    return 0;
  
  // Create the new field as a copy of the original field

  MdvxField *new_min_field = new MdvxField(curr_min_field);
  
  // Loop through the new field values, updating the stat value
  // wherever we have new data.

  Mdvx::field_header_t min_field_hdr = new_min_field->getFieldHeader();
  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  
  fl32 *min_data = (fl32 *)new_min_field->getVol();
  fl32 *data = (fl32 *)data_field.getVol();
  
  int volume_size =
    min_field_hdr.nx * min_field_hdr.ny * min_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (data[i] == data_field_hdr.missing_data_value ||
	data[i] == data_field_hdr.bad_data_value)
      continue;
    
    if (min_data[i] == min_field_hdr.missing_data_value ||
	min_data[i] == min_field_hdr.bad_data_value)
    {
      min_data[i] = data[i];
    }
    else
    {
      if (data[i] < min_data[i])
	min_data[i] = data[i];
    }
    
  } /* endfor - i */
  
  return new_min_field;
}
