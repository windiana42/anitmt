/*
 * fdbase.cc
 * 
 * Implementation of class FDBase, a base class for 
 * file descriptor and time management which works 
 * in cooperation with class FDManager. 
 *
 * Copyright (c) 2000--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include <stdio.h>   /* for ``NULL'' */
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>

#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>


#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. 
#endif


FDManager::TimerNode *FDBase::AllocTimerNode(long msec,const void *dptr)
{
	FDManager::TimerNode *n=(FDManager::TimerNode *)LMalloc(
		sizeof(FDManager::TimerNode));
	if(n)
	{
		n->next=NULL;
		n->prev=NULL;
		n->msec_val=msec;
		n->msec_left=msec;
		n->dptr=dptr;
	}
	return(n);
}


void FDBase::DeleteTimerNode(FDManager::TimerNode *n)
{
	if(!n)
	{  return;  }
#if TESTING>1
fprintf(stderr,"DeleteTimer: %ld, %ld\n",n->msec_val,n->msec_left);
#endif
	
	if(n==sh_timer)
	{
		sh_timer=NULL;
		sh_timer_dirty=1;
		fdmanager()->TimeoutChange();
	}
	
	if(timerlock>=0)
	{
		// we may not really delete, just mark for later deletion
		n->msec_val=-9999L;
		n->msec_left=-3;  // magic... (see return value of TimerLeft())
		++timerlock;
		return;
	}
	
	if(n==timers)
	{
		timers=timers->next;
		if(timers)
		{  timers->prev=NULL;  }
	}
	else
	{
		if(n->prev)  {  n->prev->next=n->next;  }
		if(n->next)  {  n->next->prev=n->prev;  }
	}
	LFree(n);
}


// Returns number of deletes timer nodes: 
// DO NOT CLEAR WHEN LOCKED!
int FDBase::ClearTimers()
{
	int ndel=0;
	#if TESTING
	if(timerlock>=0)
	{  fprintf(stderr,"BUG!! FDBase::ClearTimers() with timerlock=%d\n",timerlock);  }
	#endif
	//if(timerlock>=0)
	//{
	//	while(timers)
	//	{  DeleteTimerNode(timers);  ++ndel;  }
	//}
	while(timers)
	{
//fprintf(stderr,"<");
		FDManager::TimerNode *n=timers;
		timers=timers->next;
//fprintf(stderr,"%p/%p",n,n->next);  n->next=(FDManager::TimerNode*)0xf3f3f3f3;
		LFree(n);
//fprintf(stderr,">");
		++ndel;
	}
	sh_timer_dirty=0;
	if(sh_timer)
	{
		sh_timer=NULL;
		fdmanager()->TimeoutChange();
	}
	return(ndel);
}


// Delete those with msec=-9999: 
// Gets only called if there is something to delete. 
void FDBase::_UnlockTimers()
{
	timerlock=-1;  // important. 
	for(FDManager::TimerNode *i=timers; i; )
	{
		FDManager::TimerNode *n=i;
		i=i->next;
		if(n->msec_val==-9999L)
		{  DeleteTimerNode(n);  }
	}
}


// Go through list and find shortest enabled timer: 
struct FDManager::TimerNode *FDBase::_GetShortestTimer()
{
	// Clear dirty flag: 
	sh_timer_dirty=0;
	
	// Find first enabled timer:
	for(FDManager::TimerNode *i=timers; i; i=i->next)
	{
		if(i->msec_val<0)  continue;  // disabled or -9999 -> to be deleted
		sh_timer=i;
		goto found;
	}
	// No timer at all: 
	sh_timer=NULL;
	return(sh_timer);
	found:;
	
	// Go through rest of list and look for shorter timers: 
	long sh_msec=sh_timer->msec_left;
	for(FDManager::TimerNode *i=sh_timer; i; i=i->next)
	{
		if(i->msec_val<0)  continue;  // disabled or -9999 -> to be deleted
		if(i->msec_left>=sh_msec)  continue;
		sh_timer=i;
		sh_msec=sh_timer->msec_left;
		if(!sh_msec)  break;   // 0 msec timer
	}
	
	return(sh_timer);
}


/************************* FD Stuff **************************/


FDManager::FDNode *FDBase::AllocFDNode(int fd,short events,const void *dptr)
{
	FDManager::FDNode *n=(FDManager::FDNode *)LMalloc(
		sizeof(FDManager::FDNode));
	if(n)
	{
		n->prev=NULL;
		n->next=NULL;
		n->fd=fd;
		n->events=events;
		n->idx=-1;
		n->dptr=dptr;
	}
	return(n);
}


