/*
 * ani-parser/vnotify.cc
 * 
 * Value change/delete notify container and handler. 
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

#include "vnotify.h"
#include "exprvalue.h"


namespace ANI
{

void ValueNotifyHandler::vn_change(void * /*hook*/)
{
	// Default implementation: do nothing. 
	// No... for bug hunting we disallow that. 
	fprintf(stderr,"OOPS: VNH:n_change() not overridden\n");
	assert(0);
}

void ValueNotifyHandler::vn_delete(void * /*hook*/)
{
	// Default implementation: do nothing. 
	// No... for bug hunting we disallow that. 
	fprintf(stderr,"OOPS: VNH:n_delete() not overridden\n");
	assert(0);
}


int ValueNotifyHandler::VNInstall(ExprValue &ev,void *hook)
{
	return(ev.InstallVNH(this,hook));
}


int ValueNotifyHandler::VNUninstall(ExprValue &ev)
{
	return(ev.UninstallVNH(this));
}


void ValueNotifyHandler::UninstallAll()
{
	// Note: The actual removal is done by _EntryRemoved() called 
	//       via ValueNotifierContainer::UninstallEntry(). 
	while(!ne_list.is_empty())
	{
		assert(ne_list.first()->hdl==this);
		ValueNotifierContainer *c=ne_list.first()->container;
		c->UninstallEntry(ne_list.first());
	}
}


//------------------------------------------------------------------------------

void ValueNotifierContainer::_EmitVN_Change()
{
	for(Entry *_i=c_first; _i; )
	{
		Entry *i=_i;
		_i=_i->c_next;
		
		i->hdl->vn_change(i->hook);
	}
}

void ValueNotifierContainer::_EmitVN_Delete()
{
	for(Entry *_i=c_first; _i; )
	{
		Entry *i=_i;
		_i=_i->c_next;
		
		i->hdl->vn_delete(i->hook);
	}
}


int ValueNotifierContainer::InstallNotifier(ValueNotifyHandler *hdl,void *hook)
{
	// See handler is already in our list. 
	Entry *prev=NULL;
	for(Entry *i=c_first; i; prev=i,i=i->c_next)
	{
		if(i->hdl==hdl) return(1);
	}
	
	// Install handler: 
	Entry *e=new Entry(this,hdl,hook);
	if(prev)
	{
		assert(!prev->c_next);
		prev->c_next=e;
	}
	else
	{
		assert(!c_first);
		c_first=e;
	}
	
	// Tell handler: 
	hdl->_EntryAdded(e);
	
	return(0);
}

int ValueNotifierContainer::UninstallNotifier(ValueNotifyHandler *hdl)
{
	for(Entry *i=c_first,*prev=NULL; i; prev=i,i=i->c_next)
	{
		if(i->hdl!=hdl)  continue;
		if(prev)
		{  prev->c_next=i->c_next;  }
		else
		{  c_first=i->c_next;  }
		i->hdl->_EntryRemoved(i);
		delete i;
		return(0);
	}
	return(1);
}


void ValueNotifierContainer::UninstallEntry(Entry *e)
{
	#warning "should use doubly linked list to avoid lookup."
	for(Entry *i=c_first,*prev=NULL; i; prev=i,i=i->c_next)
	{
		if(i!=e)  continue;
		if(prev)
		{  prev->c_next=i->c_next;  }
		else
		{  c_first=i->c_next;  }
		i->hdl->_EntryRemoved(i);
		delete i;
		return;
	}
	assert(0);  // must have been in list
}

void ValueNotifierContainer::_ClearList()
{
	while(c_first)
	{
		Entry *tmp=c_first;
		c_first=c_first->c_next;
		tmp->hdl->_EntryRemoved(tmp);
		delete tmp;  // vn_delete() issued from IEV destructor
	}
}


}  // end of namespace ANI
