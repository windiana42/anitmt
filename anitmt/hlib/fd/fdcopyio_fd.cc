/*
 * fdcopyio_fd.h
 * 
 * Implementation of class FDCopyIO_FD. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/fdcopybase.h>

/*#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#  warning TESTING switched on (using assert())
#  include <assert.h>
#else
#  define assert(x)  do{}while(0)
#endif*/


void FDCopyIO_FD::reset()
{
	transferred=(copylen_t)0;
}


FDCopyIO_FD::FDCopyIO_FD(int *failflag) : 
	FDCopyIO(CPT_FD,failflag)
{
	pollid=NULL;
	max_iolen=0;  // unlimited
	reset();
}

FDCopyIO_FD::~FDCopyIO_FD()
{
	pollid=NULL;  // be sure...
}
