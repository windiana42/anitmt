/*
 * strlist.cc 
 * 
 * Implementation of simple linked string list. 
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

#include <string.h>
#include "strlist.h"

const StrList::Node *StrList::find(const char *str)
{
	if(!str)  return(NULL);
	size_t str_len=strlen(str);
	for(Node *n=list.first(); n; n=n->next)
	{
		if(n->len!=str_len)  continue;
		if(strcmp(n->str,str))  continue;
		return(n);
	}
	return(NULL);
}


int StrList::insert(const StrList *lst)
{
	for(const Node *n=lst->list.last(); n; n=n->prev)
	{
		if(this->insert(n->str,n->len))
		{  return(1);  }
	}
	return(0);
}

int StrList::append(const StrList *lst)
{
	for(const Node *n=lst->list.first(); n; n=n->next)
	{
		if(this->append(n->str,n->len))
		{  return(1);  }
	}
	return(0);
}


int StrList::_insapp(const char *str,size_t len,int where)
{
	// str is NOT NULL here. 
	Node *n=NEW<Node>();
	if(!n)  return(-1);
	n->str=StrList::alloc(len+1);
	if(!n->str)
	{  delete n;  return(-1);  }
	strncpy((char*)n->str,str,len);
	((char*)n->str)[len]='\0';
	n->len=len;
	// queue...
	if(where<0)  list.insert(n);
	else         list.append(n);
	return(0);
}


void StrList::clear()
{
	Node *n;
	while((n=list.popfirst()))
	{  delete n;  }   // deletes n->str 
}

StrList::StrList(int * /*failflag*/=NULL) : list()
{
}

StrList::~StrList()
{
	clear();
}
