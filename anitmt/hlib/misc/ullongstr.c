/*
 * ullongstr.c
 * 
 * Copyright (c) 2000 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>


#define ULong unsigned long

/* UInt64ToStr(x) converts the u_int64_t x to a string. 
 * Returns static data. 
 */
char *UInt64ToStr(u_int64_t val)
{
	static char ret[64];
	
	ULong l0 = (ULong)(val % 1000000000);  val /= 1000000000;  {
	ULong l1 = (ULong)(val % 1000000000);  val /= 1000000000;  {
	ULong l2 = (ULong)(val % 1000000000);
	
	*ret='\0';
	if(l2)      {  snprintf(ret,64,"%lu%09lu%09lu",l2,l1,l0);  }
	else if(l1) {  snprintf(ret,64,"%lu%09lu",l1,l0);  }
	else        {  snprintf(ret,64,"%lu",l0);  }
	
	} }
	return(ret);
}
