/*
 * refstrhash.cc 
 * 
 * Implementation of type-independent part of the RefStringHash class, 
 * a hash template for mapping RefStrings to any other type. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include "refstrhash.h"


int _InternalRefStringHash::_dohash(const char *x) const
{
	// Need '\0'-terminated string. 
	// Hash function borrowed from Qt. 
	unsigned int hv=0,g;
	for(const char *c=x; *c; c++)
	{
		hv=(hv<<4) + (int)(*((unsigned char*)c));
		g=hv&0xf0000000U;
		if(g)  hv^=g>>24;
		hv&=~g;
	}
	hv=hv%((unsigned int)hash_size);
//fprintf(stderr,"<%u>",hv);
	return((int)hv);
}


_InternalRefStringHash::EntryBase *_InternalRefStringHash::_search(
	const char *key,int hash_val,int lru_requeue)
{
	EntryBase *prev=NULL;
	for(EntryBase *i=hash[hash_val]; i; prev=i,i=i->next)
	{
		if(i->key!=key)  continue;
		// Found entry. 
		if(lru_requeue && prev)
		{
			prev->next=i->next;
			i->next=hash[hash_val];
			hash[hash_val]=i;
		}
		return(i);
	}
	return(NULL);
}

_InternalRefStringHash::EntryBase *_InternalRefStringHash::_search_and_dequeue(
	const char *key,int hash_val)
{
	EntryBase *prev=NULL;
	for(EntryBase *i=hash[hash_val]; i; prev=i,i=i->next)
	{
		if(i->key!=key)  continue;
		// Found entry. 
		if(prev)
		{  prev->next=i->next;  }
		else
		{  hash[hash_val]=i->next;  }
		i->next=NULL;
		return(i);
	}
	return(NULL);
}


int _InternalRefStringHash::_store(const RefString &key,const void *value,
	int lru_requeue,int dont_search)
{
	if(!key || *key.str()=='\0')
	{ return(-2);  }
	
	int hash_val=_dohash(key);
	
	if(!dont_search)
	{
		EntryBase *ent=_search(key,hash_val,lru_requeue);
		if(ent)
		{
			_AssignEntry(ent,value);
			return(1);
		}
	}
	
	EntryBase *ent=_NewEntry(key,value);
	if(!ent)
	{  return(-1);  }
	
	// Store value (as head): 
	ent->next=hash[hash_val];
	hash[hash_val]=ent;
	
	return(0);
}


int _InternalRefStringHash::_remove(const RefString &key,const void *value)
{
	if(!key || *key.str()=='\0')
	{ return(-2);  }
	
	EntryBase *ent=_search_and_dequeue(key,_dohash(key));
	if(!ent)
	{  return(1);  }
	
	if(value)
	{  _AssignEntry(ent,value);  }
	
	_DeleteEntry(ent);
	
	return(0);
}


void _InternalRefStringHash::clear()
{
	for(int i=0; i<hash_size; i++)
	{
		EntryBase *head=hash[i];
		EntryBase *tmp;
		while(head)
		{
			tmp=head;
			head=head->next;
			_DeleteEntry(tmp);
		}
		hash[i]=NULL;
	}
}


int _InternalRefStringHash::count() const
{
	int cnt=0;
	for(int i=0; i<hash_size; i++)
	{
		for(EntryBase *e=hash[i]; e; e=e->next)
		{  ++cnt;  }
	}
	return(cnt);
}


_InternalRefStringHash::_InternalRefStringHash(int _hash_size,int *failflag)
{
	int failed=0;
	
	hash_size=_hash_size<1 ? 1 : _hash_size;
	hash=(EntryBase**)LMalloc(sizeof(EntryBase*)*hash_size);
	if(!hash)  ++failed;
	else for(int i=0; i<hash_size; i++)
	{  hash[i]=NULL;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("RStrHash");  }
}

_InternalRefStringHash::~_InternalRefStringHash()
{
	// May not call clear() here becuase it uses a virtual function. 
	hash=(EntryBase**)LFree(hash);
}
