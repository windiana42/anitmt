/*
 * getprgname.c 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include "prototypes.h"

#include <string.h>


/* GetPrgName()
 * Basically something like the basename shell command. 
 * Used to get the program name from argv[0]. Will return "???" in 
 * case arg[0] is NULL or an empty string. 
 */
const char *GetPrgName(const char *arg0)
{
	if(arg0)
	{
		const char *pn=strrchr(arg0,'/');
		pn=(pn ? (pn+1) : (char*)arg0);
		if(*pn)  return(pn);
	}
	return("???");
}

