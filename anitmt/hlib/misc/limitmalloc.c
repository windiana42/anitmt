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

#include <hlib/prototypes.h>

/* Enable allocation debugging (check if LFree() gets pointers 
 * actually obtained by LMalloc, etc). For that purpose, an array 
 * of pointers is held. For simplicity, this array is of fixed size 
 * and has as many elements as you specify here. 
 */
#define AllocDebugging 16384

static size_t malloc_limit=0;   /* 0 -> unlimited */
static size_t curr_size=0;   /* current amount of malloc()ed memory */
static size_t max_size=0;    /* max amount used */

static int real_failures=0;
static int limit_failures=0;


/* Returns amount of currently allocated memory: */
size_t LMallocCurrentUsage()
{  return(curr_size);  }

/* Returns max amount of allocated memory: */
size_t LMallocMaxUsage()
{  return(max_size);  }

/* Set the limit: maximum amount of memory to aquire using LMalloc(). */
void LMallocSetLimit(size_t limit)
{  malloc_limit=limit;  }
size_t LMallocGetLimit()
{  return(malloc_limit);  }

int LMallocRequestsFailed(int flag)
{  return(flag ? real_failures : limit_failures);  }


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
#warning AllocDebugging swicthed on!! PERFORMANCE LOSS
static void *allocptr[AllocDebugging];
static int nalloc=0;
static void DebugAlloc(void *ptr);
static void DebugRealloc(void *oldptr,void *newptr);
static void DebugFree(void *ptr);
#define BUGACTION  abort();
#else  /* !AllocDebugging */
#define DebugAlloc(x)
#define DebugRealloc(x,y)
#define DebugFree(x)
#endif

void *LMalloc(size_t size)
{
	void *ptr;
	if(!size)
	{  return(NULL);  }
	if(malloc_limit)
	{
		if(curr_size+size>malloc_limit)
		{  ++limit_failures;  return(NULL);  }
	}
	ptr=malloc(size);
	if(ptr)
	{
		curr_size+=malloc_usable_size(ptr);
		if(max_size<curr_size)
		{  max_size=curr_size;  }
	}
	else
	{  ++real_failures;  }
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
		if(curr_size>=size)
		{  curr_size-=size;  }
		else  /* should never happen... */
		{  curr_size=0;  }
		free(ptr);
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
	if(curr_size>=oldsize)
	{  curr_size-=oldsize;  }
	else  /* should never happen... */
	{  curr_size=0;  }
	
	if(malloc_limit)
	{
		if(curr_size+size>malloc_limit)
		{
			curr_size+=oldsize;
			if(max_size<curr_size)
			{  max_size=curr_size;  }
			++limit_failures;
			return(NULL);
		}
	}
	
	ptr=realloc(ptr,size);
	if(ptr)
	{  curr_size+=malloc_usable_size(ptr);  }
	else  /* since a failed realloc() did not free its block. */
	{  ++real_failures;  curr_size+=oldsize;  }
	if(max_size<curr_size)
	{  max_size=curr_size;  }
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
	fprintf(stderr,"Realloc: Never allocated: %p (%d)     **************\n",
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
	fprintf(stderr,"Free: Never allocated: %p (%d)      **************\n",
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
