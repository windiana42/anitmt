/*
 * binsearch.h 
 * 
 * Binary search templates. 
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

#ifndef _HLIB_BINARYSEARCH_H_
#define _HLIB_BINARYSEARCH_H_ 1

#include <hlib/defop.h>

#error "Untested code."

// Perform binary search on passed array arr with nelem elements. 
// The array must be sorted in ascenting order. 
// Return value: 
//  0 -> not found: insert index stored in ret_idx (0..nelem)
//       (Insert index means that the passed object can be stored 
//       immediately before element [idx].) 
//  1 -> found: index stored in ret_idx (0..nelem-1)
// NOTE: You may NOT use ret_idx=NULL. 
// There are different versions of this function; one using the 
// HLib operator class, one using standard C(++) operators. 
template<class T,class K>bool BinarySearch(const T *arr,size_t nelem,
	const K &key,size_t *ret_idx)
{
	if(nelem<=1) {  if(!nelem)  {  *ret_idx=0;  return(0);  }
	                if(key>arr[0])  {  *ret_idx=1;  return(0);  }
	                *ret_idx=0;  return(key==arr[0]);  }
	size_t a=0,b=--nelem;  // NOTE: nelem decreased by 1!
	while(b-a>1)  {  size_t m=(a+b)/2;  (arr[m]<key) ? a=m : b=m;  }
	if(!a && key<arr[0])  {  *ret_idx=0;  return(0);  }
	if(b==nelem && key>arr[nelem])  {  *ret_idx=nelem+1;  return(0);  }
	if(key==arr[a])  {  *ret_idx=a;  return(1);  }
	*ret_idx=b;  return(b && key==arr[b]);
}

template<class T,class K,class _OP>bool HLBinarySearch(
	char *_arr,size_t nelem,
	const K &key,size_t *ret_idx,
	const _OP &op=HLDefaultOperators_PDT<T>)
{
	inline T &arr(size_t idx,const _OP &op)
		{  return(*(T*)(_arr+idx*op.size()));  }
	
	if(nelem<=1) {  if(!nelem)  {  *ret_idx=0;  return(0);  }
	                if(op.lt(arr(0,op),key))  {  *ret_idx=1;  return(0);  }
	                *ret_idx=0;  return(op.eq(key,arr(0,op)));  }
	size_t a=0,b=--nelem;  // NOTE: nelem decreased by 1!
	while(b-a>1)  {  size_t m=(a+b)/2;  op.lt(arr(m,op),key) ? a=m : b=m;  }
	if(!a && op.lt(key,arr(0,op)))  {  *ret_idx=0;  return(0);  }
	if(b==nelem && op.lt(arr(nelem,op),key)) { *ret_idx=nelem+1; return(0); }
	if(op.eq(key,arr(a,op)))  {  *ret_idx=a;  return(1);  }
	*ret_idx=b;  return(b && op.eq(key,arr(b,op)));
}

#endif  /* _HLIB_BINARYSEARCH_H_ */
