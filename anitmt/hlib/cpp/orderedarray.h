/*
 * orderedarray.h 
 * 
 * Complete implementation of an ordered array template for PODs. 
 * Includes binary search and alloc-ahead. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_OrderedArray_H_
#define _HLIB_OrderedArray_H_ 1

#include <hlib/defop.h>


// NOTE: This class is an ordered array of elements of type T. 
// The elements are stored in an allocated chunk of memory which 
// if LRealloc()ated when needed (applying alloc-ahead). Hence, 
// never use pointers to array elements, always use indices. 
// WHAT TO USE FOR <T>: 
// This class is probably most useful for T being integer or 
// unique pointers to a class. You may use any POD type or 
// any struct/class which is a "plain data type" (PDT; see defop.h) 
// and even for "complex data types" as long as you can provide the 
// required operator plugin OP and it has a working default constructor 
// and a working copy constructor (both for temporaries). 
template<class T,class _OP=HLDefaultOperators_PDT<T> >class OrderedArray
{
	private:
		// The actual array data fields: 
		char *_array;  // Use array(idx) and arrayP(idx) for subscript. 
		size_t asize;  // number of allocated elements
		size_t nelem;  // number of elements
		
	public:
		// These are the operators... normally, the default OPs 
		// will not occupy any place (i.e. 1byte compiler dummy or so). 
		_OP OP;
		
	private:
		// Use this for array subscript: 
		T &array(size_t idx)
			{  return(*(T*)(_array+idx*OP.size()));  }
		T *arrayP(size_t idx)
			{  return((T*)(_array+idx*OP.size()));  }
		
		int _realloc(size_t new_asize)
		{
			char *new_array=(char*)LRealloc(_array,new_asize*OP.size());
			if(!new_array && new_asize)  return(-1);
			_array=new_array;  asize=new_asize;  return(0);
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
		{	if(!OP.pdt)  for(size_t i=nelem; i--;)  OP.clr(arrayP(i));
			nelem=0;  }
	public:  _CPP_OPERATORS_FF
		OrderedArray(const _OP &op=_OP(),int * /*failflag*/=NULL) : OP(op)
			{  _array=NULL;  asize=0;  nelem=0;  }
		~OrderedArray()
			{  _clearall();  _realloc(0);  }
		
		// Get number of array elements: 
		size_t NElem() const
			{  return(nelem);  }
		
		// Get element with passed index as reference. 
		// NO RANGE CHECK!
		// IN CASE YOU MODIFY THE ELEMENT IN A WAY THAT IT HAS 
		// AN INCORRECT POSITION IN THE ARRAY, CALL ReOrder() 
		// IMMEDIATELY. 
		// Runtime is O(1). 
		T &operator[](size_t idx)
			{  return(array(idx));  }
		
		// Clear the array: 
		void Clear()
			{  _clearall();  if(asize>32)  _realloc(0);  }
		
		// Get interstion index. This returns the index where to 
		// add the passed element. 
		// Returned index is -1 if you shall not add the passed elem 
		// (allow_dup==0). Index i means that you should add ent at 
		// array[i] thus moving array[i] to array[i+1] etc. before 
		// doing so. Runtime is O(log(n)). 
		ssize_t GetInsertIdx(const T &ent,int allow_dup)
		{
			if(!nelem || OP.lt(ent,array(0)))  return(0);
			if(OP.lt(array(nelem-1),ent))  return(nelem);
			size_t a=0,b=nelem-1;
			if(!allow_dup)  {
				while(b>a)
				{  size_t m=(a+b)/2;  OP.lt(array(m),ent) ? a=m : b=m;  }
				return(OP.eq(ent,array(a)) ? -1 : a);  }
			if(allow_dup<0) while(b>a)
			{  size_t m=(a+b)/2;  OP.lt(array(m),ent) ? a=m : b=m;  }
			else while(b>a)
			{  size_t m=(a+b)/2;  OP.lt(ent,array(m)) ? b=m : a=m;  }
			return(a);
		}
		
		// Add element to the array. This is O(log(n)+n). 
		// If allow_dup is set, add element even if it is already 
		// in the list. If allow_dup is <0, add as the first elem 
		// of the equal ones, if >0 as the last. E.g. for a case 
		// insensitive match, adding "b" to "AAABBBCC" will yield: 
		//  allow_dup=-1 -> "AAAbBBBCC"  return 0
		//  allow_dup=0  -> "AAABBBCC"   return 1
		//  allow_dup=+1 -> "AAABBBbCC"  return 0
		// Runtime is O(log(n)) for the search and O(n) for the move. 
		// Return value: 
		//  -1 -> alloc failure
		//   0 -> ok, added, index stores in *ret_idx if non-NULL. 
		//   1 -> already in list and allow_dup not set
		int Insert(const T &ent,int allow_dup,ssize_t *ret_idx=NULL)
		{
			ssize_t idx=GetInsertIdx(ent,allow_dup);
			if(idx<0)  return(1);
			if(nelem>=asize && _realloc(/*new_asize=*/asize>512 ? 
				(asize+512) : (asize ? asize*2 : 16)) )  return(-1);
			if(nelem>size_t(idx))  {
				ssize_t i=nelem-1;
				OP.ini(arrayP(nelem),array(i));
				for(; i>idx; i--)  OP.ass(array(i),array(i-1));  }
			OP.ass(array(idx),ent);  ++nelem;
			if(ret_idx) *ret_idx=idx;  return(0);
		}
		
		// In case the array element with index idx was modified, 
		// call this function to have it put into the correct position 
		// in the array so that the array stays sorted. 
		// Runtime is O(log(n)) for the search and O(n) for the move. 
		// Return value: 
		//   >=0 -> new index of the element
		//    -2 -> idx is out of range
		//    -3 -> removed because elem would be duplicate and allow_dup=0
		ssize_t ReOrder(size_t idx,int allow_dup)
		{
			if(idx>=nelem)  return(-2);
			ssize_t newidx=GetInsertIdx(array(idx),allow_dup);
			if(newidx<0)  {  Remove(idx);  return(-3);  }
			if((size_t)newidx==idx)  return(idx);
			T tmp(array(idx));
			if((size_t)newidx>idx)
			     for(ssize_t i=idx; i<newidx; i++) OP.ass(array(i),array(i+1));
			else for(ssize_t i=idx; i>newidx; i--) OP.ass(array(i),array(i-1));
			OP.ass(array(newidx),tmp);  return(newidx);
		}
		
		// Reorder complete list; may be used if several elements 
		// were modified. 
		// This uses insertion sort to sort the list; hence runtime 
		// is nearly O(n) as long as n_to_be_reordered << nelems. 
		// In the worst case (list not ordered), runtime is O(n^2). 
		void ReOrder()
		{
			if(nelem<1)  return;  // 1 element is always sorted :)
			T tmp;
			for(size_t i=1; i<nelem; i++)
			{
				size_t j1=i-1;
				if(OP.le(array(j1),array(i)))  continue;
				OP.ass(tmp,array(i));
				size_t j=i;
				do
				{  OP.ass(array(j),array(j1));  if(!j) break;  j=j1--;  }
				while(OP.lt(tmp,array(j1)));
				OP.ass(array(j),tmp);
			}
		}
		
		// Find passed element; returns index. 
		// In case the passed element occurs more than once in the 
		// array, the index of the _first_ occurance is returned. 
		// Runtime is O(log(n)). 
		// If not found, returns -1. 
		ssize_t FindIdx(const T &ent)
		{
			if(!nelem || OP.lt(ent,array(0)) || 
				OP.lt(array(nelem-1),ent))  return(-1);
			size_t a=0,b=nelem-1;
			while(b-a>1)
			{  size_t m=(a+b)/2;  OP.lt(array(m),ent) ? a=m : b=m;  }
			if(OP.eq(ent,array(a)))  return(a);  // order is...
			if(OP.eq(ent,array(b)))  return(b);  // ...important!
			return(-1);
		}
		
		// Similar to FindIdx() but does not need the complete type T 
		// entry but just a key. Only useful when using special 
		// OP class. 
		template<class K>ssize_t FindKeyIdx(const K &key)
		{
			if(!nelem || OP.lt(key,array(0)) || 
				OP.lt(array(nelem-1),key))  return(-1);
			size_t a=0,b=nelem-1;
			while(b-a>1)
			{  size_t m=(a+b)/2;  OP.lt(array(m),key) ? a=m : b=m;  }
			if(OP.eq(key,array(a)))  return(a);  // order is...
			if(OP.eq(key,array(b)))  return(b);  // ...important!
			return(-1);
		}
		
		// Remove element with passed index. 
		// Runtime is O(n) for the move. 
		// Return value: 
		//   0 -> OK
		//  -2 -> idx out of range
		int RemoveIdx(size_t idx)
		{
			if(idx>=nelem)  return(-2);  --nelem;
			for(size_t i=idx; i<nelem; i++)  OP.ass(array(i),array(i+1));
			OP.clr(arrayP(nelem));  _downsize();  return(0);
		}
		
		// Remove passed element. 
		// Runtime is O(log(n)) for the search and O(n) for the move. 
		// Returns number of removed elements. 
		ssize_t Remove(const T &ent)
		{
			if(!nelem || OP.lt(ent,array(0)) || 
				OP.lt(array(nelem-1),ent))  return(0);
			size_t a=0,b=nelem-1;
			while(b>a)
			{  size_t m=(a+b)/2;  OP.lt(array(m),ent) ? a=m : b=m;  }
			if(a+1<nelem && OP.eq(array(a+1),ent))  {
				size_t a1=a;  b=nelem-1;
				while(b>a1)
				{ size_t m=(a1+b)/2; OP.lt(ent,array(m)) ? b=m : a1=m; }
			}
			// Remove entries a..b. 
			size_t delta=b-a+1;
			nelem-=delta;
			for(size_t i=a; i<nelem; i++)  OP.ass(array(i),array(i+delta));
			if(!OP.pdt) for(size_t i=nelem+delta; i-->nelem;) OP.clr(arrayP(i));
			_downsize();  return(delta);
		}
};

#endif  /* _HLIB_OrderedArray_H_ */
