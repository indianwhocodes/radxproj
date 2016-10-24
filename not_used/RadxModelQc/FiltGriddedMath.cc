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
/**
 * @file FiltGriddedMath.cc
 */
#include "FiltGriddedMath.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
FiltGriddedMath::FiltGriddedMath(const Params::data_filter_t f,
				 const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _type = P._parm_gridded_math[f.filter_index].type;
  _input = P._parm_gridded_math[f.filter_index].input;
}

//------------------------------------------------------------------
FiltGriddedMath::~FiltGriddedMath()
{
}

//------------------------------------------------------------------
void FiltGriddedMath::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
bool FiltGriddedMath::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltGriddedMath::filter(const time_t &t, const RadxRay *ray0,
			     const RadxRay &ray, const RadxRay *ray1,
			     std::vector<RayxData> &data) const
{
  RayxData r, r2;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, r))
  {
    return false;
  }
  if (!RadxApp::retrieveRay(_input, ray, data, r2))
  {
    return false;
  }

  switch (_type)
  {
  case Params::ADD:
    r.inc(r2, false);
    break;
  case Params::SUBTRACT:
    r.dec(r2, false);
    break;
  case Params::MULT:
    r.multiply(r2, false);
    break;
  default:
    break;
  }

  RadxApp::modifyRayForOutput(r, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(r);
  return true;
}

//------------------------------------------------------------------
void FiltGriddedMath::filterVolume(const RadxVol &vol)
{
}


//------------------------------------------------------------------
void FiltGriddedMath::finish(void)
{
}

