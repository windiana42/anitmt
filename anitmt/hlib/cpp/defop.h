#ifndef _HLIB_HLDefaultOperators_H_
#define _HLIB_HLDefaultOperators_H_ 1

#include <hlib/cplusplus.h>

// NOTE: 
//  It is advisable to make as many operator class member functions 
//  as possible static but it is not required that the class is pure 
//  static. It can even have own element fields (useful for types with 
//  custom size). 

// Internally used default operators: C++ operators. 
// This is the default version for plain data types (PDTs): 
template<class T>struct HLDefaultOperators_PDT
{
	// Relational operators. Only smaller and smaller/equal are needed. 
	// There is a euqan and non-equal version since using 
	// !(a==b) instead of a!=b may be inefficient for some types. 
	static inline bool lt(T const &a,T const &b)  {  return(a<b);   }
	static inline bool le(T const &a,T const &b)  {  return(a<=b);  }
	static inline bool eq(T const &a,T const &b)  {  return(a==b);  }
	static inline bool ne(T const &a,T const &b)  {  return(a!=b);  }
	
	// Assignment: 
	static inline T &ass(T &l,T const &r)  {  return(l=r);  }
	
	// If this type (T) is a plain data type or not. 
	// Plain data types are all PODs and structures without 
	// constructor,destructor,virtual functions,...
	static const bool pdt=1;
	
	// Return object size in bytes. 
	static inline size_t size() __attribute__((__const__))
		{  return(sizeof(T));  }
	
	// For non-plain types (pdt==0), these define the constructor 
	// and destructor. These functions must be able to properly set up 
	// and destroy the passed object pointer which points to size() 
	// number of bytes allocated for the object. 
	// There are two ini() functions: one for the default constructor 
	// and one for the copy constructor. 
	// NOTE: For plain data types (pdt=1), there is no guarantee that 
	//       ini() or clr() are called -- and there may even be the case 
	//       that ini() is called for an element but not clr() or vice 
	//       versa. This is because plain data types are assumed to be 
	//       "plain", i.e. do not need construction/destruction which 
	//       allows optimization to skip initailizing/cleanup loops. 
	//       pdt=0 data types, however, are guaranteed to get ini() and 
	//       clr() called properly. 
	static inline void ini(T *) __attribute__((__const__)) {}
	static inline void ini(T *p,T const &a)  {  *p=a;  }
	static inline void clr(T *) __attribute__((__const__)) {}
};

// This is the default version for non-plain data types (CDTs): 
template<class T>struct HLDefaultOperators_CDT
{
	static inline bool lt(T const &a,T const &b)  {  return(a<b);   }
	static inline bool le(T const &a,T const &b)  {  return(a<=b);  }
	static inline bool eq(T const &a,T const &b)  {  return(a==b);  }
	static inline bool ne(T const &a,T const &b)  {  return(a!=b);  }
	static inline T & ass(T &l,T const &r)  {  return(l=r);  }
	static const bool pdt=0;
	static inline size_t size() __attribute__((__const__))
		{  return(sizeof(T));  }
	static inline void ini(T *p)  {  new(p) T();  }
	static inline void ini(T *p,T const &a)  {  new(p) T(a);  }
	static inline void clr(T *p)  {  p->~T();  }
};

#endif  /* _HLIB_HLDefaultOperators_H_ */
