/*
 * heap.h 
 * 
 * Complete implementation of heap template (array-based heap). 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_HLHeap_H_
#define _HLIB_HLHeap_H_ 1

#include <hlib/defop.h>

#error "Untested and incomplete code."

// Implements standard array-based heap. 
// Heaps allow operations Store() and Remove() in O(log(n)), 
// FindLargest() in O(1) and Search() in O(n). They are suitable 
// for implementation of waiting queues and similar things where you 
// need quick access to the largest element but do not need to search 
// through the elements. 
// 
// This heap implementation uses an array to store the elements. 
// The array is resized as needed using reallocation (LRealloc()). 
// Note that there is nothing which hinders you to store several 
// identical elements in this heap (i.e. several equally large elements). 
// 
// WHAT TO USE FOR <T>: (Standard text from btree.h:)
// This class is probably most useful for T being a simple type 
// (integer, unique pointers to a class or small data-only structures). 
// You may use any POD type or any struct/class which is a "plain data type" 
// (PDT; see defop.h) and even for "complex data types" as long as you can 
// provide the required operator plugin OP and it has a working default 
// constructor and a working copy constructor (both for temporaries). 
template<class T,class _OP=HLDefaultOperators_PDT<T> >class HLHeap
{
	private:
		// The actual heap data fields: 
		size_t asize;   // Number of allocated array elements. 
		size_t nelem;   // Heap size (number of elements without 
		                // special first and last)
		char *_heap;    // Heap "root" (=array of elements). 
		
		// NOTE about indices: All user-visible indices are in range 
		//      0..NElem()-1. Internally, indices 1..NElem() are used 
		//      with 0 being special. 
		
	public:
		// These are the operators... normally, the default OPs 
		// will not occupy any place (i.e. 1byte compiler dummy or so). 
		_OP OP;
		
	private:
		// Use this for array subscript: 
		T &heap(size_t idx)
			{  return(*(T*)(_heap+idx*OP.size()));  }
		T *heapP(size_t idx)
			{  return((T*)(_heap+idx*OP.size()));  }
		
		// This was basically copied from OrderedArray: 
		int _realloc(size_t new_asize)
		{
			// Need 1 special extra element: [0]. 
			char *new_heap=(char*)LRealloc(_heap,(new_asize+1)*OP.size());
			if(!new_heap && new_asize)  return(-1);
			_heap=new_heap;  asize=new_asize;  return(0);
		}
		size_t _get_downsize()
		{
			if(asize>512) return((asize-nelem)>512 ? asize-256 : asize);
			if(nelem<=16)  return(16);
			return(asize-nelem>3*asize/4 ? asize-asize/2 : asize);
		}
		void _downsize()
		{	size_t newsize=_get_downsize();
			if(newsize!=asize)  _realloc(newsize);  }
		void _clearall()
		{
			// NOTE!! This is different in OrderedArray!!
			if(!OP.pdt)  for(size_t i=nelem; i; --i)  OP.clr(arrayP(i));
			nelem=0;
		}
		
		void _UpHeap(size_t idx)
		{
			// Store temporary in [0]: 
			OP.ini(heapP(0),heap(idx));
			while(idx>1 && OP.le(heap(idx/2),tmp))
			{  OP.ass(heap(idx),heap(idx/2);  idx/=2;  }
			OP.ass(heap(idx),heap(0));  OP.clr(heapP(0));
		}
		
		void _DownHeap(size_t idx)
		{
			size_t j,e=nelem/2;
			// Store temporary in [0]: 
			OP.ini(heapP(0),heap(idx));
			while(idx<=e)
			{
				j=idx+idx;
				if(j<nelem && OP.lt(heap(j),heap(j+1))  ++j;
				if(OP.le(heap(j),tmp))  break;
				OP.ass(heap(idx),heap(j));  idx=j;
			}
			OP.ass(heap(idx),heap(0));  OP.clr(heapP(0));
		}
		
	public:  _CPP_OPERATORS_FF
		HLHeap(const _OP &op=_OP(),int * /*failflag*/=NULL) : 
			asize(0),nelem(0),_heap(NULL),OP(op) {}
		~HLHeap()  {  _clearall();  _realloc(0);  }
		
		// Is the heap empty? Runtime O(1). 
		bool IsEmpty() const
			{  return(!nelem);  }
		
		// Get number of elements in heap: 
		size_t NElem() const
			{  return(nelem);  }
		
		// Get element with passed index as reference. 
		// NO RANGE CHECK! Range: 0..NElem()-1. 
		// IN CASE YOU MODIFY THE ELEMENT IN A WAY THAT IT HAS AN 
		// INCORRECT POSITION IN THE HEAP, CALL ReHeap() IMMEDIATELY. 
		// Runtime is O(1). 
		// Note: Probably only useful if you need to iterate through 
		//       the elements or obtained an index via some function. 
		T &operator[](size_t idx)
			{  return(heap(idx+1 /* <-- Correct! */));  }
		
		// Clear the whole heap. 
		void Clear()
			{  _clearall();  if(asize>32)  _realloc(0);  }
		
		// Get first (=largest) element in the heap (or NULL): 
		// Do not modify; if you did, call DownHeapFirst(). 
		// Runtime is O(1). 
		T *GetFirst()
			{  return(nelem ? heapP(1) : NULL);  }
		
		// See GetFirst(). 
		void DownHeapFirst()
			{  if(nelem>1) _DownHeap(1);   }
		
		// See operator[]: Make sure element with passed index is at 
		// the correct position in the heap (Re-order). 
		// Runtime is O(log(n)). 
		// Use size_change=+1, if the element was made larger. 
		// Use size_change=-1, if the element was made smaller. 
		// Use size_change=0, if you do not know. 
		void ReHeap(size_t idx,int size_change=0)
		{
			if(idx<0 || idx>=nelem)  return;  
			// This implementation could be optimized. 
			#warning "Optimize me..."
			++idx;  // <-- NOTE!
			if(size_change>=0)  _UpHeap(idx);
			if(size_change<=0)  _DownHeap(idx);
		}
		
		// Insert element into the heap. 
		// Runtime is O(log(n)). 
		// Return value: 
		//   0 -> OK
		//  -1 -> allocation failure (nothing done)
		int Insert(const T &e)
		{
			if(nelem>=asize && _realloc(/*new_asize=*/asize>512 ? 
				(asize+512) : (asize ? asize*2 : 16)) )  return(-1);
			OP.ini(heapP(++nelem),e);  _UpHeap(nelem);  return(0);
		}
		
		// Replace first (=largest) element by new one. 
		// Return value: 
		//   1 -> Nothing done: heap empty. 
		//   0 -> OK
		int ReplaceFirst(const T &e)
		{
			if(!nelem) return(1);
			OP.ass(heap(1),e);  _DownHeap(1);  return(0);
		}
		
		// Remove first (=largest) element. 
		// If e is non-NULL, store that element there (using OP::ass()) 
		// before removing it. 
		// Return value: 
		//   1 -> Nothing done: heap empty. 
		//   0 -> OK
		int PopFirst(T *e=NULL)
		{
			if(!nelem)  return(1);
			if(e)  OP.ass(*e,heap(1));
			if(nelem>1)  OP.ass(heap(1),heap(nelem));
			OP.clr(heapP(nelem--));  return(0);
		}
		
		// Find element in heap. 
		// Return pointer to element or NULL if not found. 
		// If there are more than one identical entries, the first 
		// one which is found, is returned. 
		// Runtime is O(n). 
		template<class K>T *Find(const K &key)
		{
			#error implement me
		}
		
		// Remove element with passed key. 
		// If e is non-NULL, store that element there (using OP::ass()) 
		// before removing it. 
		// IF there is more than one element matching the key, only the 
		// first one which is found, is removed. 
		// Runtime is O(n) (-> find)
		// Return value: 
		//   1 -> Element not found. 
		//   0 -> OK
		template<class K>int Remove(const K &key,T *e=NULL)
		{
			#error implement me
		}
		
		// Just like Remove() but instead of removing the element, 
		// replace it with new_ent. Same behavior and return value. 
		// Runtime is O(n) (-> find)
		template<class K>int Replace(const K &key,const T &new_ent)
		{
			#error implement me
		}
};


#endif  /* _HLIB_HLHeap_H_ */
