/*
 * mcache.hpp
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

#ifndef _Util_Malloc_Cache_H_
#define _Util_Malloc_Cache_H_ 1

#include <stddef.h>

#error THIS SOURCE IS UNUSED. 
// (Used it for: Value, Operator, OperatorNode)

// !!!!!!!!NOTE!!!!!!!!!
// MY EXPERIMENTS WITH GLIBC's MALLOC SHOWED THAT THE MALLOC CACHE 
// CANNOT INCREASE PROGRAM RUNTIME, EVEN WITH HIGHEST POSSIBLE 
// CACHE HIT RATES. DO NOT USE UNLESS YOU VERIFY THAT IT MAKES 
// YOUR PROGRAM RUN FASTER. 

//! This is a simple malloc cache. 
//! It caches memory chunks of same size in stacks. 
//! For quick access, memory chunk sizes are a multiple of 
//! base_chunk_size and the cache can only hold memory chunks 
//! in sizes base_chunk_size...base_chunk_size*cache_size. 
//! Larger memory chunks are directly redirected to malloc(). 
//! You can assign a max_free_chunks value which limits the 
//! max. number of unused chunks per chunk size. 
//! Note that sizeof(void*) bytes get lost per chunk for the 
//! cache's table back pointer. To be smarter to malloc(), the 
//! cache therefore uses usable chunk sizes of e.g. 
//! 12, 28, 44 instead of
//! 16, 32, 48 (sizeof(void*)=4; base_chunk_size=16)

//! HOW TO USE IT: 
//! The MallocCache shall help you to speed up object creation and 
//! destruction for small objects which get allocated via operator 
//! new and deleted via operator delete very often. For this to work, 
//! simply define custom operator new/delete for these objects: 
//! void *operator new(size_t sz)
//!     {  return(MCache()->alloc(sz));  }
//! void operator delete(void *ptr)
//!     {  return(MCache()->free(ptr));  }
//! The alloc and free routines behave exactly like new and delete. 
//! (In fact they call new[] char[size] and delete[].) 
//! You need at least one MallocCache per thread. MallocCache is 
//! not thread save but feel free to implement thread save routines 
//! alloc_threadsave() and free_threadsave(). 
//! Be sure not to destroy the MallocCache before all the allocated 
//! chunks are freed. (It will warn you in this case.) 

class MallocCache
{
	public:
		// THIS IS NOT THREAD-SAVE (not thread-save) 
		static MallocCache *mcache;
	private:
		size_t base_chunk_size;
		int cache_size;
		size_t max_chunk_size;  // max (total) size of chunk answered from cache
		struct CacheNode
		{
			size_t cache_hits;     // how often alloc() was answered from cache
			size_t cache_misses;   // how often alloc() asked ::malloc() 
			size_t cache_reject;   // how often free() directly called ::free()
			size_t nelems;  // number of elements in cache
			size_t max_free_chunks;  // max value of nelems
			char *top;      // top element in cache (*(char**)top is down-pointer) 
		} *table;   //
		size_t allocated_chunks;  // total number
		
		size_t _RoundupChunkSize(size_t sz)
		{
			sz+=sizeof(void*);
			return(sz+base_chunk_size-(sz-base_chunk_size)%base_chunk_size);
		}
		
		//! Allocates a chunk of memory of size sz using malloc(), 
		//! stores node_ptr at the address and returns 
		//! address+sizeof(void*). Never returns NULL. 
		inline void *_mem_alloc(size_t sz,CacheNode *node_ptr);
		inline void _mem_free(void *ptr);
	public:
		//! The max. cache-servable chunk has a size of 
		//! (cache_size+1)*base_chunk_size-sizeof(void*). 
		MallocCache(int cache_size=8,size_t base_chunk_size=16);
		~MallocCache();
		
		//! Allocate a chunk of memory. Works like operator new. 
		void *alloc(size_t size);
		//! Free a chunk allocated by this MallocCache. 
		void free(void *ptr);
		
		//! Clear the cache freeing all cached memory. 
		void clear();
};

// You could make this thread-save by using a different function: 
inline MallocCache *MCache()
{  return(MallocCache::mcache);  }

#endif  /* _Util_Malloc_Cache_H_ */
