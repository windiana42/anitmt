/*
 * get_load.cpp
 * Get the system load. Uses hlib's GetLoadAverage(). 
 *
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "prototypes.hpp"
#include <fcntl.h>
#include <string.h>
#include <assert.h>


// Returns the system load multiplied with 100: 
// Returns -1 on systems where that is not supported. 
int GetLoadValue()
{
	int lv=GetLoadAverage();
	Verbose(DBGV,"Current load average: %f\n",double(lv)/100.0);
	if(lv>=0)  return(lv);
	
	static int warned=0;
	if(!warned)
	{  Warning("Failed to get load from system; "
		"no load control feature available.\n");  ++warned;  }
	return(-1);
}