void FDBase::DeleteFDNode(FDManager::FDNode *n)
{
	if(!n)
	{  return;  }
	if(fdslock>=0)
	{
		// We may not really delete, just mark for later deletion
		n->idx=-2;   // IMPORTANT: -2
		++fdslock;
		return;
	}
	if(n==fds)
	{
		fds=fds->next;
		if(fds)
		{  fds->prev=NULL;  }
	}
	else
	{
		if(n->prev)  {  n->prev->next=n->next;  }
		if(n->next)  {  n->next->prev=n->prev;  }
	}
	// MAY NOT shutdown/close n->fd. 
	LFree(n);
}


// Returns number of deleted fd nodee and increments *pollfds 
// for each deleted fd with events!=0. 
// pollfds MAY NOT BE NULL; use ClearFDs() instead. 
// DO NOT CLEAR WHEN LOCKED!
int FDBase::_ClearFDs(int close_them,int *pollfds)
{
	int ndel=0;
	#if TESTING
	if(fdslock>=0)
	{  fprintf(stderr,"BUG!! FDBase::ClearFDs() with fdslock=%d\n",fdslock);  }
	#endif
	//if(fdslock>=0)
	//{
	//	while(fds)
	//	{  DeleteFDNode(fds);  ++ndel;  }
	//  return;
	//}
	while(fds)
	{
		FDManager::FDNode *n=fds;
		fds=fds->next;
		
		if(close_them)
		{
			// WE DO NOT shutdown the fd because that might kill a 
			// network connection used by other classes, too. 
			//shutdown(n->fd,2);
			#if TESTING
			// this means some FDBase did not close/unregister its fd. 
			fprintf(stderr,"ClearFDs(): deleting fd=%d. (close()=%d)\n",
				n->fd,close(n->fd));
			#else
			int rv;
			do
			{  rv=close(n->fd);  }
			while(rv<0 && errno==EINTR);
			#endif
		}
		if(n->events)  ++(*pollfds);
		
		LFree(n);
		++ndel;
	}
	return(ndel);
}


// Delete those with idx=-2: 
// Gets only called if there is something to delete. 
void FDBase::_UnlockFDs()
{
	fdslock=-1;  // important. 
	for(FDManager::FDNode *i=fds; i; )
	{
		FDManager::FDNode *n=i;
		i=i->next;
		if(n->idx==-2)
		{  DeleteFDNode(n);  }
	}
}

// Query PollID of specified fd: 
FDBase::PollID FDBase::FDPollID(int fd)
{
	if(fd<0)  return(NULL);
	for(FDManager::FDNode *i=fds; i; i=i->next)
	{
		if(i->fd==fd)
		{  return((PollID)i);  }
	}
	return(NULL);
}
// Query FD data (only fds that this FDBase is polling for): 
// Get custon data pointer associated with fd or NULL: 
const void *FDBase::FDDPtr(int fd)
{
	if(fd<0)  return(NULL);
	for(FDManager::FDNode *i=fds; i; i=i->next)
	{
		if(i->fd==fd)
		{  return(i->dptr);  }
	}
	return(NULL);
}
// Get events mask for this fd or -1 in case of invalid fd: 
short FDBase::FDEvents(int fd)
{
	if(fd<0)  return(-1);
	for(FDManager::FDNode *i=fds; i; i=i->next)
	{
		if(i->fd==fd)
		{  return(i->events);  }
	}
	return(-1);
}


FDBase::FDBase(int *failflag=NULL)
{
	timers=NULL;
	fds=NULL;
	deleted=1;  // Registering will set deleted=0. 
	ismanager=0;
	timerlock=-1;
	fdslock=-1;
	sh_timer=NULL;
	sh_timer_dirty=0;
	
	int retval=0;
	if(fdmanager()->Register(this))
	{
		// Damn! error registering. *this may not exist and must be deleted. 
		--retval;
	}
	
	if(failflag)
	{  *failflag+=retval;  }  // decreased as retval<0. 
	else if(retval)
	{  ConstructorFailedExit();  }  // we're not using exceptions
}


FDBase::~FDBase()
{
//fprintf(stderr,"FDBase.....");
	fdmanager()->Unregister(this,1);   // 1 -> just unregister
	
//fprintf(stderr,"1.....");
	// Everything which is left in the lists MUST now be deleted. 
	#if TESTING
	if(timerlock>=0 && fdslock>=0)
	{  fprintf(stderr,"error: ~FDBase: locks=%d,%d (BUG!!)\n",
		timerlock,fdslock);  }
	#endif
	
	// ... MUST be deleted...
	timerlock=-1;
	fdslock=-1;
	
//fprintf(stderr,"2.....");
	ClearTimers();
//fprintf(stderr,"3.....");
	int npollfds=0;  // with events!=0
	int nfds=_ClearFDs(/*close_them=*/ 1,&npollfds);
//fprintf(stderr,"4.....");
	fdmanager()->DestructionDone(this,nfds,npollfds);
//fprintf(stderr,"E\n");
}

