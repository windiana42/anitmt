/* 
 * limitmalloc.c 
 * 
 * Routines for memory allocation which limit the amount of 
 * memory beingin use at the same time. 
 * 
 * Copyright (c) 2000 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>


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
	return(ptr);
}

void *LFree(void *ptr)
{
	//fprintf(stderr,"free(%p)\n",ptr);
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
	return(ptr);
}
