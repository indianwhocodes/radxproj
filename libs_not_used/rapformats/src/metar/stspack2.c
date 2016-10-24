/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#include <rapformats/nws_oso_local.h> 
#include <ctype.h>
#include "metar_private.h"
 
/********************************************************************/
/*                                                                  */
/*  Title:         stspack2                                         */
/*  Organization:  W/OSO242 - GRAPHICS AND DISPLAY SECTION          */
/*  Date:          05 Oct 1992                                      */
/*  Programmer:    ALLAN DARLING                                    */
/*  Language:      C/2                                              */
/*                                                                  */
/*  Abstract:      The stspack2 package contains functions to       */
/*                 perform the isalnum through isxdigit functions   */
/*                 on strings.  The functions come in four forms:   */
/*                 those that test NULL delimited strings and are   */
/*                 named in the form sxxxxxxx, those that test at   */
/*                 most n characters and are named in the form      */
/*                 nxxxxxxx, those that search forward in a string  */
/*                 and are named in the form nxtyyyyy, and those    */
/*                 that search backward in a string and are named   */
/*                 in the form lstyyyyy.                            */
/*                                                                  */
/*                 The xxxxxxx is the name of the test applied to   */
/*                 each character in the string, such as isalpha,   */
/*                 thus a function to test a NULL delimited string  */
/*                 an return a nonzero value if all characters in   */
/*                 the string are digits is named sisdigit.         */
/*                                                                  */
/*                 The yyyyy is the name of the test applied to     */
/*                 characters in a string, minus the 'is' prefix.   */
/*                 Thus a function to find the next digit in a NULL */
/*                 delimited string and return a pointer to it is   */
/*                 named nxtdigit.                                  */
/*                                                                  */
/*                 The only exception to the naming rule is for the */
/*                 functions that test for hexadecimal digits.      */
/*                 These are named sisxdigi, nisxdigi, nxtxdigi,    */
/*                 and lstxdigi because of the eight character      */
/*                 function name limitation.                        */
/*                                                                  */
/*                 The nxxxxxxx class of functions will test up to  */
/*                 n characters or the first NULL character         */
/*                 encountered, whichever comes first.  For all     */
/*                 classes of functions, the string sentinal is     */
/*                 not included in the test.                        */
/*                                                                  */
/*  External Functions Called:                                      */
/*                 isalnum, isalpha, iscntrl, isdigit, isgraph,     */
/*                 islower, isprint, ispunct, isspace, isupper,     */
/*                 isxdigit.                                        */
/*                                                                  */
/*  Input:         For sxxxxxxx class functions, a pointer to a     */
/*                 NULL delimited character string.                 */
/*                                                                  */
/*                 For nxtyyyyy class functions, a pointer to a     */
/*                 NULL delimited character string.                 */
/*                                                                  */
/*                 for nxxxxxxx class functions, a pointer to a     */
/*                 character array, and a positive, nonzero integer.*/
/*                                                                  */
/*                 for lstyyyyy class functions, a pointer to a     */
/*                 character array, and a positive, nonzero integer.*/
/*                                                                  */
/*  Output:        A nonzero value if the test is true for all      */
/*                 characters in the string, a zero value otherwise.*/
/*                                                                  */
/*  Modification History:                                           */
/*                 None.                                            */
/*                                                                  */
/********************************************************************/
 
int nisalnum(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isalnum(*s))
         return (0);
 
   return (1);
 
} /* end nisalnum */
 
 
int nisalpha(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isalpha(*s))
         return (0);
 
   return (1);
 
} /* end nisalpha */
 
 
int niscntrl(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!iscntrl(*s))
         return (0);
 
   return (1);
 
} /* end niscntrl */
 
 
int nisdigit(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isdigit(*s))
         return (0);
 
   return (1);
 
} /* end nisdigit */
 
 
int nisgraph(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isgraph(*s))
         return (0);
 
   return (1);
 
} /* end nisgraph */
 
 
int nislower(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!islower(*s))
         return (0);
 
   return (1);
 
} /* end nislower */
 
 
int nisprint(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isprint(*s))
         return (0);
 
   return (1);
 
} /* end nisprint */
 
 
int nispunct(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!ispunct(*s))
         return (0);
 
   return (1);
 
} /* end nispunct */
 
 
int nisspace(char *s, int n) {

   for (; *s && (n>0); s++, n--)
 
      if (!isspace(*s))
         return (0);
 
   return (1);
 
} /* end nisspace */
 
 
int nisupper(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isupper(*s))
         return (0);
 
   return (1);
 
} /* end nisupper */
 
 
int nisxdigi(char *s, int n) {
 
   for (; *s && (n>0); s++, n--)
 
      if (!isxdigit(*s))
         return (0);
 
   return (1);
 
} /* end nisxdigi */
 
