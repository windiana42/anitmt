/*
 * searchpath.cc
 * 
 * Implementation of class FileSearchPath, a simple search path 
 * file finder. 
 *
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/prototypes.h>  /* MUST BE FIRST */

#include <hlib/searchpath.h>

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif


FileSearchPath::Node *FileSearchPath::_FindNode(const char *path,size_t strlen_path)
{
	if(!path || !(*path))
	{  return(NULL);  }
	
	size_t plen=strlen_path;
	for(const Node *i=plist.first(); i; i=i->next)
	{
		size_t ilen=i->path.len()-1;  // without trailing "/"
		if(plen<ilen)  continue;
		if(strncmp(path,i->path.str(),ilen))  continue;
		// First ilen chars match. NOTE: ilen=0 if i->path="/". 
		for(const char *c=path+ilen; *c; c++)
		{  if(*c!='/')  goto cont;  }
		// i="path/"; search="path///" -> found
		// i="/"; search="///" -> found
		// i="/"; search="" -> caught above
		return((Node*)i);
		cont:;
	}
	
	return(NULL);
}


int FileSearchPath::_RequeueIfInList(const char *path,size_t strlen_path,int pos)
{
	Node *n=_FindNode(path,strlen_path);
	if(!n)  return(0);
	if( n != ((pos>0) ? plist.last() : plist.first()) )
	{
		plist.dequeue(n);
		if(pos>0)  plist.append(n);
		else       plist.insert(n);
	}
	return(1);
}


int FileSearchPath::_IntrnlAddNode(const char *path,size_t strlen_path,int pos)
{
	RefString tmp;
	
	// Make sure there is exactly one "/" at the end. 
	size_t len=strlen_path;
	// NOTE: strlen_path>=1 here. 
	if(path[len-1]!='/')
	{
		// Must append "/": 
		if(tmp.sprintf(0,"%s/",path))
		{  return(-1);  }
	}
	else
	{
		// See how many "/" there are...
		while(len>0 && path[len-1]=='/') --len;
		// "///" -> "/"
		// "path///" -> "path/"
		if(tmp.set0(path,len+1))
		{  return(-1);  }
	}
	
	Node *n=NEW<Node>();
	if(!n)  return(-1);
	n->path=tmp;
	
	if(pos>0)  plist.append(n);
	else       plist.insert(n);
	
	return(0);
}


int FileSearchPath::Add(const RefString *path,int pos)
{
	if(!path || !path->str() || !(*(path->str())))
	{  return(-2);  }
	
	// If the node is already in the list, re-queue it acordingly: 
	size_t len=path->len();
	if(_RequeueIfInList(path->str(),len,pos))
	{  return(0);  }
	
	// See if path is suitable for us; in this case, just reference it: 
	const char *p=path->str();
	if( (len==1) ? (p[0]=='/') : (p[len-1]=='/' && p[len-2]!='/') )
	{
		Node *n=NEW<Node>();
		if(!n)  return(-1);
		n->path=*path;
		if(pos>0)  plist.append(n);
		else       plist.insert(n);
		return(0);
	}
	
	return(_IntrnlAddNode(path->str(),len,pos));
}

int FileSearchPath::Add(const char *path,int pos)
{
	if(!path || !(*path))
	{  return(-2);  }
	
	// If the node is already in the list, re-queue it acordingly: 
	size_t len=strlen(path);
	if(_RequeueIfInList(path,len,pos))
	{  return(0);  }
	
	return(_IntrnlAddNode(path,len,pos));
}


int FileSearchPath::_SearchIterate(const char *file,RefString *result,
	const char *current_file,int current_order,
	int call_virt,void *virt_ptr)
{
	if(!file || !(*file))
	{  return(-2);  }
	size_t len=strlen(file);
	if(file[len-1]=='/')
	{  return(-3);  }
	
	// Check for absolute path. In this case, no path searching is done. 
	if(*file=='/')
	{
		int srv=(call_virt ? 
			SearchCheck(file,virt_ptr) : _SearchCheck(file,virt_ptr));
		if(srv==0 && result && result->set(file))  srv=-1;
		return(srv);
	}
	
	// Check current_file: 
	if(current_file && !(*current_file))
	{  return(-2);  }
	
	// No absolute path. Iterate through the list and current path if specified. 
	char *complete=NULL;
	size_t complete_len=0;
	const Node *i=plist.first();
	for(int action=-1; action<=1; )
	{
		// *path of length path_len (NOT '\0'-terminated!) is the 
		// next dir path to try. 
		const char *path;
		size_t path_len;
		
		// Get next *path value: 
		if(action==0 && !(current_file && current_order==0))
		{
			if(!i)
			{  ++action;  continue;  }
			path=i->path;
			path_len=i->path.len();
			i=i->next;
		}
		else if(current_file && current_order==action)
		{
			// If the last char is not a "/", then we must look for the 
			// end of the dirname. 
			path=current_file;
			path_len=strlen(current_file);  // >0 here. 
			while(path_len>0 && path[path_len-1]!='/')  --path_len;
			if(!path_len)
			{  path="./";  path_len=2;  }
			++action;
		}
		else
		{  ++action;  continue;  }
		
		size_t need=path_len+len+1;
		if(complete_len<need)
		{
			LFree(complete);
			complete=(char*)LMalloc(complete_len=need);
			if(!complete)  return(-1);
		}
		strncpy(complete,path,path_len);
		strcpy(complete+path_len,file);
		int srv=(call_virt ? 
			SearchCheck(file,virt_ptr) : _SearchCheck(file,virt_ptr));
		if(srv!=1)  // found or error
		{
			if(srv==0 && result && result->set(complete))  srv=-1;
			LFree(complete);
			return(srv);
		}
	}
	LFree(complete);
	
	return(1);  // not found
}


int FileSearchPath::_SearchCheck(const char *file,void *dptr)
{
	_SCDptr *dp=(_SCDptr *)dptr;
	if(dp->mode=='o')
	{
		dp->ret_fd=open(file,dp->flags);
		return((dp->ret_fd<0) ? 1 : 0);
	}
	if(dp->mode=='a')
	{  return( access(file,dp->flags)<0 ? 1 : 0 );  }
	return(-4);  // not implemented
}


int FileSearchPath::Access(const char *file,int flags,RefString *result,
	const char *current_file,int current_order)
{
	_SCDptr dp;
	dp.mode='a';
	dp.flags=flags;
	return(_SearchIterate(file,result,current_file,current_order,0,&dp));
}

int FileSearchPath::Open(const char *file,int *flag_fd,RefString *result,
	const char *current_file,int current_order)
{
	_SCDptr dp;
	dp.mode='a';
	dp.flags=*flag_fd;
	dp.ret_fd=-1;
	int rv=_SearchIterate(file,result,current_file,current_order,0,&dp);
	if(rv==0)  // found
	{  *flag_fd=dp.ret_fd;  }
	return(rv);
}


FileSearchPath::FileSearchPath(int *failflag) : 
	plist(failflag)
{
	// empty
}

FileSearchPath::~FileSearchPath()
{
	while(!plist.is_empty())
	{  delete plist.popfirst();  }
}

