/*
 * cdrecorder.h
 * 
 * Header file for class CyclicDataRecorder. 
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

#ifndef _HLIB_CyclicDataRecorder_H_
#define _HLIB_CyclicDataRecorder_H_ 

#include "cplusplus.h"


// This is a data recorder class which has a fixed record size 
// (stored as array); new records overwrite the oldest. 
// Passed type must ne NEWarray<>()-capable and provide the routines 
// set(const T &) 
//   which does what operator=() is expected to do (not needed if you 
//   only use StorePtr() and not Store(), etc)
// zap() 
//   which clears the content of the element; useful if it contains 
//   dynamically allocated storage (see Clear(); not needed if you do 
//   not use do_zap=1 anywhere). 
template<typename T>class CyclicDataRecorder
{
	private:
		T *rec;
		size_t size;   // size of recorder array
		size_t next;   // index of next entry to store
		bool wrapped;  // wrapped around already?
		
	public:  _CPP_OPERATORS_FF
		// Pass size, i.e. (max.) number of elements to store as first 
		// argument. 
		CyclicDataRecorder(size_t _size,int *failflag=NULL)
		{
			next=0;  wrapped=0;  rec=NEWarray<T>(_size);
			if(rec || !_size)  size=_size;  else  {  size=0;
				if(failflag) --*failflag; else ConstructorFailedExit("CDR");  }
		}
		~CyclicDataRecorder()
			{  DELarray(rec);  size=0;  next=0;  }
		
		// Get number of used elements/records in the recorder: 
		size_t NUsed()  const
			{  return(wrapped ? size : next);  }
		// Get size of recorder: 
		size_t Size()  const
			{  return(size);  }
		// Already wrapped around? 
		bool Wrapped()  const
			{  return(wrapped);  }
		
		// Get i-th newest element (i=0 -> newest). 
		// Returns pointer to element or NULL in case the index is out 
		// of range. 
		T *GetN(size_t i)  const
		{	if(i>=size)  return(NULL);  i=(next+size-(i+1))%size;
			return((wrapped || i<next) ? &rec[i] : NULL);  }
		
		// Like GetN() but returns the i-th oldest element (i.e. i=0 is 
		// the oldest one). 
		T *GetO(size_t i)  const
		{	return( wrapped ? (i>=size ? NULL : &rec[(next+i)%size]) : 
			(i>=next ? NULL : &rec[i]) );  }
		
		// Store element in the recorder. 
		// Return value: 
		//   0 -> OK, stored
		//   1 -> OK, stored and now wrapping around. 
		//   2 -> size=0, cannot store
		int Store(const T &r)
		{
			if(!size) return(2);
			rec[next++].set(r);
			if(next<size) return(0);
			next=0;  wrapped=1;  return(1);
		}
		// This is a more performant way of storing an element. 
		// StorePtr() returns a pointer to the next element to be 
		// (over)written and advances the internal element pointer. 
		// Hence, you can either do
		//   Store(elem)
		// or
		//   StorePtr()->set(elem)  <-- or sth similar
		// Returns NULL if size=0. 
		T *StorePtr()
		{
			if(!size) return(NULL);
			T *ptr=&rec[next++];
			if(next>=size)  {  next=0;  wrapped=1;  }
			return(ptr);
		}
		
		// CLear the recorder. 
		// If do_zap is set, all elements get zap()'ed , otherwise 
		// simply next=0 and wrapped=0 is set. 
		void Clear(bool do_zap=1)
		{
			if(do_zap) for(size_t i=0,iend=wrapped?size:next; i<iend; i++)
				rec[i].zap();
			next=0;  wrapped=0;
		}
};

#endif  /* _HLIB_CyclicDataRecorder_H_ */
