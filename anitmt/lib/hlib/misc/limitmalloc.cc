/* 
 * limitmalloc.c 
 * 
 * Routines for memory allocation which limit the amount of 
 * memory beingin use at the same time. 
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
#include "lmalloc.h"

#if !defined(HLIB_DONT_USE_MALLOC_USABLE_SIZE)
#include <malloc.h>
#endif

#include <string.h>


/* Verbose mem debug: log every call to malloc, realloc, free and every 
 * mem check loop. (0 -> no; 1 -> only "#" on mcheck, 2 -> all */
#define VERBOSE_MEM_DEBUG 1


/* Enable allocation debugging (check if LFree() gets pointers 
 * actually obtained by LMalloc, etc). For that purpose, an array 
 * of pointers is held. (Initial size is specified via AllocDebugging; 
 * array grows automatically.) 
 */
#ifndef AllocDebugging
# if !defined(HLIB_SIZE_OPT) || (defined(HLIB_SIZE_OPT) && HLIB_SIZE_OPT==0)
#  define AllocDebugging 1024
# else
#  define AllocDebugging 0
# endif
#elif AllocDebugging<16
# undef AllocDebugging
# define AllocDebugging 1024
#endif

#if AllocDebugging
# if HAVE_MCHECK && HAVE_MPROBE
#  if !defined(USE_GNU_MEMCHECK) || (USE_GNU_MEMCHECK!=0 && USE_GNU_MEMCHECK!=1)
#   define USE_GNU_MEMCHECK 1
#  endif
# else
#  undef USE_GNU_MEMCHECK
#  define USE_GNU_MEMCHECK 0
# endif
#endif



/* NOTE: If you are not using GNU libc and/or do not have 
 * malloc_usable_size(), then define this. Each allocated block 
 * will then be assumed tobe of size 1, i.e. we're actually counting 
 * the number of *calls* to LMalloc()/LRealloc()/LFree().  
 */ 
#ifdef HLIB_DONT_USE_MALLOC_USABLE_SIZE
#  warning "******************************************************"
#  warning "*** No using malloc_usable_size().                 ***"
#  warning "*** Allocation limitation will not work.           ***"
#  warning "*** Allocation debugging cannot report lost bytes. ***"
#  warning "******************************************************"
#  define malloc_usable_size(x) 1
#endif

#if AllocDebugging && USE_GNU_MEMCHECK
# if HAVE_MCHECK_H
#  include <mcheck.h>
# endif
/* Damn: malloc_usable_size does not seem to work with mcheck?! */
# undef malloc_usable_size
# define malloc_usable_size(x) (1)
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
	max_used_chunks: 0,
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

/* Allocation failure handler which may either do nothing, print a */
/* warning or abort immediately. NOTE THAT IT MAY _NOT_ call any   */
/* (de)allocation function because this would mess up LMalloc's    */
/* internal statistics.                                            */
void (*lmalloc_failure_handler)(size_t size_for_errormessage)=NULL;


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
# if USE_GNU_MEMCHECK
#  warning "*** Using heavy GNU mcheck/mprobe.    ***"
# endif
# warning "*****************************************"
  static void **allocptr=NULL;  /* NULL -> must call InitAllocPtr() */
  static int allocsize=0;   /* size/dim of allocptr */
  static int nalloc=0;
  static void InitAllocPtr();
  static void DebugAlloc(void *ptr);
  static void DebugRealloc(void *oldptr,void *newptr);
  static void DebugFree(void *ptr);
  static void CheckAllPtrs();
  
  /* Assign value>0 to this to write message for each call      */
  /* to (de/re)allocation functions. These can be automatically */
  /* analyzed by some tool.                                     */
  /*   0 -> disable;                                            */
  /*   1 -> count seqs but no messages (useful for              */
  /*        hlib_allocdebug_abort_seq, see below)               */
  /*   2 -> like 1 but also print messages.                     */
  extern int hlib_allocdebug_trace;
  int hlib_allocdebug_trace=0;
  /* Increasing sequence number: */
  size_t hlib_allocdebug_seq=0;
  /* Set this to some value and HLib will abort() at the time  */
  /* the allocation function with the specified seq is called. */
  /* Requires hlib_allocdebug_trace>=1.                        */
  extern size_t hlib_allocdebug_abort_seq;
  size_t hlib_allocdebug_abort_seq=0xffffffff;
  
  /* If non-null, this function will be called whenever an alloc */
  /* routine is called. Useful to run a check and see if memory  */
  /* has been corrupted.                                         */
  extern void (*hlib_allocdebug_checkhdl)(void);
  void (*hlib_allocdebug_checkhdl)(void)=NULL;
  
