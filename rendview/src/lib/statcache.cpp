/*
 * statcache.cpp
 * 
 * Implementation of class FileStateCache. 
 * This class simply caches the stat(2) results. 
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

#include "statcache.hpp"


FileStateCache::Node::Node(int *failflag) : 
	path(failflag),
	mtime(HTime::Invalid)
{
	size=-2;  // "invalid"
}

/******************************************************************************/

FileStateCache::Node *FileStateCache::_FindEntry(const RefString *path)
{
	if(!path)  return(NULL);
	Node *found=NULL;
	for(Node *i=nlist.first(); i; i=i->next)
	{
		// Simply check the ref pointer instead of comparing 
		// the strings. 
		if(i->path.IsSameRef(*path))
		{  found=i;  ++first_hits;  goto foundit;  }
	}
	for(Node *i=nlist.first(); i; i=i->next)
	{
		if(!strcmp(i->path.str(),path->str()))
		{  found=i;  ++second_hits;  goto foundit;  }
	}
	++misses;
	foundit:;
	return(found);
}

FileStateCache::Node *FileStateCache::_CreateEntry(const RefString *path)
{
	Node *n=NEW<Node>();
	if(n)
	{
		n->path=*path;
		nlist.append(n);
	}
	return(n);
}


void FileStateCache::UpdateCache(const RefString *path,const HTime *mtime,
	int64_t size)
{
	if(!path)  return;
	Node *n=_FindEntry(path);
	if(!n)
	{  n=_CreateEntry(path);  }
	if(!n)  return;
	
	if(mtime)
	{  n->mtime=*mtime;  }
	n->size=size;
	
	completely_invalid=0;
}


int FileStateCache::GetCache(const RefString *path,HTime *ret_mtime,
	int64_t *ret_size)
{
	Node *n=_FindEntry(path);
	if(!n)  // also true if !path
	{  return(1);  }
	
	if(ret_mtime)
	{  *ret_mtime=n->mtime;  }
	if(ret_size)
	{  *ret_size=n->size;  }
	
	return(0);
}


void FileStateCache::RemoveCache(const RefString *path)
{
	Node *n=_FindEntry(path);
	if(!n) return;  // also true if !path
	delete nlist.dequeue(n);
}


void FileStateCache::InvalidateAll()
{
	if(!completely_invalid)
	{
		for(Node *i=nlist.first(); i; i=i->next)
		{
			i->mtime.SetInvalid();
			i->size=-2;
		}
		completely_invalid=1;
	}
}

void FileStateCache::Clear()
{
	for(;;)
	{
		Node *n=nlist.popfirst();
		if(!n)  break;
		delete n;
	}
	completely_invalid=1;
}


FileStateCache::FileStateCache(int *failflag) : 
	nlist(failflag)
{
	first_hits=0;
	second_hits=0;
	misses=0;
	
	completely_invalid=1;
}

FileStateCache::~FileStateCache()
{
	Clear();
	Verbose(DBGV,"FileStateCache: hits: first=%d, second=%d; misses: %d\n",
		first_hits,second_hits,misses);
}
