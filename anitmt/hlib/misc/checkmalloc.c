/*
 * checkmalloc.c
 *
 * Copyright (c) 1999--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>

extern char *prg_name;

void *CheckMalloc(void *ptr)
{
	if(!ptr)
	{
		fprintf(stderr,"%s: malloc() failed.\n",prg_name);
		abort();   /* Helps in debugging. */
		exit(1);
	}
	return(ptr);
}