# define BUGACTION  abort();
#else  /* !AllocDebugging */
# define DebugAlloc(x)
# define DebugRealloc(x,y)
# define DebugFree(x)
#endif

void *LMalloc(size_t size)
{
	void *ptr;
	
	#if AllocDebugging  /* needed for mcheck() */
		if(!allocptr)  InitAllocPtr();
		if(!size)  CheckAllPtrs();   /* special... */
	#endif
	
	if(!size)
	{  return(NULL);  }
	if(lmu.alloc_limit)
	{
		if(lmu.curr_used+size>lmu.alloc_limit)
		{
			++lmu.limit_failures;
			if(lmalloc_failure_handler)
			{  (*lmalloc_failure_handler)(size);  }
			return(NULL);
		}
	}
	++lmu.malloc_calls;
	ptr=malloc(size);
	if(ptr)
	{
		++lmu.used_chunks;
		if(lmu.max_used_chunks<lmu.used_chunks)
		{  lmu.max_used_chunks=lmu.used_chunks;  }
		lmu.curr_used+=malloc_usable_size(ptr);
		if(lmu.max_used<lmu.curr_used)
		{  lmu.max_used=lmu.curr_used;  }
	}
	else
	{
		++lmu.real_failures;
		if(lmalloc_failure_handler)
		{  (*lmalloc_failure_handler)(size);  }
	}
	
	#if VERBOSE_MEM_DEBUG>=2
		fprintf(stderr,"malloc(%u)=%p (**)\n",size,ptr);
	#endif
	
	#if AllocDebugging
	if(hlib_allocdebug_trace)
	{
		if(hlib_allocdebug_trace>1)
		{  fprintf(stderr,"@HLAT->malloc[%u](%u)=%p\n",
			hlib_allocdebug_seq,size,ptr);  }
		if(hlib_allocdebug_seq==hlib_allocdebug_abort_seq)
		{  abort();  }
		++hlib_allocdebug_seq;
	}
	#endif
	
	DebugAlloc(ptr);   /* only if AllocDebugging */
	return(ptr);
}

void *LFree(void *ptr)
{
	#if VERBOSE_MEM_DEBUG>=2
		fprintf(stderr,"free(%p) (**)\n",ptr);
	#endif
	
	#if AllocDebugging
	if(hlib_allocdebug_trace)
	{
		if(hlib_allocdebug_trace>1)
		{  fprintf(stderr,"@HLAT->free[%u](%p)\n",hlib_allocdebug_seq,ptr);  }
		if(hlib_allocdebug_seq==hlib_allocdebug_abort_seq)
		{  abort();  }
		++hlib_allocdebug_seq;
	}
	#endif
	
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
	
	#if 0   /* <-- Use malloc() and free() instead if realloc(). */
	#warning "<--NO REALLOC--!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!-->"
	{
		/* This depends on a working malloc_usable_size(). */
		void *nptr=LMalloc(size);
		size_t osize=malloc_usable_size(ptr);
		if(!nptr)  return(NULL);
		memcpy(nptr,ptr,osize<size ? osize : size);
		LFree(ptr);
		return(nptr);
	}
	#endif
	
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
			if(lmalloc_failure_handler)
			{  (*lmalloc_failure_handler)(size);  }
			return(NULL);
		}
	}
	
	++lmu.realloc_calls;
	ptr=realloc(ptr,size);
	if(ptr)
	{  lmu.curr_used+=malloc_usable_size(ptr);  }
	else  /* since a failed realloc() did not free its block. */
	{
		++lmu.real_failures;
		lmu.curr_used+=oldsize;
		if(lmalloc_failure_handler)
		{  (*lmalloc_failure_handler)(size);  }
	}
	if(lmu.max_used<lmu.curr_used)
	{  lmu.max_used=lmu.curr_used;  }
	
	#if VERBOSE_MEM_DEBUG>=2
		fprintf(stderr,"realloc(%p,%u)=%p (**)\n",_old_ptr,size,ptr);
	#endif
	
	#if AllocDebugging
	if(hlib_allocdebug_trace)
	{
		if(hlib_allocdebug_trace>1)
		{  fprintf(stderr,"@HLAT->realloc[%u](%p,%u)=%p\n",hlib_allocdebug_seq,
			_old_ptr,size,ptr);  }
		if(hlib_allocdebug_seq==hlib_allocdebug_abort_seq)
		{  abort();  }
		++hlib_allocdebug_seq;
	}
	#endif
	
	DebugRealloc(_old_ptr,ptr);   /* only if AllocDebugging */
	
	return(ptr);
}


