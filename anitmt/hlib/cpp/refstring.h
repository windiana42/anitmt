/*
 * refstring.h 
 * 
 * RefString, an allocate once & reference string. 
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
#include <string.h>
#include "cplusplus.h"

// How it works: Simply allocate a RefString on the stack and store 
// any string in it. You may copy the RefString via the copy 
// constructor or operator=() in order to only copy a reference to 
// the string and not the whole string. The stored string will not be 
// freed until the last RefString referring to the string gets 
// destroyed. The RefString will never DELETE itself. 
// NOTE: String allocation is done via LMalloc()/LFree(). 
// RefString can store two types of strings: 
//  1. Normal 0-terminated strings (mostly the functions without 
//     a size argument). No string length is stored for these 
//     strings. 
//  2. Data strings which may contain any number of '\0'-chars. 
//     String length (size_t) is stored together with the string 
//     at the cost of additional 4 or 8 (sizeof(size_t)) bytes. 
// RefString also has functions for string manipulation. These 
// functions are implemented in a different file than the base 
// functionality so that they do not get liked into your code if 
// you do not use them. 
// NOTE: YOU CANNOT SHARE REFERENCES ACCROSS DIFFERENT THREADS IN 
//       MULTITHREADED APPLICATIONS. 
//       DON'T USE RefString IN MULTITHREADED APPLICATIONS AS THERE 
//       IS NO REFERENCE COUNTER SPINLOCK. 

class RefString
{
	private:
		// Pointer to ref counter. 
		// NOTE: lowest bit of ref counter is size flag! 
		//       ref counter is always increased/decreased by 2 NOT by 1. 
		// String begins at (ref+1). 
		int *ref;
		
		// Returns size flag. 
		// ONLY CALL IF ref!=NULL. 
		inline int _sflag() const
		{  return((*ref) & 1);  }
		
		// Returns pointer to string. 
		// ONLY CALL IF ref!=NULL. 
		inline char *_str() const
		{  return(((char*)(ref+1)));  }
		
		// Reads length from (size_t*)ref-1. 
		// ONLY CALL IF ref!=NULL and _sflag()==1. 
		inline size_t _rlength() const
		{  return(*(((size_t*)ref)-1));  }
		
		// Dereference string: 
		inline void _deref()
		{
			if(!ref)  return;
			// NOTE: lowest bit is size flag!
			(*ref)-=2;
			if(*ref<=1)  _destroy();
			ref=NULL;
		}
		inline void _addref()   // Reference string: 
		{  if(ref)  (*ref)+=2;  }
		
		int *_alloc(size_t datalen,int sflag);
		int _copy(const char *str,const size_t *lenptr);
		void _setup_constr(int *failflag,const char *str_to_copy,const size_t *lenptr);
		void _destroy();
		
		// This function makes a private copy of the string. 
		// If _deref() is called first to get rid of old string. 
		// Nothing is done if the reference is NULL of the ref 
		// count is 1 (this instance is the only user of the string). 
		// Return value: 
		//   0 -> success 
		//  -1 -> allocation failed (string then contains NULL reference). 
		// [Implementer's NOTE: *ref=0,1 -> 0 references (never happens); 
		//  ref=2,3 -> 1 reference; both cases no need to copy as we're 
		//  the exclusive user.]
		// [CURRENTLY UNUSED]
		int _detach()
		{  return(!ref ? 0 : (*ref<=3 ? 0 : (_sflag() ? 
			set(_str(),_rlength()) : set(_str()))));  }
		// Re-allocation function; longer docu: see implementation. 
		int _realloc(size_t newlen,int sflag,int wcopy);
		
		// Prepend/append: pre_app=-1 -> append; pre_app=+1 -> prepend
		int _pre_app_end(const char *str,int pre_app);
		int _pre_app_end(const char *str,size_t len,int pre_app);
		// Trunc/skip: tr_sk=-1 -> trunc; tr_sk=+1 -> skip
		int _trunc_skip(size_t len,int tr_sk);
	public:  _CPP_OPERATORS_FF
		// Set up a NULL reference (use set() and deref() later)
		// (int *failflag not needed as it never fails; just kept for 
		// style reasons.)
		explicit RefString(int * =NULL)  {  ref=NULL;  }
		// Creation constructor: 
		// str_to_copy is the string to be copied into the RefString. 
		// Data for that string is allocated and the reference set up. 
		// The first variant expects a '\0'-terminated string and creates 
		//    a RefString able to contain '\0'-terminated strings, thus 
		//    one without a size field. 
		// The second variant is the 8-bit-clean data variant. The 
		//    RefString also allocates a size field and thus the string 
		//    does not have to be '\0'-terminated and may contain any 
		//    number of NUL-chars. (len is the size of the data to be 
		//    copied from passed buffer, NOT the upper limit of the string 
		//    length.) 
		// NOTE: You may set str_to_copy to NULL which has the same effect 
		//       as using the plain RefString() constructor above. 
		RefString(const char *str_to_copy,int *failflag=NULL)
			{  _setup_constr(failflag,str_to_copy,NULL);  }
		RefString(const char *str_to_copy,size_t len,int *failflag=NULL)
			{  _setup_constr(failflag,str_to_copy,&len);  }
		// Copy constructor: 
		RefString(const RefString &src)
			{  ref=(int*)(src.ref);  _addref();  }
		// Assignment (operator and method): 
		// (You cannot assign a const char!) 
		void set(const RefString &src)
			{  _deref();  ref=(int*)(src.ref);  _addref();  }
		RefString &operator=(const RefString &src)
			{  set(src);  return(*this);  }
		// Destructor (destroys just this reference): 
		~RefString()
			{  _deref();  }
		
		// Get the string stored in the RefString: 
		// It is (of course) NOT copied. 
		const char *str() const
			{  return(ref ? _str() : NULL);  }
		operator const char*() const  {  return(str());  }
		
		// Get the length of the stored string. This either returns 
		// the length as saved in the size field (if the string is one 
		// with size field) or calls strlen(). 
		// If strlen() is called, len() returns the length without the 
		// terminating '\0', and len0() with the terminating NUL char. 
		// If no string is stored (NULL reference), length 0 is returned. 
		size_t len() const
			{  return(ref ? (_sflag() ? _rlength() : strlen(_str())) : 0);  }
		size_t len0() const
			{  return(ref ? (_sflag() ? _rlength() : (strlen(_str())+1)) : 0);  }
		
		// Returns string type: 
		//  -1 -> NULL reference 
		//   0 -> normal '\0'-termiated string (sflag=0)
		//   1 -> data string (with size field)
		int stype() const
			{  return(ref ? _sflag() : -1);  }
		
		// Simply dereference the string. This RefString will then 
		// just contain a NULL pointer (str() will return NULL). 
		// deref() has the same effect as set(NULL). 
		void deref()  {  _deref();  }
		
		// This works like operator=() but with strings to be copied. 
		// First, dereferences the currently used string, then 
		// it assignes the new string. 
		// The first variant (without length spec) creates a 
		// '\0'-terminated string reference, the second one (with length 
		// specification) expects str_to_copy to conatain >length< bytes 
		// of data and allocates an 8-bit-clean string of the specified 
		// size. (See also constructor.) 
		// Return value: 
		//   0 -> success
		//  -1 -> memory allocation failed (string now NULL). 
		// set(NULL) has the same effect as deref(). 
		int set(const char *str_to_copy)
			{  _deref();  return(_copy(str_to_copy,NULL));  }
		int set(const char *str_to_copy,size_t length)
			{  _deref();  return(_copy(str_to_copy,&length));  }
		
		// Like snprintf() on RefString. The passed format string and 
		// args are formatted into the string. deref() is called before, 
		// so printf() will not append to the string but act like set().
		// maxlen is an upper limit for the final string length (including 
		// the terminating '\0'-char; set to 0 for 'unlimited'). 
		// The string is first formatted into a 64 bytes prealloc buffer 
		// via vsnprintf() to see how long it will get. If less that 64 
		// bytes, the result is simply copied otherwise it is re-formatted 
		// using vsnprintf() again. 
		// Return value: 
		//  1 -> string was truncated (due to maxlen)
		//  0 -> success
		// -1 -> allocation failure (ref now NULL)
		// The new string is always a '\0'-terminated string without 
		// size field. 
		int sprintf(size_t maxlen,const char *fmt,...);
		
		/*** STRING MANIPULATION ROUTINES ***/
		// There are usually two versions of the functions: 
		// * functions without size argument: May be used with 
		//   '\0'-terminated strings and data strings containing a 
		//   length field. If used in combination with data strings 
		//   (the ones having a length field), the terminating '\0' 
		//   is NOT copied. Normal strings without length field 
		//   always stay '\0'-terminated. 
		// * functions with size argument: May only be used with 
		//   data strings (the ones having a length field). 
		// The return value of all operations requiring (potential) 
		// memory allocation includes: 
		//  0 -> success
		// -1 -> allocation failed (LMalloc()); reference now NULL. 
		// -2 -> this is a '\0'-terminated string (and you wanted to 
		//       use a function with length argument on it). 
		
		// Append specified string to the end of the string. 
		int append(const char *str)
			{  return(_pre_app_end(str,-1));  }
		int append(const char *str,size_t len)
			{  return(_pre_app_end(str,len,-1));  }
		
		// Insert specified string before the beginning of the string. 
		int prepend(const char *str)
			{  return(_pre_app_end(str,+1));  }
		int prepend(const char *str,size_t len)
			{  return(_pre_app_end(str,len,+1));  }
		
		// Truncate string to specified length / skip first nbytes 
		// bytes. For '\0'-terminated strings, the '\0' does not 
		// count for the length. 
		// Return value: 
		//  0 -> success
		// -1 -> allocation failed
		int trunc(size_t len)
			{  return(_trunc_skip(len,-1));  }
		int skip(size_t nbytes)
			{  return(_trunc_skip(nbytes,+1));  }
		
		// Change string type: 
		// sflag: 0 -> change string to '\0'-terminated normal string. 
		//             (string will be '\0'-terminated) 
		//        1 -> change string to data string with length field. 
		//             ('\0'-termination gets removed) 
		// NOTE: changing the string type requires a complete string 
		//       reallocation and is thus not very efficient. 
		// When converting a data string into a normal (NUL-term.) 
		// string, the data string may not contain '\0'-chars. The only 
		// excpetion is a '\0'-char as the last char. 
		// Return value: 
		//  0 -> success (or there was nothing to do) 
		// -1 -> allocation failed (string now NULL)
		// -2 -> data string contains '\0'-chars (only in calls with 
		//       sflag=0)
		int chtype(int sflag);
};

#endif  /* _HLIB_RefString_H_ */
