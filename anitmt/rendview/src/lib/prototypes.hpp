/*
 * prototypes.hpp
 * 
 * Some misc. prototypes...
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _RNDV_LIB_PROTOTYPES_HPP_
#define _RNDV_LIB_PROTOTYPES_HPP_ 1

#include <hlib/prototypes.h>

void Error(const char *fmt,...);
void Warning(const char *fmt,...);
void Verbose(const char *fmt,...);
void VerboseSpecial(const char *fmt,...);

// Returns number of CPUs; assumes one CPU is detection fails. 
int GetNumberOfCPUs();

// Returns the system load multiplied with 100: 
// Returns -1 on systems where that is not supported. 
int GetLoadValue();

#endif  /* _RNDV_LIB_PROTOTYPES_HPP_ */
