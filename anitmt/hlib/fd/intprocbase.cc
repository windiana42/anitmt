/*
 * intprocbase.cc
 * 
 * Implementation of subclasses of the class InternalProcessManager. 
 *
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>  /* MUST BE FIRST */

#include <string.h>
#include <stdarg.h>

#include <hlib/refstrlist.h>

#include "htime.h"
#include "fdmanager.h"
#include "fdbase.h"
//#include "intprocbase.h"
#include "procmanager.h"



// use environment as passed to ProcessManager(). 
InternalProcessBase::ProcEnv::ProcEnv()
{
	env=ProcessManager::manager->GetEnv();
	freearray=0;
}

InternalProcessBase::ProcEnv::ProcEnv(const char *env0,...)
{
	// Count them...
	int n=1;
	if(env0)
	{
		++n;
		va_list ap;
		va_start(ap,env0);
		const char *s;
		while((s=va_arg(ap,const char*)))  ++n;
		va_end(ap);
	}
	const char **xenv=(const char **)LMalloc(n*sizeof(char *));
	if(!xenv)
	{
		freearray=2;   // StartProcess will detect that. 
		env=NULL;
		return;
	}
	
	// ...and assign the pointers. 
	n=0;
	if(env0)
	{
		xenv[n++]=env0;
		va_list ap;
		va_start(ap,env0);
		const char *s;
		while((s=va_arg(ap,const char*)))
		{  xenv[n++]=s;  }
		va_end(ap);
	}
	xenv[n]=NULL;  // important
	
	env=(char *const *)xenv;
	freearray=1;
}


InternalProcessBase::ProcArgs::ProcArgs(const char *args0,...)
{
	// Count them...
	int n=1;
	if(args0)
	{
		++n;
		va_list ap;
		va_start(ap,args0);
		const char *s;
		while((s=va_arg(ap,const char*)))  ++n;
		va_end(ap);
	}
	const char **xargs=(const char **)LMalloc(n*sizeof(char *));
	if(!xargs)
	{
		freearray=2;   // StartProcess will detect that. 
		args=NULL;
		return;
	}
	
	// ...and assign the pointers. 
	n=0;
	if(args0)
	{
		xargs[n++]=args0;
		va_list ap;
		va_start(ap,args0);
		const char *s;
		while((s=va_arg(ap,const char*)))
		{  xargs[n++]=s;  }
		va_end(ap);
	}
	xargs[n]=NULL;  // important
	
	args=(char *const *)xargs;
	freearray=1;
}


InternalProcessBase::ProcArgs::ProcArgs(const RefStrList *arglist)
{
	args=NULL;
	if(!arglist)
	{  freearray=2;  return;  }   // StartProcess will detect that. 
	
	// Count them...
	int n=1;
	for(const RefStrList::Node *i=arglist->first(); i; i=i->next)
	{
		if(i->stype()!=0)  // must be '\0'-terminated string
		{  freearray=3;  return;  }   // StartProcess will detect that. 
		++n;
	}
	const char **xargs=(const char **)LMalloc(n*sizeof(char *));
	if(!xargs)
	{  freearray=2;  return;  }   // StartProcess will detect that. 
	
	// ...and assign the pointers. 
	n=0;
	for(const RefStrList::Node *i=arglist->first(); i; i=i->next)
	{  xargs[n++]=i->str();  }
	xargs[n]=NULL;  // important
	
	args=(char *const *)xargs;
	freearray=1;
}



InternalProcessBase::ProcPath::ProcPath(const char *name,const char *searchpath0,...)
{
	plist=NULL;
	path=name;
	
	// Count them...
	int n=1;
	if(searchpath0)
	{
		++n;
		va_list ap;
		va_start(ap,searchpath0);
		const char *s;
		while((s=va_arg(ap,const char*)))  ++n;
		va_end(ap);
	}
	const char **xsearchpath=(const char **)LMalloc(n*sizeof(char *));
	if(!xsearchpath)
	{
		freearray=2;   // StartProcess will detect that. 
		searchpath=NULL;
		return;
	}
	
	// ...and assign the pointers. 
	n=0;
	if(searchpath0)
	{
		xsearchpath[n++]=searchpath0;
		va_list ap;
		va_start(ap,searchpath0);
		const char *s;
		while((s=va_arg(ap,const char*)))
		{  xsearchpath[n++]=s;  }
		va_end(ap);
	}
	xsearchpath[n]=NULL;  // important
	
	searchpath=(char *const *)xsearchpath;
	freearray=1;
}


// our: 1 -> ourfd; 0 -> destfd
// Return value: idx or -1
#if 0  /* now in intprocbase.h because old gcc needs it there... */
inline int InternalProcessBase::ProcFDs::_Find(
	const int *array,int n,int tofind) const
{
	for(const int *i=array,*ie=array+n; i<ie; i++)
	{
		if((*i)==tofind)
		{  return(i-array);  }
	}
	return(-1);
}
#endif


int InternalProcessBase::ProcFDs::Add(int ofd,int dfd)
{
	if(ofd<0 || dfd<0)
	{  return(-3);  }
	if(_Find(destfd,n,dfd)>=0)
	{  return(-2);  }
	
	if(n>=dim)
	{
		dim+=16;
		int *newp=(int*)LMalloc(2*dim*sizeof(int));
		if(!newp)  return(-1);
		int *_ourfd=ourfd;
		int *_destfd=destfd;
		ourfd=newp;
		destfd=newp+dim;
		memcpy(ourfd,_ourfd,n*sizeof(int));
		memcpy(destfd,_destfd,n*sizeof(int));
		LFree(_ourfd);  // need not free _destfd =_ourfd+old_dim
	}
	
	ourfd[n]=ofd;
	destfd[n]=dfd;
	++n;
	
	return(0);
}


// Return value: 0 -> OK; -1 -> LMalloc() failed. 
int InternalProcessBase::ProcFDs::Copy(const ProcFDs &pfds)
{
	ourfd=(int*)LFree(ourfd);
	destfd=NULL;
	dim=0; n=0;
	
	if(pfds.n)
	{
		int *newp=(int*)LMalloc(2*pfds.n*sizeof(int));
		if(!newp)  return(-1);
		n=pfds.n;
		dim=n;
		ourfd=newp;
		destfd=newp+dim;
		memcpy(ourfd,pfds.ourfd,n*sizeof(int));
		memcpy(destfd,pfds.destfd,n*sizeof(int));
	}
	return(0);
}
