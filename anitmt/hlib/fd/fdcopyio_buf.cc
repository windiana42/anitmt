/*
 * fdcopyio_buf.h
 * 
 * Implementation of class FDCopyIO_Buf. 
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

#define HLIB_IN_HLIB 1
#include <hlib/fdcopybase.h>

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#  warning TESTING switched on (using assert())
#  include <assert.h>
#else
#  define assert(x)  do{}while(0)
#endif


int FDCopyIO_Buf::dataptr(DataPtr *dp)
{
	// dp=NULL is special case: return 0 to show 
	// FDCopyPump that we support dataptr(). 
	if(!dp)  return(0);
	
	dp->buf=buf+bufdone;
	dp->buflen=buflen-bufdone;
	dp->more=0;   // dp->buflen is all, there is not more buffer
	
	return(0);
}

int FDCopyIO_Buf::datadone(DataDone *dd)
{
	assert(dd->buf==buf);
	bufdone+=dd->donelen;
	assert(bufdone<=buflen);
	return(0);
}


void FDCopyIO_Buf::reset()
{
	buf=NULL;
	buflen=0;
	bufdone=0;
}


FDCopyIO_Buf::FDCopyIO_Buf(int *failflag) : 
	FDCopyIO(CPT_Buf,failflag)
{
	reset();
}

FDCopyIO_Buf::~FDCopyIO_Buf()
{
	buf=NULL;  // be sure...
}
