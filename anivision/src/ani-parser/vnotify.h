/*
 * ani-parser/vnotify.h
 * 
 * Value change/delete notifier container/handler. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_ANI_VALUENOTIFY_H_
#define _ANIVISION_ANI_VALUENOTIFY_H_

#include <hlib/linkedlist.h>


namespace ANI
{

// Defined in "exprvalue.h"
class ExprValue;

// As defined below: 
class ValueNotifyHandler;
class ValueNotifierContainer;


// This is the notifier class attached to the ExprValue (atually 
// InternalExprValue). It manages the handler list. etc. 
class ValueNotifierContainer
{
	public:
		// Entry in the notifier list: 
		// NOTE: LinkedList<> held by ValueNotifyHandler. 
		struct Entry : LinkedListBase<Entry>
		{
			// Next entry in ValueNotifierContainer's list: 
			Entry *c_next;
			// Pointer to the handler (for virtual functions): 
			ValueNotifyHandler *hdl;
			// Hook for the notify handler to hang arbitrary data. 
			void *hook;
			// Associated container (so that the handler knows...)
			ValueNotifierContainer *container;
			
			_CPP_OPERATORS
			Entry(ValueNotifierContainer *_container,
				ValueNotifyHandler *_hdl,void *_hook=NULL) : 
				LinkedListBase<Entry>()
				{  c_next=NULL; hdl=_hdl; hook=_hook; container=_container;  }
			~Entry()
				{}
		}__attribute__((__packed__));
		
	private:
		// "List" of notifiers which want to receive value notifications. 
		// Uses c_next as chain. 
		Entry *c_first;
		
		void _ClearList();
		void _EmitVN_Delete();
		void _EmitVN_Change();
	public: _CPP_OPERATORS
		ValueNotifierContainer() : c_first(NULL)  { }
		~ValueNotifierContainer()
			{  if(c_first)  _ClearList();  }
		
		//*** Interface to ExprValue ***
		
		// Emit the vn_change notify: 
		inline void EmitVN_Change()
			{  if(c_first) _EmitVN_Change();  }
		// Emit the vn_delete notify: 
		inline void EmitVN_Delete()
			{  if(c_first) _EmitVN_Delete();  }
		
		//--------------------------------------------------------------
		//*** Interface to ValueNotifyHandler ***
		
		// Install a notifier. 
		// Install handler *hdl for the value; hook is an arbitrary 
		// pointer which may be used by the caller and the handler 
		// to hang arbitrary data on. 
		// Return value: 
		//   0 -> OK, installed
		//   1 -> handler was already in list for this variable
		int InstallNotifier(ValueNotifyHandler *hdl,void *hook=NULL);
		
		// Uninstall a notifier: 
		//   0 -> OK, installed
		//   1 -> handler was not installed
		int UninstallNotifier(ValueNotifyHandler *hdl);
		
		// Directly uninstall a known notifier: 
		void UninstallEntry(Entry *e);
}__attribute__((__packed__));


// Derive your class from this one to receive value notifications. 
// One ValueNotifyHandler can attach itself to any number of 
// ExprValues. 
class ValueNotifyHandler
{
	friend class ValueNotifierContainer;
	private:
		// List of notifier entries created by us. 
		LinkedList<ValueNotifierContainer::Entry> ne_list;
		
		// Used by the container to keep the ne_list in sync: 
		inline void _EntryAdded(ValueNotifierContainer::Entry *e)
			{  ne_list.append(e);  }
		inline void _EntryRemoved(ValueNotifierContainer::Entry *e)
			{  ne_list.dequeue(e);  }
	protected:
		// These are the virtual functions which get 
		// called as value notification: 
		// Value change: 
		virtual void vn_change(void *hook);
		// Value gets deleted: 
		virtual void vn_delete(void *hook);
	public:  _CPP_OPERATORS
		ValueNotifyHandler() : ne_list() { }
		virtual ~ValueNotifyHandler()  {  UninstallAll();  }
		
		// Use these functions to install/remove the handler: 
		// Return value: 
		//   0 -> OK, installed
		//  -1 -> cannot install, ev is a NULL ref
		//   1 -> already installed
		int VNInstall(ExprValue &ev,void *hook=NULL);
		// Return value: 
		//   0 -> OK, removed
		//   1 -> not installed
		int VNUninstall(ExprValue &ev);
		
		// Uninstall all notifiers from us. Also done by destructor. 
		void UninstallAll();
};

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_VALUENOTIFY_H_ */
