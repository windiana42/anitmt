/*
 * cfexit.cc
 * 
 * Constructor failed exit routine.
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
#include "cplusplus.h"

void ConstructorFailedExit(const char *opt)
{
	fprintf(stderr,"%s: constructor (%s) failed\n",prg_name,opt ? opt : "");
	abort();  // It might be better to abort than to exit as an aid in debugging. 
	exit(1);
}
