/*
 * cplusplusff.cc
 * 
 * Routines for _CPP_OPERATORS_FF macro. 
 * (Reserve memory/apply reservation -- simple.) 
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


#ifndef TESTING
#define TESTING 1
#endif

static size_t newprepsize=0;
static void *newprepmem=NULL;

#if TESTING
#warning TESTING switched on. 
#endif

// ret: 0 -> success; -1 -> malloc() failed. 
// _ptr: NULL -> use LMalloc(); else use _ptr. 
int _NewPrepareMemory(size_t size,void *_ptr=NULL)
{
	#if TESTING
	// If you get the followin error, then one possibility is that 
	// you simply forgot to use _CPP_OPERATORS_FF in the public part 
	// of the class to be allocated. 
	if(newprepmem || newprepsize)
	{  fprintf(stderr,"Warning memory reserved but not used?! %p, %u\n",
		newprepmem,newprepsize);  }
	#endif
	if(newprepmem)
	{  LFree(newprepmem);  }
	newprepsize=0;
	newprepmem = _ptr ? _ptr : LMalloc(size);
	if(!newprepmem)
	{  return(-1);  }  // malloc failed. 
	newprepsize=size;
	return(0);  // success. 
}


void *_NewPrepareApply(size_t size)
{
	if(newprepsize>=size)  // >=, as we sometimes want to allocate more 
	{                      // than operator new thinks...
		void *ptr=newprepmem;
		newprepmem=NULL;
		newprepsize=0;
		return(ptr);
	}
	
	#if TESTING
	if(newprepsize)
	{  fprintf(stderr,"error: reserved %u bytes, want to use %u.\n",
		newprepsize,size);  exit(1);  }
	#endif
	
	// fall back to behavior of operator new in _CPP_OPERATORS: 
	return(CheckMalloc(LMalloc(size)));
}

