/*
 * refstring.h 
 * 
 * Simple implementation of a allocate once & reference - string. 
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

#ifndef _HLIB_RefString_H_
#define _HLIB_RefString_H_ 1

#include <stddef.h>
#include "cplusplus.h"

// How it works: Simply allocate a RefString on the stack and store 
// any string in it. You may copy the RefString via the copy 
// constructor or operator=() in order to only copy a reference to 
// the string and not the whole string. The stored string will not be 
// freed until the last RefString referring to the string gets 
// destroyed. The RefString will never DELETE itself. 
// NOTE: String allocation is done via LMalloc()/LFree(). 

class RefString
{
	private:
		int *ref;    // pointer to reference counter and string @(ref+1)
		
		inline void _deref()   // Dereference string: 
			{  if(ref) {  --(*ref);  if(*ref<=0)  _destroy(); ref=NULL; }  }
		inline void _addref()   // Reference string: 
			{  if(ref) ++(*ref);  }
		
		int _copy(const char *str);
		void _destroy();
	public:  _CPP_OPERATORS_FF
		// Set up a NULL reference (use set() and deref() later)
		// (int *failflag not needed as it never fails; just kept for 
		// style reasons.)
		explicit RefString(int * =NULL)  {  ref=NULL;  }
		// Creation constructor: 
		// NOTE: You may set str_to_copy to NULL which has the same effect 
		//       as using the plain RefString() constructor above. 
		RefString(const char *str_to_copy,int *failflag=NULL);
		// Copy constructor: 
		RefString(const RefString &src)
			{  ref=(int*)(src.ref);  _addref();  }
		// Assignment: (You cannot assign a const char!) 
		RefString &operator=(const RefString &src)
			{  _deref();  ref=(int*)(src.ref);  _addref();  return(*this);  }
		// Destructor (destroys just this reference): 
		~RefString()
			{  _deref();  }
		
		// Get the string stored in the RefString: 
		// It is (of course) NOT copied. 
		const char *str() const
			{  return(ref ? ((const char*)(ref+1)) : NULL);  }
		operator const char*() const  {  return(str());  }
		
		// Simply dereference the string. This RefString will then 
		// just contain a NULL pointer (str() will return NULL). 
		// deref() has the same effect as set(NULL). 
		void deref()  {  _deref();  }
		
		// This works like operator=() but with strings to be copied. 
		// First, dereferences the currently used string, then 
		// it assignes the new string. 
		// Return value: 
		//   0 -> success
		//  -1 -> memory allocation failed (string now NULL). 
		// set(NULL) has the same effect as deref(). 
		int set(const char *str_to_copy)
			{  _deref();  return(_copy(str_to_copy));  }
};

#endif  /* _HLIB_RefString_H_ */
