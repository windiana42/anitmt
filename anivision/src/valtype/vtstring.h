/*
 * valtype/vtstring.h
 * 
 * String value header. 
 * This is part of the AniVision project. 
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

#ifndef _VALTYPES_VT_STRING_H_
#define _VALTYPES_VT_STRING_H_ 1

#include <string.h>

// This does some sanity checks: 
#define str_assert(x) assert(x)

class String;

class InternalString : ValueNamespace
{
	friend class String;
	friend String operator+(const String &a,const String &b);
	// All is private; not meant for public use; use String instead. 
	
	int len;     // length of string (without '\0' at the end)
	int asize;   // allocated size (if str!=0 always >=len+1 because of term. '\0')
	char *str;   // the actual string (terminated by '\0')
	int refcnt;  // reference counter
	
	inline void *operator new(size_t size)
		{  return(ValAlloc(size));  }
	inline void operator delete(void *ptr)
		{  ValFree(ptr);  }
	
	// This class is NOT C++ save; never use operator= or 
	// copy constructor. 
	inline InternalString()
		{  len=0;  asize=0;  str=NULL;  refcnt=0;  }
	inline InternalString(const char *src)  // src NOT NULL
		{  len=strlen(src);  asize=len+1;  str=ALLOC<char>(asize);
			memcpy(str,src,asize);  refcnt=0;  }
	// Set concatenation of str1 and str2; neither may be NULL. 
	InternalString(const char *src1,const char *src2);
	// Construct string of specified preallocated length: 
	// Do not use alen=0. 
	inline InternalString(int alen)
		{  refcnt=0; len=0; str_assert(alen);
			str=ALLOC<char>(asize=alen);  *str='\0';  }
	inline ~InternalString()
		{  str_assert(refcnt==0);  str=FREE(str);  }
	
	// Aquire/release reference: 
	inline void aqref()  {  ++refcnt;  }
	inline void deref()  {  if(--refcnt<=0)  delete this;  }
	
	// Re-allocate buffer: 
	// Will NOT change len, so be careful. 
	inline void realloc(int newsize)
		{  str=REALLOC(str,asize=newsize);  }
};

class String : public ValueBase
{
	public:
		static const VType vtype=VTString;
	private:
		InternalString *istr;
		
	public:
		// Constructor: 
		inline String() : ValueBase()  // NULL ref
			{  istr=NULL;  }
		inline String(const String &s) : ValueBase()
			{  istr=s.istr;  if(istr) istr->aqref();  }
		inline String(const char *src) : ValueBase()
			{  if(src) { istr=new InternalString(src); istr->aqref(); }
				else istr=NULL;  }
		// Construct string of specified preallocated length: 
		// Do not use alen=0. 
		inline String(int alen) : ValueBase()
			{  istr=new InternalString(alen);  istr->aqref();  }
		inline ~String()
			{  if(istr) istr->deref();  }
		
		// Get length of string: 
		int len() const  {  return(istr ? istr->len : 0);  }
		
		// Be very careful with this one: returns a '\0'-terminated 
		// string or NULL. Note that the pointer may change when 
		// we reallocate the memory. 
		const char *str() const 
			{  return(istr ? istr->str : NULL);  }
		
		// This is true if this is a NULL-ref: 
		bool operator!() const  {  return(!istr);  }
		
		// Assignment: 
		// May use set(NULL) to set null-ref. 
		inline void set(const String &s)
			{  if(istr) istr->deref();  istr=s.istr; if(istr) istr->aqref();  }
		inline void set(const char *src)
			{  if(istr) istr->deref();  if(!src)  istr=NULL;  else
				{ istr=new InternalString(src);  istr->aqref(); }  }
		inline String &operator=(const String &s)
			{  set(s);  return(*this);  }
		inline String &operator=(const char *s)
			{  set(s);  return(*this);  }
		
		// Compare: 
		bool operator==(const String &b) const;
		inline bool operator!=(const String &b) const
			{  return(!operator==(b));  }
		bool operator==(const char *s) const
			{  if(!istr) return(!s);  return(!strcmp(istr->str,s));  }
		inline bool operator!=(const char *s) const
			{  return(!operator!=(s));  }
		
		// Append string: 
		void append(const String &s);
		void append(const char *s);
		void append(const char *c,int len);
		String &operator+=(const String &s)
			{  append(s);  return(*this);  }
		String &operator+=(const char *s)
			{  append(s);  return(*this);  }
		// Prepend string: 
		void prepend(const String &s);
		void prepend(const char *s);
		
		// Other string "arithmetics": 
		friend String operator+(const String &a,const String &b);
		friend String operator+(const char *a,const String &b);
		friend String operator+(const String &a,const char *b);
		
		// This replaces the string *this by string b; replacement 
		// begins at position idx in *this. Returns 0 on success and 
		// 1 if idx is out of range (<0 or >length).
		int replace_idx(int idx,const String &b);
		
		// Needed for templates...
		inline bool is_null() const  {  return(!istr);  }
		
		// Truncate string to specified length / skip first nbytes 
		// bytes. 
		void trunc(int len);
		void skip(int len);
		
		// Write formatted message: 
		void sprintf(const char *fmt,...)
			__attribute__ ((__format__ (__printf__, 2, 3)));
		
		// Make string representation of value [overridden virtual]: 
		String ToString() const;
		ValueBase *clone() const;   // <-- [overridden virtual]
		void forceset(ValueBase *vb);  //  <-- [overridden virtual]
}__attribute__((__packed__));

#endif  /* _VALTYPES_VT_STRING_H_ */
