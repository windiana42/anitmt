/*
 * convtoa.hpp 
 * Convert to ASCII (header file) 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   May 2001   started writing
 *
 */
 
#ifndef _Inc_IO_ConvToAscii_H_
#define _Inc_IO_ConvToAscii_H_ 1

#include <val/val.hpp>

namespace output_io
{

// Writes the integer x in decimal notation to d which is assumed to be 
// long enough. 
// Return value: next free char; not '\0'-terminated. 
// On a PII-350, this function needs 38 seconds to write all integers 
// from 0...-1000000 (sprint(,"%d",i) needs 150 seconds). 
// This is nearly 4 times as fast as sprintf. 
extern char *Int2StrDec(char *d,int x);

// Writes the double value x in 1.1111e11-notation to d. 
// digits specifies the precision (the number of digits after the `.'). 
// d is assumed to be large enough. 
// NaN, -Inf and +Inf are handeled without problems. 
// The passed double value is rounded correctly. 
// Return value: next free char; not '\0'-terminated. 
// On my PII-350, Double2Str(,cal,5) is four times as fast as 
// sprintf(,"%.5e",val). 
extern char *Double2Str(char *d,double x,int ndigits);

// Converts a vector as ``<x,y,z>'' using Double2Str(). 
char *Vector2Str(char *d,const class values::Vector &v,int ndigits);

// Really fast: translate an integer into some number-and-digit 
// ASCII output which is unique for each integer. 
// The first char is either `b' (negative) or `a' (positive). 
// (NOTE: The most significant digits are the last ones.)
// Return value: next free char; not '\0'-terminated. 
extern char *IntEncode(char *d,int x);

// Even faster than the version for signed integers. 
// (NOTE: The most significant digits are the last ones.)
// Return value: next free char; not '\0'-terminated. 
extern char *IntEncode(char *d,unsigned int x);

// Store an object identifier name: 
// ``_aniTMT_xxx_'' where xxx is the passed id. 
// Reserve 64 bytes for the string. 
extern char *ObjectIdent(char *d,unsigned int x);

}  // namespace end 

#endif  /* _Inc_IO_ConvToAscii_H_ */
