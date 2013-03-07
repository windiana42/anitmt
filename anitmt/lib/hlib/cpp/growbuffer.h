/*
 * growbuffer.h 
 * 
 * Header def for growing buffer class. 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

// GrowBuffer is a small class which allows you 
// * to allocate a buffer (just like char buf[size]) but on the 
//   heap instead of the stack. 
// * implement with minimal extra-effort a fixed-size buffer which 
//   starts with zero size and grows up to the size you neeed it 
//   (e.g. as slide buffer)
// Note that the stored strings may be binary (thus may contain 
// '\0'-chars) and do not have to be '\0'-terminated. 
// NOTE: The buffer is never made smaller unless you set free_rest 
//       in a call to trunc()/skip() and the like. If you want 
//       to append too much, it gets inlarged. This can fail, 
//       of course, so check return values... 
// NOTE: (De)Allocation is done via LMalloc/LFree. 

#ifndef _HLIB_GrowBuffer_H_
#define _HLIB_GrowBuffer_H_ 1

#include "cplusplus.h"

#include <string.h>

class GrowBuffer
{
	protected:
		char *_str;
		size_t _size;  // length of reserved memory region
		size_t _len;
		
		int _realloc(size_t newsize);
		// DO NOT assign or copy-construct: 
		void operator=(const GrowBuffer &) {}
		GrowBuffer(const GrowBuffer &) {}
	public:  _CPP_OPERATORS_FF
		// Reserves a buffer of specified length. 
		// Check failflag for allocation failure. 
		GrowBuffer(size_t reserv_len,int *failflag=NULL);
		~GrowBuffer();
		
		// Get the string which is stored inside this class. 
		// NOTE: the string is NULL if size=0. 
		operator char*()  {  return(_str);  }
		operator const char*() const  {  return(_str);  }
		char *str()  {  return(_str);  }
		const char *str() const  {  return(_str);  }
		
		// Give the class a normal char-buffer feeling...
		char *operator+(size_t i)  {  return(_str+i);  }  // unsigned size_t versions
		char *operator-(size_t i)  {  return(_str-i);  }
		char &operator[](size_t i)  {  return(_str[i]);  }
		char *operator+(ssize_t i)  {  return(_str+i);  }   // signed ssize_t versions
		char *operator-(ssize_t i)  {  return(_str-i);  }
		char &operator[](ssize_t i)  {  return(_str[i]);  }
		// All the same operating on const *this-pointers: 
		const char *operator+(size_t i) const  {  return(_str+i);  }  // unsigned size_t versions
		const char *operator-(size_t i) const  {  return(_str-i);  }
		const char &operator[](size_t i) const  {  return(_str[i]);  }
		const char *operator+(ssize_t i) const  {  return(_str+i);  }  // signed ssize_t versions
		const char *operator-(ssize_t i) const  {  return(_str-i);  }
		const char &operator[](ssize_t i) const  {  return(_str[i]);  }
		
		// size -> size of buffer 
		// len -> actual length of string inside buffer 
		size_t size() const  {  return(_size);  }
		size_t len()  const  {  return(_len);  }
		
		// Clear the buffer and free it is free_it!=0. 
		// (Clearing a buffer means setting the size to 0). 
		int clear(int free_it=0)
			{  return(trunc(0,free_it));  }
		
		// Truncate the buffer to specified size. Nothing 
		// happens if newsize>=len. If free_rest is set, then 
		// the buffer is resized to have exactly the specified 
		// size. 
		// Return value: 
		//   0 -> OK
		//  -1 -> LReAlloc failed (this should never happen as 
		//        the mem chunk is made smaller)
		int trunc(size_t newsize,int free_rest=0);
		
		// Like trunc() but it skips the first nbytes bytes. 
		// If free_rest is set, then
		// the buffer is resized to have exactly the size of the 
		// data left over. 
		// Return value: see trunc()
		int skip(size_t nbytes,int free_rest=0);
		
		// Inlarge buffer to specified size. Nothing happens if 
		// newsize<=size. 
		// Return value: 
		//   0 -> OK
		//  -1 -> LReAlloc failed. 
		int inlarge(size_t newsize)
			{  return((newsize>_size) ? _realloc(newsize) : 0);  }
		
		// Overwrite buffer with specified string: 
		// Return value: 
		//   0 -> OK
		//  -1 -> allocation failed (only if buffer had to be enlarged)
		int set(const char *str)
			{  return(set(str,str ? (strlen(str)+1) : 0));  }
		int set(const char *buf,size_t len);
		
		// Append passed string to the buffer: 
		// append0() will NOT append the trailing '\0'. 
		// Return value: 
		//   0 -> OK
		//  -1 -> allocation failed (only if buffer had to be enlarged)
		int append(const char *str)
			{  return(append(str,str ? (strlen(str)+1) : 0));  }
		int append0(const char *str)
			{  return(append(str,str ? strlen(str) : 0));  }
		int append(const char *buf,size_t len);
};

#endif  /* _HLIB_GrowBuffer_H_ */
