/*
 * statcache.hpp
 * 
 * Header file for class FileStateCache. 
 * This class simply caches the stat(2) results. 
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

#ifndef _RNDV_LIB_STATCACHE_HPP_
#define _RNDV_LIB_STATCACHE_HPP_ 

#include "prototypes.hpp"

#include <hlib/linkedlist.h>
#include <hlib/htime.h>
#include <hlib/refstring.h>


// This class simply caches the stat(2) results. 
class FileStateCache
{
	private:
		struct Node : LinkedListBase<Node>
		{
			RefString path;
			
			// Cache only these properties: 
			HTime mtime;
			int64_t size;  // -2 -> invalid
			
			_CPP_OPERATORS_FF
			Node(int *failflag);
			~Node() {}
		};
		
		// For debugging: 
		int first_hits;
		int second_hits;
		int misses;
		
		// 1, if the complete stack (all elementd) are invalid. 
		// 0 -> complete stack MAY be invalid but probably is not. 
		// Speeds up InvalidateAll(). 
		int completely_invalid : 1;
		int : 31;
		
		// Currently, the implementation is quite simple: 
		// Just a normal linked list. 
		LinkedList<Node> nlist;
		
		// Look up an entry based on the path: 
		Node *_FindEntry(const RefString *path);
		// Create a new entry for the passed path. 
		Node *_CreateEntry(const RefString *path);
		
	public:  _CPP_OPERATORS_FF
		FileStateCache(int *failflag=NULL);
		~FileStateCache();
		
		// Add/update cache element: 
		// Simply stores the values as passed, even if they 
		// are invalid. On alloc failure, simply don't cache it. 
		void UpdateCache(const RefString *path,const HTime *mtime,
			int64_t size);
		
		// Get cached values. 
		// May pass NULL if not interested in mtime/size. 
		// Return value: 
		//   0 -> OK
		//   1 -> path was not in the cache
		int GetCache(const RefString *path,HTime *ret_mtime,int64_t *ret_size);
		
		// Remove corresponding entry from the cache: 
		void RemoveCache(const RefString *path);
		
		// Set invalid entries for all elements in cache. 
		// (HTime::Invalid, size=-2 <- !NOTE!). 
		void InvalidateAll();
		
		// CLear the cache freeing all elements. 
		void Clear();
};

#endif  /* _RNDV_LIB_STATCACHE_HPP_ */
