/*
 * llongstr.c
 * 
 * Copyright (c) 2000--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/prototypes.h>


/* Int64ToStr(x) converts the signed int64_t x to a string. 
 * Returns static data. 
 */
char *Int64ToStr(int64_t val)
{
	static char ret[64];
	long sign=+1;
	if(val<0)
	{  sign=-1;  val=-val;  }
	
	{
	long l0 = (long)(val % 1000000000);  val /= 1000000000;  {
	long l1 = (long)(val % 1000000000);  val /= 1000000000;  {
	long l2 = (long)(val % 1000000000);
	
	*ret='\0';
	if(l2)      {  snprintf(ret,64,"%ld%09ld%09ld",sign*l2,l1,l0);  }
	else if(l1) {  snprintf(ret,64,"%ld%09ld",sign*l1,l0);  }
	else        {  snprintf(ret,64,"%ld",sign*l0);  }
	
	} } }
	return(ret);
}

