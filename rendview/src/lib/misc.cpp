/*
 * misc.cpp
 * Misc routines...
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

#include <string.h>


RendViewOpMode GetRendViewOpMode(const char *pname)
{
	if(!strcasecmp(pname,"rendview"))   return(RVOM_RendView);
	if(!strcasecmp(pname,"ldrclient"))  return(RVOM_LDRClient);
	if(!strcasecmp(pname,"ldrserver"))  return(RVOM_LDRServer);
	return(RVOM_None);
}


void ConvertTimeout2MSec(long *timeout,const char *which)
{
	// Convert timeout from seconds to msec: 
	if((*timeout)<=0)
	{  (*timeout)=-1;  }
	else
	{
		long old=*timeout;
		(*timeout)*=1000;
		if((*timeout)/1000!=old)
		{
			Warning("Integer overflow for timeout %ld seconds%s%s%s. "
				"Disabled.\n",old,
				which ? " (" : "",which ? which : "",which ? ")" : "");
			*timeout=-1;
		}
	}
}