#if AllocDebugging

static void _AllocDump(const char *str)
{
	int j=1;
	fprintf(stderr,"AllocDump[%s](%p",str,allocptr[0]);
	for(; j<nalloc; j++)
	{  fprintf(stderr,",%p",allocptr[j]);  }
	fprintf(stderr,")\n");
}

static void CheckAllPtrs()
{
	if(hlib_allocdebug_checkhdl)
	{  (*hlib_allocdebug_checkhdl)();  }
	
	#if USE_GNU_MEMCHECK
	#if VERBOSE_MEM_DEBUG>=2
	fprintf(stderr,"mcheck[%d]...",nalloc);
	#elif VERBOSE_MEM_DEBUG
	fprintf(stderr,"#");
	#endif
	int i=1;
	for(; i<nalloc; i++)
	{
		int pv=mprobe(allocptr[i]);
		switch(pv)
		{
			case MCHECK_OK:  goto cont;
			case MCHECK_DISABLED:
				fprintf(stderr,"mcheck: disabled.\n");
				break;
 			case MCHECK_HEAD:  /* fall through */
			case MCHECK_TAIL:
				fprintf(stderr,"mcheck: ptr=%p: memory clobbered %s "
					"of allocated block *************\n",
					allocptr[i],pv==MCHECK_HEAD ? "before start" : "past end");
				break;
			case MCHECK_FREE:
				fprintf(stderr,"mcheck: ptr=%p: already freed ***********\n",
					allocptr[i]);
				break;
			default:
				fprintf(stderr,"mcheck: ptr=%p: unknown mprobe() val %d\n",
					allocptr[i],pv);
				break;
					
		}
		BUGACTION
		cont:;
	}
	#if VERBOSE_MEM_DEBUG>=2
	fprintf(stderr,"OK\n");
	#endif
	#endif  /* USE_GNU_MEMCHECK */
}

static void InitAllocPtr()
{
	int i;
	
	#if USE_GNU_MEMCHECK
	int rv=mcheck(NULL);
	if(rv!=0)
	{  fprintf(stderr,"%s: mcheck() initialisation failed: rv=%d\n",
		prg_name,rv);  abort();  }
	#endif
	
	fprintf(stderr,"%s: (HLib allocation debugging enabled%s.)\n",prg_name,
	#if USE_GNU_MEMCHECK
			" [GNU mcheck]"
	#else
			""
	#endif
		);
	
	/* Set up pointer array: */
	allocsize=AllocDebugging;
	if(allocsize<8)  allocsize=8;
	allocptr=(void**)malloc(allocsize*sizeof(void*));
	if(!allocptr)
	{  fprintf(stderr,"%s: alloc debugging: ran out of memory (n=%d).",
		prg_name,allocsize);  abort();  }
	
	/* Initialize with zero to be sure: */
	for(i=0; i<allocsize; i++)
	{  allocptr[i]=NULL;  }
	
	/* Init done: */
	nalloc=0;
}

/* Do binary search in allocptr array; return 1 if found and 0 if not. 
 * ret_idx returns the index of the element if found or before which 
 * element where to insert if not found. */
