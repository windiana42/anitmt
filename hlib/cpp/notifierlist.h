/*
 * notifierlist.h
 * 
 * Implementation of simple notifier list and notifier handler. 
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


#ifndef _HLIB_HLNotifierList_H_
#define _HLIB_HLNotifierList_H_ 1

#include <hlib/linkedlist.h>

// These are notifier list and notifier handler class; they work as follows: 
// The NotifierList holds a list of any number of NotifyHandler classes. 
// If you have to make a notification, just call NotifierList::CallNotifiers() 
// and all the attached NotifyHandler will receive a virtual function call 
// to function handlenotify(). 
// NOTE that each NotifyHandler can only be enqueued at ONE NotifierList. 
// The NotifyHandler knows which list it is currently queued at and hence 
// you can add/remove handlers from in O(1) time. 
// When the NotifyHandler gets destroyed, it automatically detaches from the 
// NotifierList he is queued at. 
// If a NotifierList gets destroyed, it calls deletenotify() for each attached 
// handler. The default action will be that the handler deletes itself but 
// for other uses, a simple detach may be enough. 

template<class T>class NotifierList;

template<class T>class NotifyHandler : LinkedListBase< NotifyHandler<T> >
{
	friend class NotifierList<T>;
	friend class LinkedList< NotifyHandler<T> >;
	private:
		// Pointer to the handler list we're currently attacheda at or NULL. 
		NotifierList<T> *nlist;
		
	protected:
		// This function must be overridden to catch the notifications. 
		// Both parameters are passed just as specified at CallNotifiers(). 
		// NOTE: During this call, the handler may Detach(). 
		virtual void handlenotify(int /*ntype*/,T * /*ptr*/) HL_PureVirt(;)
		
		// This gets called for all notify handlers which are attached to 
		// a notifier list at the time the list gets destroyed. Default 
		// implementetion is to delete ourselves. 
		// NOTE: This function MUST make sure that this notify handler 
		//       gets dequeued from the notifier list. Self-destruction 
		//       will do so (see the destructor). 
		virtual void deletenotify()
			{  delete this;  }
		
	public:  _CPP_OPERATORS_FF
		NotifyHandler(int *failflag=NULL) : 
			LinkedListBase< NotifyHandler<T> >()  {  nlist=NULL;  }
		virtual ~NotifyHandler()
			{  Detach();  }
		
		// Get pointer to notifier list we're currently attached to: 
		// Returns NULL if not attached to a notifier list. 
		NotifierList<T> *GetNList() const
			{  return(nlist);  }
		
		// Detach from the list we're currently attached. 
		// Returns pointer to this handler (this). 
		inline NotifyHandler *Detach();
};


template<class T>class NotifierList
{
	friend class NotifyHandler<T>;
	private:
		LinkedList< NotifyHandler<T> > nhlist;
		
		// NOT C++-safe: 
		NotifierList &operator=(const NotifierList &) { return(*this); }
		NotifierList(const NotifierList &){}
	public:  _CPP_OPERATORS_FF
		// Create a notifier list. 
		NotifierList(int *failflag=NULL) : nhlist(failflag) {}
		// Destroy a notifier list; all handlers will receive a 
		// deletenotify(). 
		~NotifierList()
			{  while(nhlist.first())  nhlist.first()->deletenotify();  }
		
		// Is then notifier list empty?
		bool is_empty() const
			{  return(nhlist.is_empty());  }
		
		// Clear the notifier list, i.e. remove all elements. 
		// (This will not destroy the attached handlers.) 
		void Clear()
		{
			NotifyHandler<T> *i;
			while((i=nhlist.popfirst()))  i->nlist=NULL;
		}
		
		// Call all notifiers int the chain and pass the parameters to the 
		// handlenotify() functions. 
		// Returns number of notifiers called. 
		int CallNotifiers(int ntype,T *ptr)
		{
			int num=0;
			for(NotifyHandler<T> *_i=nhlist.first(),*i; _i; ++num)
			{  i=_i;  _i=_i->next;  i->handlenotify(ntype,ptr);  }
			return(num);
		}
		
		// Append a notify handler. 
		// Return value: 
		//   1 -> nothing done since this handler is already attached to 
		//        this list 
		//   0 -> OK
		//  -2 -> this handler is already attached to some other class
		int Append(NotifyHandler<T> *nh)
		{
			if(nh->nlist)  return(nh->nlist==this ? 1 : -2);
			nhlist.append(nh);  nh->nlist=this;  return(0);
		}
		
		// Remove a notify handler. (This will not destroy the handler.) 
		// Return value: 
		//   0 -> OK
		//  -2 -> this handler is not attached to this list
		int Remove(NotifyHandler<T> *nh)
		{
			if(nh->nlist!=this)  return(-2);
			nhlist.dequeue(nh);  nh->nlist=NULL;  return(0);
		}
};

// Must be placed here. 
template<class T>inline NotifyHandler<T> *NotifyHandler<T>::Detach()
	{  if(nlist)  nlist->nhlist.dequeue(this)->nlist=NULL;  return(this);  }

#endif  /* _HLIB_HLNotifierList_H_ */
