/*
 * cpbase.cc
 * 
 * Implementation of class FDCopyBase, a base class for copying 
 * from and to file descriptors which works in cooperation 
 * with class FDCopyManager. 
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

#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/cpmanager.h>
#include <hlib/cpbase.h>


#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. 
#warning Using assert()
#include <assert.h>
#else
#define assert(x)
#endif


FDCopyBase::FDCopyBase(int *failflag) :
	LinkedListBase<FDCopyBase>(),
	rlist(failflag)
{
	int failed=0;
	
	if(cpmanager()->Register(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  return;  }
	else if(failed)
	{  ConstructorFailedExit("CPC");  }
}

FDCopyBase::~FDCopyBase()
{
	// Simply unregister; the server will clean up rlist. 
	cpmanager()->Unregister(this);
	
	assert(rlist.is_empty());
}


/******************************************************************************/

FDCopyManager::CopyRequest::CopyRequest(int * /*failflag*/)
{
	srcfd=destfd=-1;
	srcbuf=destbuf=NULL;
	len=srclen=destlen=0;
	
	req_timeout=-1;
	read_timeout=-1;
	write_timeout=-1;
	
	dptr=NULL;
	progress_mask=PAQuery;
	
	recv_fdnotify=0;
	
	iobufsize=FDCopyManager::default_iobufsize;
	low_read_thresh=-1;
	high_read_thresh=-1;
	low_write_thresh=-1;
	high_write_thresh=-1;
	
	max_read_len=0;
	max_write_len=0;
	
	errcode=0;
}

FDCopyManager::CopyRequest::~CopyRequest()
{
	// Only cancel the most important fields...
	srcbuf=destbuf=NULL;
	len=0;
	dptr=NULL;
}