static int _FindAllocPtr(void *ptr,int *ret_idx)
{
	int a,b;
	if(!nalloc)
	{  *ret_idx=0;  return(0);  }
	a=0;
	b=nalloc-1;
	if(ptr<allocptr[a])  {  *ret_idx=0;  return(0);  }
	if(ptr>allocptr[b])  {  *ret_idx=nalloc;  return(0);  }
	while(b-a>1)
	{
		int m=(a+b)/2;
		if(allocptr[m]<ptr) a=m; else b=m;
	}
	if(ptr==allocptr[a])  {  *ret_idx=a;  return(1);  }
	*ret_idx=b;
	return(ptr==allocptr[b]);
}
/* Insert pointer into the array at specified position. */
static void _InsertAllocPtr(void *ptr,int idx)
{
	if(nalloc>=allocsize)
	{
		/* Must enlarge pointer array: */
		int oldsize=allocsize,i;
		/* NOTE: This depends on allocsize to be >=2. */
		allocsize+=allocsize/2;
		allocptr=(void**)realloc(allocptr,allocsize*sizeof(void*));
		if(!allocptr)
		{  fprintf(stderr,"%s: alloc debugging: ran out of memory (n=%d,%d).",
			prg_name,nalloc,allocsize);  abort();  }
		for(i=oldsize; i<allocsize; i++)
		{  allocptr[i]=NULL;  }
	}
	
	/* Actually do the insertion: */
	int i;
	for(i=nalloc; i>idx; i--)  allocptr[i]=allocptr[i-1];
	allocptr[idx]=ptr;
	++nalloc;
}
/* Remove pointer with specified index from array. */
static void _RemoveAllocPtr(int idx)
{
	int i;
	if(--nalloc<0)
	{
		fprintf(stderr,"Free: OOPS: nalloc=%d<0.    **************\n",nalloc);
		BUGACTION
	}
	for(i=idx; i<nalloc; i++)  allocptr[i]=allocptr[i+1];
	allocptr[nalloc]=NULL;
}
/* Bring modified pointer to the correct position in the array. */
static void _ReorderAllocPtr(int idx)
{
	void *ptr=allocptr[idx];
	while(idx+1<nalloc && allocptr[idx+1]<ptr)
	{  allocptr[idx]=allocptr[idx+1];  ++idx;  }
	while(idx>0 && allocptr[idx-1]>ptr)
	{  allocptr[idx]=allocptr[idx-1];  --idx;  }
	allocptr[idx]=ptr;
}

static void DebugAlloc(void *ptr)
{
	int i;
	if(!allocptr)  InitAllocPtr();
	CheckAllPtrs();
	if(!ptr)  return;
	if(_FindAllocPtr(ptr,&i))
	{
		fprintf(stderr,"Alloc: Duplicate alloc: %p (%d,%d)       **************\n",
			ptr,i,nalloc);
		BUGACTION
		return;
	}
	_InsertAllocPtr(ptr,i);
}
static void DebugRealloc(void *oldptr,void *newptr)
{
	int i,j;
	if(!allocptr)  InitAllocPtr();
	if(!_FindAllocPtr(oldptr,&i))
	{
		fprintf(stderr,"Realloc: Not allocated: %p (%d)     **************\n",
			oldptr,nalloc);
		BUGACTION
		return;
	}
	if(oldptr==newptr)  return;
	if(_FindAllocPtr(newptr,&j))
	{
		fprintf(stderr,"Realloc(%p -> %p): Duplicate alloc: %p (%d,%d) **************\n",
			oldptr,newptr,newptr,j,nalloc);
		BUGACTION
		return;
	}
	allocptr[i]=newptr;
	_ReorderAllocPtr(i);
	CheckAllPtrs();  /* MUST BE DONE AT THE END! */
}
static void DebugFree(void *ptr)
{
	int i; 
	if(!allocptr)  InitAllocPtr();
	CheckAllPtrs();
	if(!ptr)  return;
	if(!_FindAllocPtr(ptr,&i))
	{
		fprintf(stderr,"Free: Not allocated: %p (%d)      **************\n",
			ptr,nalloc);
		BUGACTION
		return;
	}
	_RemoveAllocPtr(i);
}
#endif
