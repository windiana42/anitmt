/*
 * refnode.cc 
 * 
 * Reference counting node: implementation of assertions. 
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

#define HLIB_IN_HLIB 1
#include "refnode.h"


void InternalRefNodeBase::_failassert()
{
	abort();
}

InternalRefNodeBase &InternalRefNodeBase::operator=(
	const InternalRefNodeBase &)
{
	abort();
	return(*this);
}

InternalRefNodeBase::InternalRefNodeBase(const InternalRefNodeBase &)
{
	abort();
}


//------------------------------------------------------------------------------

#if 0

class Buffer : InternalRefNodeBase
{
	template<class T>friend class RefNode;
	private:
		char *buf;
	public:
		int size;
	
	public:  _CPP_OPERATORS
		Buffer()  {  }
		~Buffer()  {  }
		
		int foo(char *x,int y) { return(size); }
};

void foo()
{
	RefNode<Buffer> b(new Buffer());
	
	// Copy constructor and assignment: 
	RefNode<Buffer> c(b);
	c=b;
	
	// Test for NULL refs: 
	if(c || !b);
	
	// Note that this will access the internal class transparently: 
	c->size=17;
	c->foo(NULL,3);
}

#endif
