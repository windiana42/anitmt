/*
 * mcache.cpp
 * 
 * Simple malloc cache to speed up frequent creation and 
 * destruction of objects which call MallocCache functions 
 * inside their operator new/delete. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "mcache.hpp"
#include <stdio.h>
#include <assert.h>

#error THIS SOURCE IS UNUSED. 

// Whether to do statistics or not: 
// 0 -> none at all
// 1 -> only alloc/free counter (check for unfreed chunks) 
// 2 -> full statictics 
// 3 -> full statistics and report to stderr by destructor 
#define STATISTICS 3

// static global handler: 
MallocCache *MallocCache::mcache=NULL;

inline void *MallocCache::_mem_alloc(size_t sz,CacheNode *node_ptr)
{
	char **ptr=(char**)(new char[sz]);
	*(ptr++)=(char*)node_ptr;
//fprintf(stderr,"ALLOC(node=%p,ptr=%p)\n",node_ptr,(char*)ptr);
	return((char*)ptr);
}

inline void MallocCache::_mem_free(void *_ptr)
{
	char *ptr=(char*)_ptr;
	delete[] ptr;
}


void *MallocCache::alloc(size_t sz)
{
	if(!sz)  return(NULL);
	// This statistic is only correct of operator new never calls 
	// an expection: 
	#if STATISTICS>=1
	++allocated_chunks;
	#endif
	size_t msz=_RoundupChunkSize(sz);
	if(msz>max_chunk_size)  // Too large. Allocate directly. 
	{  return(_mem_alloc(sz+sizeof(void*),NULL));  }
	
	// Try to answer from cache: 
	CacheNode *cn=&table[msz/base_chunk_size-1];
assert(cn>=table && cn<table+cache_size);
	if(!cn->nelems)  // Cache is empty; must call malloc(). 
	{
assert(!cn->top);
		#if STATISTICS>=2
		++cn->cache_misses;
		#endif
		return(_mem_alloc(msz+sizeof(void*),cn));
	}
	
	// Answer from cache: 
assert(cn->top);
	char **ptr=(char**)cn->top;  // This is the chunk to use. 
	cn->top=*ptr;  // Read the down pointer stored in the chunk. 
	*(ptr++)=(char*)cn;  // Store the cache node pointer in the chunk. 
	--cn->nelems;  // Get the counters right...
	#if STATISTICS>=2
	++cn->cache_hits;
	#endif
	return((char*)ptr);  // ...done. 
}


void MallocCache::free(void *_ptr)
{
	if(!_ptr)  return;
	#if STATISTICS>=1
	--allocated_chunks;
	#endif
	char *ptr=((char*)_ptr)-sizeof(void*);
	CacheNode *cn=*(CacheNode **)ptr;
//fprintf(stderr,"FREE(ptr=%p,node=%p)->",_ptr,cn);
	if(!cn)  // too large (no associated cache node)
	{  _mem_free(ptr);  return;  }
	// Add chunk to cache: 
	if(cn->nelems>=cn->max_free_chunks)  // no: cache is full
	{
		_mem_free(ptr);
		#if STATISTICS>=2
		++cn->cache_reject;
		#endif
//fprintf(stderr,"BB(%d,%d)\n",cn->nelems,cn->max_free_chunks);  
		return;
	}
	*((char**)ptr)=cn->top;  // set down pointer
	cn->top=ptr;  // put ptr on top of stack
	++cn->nelems;
//fprintf(stderr,"CC(%d)\n",cn->nelems);  
}


void MallocCache::clear()
{
	for(CacheNode *n=table,*nend=table+cache_size; n<nend; n++)
	{
		while(n->top)
		{
			char **tmp=(char**)n->top;
			n->top=*tmp;  // descend down pointer (stack downwards) 
			_mem_free(tmp);
		}
		n->nelems=0;
	}
}


MallocCache::MallocCache(int _cache_size,size_t _base_chunk_size)
{
	cache_size = (_cache_size<1) ? 1 : _cache_size;
	base_chunk_size = (_base_chunk_size<sizeof(void*)) ? 
		sizeof(void*) : _base_chunk_size;
	table = new CacheNode[cache_size];
	max_chunk_size=size_t(cache_size+1)*base_chunk_size;
	allocated_chunks=0;
	size_t isz=base_chunk_size;
	for(CacheNode *n=table,*nend=table+cache_size; n<nend; 
		n++,isz+=base_chunk_size)
	{
		n->cache_hits=0;
		n->cache_misses=0;
		n->cache_reject=0;
		n->nelems=0;
		n->top=NULL;
		n->max_free_chunks=65536/isz;
		if(n->max_free_chunks<16)
		{  n->max_free_chunks=16;  }
	}
	
	// set up static global handler: 
	mcache=this;
}

MallocCache::~MallocCache()
{
	#if STATISTICS>=1
	if(allocated_chunks)
	{  fprintf(stderr,"AIIE!! Destroying MallocCache with %u unfreed chunks.\n",
		allocated_chunks);  }
	#endif
	
	#if STATISTICS>=3
	// Dump statistics: 
	printf("-----------------<MallocCache Statistics>-----------------\n");
	printf("ChunkSz  Hits       Misses     Rejects    NElems  MaxFree\n");
	size_t isz=base_chunk_size;
	for(CacheNode *n=table,*nend=table+cache_size; n<nend; 
		n++,isz+=base_chunk_size)
	{
		printf("%7u  %9u  %9u  %9u  %7u  %7u\n",
			isz,
			n->cache_hits,n->cache_misses,n->cache_reject,
			n->nelems,n->max_free_chunks);
	}
	printf("----------------------------------------------------------\n");
	#endif
	
	clear();
	delete[] table;
	
	// clear static global handler: 
	mcache=NULL;
}
