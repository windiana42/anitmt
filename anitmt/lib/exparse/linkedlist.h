/*
 * linkedlist.h 
 * 
 * Complete implementation of a doubly linked list. 
 * List elements must be derived from LinkedListBase. 
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

#ifndef _HLIB_LinkedList_H_
#define _HLIB_LinkedList_H_ 1

#include <stddef.h>


template<class T> struct LinkedListBase
{
	T *prev,*next;
	
	LinkedListBase()  {  prev=next=NULL;  }
	~LinkedListBase()  { }
};


template<class T>class LinkedList
{
	private:
		T *lfirst,*llast;
		// Don't use: (use assign_list in rare cases): 
		LinkedList &operator=(const LinkedList &l)
		{  lfirst=l.lfirst;  llast=l.llast;  return(*this);  }
		LinkedList(const LinkedList &l)
		{  lfirst=l.lfirst;  llast=l.llast;  }
	public:
		LinkedList()  {  lfirst=llast=NULL;  }
		// It is your responsibility to properly empty the 
		// list before destroying it. 
		~LinkedList()  {  }
		
		// Get first and last element of the list (or NULL) 
		T *first()  const  {  return(lfirst);  }
		T *last()   const  {  return(llast);   }
		
		bool is_empty()  const  {  return(!lfirst);  }
		
		// Use with care!! Just copied pointer to head and tail of 
		// list; if you modify one of the lists, the other one 
		// will get inconsistent if lfirst/llast get modified. 
		void assign_list(const LinkedList<T> *l)
		{  lfirst=l->lfirst;  llast=l->llast;  }
		
		// Insert p at the beginning of the list: 
		void insert(T *p)
		{
			if(!p)  return;
			p->next=lfirst;
			p->prev=NULL;
			//if(lfirst)  lfirst->prev=p;
			//else  llast=p;
			(lfirst ? lfirst->prev : llast)=p;
			lfirst=p;
		}
		
		// Append p to the end of the list: 
		void append(T *p)
		{
			if(!p)  return;
			p->prev=llast;
			p->next=NULL;
			//if(llast)  llast->next=p;
			//else  lfirst=p;
			(llast ? llast->next : lfirst)=p;
			llast=p;
		}
		
		// Dequeues *oldp and puts newp at the place where oldp was. 
		// Make sure neither oldp nor newp are NULL. 
		void replace(T *newp,T *oldp)
		{
			newp->next=oldp->next;
			newp->prev=oldp->prev;
			if(lfirst==oldp)  lfirst=newp;
			if(llast==oldp)   llast=newp;
			oldp->prev=oldp->next=NULL;
		}
		
		// Put p before/after `where´ into the queue: 
		// Make sure, `where´ is not NULL. 
		void queuebefore(T *p,T *where)
		{
			if(!p)  return;
			if(where==lfirst)
			{  insert(p);  return;  }
			p->prev=where->prev;
			where->prev->next=p;
			where->prev=p;
			p->next=where;
		}
		void queueafter(T *p,T *where)
		{
			if(!p)  return;
			if(where==llast)
			{  append(p);  return;  }
			p->next=where->next;
			where->next->prev=p;
			where->next=p;
			p->prev=where;
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
			if(p==lfirst)  lfirst=lfirst->next;
			else  p->prev->next=p->next;
			if(p==llast)  llast=llast->prev;
			else  p->next->prev=p->prev;
			p->next=p->prev=NULL;
			return(p);
		}
		
		// Dequeue first/last element and return it (or NULL): 
		T *popfirst()
		{
			if(!lfirst)  return(lfirst);
			T *p=lfirst;
			lfirst=lfirst->next;
			if(lfirst)  lfirst->prev=NULL;
			else  llast=NULL;
			p->next=NULL;
			return(p);
		}
		T *poplast()
		{
			if(!llast)  return(llast);
			T *p=llast;
			llast=llast->prev;
			if(llast)  llast->next=NULL;
			else  lfirst=NULL;
			p->prev=NULL;
			return(p);
		}
		
		// Find element p in this list; returns either p or NULL. 
		T *find(T *p)  const
		{
			for(T *i=lfirst; i; i=i->next)
			{  if(i==p)  return(p);  }
			return(NULL);
		}
		
		// Cound the elements in the list: 
		int count()  const
		{
			int cnt=0;
			for(T *i=lfirst; i; i=i->next,cnt++);
			return(cnt);
		}
};

#endif /* _HLIB_LinkedList_H_ */
