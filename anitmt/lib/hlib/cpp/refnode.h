/*
 * refnode.h 
 * 
 * Reference counting node and base class for derivation of internally 
 * used unique class. 
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

#ifndef _HLIB_RefNode_H_
#define _HLIB_RefNode_H_ 1

#include "cplusplus.h"

//------------------------------------------------------------------------------
// 
// HOW IT IS MEANT TO WORK
// =======================
// 
// You have some resource which needs to be shared using reference counting. 
// Then, put this into the "internal class" which you derive from 
// InternalRefNodeBase (which will add an integer for reference counting 
// as the only overhead). 
// Let's say the shared resource is 
// 
//   class _IBuffer : InternalRefNodeBase
//   {
//       char *allocated;
//       size_t size;
//       public:
//           int BoFoo(char *x,int y);
//           int pubval;
//         ...
//   };
// 
// Now, we need to create such an object once and want to have it shared: 
// 
//   _IBuffer *internal_b=new _IBuffer(...);
//   RefNode<_IBuffer> b(internal_b);
// 
// Now, we can savely go on using the RefNode<_IBuffer>: 
// 
//   RefNode<_IBuffer> c=b;
//
// In order to avoid all that template typing, I recommend to use a typedef: 
// 
//   typedef RefNode<_IBuffer> Buffer;
// 
// Access to all elements in the internal ref node can easily be obtained 
// through the overloaded operator->(): 
// 
//   c->BoFoo("hello",5);
//   c->pubval=123;
// 
// Of course, you should never delete the internal class directly. It 
// will delete itself automatically once all references to it are deleted, 
// i.e. the reference counter gets zero. 
// 
// NOTE: Reference counting is an easy way to control the life of an object. 
//       But it is unable to detect closed loops of unreferenced objects. 
//       E.g. A holds a reference of B and B holds a reference of A but 
//       nobody else holds a reference to neither A nor B, these objects 
//       should get deleted but will NOT. 
//       If such cases can happen in your program, you may not use reference 
//       counting but some more advanced garbage collection algorithm. 
// 
//------------------------------------------------------------------------------


template<class T>class RefNode;

// This is the base class for the internally used class. 
class InternalRefNodeBase
{
	template<class T>friend class RefNode;
	private:
		int refcnt;
		
		// Failed assertion: 
		void _failassert();
		
		// Do not use: 
		InternalRefNodeBase(const InternalRefNodeBase &);
		InternalRefNodeBase &operator=(const InternalRefNodeBase &);
	public:  _CPP_OPERATORS_FF
		InternalRefNodeBase(int * /*failflag*/=NULL)  {  refcnt=0;  }
		~InternalRefNodeBase()  {  if(refcnt) _failassert();  }
}__attribute__((__packed__));


// This is the reference counting C++-safe type for the user. 
// The type T must be derived from InternalRefNodeBase. 
// This type is NOT meant to be used as a base class. 
template<class T>class RefNode
{
	private:
		T *in;   // internal node
		
		inline void _aqref()
			{  if(in) ++in->InternalRefNodeBase::refcnt;  }
		inline void _deref()
			{  if(in && --in->InternalRefNodeBase::refcnt<=0) delete in;  }
	public:
		// Normal C++ constructors: 
		inline RefNode()  {  in=NULL;  }
		inline RefNode(const RefNode &r)  {  in=r.in;  _aqref();  }
		inline ~RefNode()  {  _deref();  in=NULL;  }
		
		// Normal C++ assignment. 
		inline RefNode &operator=(const RefNode &r)
			{  _deref();  in=r.in;  _aqref();  return(*this);  }
		
		// This is like assigning a NULL ref: 
		inline void deref()  {  _deref();  }
		
		// This can be used to check for NULL refs: 
		inline bool operator!()  {  return(!in);  }
		inline operator bool()  {  return((bool)in);  }
		
		// Spcial to create the first RefNode for an internal RefNode: 
		inline RefNode(T *_in)  {  in=_in;  _aqref();  }
		inline RefNode &operator=(T *_in)
			{  _deref();  in=_in;  _aqref();  return(*this);  }
		
		// Access to the internal class: 
		inline T *operator->()  {  return(in);  }
		inline const T*operator->() const  {  return(in);  }
};

#endif /* _HLIB_RefNode_H_ */
