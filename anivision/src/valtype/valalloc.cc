/*
 * valtype/valalloc.cc
 * 
 * Value allocation code. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "valtypes.h"

#include <hlib/lmalloc.h>
//#include <hlib/linkedlist.h>

// Typical alloc sizes in value context: 
//   Value       -> 8,12 bytes (24 for Range)
//   double[3]   ->   24 bytes
//   double[3*3] ->   72 bytes
//   double[4*4] ->  128 bytes


// All memory returned by this allocator is aligned on 8 byte boundaries: 
static const size_t req_align=8;

#warning "Need a faster implementation."
// ...meanwhile we can collect some statistics. 
#define MAXSTATSIZE 1024
#define STATDIV 4
static int alloc_counter[MAXSTATSIZE/STATDIV];
static size_t alloc_cnt=0,free_cnt=0;
static int _Init()
{
	for(int i=0; i<MAXSTATSIZE/STATDIV; i++)
	{  alloc_counter[i]=0;  }
	return(0);
}
static int _init_done=_Init();
void ValAllocPrintStat()
{
	printf("ValAlloc statistics: Counts: alloc=%d, free=%d, delta=%d\n",
		alloc_cnt,free_cnt,alloc_cnt-free_cnt);
	for(int i=0; i<MAXSTATSIZE/STATDIV; i++)
	{
		if(!alloc_counter[i])  continue;
		printf("  size=%4d;  count=%10d\n",
			i*STATDIV,alloc_counter[i]);
	}
	
	// Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sAlloc: %u bytes in %d chunks; Peak: %u by,%d chks; "
		"(%u/%u/%u)%s\n",
		prg_name,
		lmu.curr_used ? "*** " : "",
		lmu.curr_used,lmu.used_chunks,lmu.max_used,lmu.max_used_chunks,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls,
		lmu.curr_used ? " ***" : "");
}


void *ValueNamespace::_ValAlloc(size_t size)
{
	assert(((int)size)>0);  // especially NOT <0. 
	
	// size!=0 here 
	void *ptr=LMalloc(size);
	if(!ptr) CheckMalloc(ptr);
	
	if(size<MAXSTATSIZE)
	{  ++alloc_counter[size/STATDIV];  }
	++alloc_cnt;
	
	return(ptr);
}

void *ValueNamespace::_ValRealloc(void *ptr,size_t size)
{
	assert(((int)size)>0);  // especially NOT <0. 
	
	void *nptr=LRealloc(ptr,size);
	if(!nptr) CheckMalloc(nptr);
	
	if(size<MAXSTATSIZE)
	{  ++alloc_counter[size/STATDIV];  }
	
	return(nptr);
}

void ValueNamespace::_ValFree(void *ptr)
{
	// ptr is non-NULL here 
	
	++free_cnt;
	
	LFree(ptr);
}


void ValueNamespace::_illsize(int size)
{
	fprintf(stderr,"valtypes: Attempt to alloc array of illegal size %d\n",
		size);
	assert(0);
	exit(1);
}
