/* 
 * limitmalloc.c 
 * 
 * Routines for memory allocation which limit the amount of 
 * memory beingin use at the same time. 
 * 
 * Copyright (c) 2000--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include <hlib/lmalloc.h>

#if !defined(HLIB_DONT_USE_MALLOC_USABLE_SIZE)
#include <malloc.h>
#endif

#include <string.h>


/* Enable allocation debugging (check if LFree() gets pointers 
 * actually obtained by LMalloc, etc). For that purpose, an array 
 * of pointers is held. For simplicity, this array is of fixed size 
 * and has as many elements as you specify here. 
 */
#if !defined(HLIB_SIZE_OPT) || (defined(HLIB_SIZE_OPT) && HLIB_SIZE_OPT==0)
# define AllocDebugging 16384
#else
# define AllocDebugging 0
#endif

/* NOTE: If you are not using GNU libc and/or do not have 
 * malloc_usable_size(), then define this. Each allocated block 
 * will then be assumed tobe of size 1, i.e. we're actually counting 
 * the number of *calls* to LMalloc()/LRealloc()/LFree().  
 */ 
#ifdef HLIB_DONT_USE_MALLOC_USABLE_SIZE
#  warning "*****************************************************"
#  warning "*** No using malloc_usable_size().                ***"
#  warning "*** Allocation limitation will not work.          ***"
#  warning "*** Allocation debugging cannot report lost bytes ***"
#  warning "*****************************************************"
#  define malloc_usable_size(x) 1
#endif


static struct LMallocUsage lmu=
{
	alloc_limit: 0,
	curr_used: 0,
	max_used: 0,
	malloc_calls: 0,
	realloc_calls: 0,
	free_calls: 0,
	used_chunks: 0,
	real_failures: 0,
	limit_failures: 0
};


/* Set the limit: maximum amount of memory to aquire using LMalloc(). */
void LMallocSetLimit(size_t limit)
{  lmu.alloc_limit=limit;  }

/* Get the LMallocUsage content: */
void LMallocGetUsage(struct LMallocUsage *dest)
{
	if(dest)
	{  memcpy(dest,&lmu,sizeof(lmu));  }
}


/* In case someone (me) does some #define hacks for debugging purposes: */
#ifdef LMalloc
# undef LMalloc
# define LMalloc XXXLMalloc
#endif
#ifdef LFree
# undef LFree
# define LFree XXXLFree
#endif

#if AllocDebugging
# warning "*****************************************"
# warning "*** Allocation debugging switched on. ***"
# warning "*****************************************"
  static void *allocptr[AllocDebugging];
  static int nalloc=0;
  static void DebugAlloc(void *ptr);
  static void DebugRealloc(void *oldptr,void *newptr);
  static void DebugFree(void *ptr);
# define BUGACTION  abort();
#else  /* !AllocDebugging */
# define DebugAlloc(x)
# define DebugRealloc(x,y)
# define DebugFree(x)
#endif

void *LMalloc(size_t size)
{
	void *ptr;
	if(!size)
	{  return(NULL);  }
	if(lmu.alloc_limit)
	{
		if(lmu.curr_used+size>lmu.alloc_limit)
		{  ++lmu.limit_failures;  return(NULL);  }
	}
	++lmu.malloc_calls;
	ptr=malloc(size);
	if(ptr)
	{
		++lmu.used_chunks;
		lmu.curr_used+=malloc_usable_size(ptr);
		if(lmu.max_used<lmu.curr_used)
		{  lmu.max_used=lmu.curr_used;  }
	}
	else
	{  ++lmu.real_failures;  }
	//fprintf(stderr,"malloc(%u)=%p\n",size,ptr);
	DebugAlloc(ptr);   /* only if AllocDebugging */
	return(ptr);
}

void *LFree(void *ptr)
{
	//fprintf(stderr,"free(%p)\n",ptr);
	DebugFree(ptr);   /* only if AllocDebugging */
	if(ptr)
	{
		size_t size=malloc_usable_size(ptr);
		if(lmu.curr_used>=size)
		{  lmu.curr_used-=size;  }
		else  /* should never happen... */
		{  lmu.curr_used=0;  }
		free(ptr);
		++lmu.free_calls;
		--lmu.used_chunks;
	}
	return(NULL);
}

void *LRealloc(void *ptr,size_t size)
{
	size_t oldsize;
	#if AllocDebugging
	void *_old_ptr=ptr;
	#endif
	
	if(!ptr)
	{  return(LMalloc(size));  }
	if(!size)
	{  return(LFree(ptr));  }
	
	oldsize=malloc_usable_size(ptr);
	if(lmu.curr_used>=oldsize)
	{  lmu.curr_used-=oldsize;  }
	else  /* should never happen... */
	{  lmu.curr_used=0;  }
	
	if(lmu.alloc_limit)
	{
		if(lmu.curr_used+size>lmu.alloc_limit)
		{
			lmu.curr_used+=oldsize;
			if(lmu.max_used<lmu.curr_used)
			{  lmu.max_used=lmu.curr_used;  }
			++lmu.limit_failures;
			return(NULL);
		}
	}
	
	++lmu.realloc_calls;
	ptr=realloc(ptr,size);
	if(ptr)
	{  lmu.curr_used+=malloc_usable_size(ptr);  }
	else  /* since a failed realloc() did not free its block. */
	{  ++lmu.real_failures;  lmu.curr_used+=oldsize;  }
	if(lmu.max_used<lmu.curr_used)
	{  lmu.max_used=lmu.curr_used;  }
	DebugRealloc(_old_ptr,ptr);   /* only if AllocDebugging */
	return(ptr);
}


#if AllocDebugging
static void DebugAlloc(void *ptr)
{
	int i;
	if(!ptr)  return;
	for(i=0; i<nalloc; i++)
	{
		if(ptr!=allocptr[i])  continue;
		fprintf(stderr,"Alloc: Duplicate alloc: %p (%d,%d)       **************\n",
			ptr,i,nalloc);
		BUGACTION
		return;
	}
	if(nalloc>=AllocDebugging)
	{  fprintf(stderr,"Too much alloc (%d), increase AllocDebugging\n",
		nalloc);  abort();  }
	allocptr[nalloc++]=ptr;
}
static void DebugRealloc(void *oldptr,void *newptr)
{
	int i=0;
	for(; i<nalloc; i++)
	{  if(oldptr==allocptr[i])  goto found;  }
	fprintf(stderr,"Realloc: Not allocated: %p (%d)     **************\n",
		oldptr,nalloc);
	BUGACTION
	return;
	found:;
	allocptr[i]=newptr;
}
static void DebugFree(void *ptr)
{
	int i=0; 
	if(!ptr)  return;
	for(; i<nalloc; i++)
	{  if(ptr==allocptr[i])  goto found;  }
	fprintf(stderr,"Free: Not allocated: %p (%d)      **************\n",
		ptr,nalloc);
	BUGACTION
	return;
	found:;
	--nalloc;
	for(; i<nalloc; i++)
	{  allocptr[i]=allocptr[i+1];  }
	/*memmove(allocptr+i,allocptr+i+1,nalloc-i);*/
}
#endif
