/*
 * refstrlist.cc 
 * 
 * Implementation of simple linked string list. 
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

#include "refstrlist.h"

#include <string.h>


const RefStrList::Node *RefStrList::find(const char *str)
{
	if(!str)
	{  // Find first NULL-ref: 
		for(Node *n=list.first(); n; n=n->next)
		{
			if(!n->str())
			{  return(n);  }
		}
	}
	else
	{
		size_t str_len=strlen(str);
		for(Node *n=list.first(); n; n=n->next)
		{
			int t=n->stype();
			if(t<0)  continue;
			if(!t)  // normal '\0'-terminated string
			{  if(strcmp(n->str(),str))  continue;  }
			else   // data string
			{
				size_t len=n->len();
				// Ignore last '\0'-char if there is one. 
				if(len && *(n->str()+len-1)=='\0')
				{  --len;  }
				if(len!=str_len)  continue;
				if(strncmp(n->str(),str,str_len))  continue;
			}
			return(n);
		}
	}
	return(NULL);
}


int RefStrList::queue(const RefString &str,Node *where,int loc)
{
	if(!loc || !where)  return(-2);
	Node *n=NEW<Node>();
	if(!n)  return(-1);
	n->set(str);  // cannot fail (REF-string)
	// queue...
	list.queue(n,where,loc);
	return(0);
}


int RefStrList::insert(const RefStrList *lst)
{
	for(const Node *n=lst->list.last(); n; n=n->prev)
	{
		if(this->insert(*n))
		{  return(1);  }
	}
	return(0);
}

int RefStrList::append(const RefStrList *lst)
{
	for(const Node *n=lst->list.first(); n; n=n->next)
	{
		if(this->append(*n))
		{  return(1);  }
	}
	return(0);
}


int RefStrList::_insapp(const RefString &ref,int where)
{
	Node *n=NEW<Node>();
	if(!n)  return(-1);
	n->set(ref);  // cannot fail (REF-string)
	// queue...
	if(where<0)  list.insert(n);
	else         list.append(n);
	return(0);
}

int RefStrList::_insapp(const char *str,int where)
{
	RefString cp;
	if(cp.set(str))
	{  return(-1);  }
	return(_insapp(cp,where));
}


void RefStrList::clear()
{
	Node *n;
	while((n=list.popfirst()))
	{  delete n;  }   // delete node with string ref
}

RefStrList::RefStrList(int * /*failflag*/=NULL) : list()
{
}

RefStrList::~RefStrList()
{
	clear();
}
