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
/////////////////////////////////////////////////////////////
// pirepXml.hh
//
// C++ class for wrapping pirep.h, and adding XML components.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2007
//////////////////////////////////////////////////////////////

#ifndef _pirepXml_hh
#define _pirepXml_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaXml.hh>
#include <rapformats/pirep.h>

using namespace std;

class pirepXml {

public:

  // constructor

  pirepXml();

  // destructor

  ~pirepXml();

  //////////////////////// set methods /////////////////////////

  // reset to default state
  
  void reset();

  // set full message string

  void setMessage(const string &text) { _message = text; }
  
  // set pirep struct

  void setStruct(const pirep_t &pirep) { _pirep = pirep; }

  //////////////////////// get methods /////////////////////////

  // get message string

  inline const string &getMessage() const { return _message; }

  // get pirep struct
  
  inline const pirep_t &getStruct() const { return _pirep; }
  
  ///////////////////////////////////////////
  // assemble buffer starting with pirep_t struct
  // and optionally followed by raw text

  void assemble(bool appendRawText);
  
  // assemble as full XML

  void assembleAsXml();

  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ///////////////////////////////////////////
  // load XML
  
  void loadXml(string &xml) const;
  
  ///////////////////////////////////////////
  // load message as XML
  
  void loadMessageXml(string &xml) const;
    
  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void print(FILE *out, string spacer = "") const;

  // Print an XML representation of the object.
  
  void printAsXml(ostream &out) const;

protected:
private:

  // raw text string

  string _message;

  // pirep_t struct

  pirep_t _pirep;

  // buffer for assemble / disassemble

  MemBuf _memBuf;

  // functions

  int _disassembleXml(const char *buf, int len);
  int _disassembleMessageXml(const char *buf, int len);

};

#endif


