/*
 * linkedlist.h 
 * 
 * Complete implementation of a doubly linked list. 
 * List elements must be derived from LinkedListBase. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_LinkedList_H_
#define _HLIB_LinkedList_H_ 1

#include "cplusplus.h"

#include <stddef.h>


template<class T> struct LinkedListBase
{
	T *prev,*next;
	
	LinkedListBase()  {  prev=next=NULL;  }
	~LinkedListBase()  { }
};


// Ugly macro used so that we can read things more easily.
#define _LLB LinkedListBase<T>

template<class T> class LinkedList
{
	private:
		T *lfirst,*llast;
		// Don't use: (use assign_list in rare cases): 
		LinkedList &operator=(const LinkedList &l)
		{  lfirst=l.lfirst;  llast=l.llast;  return(*this);  }
		LinkedList(const LinkedList &l)
		{  lfirst=l.lfirst;  llast=l.llast;  }
	public:  _CPP_OPERATORS_FF
		LinkedList(int * /*failflag*/=NULL)  {  lfirst=llast=NULL;  }
		// It is your responsibility to properly empty the 
		// list before destroying it. 
		~LinkedList()  {  }
		
		// Get first and last element of the list (or NULL) 
		T *first()  const  {  return(lfirst);  }
		T *last()   const  {  return(llast);   }
		
		bool is_empty()  const  {  return(!lfirst);  }
		
		// Get next/prev element from passed element. 
		// May be used if you experience ambiguities using multiple 
		// inheritance. 
		T *next(T *p)  const  {  return(p->_LLB::next);  }
		T *prev(T *p)  const  {  return(p->_LLB::prev);  }
		const T *next(const T *p)  const  {  return(p->_LLB::next);  }
		const T *prev(const T *p)  const  {  return(p->_LLB::prev);  }
		
		// Use with care!! Just copies pointer to head and tail of 
		// list; if you modify one of the lists, the other one 
		// will get inconsistent if lfirst/llast get modified. 
		void assign_list(const LinkedList<T> *l)
		{  lfirst=l->lfirst;  llast=l->llast;  }
		
		// This is similar, just that the list pointers are swapped. 
		void swap_list(LinkedList<T> *l)
		{  T *tmp=lfirst;  lfirst=l->lfirst;  l->lfirst=tmp;
		      tmp=llast;   llast= l->llast;   l->llast= tmp;  }
		
		// Insert p at the beginning of the list: 
		void insert(T *p)
		{
			if(!p)  return;
			p->_LLB::next=lfirst;
			p->_LLB::prev=NULL;
			//if(lfirst)  lfirst->_LLB::prev=p;
			//else  llast=p;
			(lfirst ? lfirst->_LLB::prev : llast)=p;
			lfirst=p;
		}
		
		// Append p to the end of the list: 
		void append(T *p)
		{
			if(!p)  return;
			p->_LLB::prev=llast;
			p->_LLB::next=NULL;
			//if(llast)  llast->_LLB::next=p;
			//else  lfirst=p;
			(llast ? llast->_LLB::next : lfirst)=p;
			llast=p;
		}
		
		// Dequeues *oldp and puts newp at the place where oldp was. 
		// Make sure neither oldp nor newp are NULL. 
		// Returns oldp (so that you may pass it to operator delete). 
		T *replace(T *newp,T *oldp)
		{
			newp->_LLB::next=oldp->_LLB::next;
			newp->_LLB::prev=oldp->_LLB::prev;
			if(lfirst==oldp)  lfirst=newp;
			if(llast==oldp)   llast=newp;
			oldp->_LLB::prev=oldp->_LLB::next=NULL;
			return(oldp);
		}
		
		// Put p before/after `where´ into the queue: 
		// Make sure, `where´ is not NULL. 
		void queuebefore(T *p,T *where)
		{
			if(!p)  return;
			if(where==lfirst)
			{  insert(p);  return;  }
			p->_LLB::prev=where->_LLB::prev;
			where->_LLB::prev->_LLB::next=p;
			where->_LLB::prev=p;
			p->_LLB::next=where;
		}
		void queueafter(T *p,T *where)
		{
			if(!p)  return;
			if(where==llast)
			{  append(p);  return;  }
			p->_LLB::next=where->_LLB::next;
			where->_LLB::next->_LLB::prev=p;
			where->_LLB::next=p;
			p->_LLB::prev=where;
		}
		
		// Queue before/after `where´, depending on loc. 
		// loc<0 -> before; loc>0 -> after; loc==0 -> do nothing 
		void queue(T *p,T *where,int loc)
		{
			if(loc<0)  queuebefore(p,where);
			if(loc>0)  queueafter(p,where);
		}
		
		// Dequeues element p (and returns p). 
		T *dequeue(T *p)
		{
			if(!p)  return(p);
			if(p==lfirst)  lfirst=lfirst->_LLB::next;
			else  p->_LLB::prev->_LLB::next=p->_LLB::next;
			if(p==llast)  llast=llast->_LLB::prev;
			else  p->_LLB::next->_LLB::prev=p->_LLB::prev;
			p->_LLB::next=p->_LLB::prev=NULL;
			return(p);
		}
		
		// Dequeue first/last element and return it (or NULL): 
		T *popfirst()
		{
			if(!lfirst)  return(lfirst);
			T *p=lfirst;
			lfirst=lfirst->_LLB::next;
			if(lfirst)  lfirst->_LLB::prev=NULL;
			else  llast=NULL;
			p->_LLB::next=NULL;
			return(p);
		}
		T *poplast()
		{
			if(!llast)  return(llast);
			T *p=llast;
			llast=llast->_LLB::prev;
			if(llast)  llast->_LLB::next=NULL;
			else  lfirst=NULL;
			p->_LLB::prev=NULL;
			return(p);
		}
		
		// Find element p in this list; returns either p or NULL. 
		const T *find(const T *p)  const
		{
			for(T *i=lfirst; i; i=i->_LLB::next)
			{  if(i==p)  return(p);  }
			return(NULL);
		}
		
		// Cound the elements in the list: 
		int count()  const
		{
			int cnt=0;
			for(T *i=lfirst; i; i=i->_LLB::next,cnt++);
			return(cnt);
		}
};

#undef _LLB

#endif /* _HLIB_LinkedList_H_ */
