/*
 * fdmanager.cc
 * 
 * Implementation of class FDManager, a class for 
 * file descriptor and time management which works in 
 * cooperation with classes derived from class FDBase. 
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

#define DEBUG 0

//------------------------------------------------------------------------------
// TODO LIST:
//  * NEED TO SEE WHAT TO DO WHEN WE HAVE NO FREE SIGNODES LEFT.  
//  * what to do if pollfd array LMalloc() fails? 
//    (or geerally switch to malloc() for that array?)
//  * can make __GetTimeout() faster?
//  * can make AlignTimer() faster?
//  * check timer alignment code...?
//------------------------------------------------------------------------------

#warning Does FDManager::SigInfo need _CPP_OPERATORS?

#include <hlib/prototypes.h>  /* MUST BE FIRST */

#include <string.h>

#include <hlib/htime.h>
#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>

#ifndef TESTING
#define TESTING 1
#endif

// TESTING_CHECK is does more internal control tests which will 
// probably slow down things quite much. Only enable for debug 
// reasons. 
#define TESTING_CHECK 1

#if TESTING
#  warning TESTING switched on. 
#endif

#if !TESTING
#  undef TESTING_CHECK
#  define TESTING_CHECK 0
#endif

#if TESTING_CHECK
#  warning !! ESTING_CHECK switched on. This will slow down performance. !!
#endif

#if DEBUG
#  if !TESTING
#    define DEBUG 0
#  else
#    warning DEBUG switched on. 
#    define pdebug(fmt...)   fprintf(stdout, "FD:" fmt);
#  endif
#else
#  define pdebug(fmt...)
#endif


/* global manager */
class FDManager *FDManager::manager=NULL;


#ifdef HLIB_CRIPPLED_SIGINFO_T
static RETSIGTYPE _fd_sig_handler_simple(int sig)
{
	if(!FDManager::manager)  return;
	siginfo_t tmp;
	tmp.si_signo=sig;
	FDManager::manager->CaughtSignal(&tmp);
}
#else
static void _fd_sig_handler_sigaction(int, siginfo_t *info, void *)
{
	if(!FDManager::manager)  return;
	// We don't need locking here as all other signals are blocked 
	// during signal handler execution (see FDManager::FDManager(), 
	// call to sigaction(2)). 
	FDManager::manager->CaughtSignal(info);
}
#endif


static int _fd_signals[]=
{
	SIGHUP, SIGINT, SIGTERM, 
	SIGUSR1, SIGUSR2, 
	SIGPIPE, SIGCHLD, 
	SIGWINCH, 
	SIGTSTP, SIGCONT, // terminal stop (^Z); cont with SIGCONT
	-22000 };

// THERE MAY BE ONLY ONE FDManager. 

// Be sure... a close checking for EINTR (if that will ever happen...)
static inline int fd_close(int fd)
{
	int rv;
	do
	{  rv=close(fd);  }
	while(rv<0 && errno==EINTR);
	return(rv);
}

/************************ INLINE FROM FDBase *************************/
// NOW IN fdbase.h AS SOME EARLIER GCC VERSIONS SOMETIMES GENERATE 
// PROBLEMS WHEN LINKING. 

/************************ FDManager *************************/

// Make sure that the number of free sig nodes is increased by delta 
// (decreasing it if delta<0). 
void FDManager::ExtraSigNodes(int delta)
{
	extra_signodes+=delta;
	if(delta>0)
	{  _EnsureFreeSigNodes();  }  // min_free+extra
	else if(extra_signodes<0)
	{
		#if TESTING
		fprintf(stderr,"FD: BUG! extra_signodes=%d <0 (%d)\n",
			extra_signodes,delta);
		#endif
		extra_signodes=0;
	}
}

// "Free" SigNode putting it into the list of free nodes (or really 
// free'ing it). 
void FDManager::_FreeSigNode(SigNode *sn)
{
	if(!sn)  return;
	if(free_signodes>=max_free_signodes)
	{  LFree(sn);  return;  }
	// Add it to the list of free signal nodes: 
	sn->next=first_free_signode;
	first_free_signode=sn;
	++free_signodes;
}

// "Alloc" a signal node by dequeuing it from the list of free nodes. 
// Returns NULL if there is none left. 
FDManager::SigNode *FDManager::_AllocSigNode()
{
	if(!first_free_signode)
	{  return(NULL);  }
	
	SigNode *sn=first_free_signode;
	
	// Dequeue the signal node from the list of free nodes: 
	first_free_signode=first_free_signode->next;
	--free_signodes;
	#if TESTING
	if(free_signodes<0)
	{  fprintf(stderr,"FD: **BUG!! free_signodes=%d in AllocSigNode()\n",
		free_signodes);  }
	#endif
	
	return(sn);
}


// If there are less than n free signal nodes in first_free_signode, 
// allocate some. 
// Return value: 0 -> OK; -1 -> allocation failure. 
int FDManager::__EnsureFreeSigNodes(int n)
{
	while(free_signodes<n)
	{
		SigNode *sn=(SigNode*)LMalloc(sizeof(SigNode));
		if(!sn)  return(-1);
		sn->next=first_free_signode;
		first_free_signode=sn;
		++free_signodes;
	}
	return(0);
}

void FDManager::CaughtSignal(siginfo_t *info)
{
	// Don't need locking here. All signals are blocked during the 
	// execution of this function. 
	#if TESTING
	static int lock=0;
	if(lock)
	{  fprintf(stderr,"FD: ** BUG!! Re-entered CaughtSignal()\n");  abort();  }
	lock=1;
	#endif
	
	// Okay, we must store the signal in a signal node. However, it is 
	// unsave to allocate the node here becuase the signal may just 
	// have interrupted a call to malloc() and we may destroy data...
	// So, it is necessary to store it in one of the pre-allocated 
	// signal nodes. 
	
	// "Allocate" signal node: 
	SigNode *sn=_AllocSigNode();
	if(!sn)   // Damn.. we cannot handle it as there is no free node left...
	{
		// FIXME:
		// In this case it is probably best to check if this signal 
		// was already sent and if so to overwrite an older signal 
		// node containing a signal which was sent more than once 
		// (e.g. SIGINT, SIGTERM, SIGCHLD [as ProcessManager knows 
		// how to deal with less signals than dead processes]). 
		#if TESTING
		fprintf(stderr,"Oops: Too many signals; no free signal node left.\n");
		lock=0;
		abort();
		#endif
		return;
	}
	
	// Set fields: 
	sn->next=NULL;
	sn->info.time=HTime(HTime::Curr);
	memcpy(&sn->info.info,info,sizeof(sn->info.info));
	
	// Append signal to the queue of pending signals: 
	if(last_pending_sig)
	{  last_pending_sig->next=sn;  }
	else
	{  first_pending_sig=sn;  }
	last_pending_sig=sn;
	
	++pending_signum;
	
	if(!sig_pipe_bytes)
	{
		// First signal since last check: 
		// Write to signal detection pipe...
		char buf='s';
		ssize_t wr;
		do
		{  wr=write(sig_pipe_fd_w,&buf,1);  }
		while(wr<0 && errno==EINTR);
		#if TESTING
		if(wr!=1)
		{  fprintf(stderr,"FD: Oops: writing to pipe failed (%d) %s\n",
			wr,strerror(errno));  abort();  }
		#endif
		++sig_pipe_bytes;  // wrote a byte to the signal pipe
	}
	
	#if TESTING
	lock=0;
	#endif
}


// Make signal detection pipe empty. 
void FDManager::_ClearSigPipe()
{
	// Clear signal detection pipe:
	ssize_t rd=0;
	char buf[4];
	// Note that the pipe is non-blocking. 
	// (We should actually never read more than 1 char.) 
	do
	{  rd=read(sig_pipe_fd_r,buf,4);  }
	while((rd<0 && errno==EINTR) || rd==4);
	#if TESTING
	if(rd<0 && errno!=EWOULDBLOCK)
	{  fprintf(stderr,"FD: error reading sig pipe: %s\n",
		strerror(errno));  }
	#endif
	// Completely read pipe, but do NOT set 
	// sig_pipe_bytes to 0 here!!
}


// Checks if that signal is queued for delivery: 
// Returns number of queued signals of type sig. 
int FDManager::SigPending(int sig)
{
	int cnt=0;
	for(SigNode *sn=first_pending_sig; sn; sn=sn->next)
	{
		if(sn->info.info.si_signo==sig)
		{  ++cnt;  }
	}
	return(cnt);
}


// return: -1 -> FAILED!! 0 -> OK
int FDManager::Register(FDBase *fdb)
{
	if(fdblist.FindNode(fdb))
	{
		pdebug("FDBase %p (manager=%d) attempted to register twice.\n",
			fdb,fdb->manager);
		return(0);   // already have you
	}
	if(fdblist.Add(fdb))
	{  return(-1);  }
	fdb->deleted=0;
	if(!fdb->manager_type)   // one more non-manager
	{  ++fdblist.n_no_managers;  }
	else if(fdb->manager_type==MT_Timeout)
	{  timeout_manager=fdb;  }
	pdebug("registered FDBase %p (manager=%d)\n",fdb,fdb->manager);
	return(0);
}


int FDManager::SetManager(FDBase *fdb,ManagerType mtype)
{
	FDBNode *n=fdblist.FindNode(fdb);
	if(!n)  return(-1);
	if(!fdb->manager_type)  --fdblist.n_no_managers;
	fdb->manager_type=mtype;
	if(!fdb->manager_type)  ++fdblist.n_no_managers;
	else if(fdb->manager_type==MT_Timeout)
	{  timeout_manager=fdb;  }
	return(0);
}


// flag: 1 -> just unregister
//       2 -> unregister and destruct 
//       3 -> unregister, destruct and free(fdb)
void FDManager::Unregister(FDBase *fdb,int flag)
{
	if(!fdb)  return;
	pdebug("unregister FDBase %p (deleted=%d, flag=%d)\n",
		fdb,fdb->deleted,flag);
	if(fdb->deleted)  // Hey, he already gets deleted, can't he wait or what?
	{  return;  }
	
	// So, if he only wants to unregister he's probably allocated on the 
	// stack and the call came from the FDBase desctructor. So we may not 
	// access his memory after we returned from this function. 
	
	FDBNode *i=fdblist.FindNode(fdb);
	pdebug("Node for fdb=%p = %p\n",fdb,i);
	if(!i)
	{  return;  }
	
	if(!fdb->manager_type)   // one non-manager unregistered 
	{  --fdblist.n_no_managers;  }
	else if(fdb->manager_type==MT_Timeout)
	{  timeout_manager=NULL;  }
	
	// Move i from the list of FDBases to the list of dead FDBases: 
	// If iterator_next is set, it may be necessary to update it. 
	if(iterator_next==i)     {  iterator_next=   i->next;  }
	if(iterator_next_ds==i)  {  iterator_next_ds=i->next;  }
	fdblist.DeadNode(i);
	
	// Set the delete flag: 
	i->fdb->deleted=flag;
	
	// DestructionDone() checks if the FDs change; timer change gets 
	// reported by FDBase via TimeoutChange(). 
	
	if(flag==1)
	{  i->fdb=NULL;  }  // May not access any more. 
}


// Called by FDBase to keep counters up to date. 
// NEVER CALL DIRECTLY. 
void FDManager::DestructionDone(FDBase *,int nfds,int npollfds)
{
	// This is no longer needed as ~FDBase calls TimeoutChange() if needed. 
	//if(ntimers)
	//{  ++timerlist_change_serial;  }
	if(nfds)
	{
		fd_nnodes-=nfds;
		pollnodes-=npollfds;
		++fdlist_change_serial;
	}
}


void FDManager::__TidyUp()
{
	// fdblist.fdead!=NULL already checked. 
	
	while(fdblist.fdead)   // only entries in dead list
	{
		FDBNode *tmp=fdblist.fdead;
		
		// Note: if Unregister() was called with flag=1 
		// (just unregister), then we may not access the 
		// data in tmp->fdb. In this case, tmp->fdb is NULL. 
		if(!tmp->fdb)
		{  fdblist.DeleteNode(tmp,/*deadlist=*/1);  }
		else
		{
			#if TESTING
			if(!tmp->fdb->deleted)
			{  fprintf(stderr,"BUG!! non-deleted FDBase in dead list.\n");  }
			#endif
			
			if(tmp->fdb->deleted==2)
			{
				// The destructor will call Unregister() but that does 
				// not matter as unregister will do nothing because 
				// fdb->deleted!=0. 
				tmp->fdb->~FDBase();  // NOT FDBase::~FDBase(). 
			}
			else if(tmp->fdb->deleted==3)
			{  delete tmp->fdb;  }   // DO NOT SET tmp->fdb=NULL. 
			
			fdblist.DeleteNode(tmp,/*deadlist=*/1);
		}
	}
	
	#if TESTING
	if(fdblist.n_no_managers<0)
	{  fprintf(stderr,"***BUG! n_no_managers=%d ***\n",
		fdblist.n_no_managers);  }
	#endif
}


void FDManager::__UpdateFDArray()
{
	static const int fd_thresh=48;
	
	// fdlist_change_serial!=0 already checked. 
	npfds=pollnodes;  // number of nodes with events!=0
	if(npfds)
	{
		if(npfds>pfd_dim ||  // allocate more mem (NEVER USE >=pfd_dim)
		   pfd_dim>(3*fd_thresh)/2+npfds)   // allocate less mem 
		{
			// Note: pfd[-1] is alloc base and pipe fd internally used by 
			//       FDManager against poll/signal race. 
			if(pfd!=pipe_pfd+1)
			{
				LFree(pfd-1);
				pfd=pipe_pfd+1;
			}
			
			retry:;
			// We allocate some mem ahead... 
			pfd_dim=fd_thresh*(((npfds-1)/fd_thresh)+1);
			if(pfd_dim<npfds+fd_thresh/2)
			{  pfd_dim+=fd_thresh/2;  }
			
			#if TESTING
			if(npfds>pfd_dim || pfd_dim>(3*fd_thresh)/2+npfds)
			{  fprintf(stderr,"FD Re-Alloc is buggy. %u %u\n",npfds,pfd_dim);  }
			#endif
			
			pfd=(pollfd*)LMalloc(sizeof(pollfd)*pfd_dim + 1);
			if(!pfd)
			{
				// Damn! Re-allocation of pollfd array failed. 
				// Memory is REALLY LOW if we cannot get these few bytes. 
				// NOTE: WE CAN GIVE UP COMPLETELY HERE IF THAT IS DUE TO 
				//       LMalloc() LIMIT!
				#warning FIXME!!!
				pfd_dim=0;
				// OK, what can we do?! Wait until memory is back again. 
				usleep(25000);  // 25 msec delay (do not comsume lots of CPU)
				goto retry;
			}
			
			// Note: pfd[-1] is alloc base and pipe fd internally used by 
			//       FDManager against poll/signal race. 
			*(pfd++)=*pipe_pfd;   // implicite copy
		}
	}
	else if(pfd_dim>=fd_thresh)
	{
		pfd_dim=0;
		if(pfd!=pipe_pfd+1)
		{
			LFree(pfd-1);
			pfd=pipe_pfd+1;
		}
	}
	
	int idx=0;
	pollfd *p=pfd;
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next)
	{
		FDBase *fdb=fb->fdb;
		#if TESTING
		if(!fdb || fdb->deleted)
		{
			fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
				fdb,fdb->deleted,__LINE__);
			continue;
		}
		#endif
		for(FDNode *i=fdb->fds; i; i=i->next)
		{
			if(i->idx==-2)
			{
				#if TESTING
				fprintf(stderr,"OOPS: idx==-2 while fd list rebuild\n");
				#endif
				continue;
			}
			if(i->events)
			{
				#if TESTING
				if(idx>=npfds)
				{
					fprintf(stderr,"OOPS: GREAT internal error: pollnodes=%d too small.\n",
						pollnodes);
					exit(1);
				}
				#endif
				p->fd=i->fd;
				p->events=i->events;
				i->idx=idx;
				++idx;  ++p;
			}
			else
			{  i->idx=-1;  }
		}
	}
	#if TESTING
	if(idx!=pollnodes)
	{  fprintf(stderr,"internal error: pollnodes=%d != idx=%d\n",
		pollnodes,idx);  }
	npfds=idx;
	#endif
	
	fdlist_change_serial=0;
}


// Ugly large macro inserted at apropriate position (in MainLoop()) to 
// check if the fd array and list are constatent. 
#if TESTING_CHECK
#define TestingCheckFDStuff \
{ \
	if(fdlist_change_serial) \
	{  fprintf(stderr,"OOPS: fdlist_change_serial=%d in TestingCheckFDStuff\n", \
		fdlist_change_serial);  }  \
	char flag[pollnodes]; \
	memset(flag,0,pollnodes); \
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next) \
	{ \
		FDBase *fdb=fb->fdb; \
		if(!fdb || fdb->deleted) \
		{  fprintf(stderr,"BUG!![tst] fdb=%p, deleted=%d in FDBase list (%d)\n", \
			fdb,fdb->deleted,__LINE__);  continue;  } \
		for(FDNode *i=fdb->fds; i; i=i->next) \
		{ \
			if(i->fd<0) \
			{  fprintf(stderr,"BUG!![tst] illegal fd %d\n",i->fd);  continue;  } \
			if(i->idx==-2) \
			{  fprintf(stderr,"BUG!![tst] fd=%d has idx=-2.\n",i->fd);  continue;  } \
			if((i->events && i->idx<0) || \
			   (!i->events && i->idx>=0)) \
			{  fprintf(stderr,"BUG!![tst] fd=%d has events=%d and idx=%d\n", \
				i->fd,i->events,i->idx);  ++fdlist_change_serial;  continue;  } \
			if(i->idx>=0) \
			{ \
				if(i->idx>=pollnodes) \
				{  fprintf(stderr,"BUG!![tst] fd=%d has idx=%d out of range (%d)\n", \
					i->fd,i->idx,pollnodes);  continue;  } \
				struct pollfd *p=&pfd[i->idx]; \
				if(p->fd!=i->fd || p->events!=i->events) \
				{  fprintf(stderr,"BUG!![tst] data inconsistent: " \
					"array=%d,%d; list=%d,%d\n", \
					p->fd,p->events,i->fd,i->events);  continue;  } \
				++flag[i->idx]; \
			} \
		} \
	} \
	for(int i=0; i<pollnodes; i++) \
	{ \
		if(int(flag[i])!=1) \
		{  fprintf(stderr,"BUG!![tst] pfd array element[%d] (fd=%d) %s!!", \
			i,pfd[i].fd,(flag[i] ? "USED MORE TNAN ONCE" : "NOT USED"));  } \
	} \
}
#endif


static sigset_t prev_sigset;
static sigset_t full_sigset;
static inline void _fd_block_signals()
{  sigprocmask(SIG_SETMASK,&full_sigset,&prev_sigset);  }
static inline void _fd_unblock_signals()
{  sigprocmask(SIG_SETMASK,&prev_sigset,NULL);  }

void FDManager::_ZapSignals()
{
	// Free all pending signals: 
	_fd_block_signals();
	while(first_pending_sig)
	{
		SigNode *sn=first_pending_sig;
		first_pending_sig=first_pending_sig->next;
		_FreeSigNode(sn);
	}
	last_pending_sig=0;
	pending_signum=0;
	_fd_unblock_signals();
}

// please use _DeliverPendingSignals(). 
void FDManager::__DeliverPendingSignals(int tidy_up)
{
	// Pending signals get delivered in the same order they were received. 
	// Make sure this function is never called recursively. This should 
	// never be needed. 
	static int delivering=0;
	if(delivering)
	{
		#if TESTING
		fprintf(stderr,"OOPS: *** Blocked recursive call of "
			"DeliverPendingSignals()\a\n");
		#endif
		return;
	}
	delivering=1;
	
	int dequeued=0;
	for(;;)
	{
		// Make sure there are still some nodes in the free signal queue left:
		// (But only _some_ as they will be free'd now.)
		_EnsureFreeSigNodes(min_free_signodes/4);
		
		// Reset the queue. I handle all signals in the queue but 
		// reset the queue so that I do not run into trouble with 
		// signals arriving while dequeuing the last node. 
		// Therefore, only this list clearing has to be locked. 
		_fd_block_signals();
		SigNode *first=first_pending_sig;
		first_pending_sig=NULL;
		last_pending_sig=NULL;
		// pending_signum is set to 0 when the function returns. 
		// Otherwise, the sig limit may get exceeded as half of the 
		// signals is in the *first queue and other half in 
		// *first_pending_sig. 
		pending_signum-=dequeued;
		_fd_unblock_signals();
		
		if(!first)  break;
		do  // cycle through queued pending signals
		{
			SigNode *sn=first;
			first=first->next;
			// Signal must already be dequeued when it is delivered 
			// so that SigPending() does not count it as pending. 
			_DeliverSignal(sn);
			// "Free" signal node: 
			_FreeSigNode(sn);
			++dequeued;
		}
		while(first);
	}
	
	pending_signum=0;
	delivering=0;
	
	if(tidy_up)
	{  _TidyUp();  }   // is inline...
}


void FDManager::_DeliverSignal(SigNode *sn)
{
	// THIS FUNCTION MAY NOT CALL _TidyUp(). 
	
	// Deliver the signal...
	FDBase::SigInfo *sinfo=&sn->info;
	void (FDBase::*handlerptr)(void) = NULL;
	switch(sinfo->info.si_signo)
	{
		case SIGHUP:   handlerptr=&FDBase::sighup;    break;
		case SIGINT:   handlerptr=&FDBase::sigint;    break;
		case SIGTERM:  handlerptr=&FDBase::sigterm;   break;
	}
	
	// NOTE: signotify() can call DeleteMe() of itself and other 
	//       FDBase-derived classes, too. Their nodes then get 
	//       moved to the dead list. That's why we need the 
	//       iterator_next trick. 
	//       As _DeliverSignal() may get called from within other 
	//       delivering functions, this function needs its own 
	//       value iterator_next_ds. 
	#if TESTING
	if(iterator_next_ds)
	{  fprintf(stderr,"OOPS: BUG!! iterator_next_ds!=NULL (A)\n");  abort();  }
	#endif
	for(FDBNode *i=fdblist.first; i; i=iterator_next_ds)
	{
		iterator_next_ds=i->next;
		FDBase *fdb=i->fdb;
		#if TESTING
		if(!fdb || fdb->deleted)
		{  fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
			i->fdb,i->fdb->deleted,__LINE__);  continue;  }
		#endif
		// (virtual function call)
		if(fdb->signotify(sinfo))
		{
			// Also call old-style function if available: 
			if(handlerptr)
			{  (fdb->*handlerptr)();  }
		}
	}
}


// inline as called only from one pos. 
inline void FDManager::_DeliverFDNotify(const HTime *fdtime)
{
	// Make sure to deal with arrived signals: 
	_DeliverPendingSignals();  // inline check (does _TidyUp())
	//_TidyUp();  // inline check 
	// NOTE: fdnotify() can call DeleteMe() of itself and other 
	//       FDBase-derived classes, too. Their nodes then get 
	//       moved to the dead list. That's why we need the 
	//       iterator_next trick. 
	#if TESTING
	if(iterator_next)
	{  fprintf(stderr,"OOPS: BUG!! iterator_next!=NULL (B)\n");  abort();  }
	#endif
	HTime fdtime_cp;
	for(FDBNode *fb=fdblist.first; fb; fb=iterator_next)
	{
		iterator_next=fb->next;
		FDBase *fdb=fb->fdb;
		#if TESTING
		if(!fdb || fdb->deleted)
		{
			fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
				fdb,fdb->deleted,__LINE__);
			continue;
		}
		if(!fdb->fdslock)  // already locked
		{  fprintf(stderr,"FD: *** OOPS: fds already locked! (FDN)\n");  }
		#endif
		
		// Make sure to deal with arrived signals: 
		// It is important to set tidy_up to 0 here because: 
		// An FDBase might call DeleteMe() during signotify() 
		// called by _DeliverPendingSignals(). Then, the node 
		// will be moved to dead list and we may still access 
		// it here. However, if _TidyUp() was called, the dead 
		// node gets freed and accessing i->deleted, i->LockFDs() 
		// etc. will mess up memory (crash risk). 
		_DeliverPendingSignals(/*tidy_up=*/0);  // inline check 
		// Check if some FDBases quit during _DeliverPendingSignals(): 
		if(!fb->fdb)      continue;   // MUST use fb->fdb, not fdb.
		if(fdb->deleted)  continue;
		
		fdb->LockFDs();
		for(FDNode *i=fdb->fds; i; i=i->next)
		{
			// newly added or empty entries have idx=-1;
			// entries to remove have idx=-2. 
			if(i->idx<0)  continue;
			
			pollfd *p=&pfd[i->idx];
			#if TESTING
			if(p->fd!=i->fd || p->events!=i->events)
			{  fprintf(stderr,"internal error: fd=%d,%d; events=%d,%d\n",
				p->fd,i->fd,p->events,i->events);  exit(1);  }
			#endif
			if(p->revents)
			{
				// Make sure we can buffer signals: 
				_EnsureFreeSigNodes();
				
				// already checked: we will not send fdnotify() 
				// on FDBases which will get deleted. 
				fdtime_cp=*fdtime;
				FDInfo fdi;
				fdi.pollid=(PollID)p;
				fdi.fd=p->fd;
				fdi.events=p->events;
				fdi.revents=p->revents;
				fdi.current=&fdtime_cp;  // (copy)
				fdi.dptr=i->dptr;
				// (virtual function call)
				fdb->fdnotify(&fdi);
			}
			
			// There is an invalid fd; we unpoll that if the 
			// class did not do that. 
			if(p->revents & POLLNVAL)
			{
				#if TESTING
				fprintf(stderr,"Oops: FDManager: unpolling illegal fd=%d\n",p->fd);
				#endif
				fdb->CloseFD(p->fd);  // be sure...
			}
		}
		fdb->UnlockFDs();
	}
	_TidyUp();  // inline check 
}


// inline as called only from one pos. 
inline void FDManager::_DeliverTimers(long elapsed,HTime *currtv)
{
	// Make sure to deal with arrived signals: 
	_DeliverPendingSignals();  // inline check (does _TityUp())
	//_TidyUp();  // inline check 
	HTime _currv;
	TimerInfo tinfo;
	// NOTE: timernotify() can call DeleteMe() of itself and other 
	//       FDBase-derived classes, too. Their nodes then get 
	//       moved to the dead list. That's why we need the 
	//       iterator_next trick. 
	#if TESTING
	if(iterator_next)
	{  fprintf(stderr,"OOPS: BUG!! iterator_next!=NULL (C)\n");  abort();  }
	#endif
	for(FDBNode *fb=fdblist.first; fb; fb=iterator_next)
	{
		iterator_next=fb->next;
		FDBase *fdb=fb->fdb;
		#if TESTING
		if(!fdb || fdb->deleted)
		{
			fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
				fdb,fdb->deleted,__LINE__);
			continue;
		}
		#endif
		
		// Make sure to deal with arrived signals: 
		// It is important to set tidy_up=0 here. Read the 
		// comment in _DeliverFDNotify() do know why. 
		_DeliverPendingSignals(/*tidy_up=*/0);  // inline check 
		// Check if some FDBases quit during _DeliverPendingSignals():
		if(!fb->fdb)      continue;   // MUST use fb->fdb, not fdb. 
		if(fdb->deleted)  continue;
		
		fdb->LockTimers();
		for(TimerNode *i=fdb->timers; i; i=i->next)
		{
			if(i->msec_val<0L)  continue;  // disabled/deleted timer 
			
			// This will not change the FDBase::sh_timer: 
			i->msec_left-=elapsed;
			if(i->msec_left<=0L)
			{
				// Make sure we can buffer signals: 
				_EnsureFreeSigNodes();
				
				fdb->_DoResetTimer(i);  // reset timer
				// _DoResetTimer() will increment timeout_change. 
				_currv=*currtv;  // implicit copy
				tinfo.tid=(TimerID)i;
				tinfo.current=&_currv;
				tinfo.dptr=i->dptr;
				// (virtual function call)
				fdb->timernotify(&tinfo);
			}
		}
		fdb->UnlockTimers();  // finally remove those "deleted".
	}
	_TidyUp();  // inline check 
}


int FDManager::MainLoop()
{
	quitval=-1;
	HTime timertv;
	timertv.SetCurr();
	for(;;)
	{
		long timeout0=_GetTimeout(&timertv);
		timeout_change=0;
		HTime starttv,currtv;
		
		// Check file descriptors: 
		starttv=timertv;  //implicit copy
		for(long timeout=timeout0;;)
		{
			// Make sure signal detection pipe is empty: 
			if(sig_pipe_bytes>0)
			{  _ClearSigPipe();  }
			
			// Tell signal catcher to write to pipe when signals are caught: 
			// Order: sig_pipe_bytes=0 -> _DeliverPendingSignals() -> poll(). 
			sig_pipe_bytes=0;
			
			// Deliver signals: 
			_DeliverPendingSignals();  // inline check 
			
			// Cleanup stuff: 
			_TidyUp();
			if(!fdblist.n_no_managers ||  // no non-managers left
			   quitval>=0)
			{
				// We're about to quit. 
				return((quitval>=0) ? quitval : 0);
			}
			// Prepare fd array for poll(): 
			// MAY NOT CALL ANY OF THE DELIVERY FUNCTIONS BETWEEN
			// _UpdateFDArray() and poll(). 
			_UpdateFDArray();   // inline; updates if fdlist_change_serial
			
			#if TESTING_CHECK
			// Check if fd array is actually consistent with fd list: 
			TestingCheckFDStuff
			#endif
			#if TESTING
			if(pfd[-1].fd!=sig_pipe_fd_r ||
			   pfd[-1].events!=POLLIN)
			{  fprintf(stderr,"FD: BUG!! pfd[-1] corrupt: %d,%d\n",
				pfd[-1].fd,pfd[-1].events);  }
			#endif

			// Is this loop needed?? Not for linux (see fs/select.c). 
			// Note: pfd[-1] is alloc base and internally used. 
			//for(pollfd *p=pfd-1,*pend=&pfd[npfds]; p<pend; p++)
			//{  p->revents=0;  }
			
			int ret=0;
			// Note: pfd[-1] is alloc base and internally used against 
			//       poll & signal race condition. 
			if(npfds || timeout)  // timeout!=0 (<0 or >0)
			{
			//HTime t0(HTime::Curr);
				// ACTUALLY CALL poll()...
				ret=poll(pfd-1,npfds+1,timeout);    // timeout=-1 also valid. 
			//fprintf(stderr,">>%d (t=%d, n=%d, rv=%d, rev=%d, %d)<<    ",
			//	t0.Elapsed(HTime::msec),timeout,npfds,ret,pfd[-1].revents,sig_pipe_bytes);
				// Set time when poll returns so that this time can be passed 
				// to the calls to fdnotify(). 
				if(ret>0)
				{  currtv.SetCurr();  }
				
				int spb=sig_pipe_bytes;
				// Tell signal catcher not to write to pipe when signals are 
				// caught: 
				sig_pipe_bytes=-1;
				// Check if we got POLLIN from signal detection pipe: 
				if(ret>0 && (pfd[-1].revents & (POLLIN | POLLERR | POLLHUP)))
				{
					++spb;  // so that _ClearSigPipe() will be done below
					--ret;  // decrease ret val for all the FDBase fd's
				}
				
				// Make signal detection pipe empty if needed. 
				if(spb>0)
				{  _ClearSigPipe();  }
			}
			
			if(ret<0)   // poll returns error. Hmm... let's look closer...
			{
				switch(errno)
				{
					case EINTR:  // OK, signal just arrived. 
						// _DeliverPendingSignals();
						break;
					case EAGAIN:  // will poll() ever return this?! (never mind; ignore.)
						break;
					case ENOMEM:  // oops... bad, bad.
						usleep(50000);   // sleep 50 msec; better than nothing. 
						break;           // (The CPU will like that, I guess...)
					default:
						fprintf(stderr,"%s: poll: %s\n",prg_name,strerror(errno));
						return(-1);
				}
			}
			else if(ret>0)
			{  _DeliverFDNotify(&currtv);  }  // calls _DeliverPendingSignals() 
			
			_DeliverPendingSignals();  // inline check 
			
			if(!timeout || !ret || 
			   timeout_change || 
			   fdlist_change_serial )
			{  break;  }
			
			currtv.SetCurr();   // current time
			long elapsed=starttv.MsecElapsed(&currtv);
			
			if(elapsed>=timeout0)
			{  break;  }
			
			timeout=timeout0-elapsed;
		}
		
		// Check timers: 
		{
			currtv.SetCurr();  // important
			long elapsed=timertv.MsecElapsed(&currtv);  // time since last timer update. 
			//fprintf(stderr,"timer_elapsed=%ld\n",elapsed);
			
			timertv=currtv;  // implicit copy
			
			if(timeout0>=0)  // there are timers
			{
				last_timeout-=elapsed;
				_DeliverTimers(elapsed,&currtv);  // calls _DeliverPendingSignals() 
				#if TESTING
				// NOTE: * last_timeout-=elapsed; above (!!)
				//       * last_timeout=-1 is ``infinity'' which is 
				//         35 minutes in case of usleep() [no fds] 
				if(last_timeout<-1 && !timeout_change)
				{
					fprintf(stderr,"BUG: last_timeout<-1 (%ld) but no "
						"timer elapsed\n",last_timeout);  
					++timeout_change;
				}
				#endif
			}
		}
		
		_DeliverPendingSignals();  // inline check 
	}
	return(0);  // never reached. 
}


int FDManager::noop(int x)
{  return(x);  }   // just returns x ; DO NO INLINE. 


static void _fd_sigaction_failed()
{
	fprintf(stderr,"%s: sigaction failed: %s\n",
		prg_name,strerror(errno));
	//abort();
}


FDManager::FDManager(int *failflag=NULL) : 
	fdblist()
{
	int failed=0;
	
	timeout_manager=NULL;
	
	timeout_change=1;  // yes...
	fdlist_change_serial=0;
	
	iterator_next=NULL;
	iterator_next_ds=NULL;
	
	//sigemptyset(&pending_sigs);
	first_pending_sig=NULL;
	last_pending_sig=NULL;
	pending_signum=0;
	first_free_signode=NULL;
	free_signodes=0;
	max_free_signodes=64;
	min_free_signodes=32;
	extra_signodes=0;
	if(_EnsureFreeSigNodes())
	{  ++failed;  }
	
	sig_pipe_fd_r=-1;
	sig_pipe_fd_w=-1;
	int fdx[2];
	if(pipe(fdx)<0)
	{  ++failed;  }
	else
	{
		sig_pipe_fd_r=fdx[0];
		sig_pipe_fd_w=fdx[1];
	}
	if(SetNonblocking(sig_pipe_fd_r))
	{  ++failed;  }
	sig_pipe_bytes=-1;  // `do not write to pipe on signal'
	
	pipe_pfd=(pollfd*)LMalloc(sizeof(pollfd));
	if(!pipe_pfd)
	{  pfd=NULL;  ++failed;  }
	else
	{
		pipe_pfd->fd=sig_pipe_fd_r;
		pipe_pfd->events=POLLIN;
		pipe_pfd->revents=0;   // to be sure...
		pfd=pipe_pfd+1;
	}
	pfd_dim=0;  // correct...
	npfds=0;
	
	fd_nnodes=0;
	pollnodes=0;
	
	last_timeout=0;
	
	//quitval=-1;  <-- set in MainLoop(). 
	
	// Set up signal handlers: 
	sigfillset(&full_sigset);
	sigemptyset(&prev_sigset);
	
	struct sigaction sact;
	memset(&sact,0,sizeof(sact));
	#ifdef HLIB_CRIPPLED_SIGINFO_T
	sact.sa_handler=&_fd_sig_handler_simple;
	sact.sa_flags=0;   // do NOT add SA_RESTART here.
	#else
	sact.sa_sigaction=&_fd_sig_handler_sigaction;
	sact.sa_flags=SA_SIGINFO;   // do NOT add SA_RESTART here.
	#endif
	sigfillset(&sact.sa_mask);  // block all signals during signal handler execution
	//sact.sa_restorer=NULL;
	for(int i=0; _fd_signals[i]!=-22000; i++)
	{
		if(sigaction(_fd_signals[i],&sact,NULL))
		{  _fd_sigaction_failed();  ++failed;  }
	}
	
	if(failed)
	{
		if(failflag)
		{  *failflag-=failed;  return;  }
		ConstructorFailedExit("FD");
	}
	
	/*--- DO NOT USE >int failed< BELOW HERE. ---*/
	
	// Init global manager: 
	#if TESTING
	if(manager)
	{  fprintf(stderr,"%s: more than one FDManager.\n",prg_name);  abort();  }
	#endif
	
	manager=this;
}


FDManager::~FDManager()
{
//fprintf(stderr,"FDManager.....1");
	// Free all pending signals: 
	_ZapSignals();
	
//fprintf(stderr,"2.....");
	// delete all our FDBases
	_TidyUp();
//fprintf(stderr,"3.....");
	for(FDBNode *i=fdblist.first; i; i=i->next)
	{  i->fdb->DeleteMe();  }
//fprintf(stderr,"4.....");
	_TidyUp();
//fprintf(stderr,"5.....");
	// When deleting, the fdbs unregister themselves, so 
	// no node should be left. 
	
	#if TESTING
	if(fdblist.first)  // well, not all FDBase classes unregistered!!
	{  fprintf(stderr,"NOT ALL FDBase classes unregistered.\n");  }
	if(fdblist.fdead)
	{  fprintf(stderr,"~FDManager: DEAD LIST NOT EMPTY.\n");  }
	#endif
	
	// pipe_pfd=NULL can only be the case if the constructor failed. 
	// In that case, pfd is not allocated. 
	if(pipe_pfd)
	{
		if(pfd!=pipe_pfd+1)  // correct. 
		{
			LFree(pfd-1);  // Note: pfd[-1] is alloc base and internally used. 
			pfd=NULL;
		}
		LFree(pipe_pfd);
	}
	
//fprintf(stderr,"6.....");
	
	#if TESTING
	if(fd_nnodes || pollnodes)
	{  fprintf(stderr,"internal warning: ~FDManager: "
		"fd_nnodes=%d, pollnodes=%d ",fd_nnodes,pollnodes);  }
	#endif
	
	// Cleanup global manager: 
	struct sigaction sact;
	memset(&sact,0,sizeof(sact));
	sact.sa_handler=SIG_DFL;  // default handler
	sigemptyset(&sact.sa_mask);
	sact.sa_flags=0;
	//sact.sa_restorer=NULL;
	for(int i=0; _fd_signals[i]!=-22000; i++)
	{
		if(sigaction(_fd_signals[i],&sact,NULL))
		{  _fd_sigaction_failed();  }
	}
	
//fprintf(stderr,"7.....");
	// Free signals again as they might have arrived before 
	// sigaction() was called. 
	_ZapSignals();
	
//fprintf(stderr,"8.....");
	// Actually _free_ all the signal nodes: 
	while(first_free_signode)
	{
		SigNode *sn=first_free_signode;
		first_free_signode=first_free_signode->next;
		LFree(sn);
		--free_signodes;
	}
	
	#if TESTING
	if(free_signodes)
	{  fprintf(stderr,"FD: ** BUG!! free_signodes=%d !=0 in ~FDManager\n",
		free_signodes);  }
	if(extra_signodes)
	{  fprintf(stderr,"FD: BUG?! extra_signodes=%d !=0 in FDManager\n",
		extra_signodes);  }
	#endif
	
	if(sig_pipe_fd_r>=0)  fd_close(sig_pipe_fd_r);  sig_pipe_fd_r=-1;
	if(sig_pipe_fd_w>=0)  fd_close(sig_pipe_fd_w);  sig_pipe_fd_w=-1;
	
	#if TESTING
	if(timeout_manager)
	{  fprintf(stderr,"FD: *** OOPS: timeoutmanager still there.\n");  }
	#endif
	
//fprintf(stderr,"E\n");
	manager=NULL;
}


/*********************** FDManager::FDBList **************************/


void FDManager::FDBList::_DequeueNode(FDBNode *n,int deadlist)
{
	FDBNode **fst = deadlist ? (&fdead) : (&first);
	if(n==(*fst))
	{
		(*fst)=(*fst)->next;
		if(*fst)
		{  (*fst)->prev=NULL;  }
	}
	else
	{
		if(n->prev)  {  n->prev->next=n->next;  }
		if(n->next)  {  n->next->prev=n->prev;  }
	}
}


void FDManager::FDBList::DeleteNode(FDBNode *n,int deadlist)
{
	if(!n)
	{  return;  }
	_DequeueNode(n,deadlist);
	pdebug("FD: Deleting node %p.\n",n);
	LFree(n);
}


// Move node from normal list (*first) to dead list (*fdead). 
void FDManager::FDBList::DeadNode(FDBNode *n)
{
	if(!n)
	{  return;  }
	// First, dequeue it in the normal list: 
	_DequeueNode(n,/*deadlist=*/0);
	// Then, add the node to dead list: 
	n->prev=NULL;
	n->next=fdead;
	if(fdead)
	{  fdead->prev=n;  }
	fdead=n;
}


int FDManager::FDBList::Add(FDBase *fdb)
{
	FDManager::FDBNode *n=(FDManager::FDBNode *)LMalloc(sizeof(FDManager::FDBNode));
//fprintf(stderr,"$$$$$>fdb=%p; n=%p<$$$$$\n",fdb,n);
	if(!n)
	{  return(-1);  }
	n->next=first;
	n->prev=NULL;
	n->fdb=fdb;
	if(first)
	{  first->prev=n;  }
	first=n;
	return(0);
}


FDManager::FDBNode *FDManager::FDBList::FindNode(FDBase *fdb)
{
	//if(fdb->fdnode)
	//	if(fdb->fdnode->fdb==fdb)
	//		return(fdb->fdnode);
	
	for(FDBNode *i=first; i; i=i->next)
	{
		if(i->fdb==fdb)
		{  return(i);  }
	}
	return(NULL);
}


void FDManager::FDBList::Clear()
{
	// (should never be necessary)
	while(first)
	{
		#if TESTING
		fprintf(stderr,"FDBList::Clear(): non-unregistered FDBase.\n");
		#endif
		FDManager::FDBNode *n=first;
		first=first->next;
		LFree(n);
	}
	while(fdead)
	{
		#if TESTING
		fprintf(stderr,"FDBList::Clear(): Entry in dead list.\n");
		#endif
		FDManager::FDBNode *n=fdead;
		fdead=fdead->next;
		LFree(n);
	}
}


/******************** Timer Stuff ************************/

// n may be already in the queue; 
// timers with interval 0 are not aligned. 
void FDManager::AlignTimer(FDManager::TimerNode *n,int align)
{
	if(!align || n->msec_val<=0L || 
	   n->msec_val>=0x7fffffff/100)   // otherwise long overflow in percent calculation. 
	{  return;  }   // no alignment allowed/possible
	
	long percent = align & FDAT_PercentMask;
	if(percent<0)
	{  return;  }
	if(percent>100)   // well... we accept 100 due to integer arithmetics...
	{  percent=100;  }
	
	// FIXME: Can speed up? (Be careful: good alignment is worth much) 
	
	#if TESTING
	FDManager::TimerNode *alg_long=NULL,*alg_short=NULL,*alg_used=NULL;
	#define timASSIGN_LONG   alg_long=i;
	#define timASSIGN_SHORT  alg_short=i;
	#define timUSED_LONG   alg_used=alg_long;
	#define timUSED_SHORT  alg_used=alg_short;
	#else
	#define timASSIGN_LONG
	#define timASSIGN_SHORT
	#define timUSED_LONG
	#define timUSED_SHORT
	#endif
	
	/* New algorithm: not larger but better. */
	int a2short = (align & FDAT_AlignToShorter);
	long shorter,longer;
	long minlong=0,maxshort=0;  // no need to initialize; only against compiler warning
	int isfirst=1;
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next)
	{
		FDBase *fdb=fb->fdb;
		#if TESTING
		if(!fdb || fdb->deleted)
		{
			fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
				fdb,fdb->deleted,__LINE__);
			continue;
		}
		#endif
		for(FDManager::TimerNode *i=fdb->timers; i; i=i->next)
		{
			if(i!=n && i->msec_val>0 && 
			   (!a2short || i->msec_left<=n->msec_val) )
			{
				if(!(i->msec_val % n->msec_val))  // n->msec_val <= i->msec_val
				{
					shorter = i->msec_left % n->msec_val;
					longer = shorter ? (shorter + n->msec_val) : 0;
					goto checkminmax;
				}
				else if(!(n->msec_val % i->msec_val))  // n->msec_val >= i->msec_val
				{
					longer = n->msec_val + i->msec_left;
					shorter = longer - i->msec_val;
					checkminmax:;
					// keep track of minimal longer and maximal shorter values. 
					if(isfirst)
					{
						minlong=longer;     timASSIGN_LONG
						maxshort=shorter;   timASSIGN_SHORT
						isfirst=0;
					}
					else
					{
						if(minlong>longer)
						{  minlong=longer;    timASSIGN_LONG   }
						if(maxshort<shorter)
						{  maxshort=shorter;  timASSIGN_SHORT  }
					}
				}
			}
		}
	}
	
	// If alignment possible and necessary: 
	if(!isfirst && minlong)  // !minlong -> no alignment necessary. 
	{                        // isfirst -> no alignment possible
		// align...
		if((align & FDAT_FirstEarlier) && 
		   (align & FDAT_FirstLater)   )
		{
			// use minlong or minshort whatever is nearer to desired value: 
			long longpercent = ((minlong*100)/n->msec_val)-100;
			long shortpercent = 100-((maxshort*100)/n->msec_val);
			if(longpercent>=shortpercent)
			{  goto timshort_jmp;  }
			else 
			{  goto timlong_jmp;  }
		}
		else if(align & FDAT_FirstLater)
		{
			timlong_jmp:;
			// use minlong: 
			if(minlong<=((n->msec_val*(100+percent))/100))
			{
				n->msec_left=minlong;
				timUSED_LONG
			}
		}
		else if(align & FDAT_FirstEarlier)
		{
			timshort_jmp:;
			// use maxshort: 
			if(maxshort>=((n->msec_val*(100-percent))/100))
			{
				n->msec_left=maxshort;
				timUSED_SHORT
			}
		}
	}
	
	#if TESTING
	#undef timASSIGN_LONG
	#undef timASSIGN_SHORT
	fprintf(stderr,"aligned timer: align=%c%c%ld, val=%ld, left=%ld",
		((align & FDAT_FirstLater) && (align & FDAT_FirstEarlier)) ? '±' : 
			(align & FDAT_FirstLater) ? '+' : '-',
		a2short ? 's' : ' ',
		percent,
		n->msec_val,n->msec_left);
	if(alg_used)
	{  fprintf(stderr,"  [aligned to: val=%ld, left=%ld]\n",
		alg_used->msec_val,alg_used->msec_left);  }
	else
	{  fprintf(stderr,"  [NOT aligned; isfirst=%d]\n",isfirst);  }
	#endif
}


FDManager::TimerID FDManager::InstallTimer(FDBase *fdb,long msec,int align,const void *dptr)
{
	if(!fdb)
	{  return(NULL);  }
	if(msec<-1L)   // -1 -> disabled. 
	{  return(NULL);  }
	
	FDManager::TimerNode *n=fdb->AllocTimerNode(msec,dptr);
	if(!n)
	{  return(NULL);  }
	fdb->AddTimerNode(n);  // (at beginning)
	AlignTimer(n,align);
	fdb->_MsecLeftChanged(n,-1);  // ...will increment timeout_change if needed
	
	return((TimerID)n);
}


int FDManager::UpdateTimer(FDBase *fdb,TimerID tid,long msec,int align)
{
	if(!fdb)
	{  return(0);  }
	if(!tid)   // -1 -> disabled. 
	{  return(-2);  }
	
	#if TESTING_CHECK
	for(FDManager::TimerNode *i=fdb->timers; i; i=i->next)
	{  if((TimerID)i==tid)  goto found;  }
	fprintf(stderr,"**BUG in application: "
		"attempt to update non-existant timer %p (%ld,%d)\n",tid,msec,align);
	return(-2);
	found:;
	#endif
	
	FDManager::TimerNode *i=(FDManager::TimerNode *)tid;
	
	if(i->msec_val<0 && msec<0)   // (deleted or) disabled timer
	{  return(0);  }   // nothing to do disabling disabled timer...
	
	// just re-set and update timer: 
	long old_msec_left=i->msec_left;
	i->msec_val=msec;
	i->msec_left=msec;
	AlignTimer(i,align);
	fdb->_MsecLeftChanged(i,old_msec_left);  // ...will increment timeout_change if needed
	
	return(0);
}


int FDManager::ResetTimer(FDBase *fdb,TimerID tid,int align)
{
	if(!fdb)
	{  return(0);  }
	if(!tid)
	{  return(-2);  }
	
	#if TESTING_CHECK
	for(FDManager::TimerNode *i=fdb->timers; i; i=i->next)
	{  if((TimerID)i==tid)  goto found;  }
	fprintf(stderr,"**BUG in application: "
		"attempt to reset non-existant timer %p.\n",tid);
	return(-2);
	found:;
	#endif
	
	FDManager::TimerNode *i=(FDManager::TimerNode *)tid;
	if(i->msec_val<=0)
	{  return(1);  }
	long old_msec_left=i->msec_left;
	i->msec_left=i->msec_val;
	AlignTimer(i,align);
	fdb->_MsecLeftChanged(i,old_msec_left);  // ...will increment timeout_change if needed
	
	return(0);
}


int FDManager::KillTimer(FDBase *fdb,TimerID tid)
{
	if(!fdb)
	{  return(0);  }
	if(!tid)
	{  return(-2);  }
	
	#if TESTING_CHECK
	for(FDManager::TimerNode *i=fdb->timers; i; i=i->next)
	{  if((TimerID)i==tid)  goto found;  }
	fprintf(stderr,"**BUG in application: "
		"attempt to delete non-existant timer %p\n",tid);
	return(-2);
	found:;
	#endif
	
	FDManager::TimerNode *nd=(FDManager::TimerNode *)tid;
	fdb->DeleteTimerNode(nd);  // ...will increment timeout_change if needed
	
	return(0);
}


void FDManager::KillAllTimers(FDBase *fdb)
{
	if(fdb->ClearTimers())  // (returns number of deleted timers) 
	{
		// FDBase calls TimeoutChange() if needed, so the next line is no 
		// longer needed. 
		//++timerlist_change_serial;
	}
}


long FDManager::__GetTimeout()
{
	long msec=-1;
	
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next)
	{
		#if TESTING
		if(!fb->fdb || fb->fdb->deleted)
		{
			fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
				fb->fdb,fb->fdb->deleted,__LINE__);
			continue;
		}
		#endif
		
		// Get timer with smallest msec_left value from FDBase fb: 
		TimerNode *sh=fb->fdb->ShortestTimer();
		if(!sh)  continue;   // fb has no timers
		
		#if TESTING
		// This means that the ``shortest timer'' is actually disabled or 
		// getting deleted which may not happen here. 
		if(sh->msec_val<0)
		{  fprintf(stderr,"FD: BUG!! _GetTimeout(): sh->msec_val=%ld <0\n",
			sh->msec_val);  }
		#endif
		
		if(msec>0 && msec<sh->msec_left)  continue;
		
		msec=sh->msec_left;
		#if TESTING
		if(sh->msec_left<0)
		{
			fprintf(stderr,"BUG: _GetTimeout(): msec_left=%ld "
				"< 0",sh->msec_left);
			msec=0;
		}
		#endif
		
		if(!msec)  break;   // 0 msec timer
	}
	
	#if 0
	----ORIGINAL ALGORITHM----
	
	// FIXME: can I speed up that? 	
	// One way would be to sort the timer list (first the enabled ones, 
	// then the disabled ones) but that would lead into trouble when 
	// calling UpdateTimer() while the timer list is locked. In that 
	// case, UpdateTimer() might have to disable a timer and move it 
	// to the end of the timer list while e.g. _DeliverTimers() iterates 
	// through the timer list. 
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next)
	{
		#if TESTING
		if(!fb->fdb || fb->fdb->deleted)
		{
			fprintf(stderr,"BUG!! fdb=%p, deleted=%d in FDBase list (%d)\n",
				fb->fdb,fb->fdb->deleted,__LINE__);
			continue;
		}
		#endif
		for(FDManager::TimerNode *i=fb->fdb->timers; i; i=i->next)
		{
			if(i->msec_val<0L)  continue;  /* disabled timer */
			
			if(msec<0 || msec>i->msec_left)
			{
				msec=i->msec_left;
				#if TESTING
				if(i->msec_left<0)
				{
					fprintf(stderr,"BUG: _GetTimeout(): msec_left=%ld "
						"< 0",i->msec_left);
					msec=0;
				}
				#endif
				if(!msec)  goto doret;
			}
		}
	}
	doret:;
	#endif
	
	return(msec);
}

// Returns -1 if no timers 
inline long FDManager::_GetTimeout(const HTime *current)
{
	// Give TimeoutManager a chance to update the timeout node: 
	if(timeout_manager)
	{
		// NOTE: This will NOT make timeout_manager be linked in as we 
		//       only see the ManagerType set by SetManager() and then 
		//       use virtual timernotify(). So, FDManager does not 
		//       require TimeoutManager (and it will not be liked in 
		//       unless used by user). 
		TimerInfo tinfo;
		HTime tmp=*current;
		tinfo.tid=NULL;  // special
		tinfo.current=&tmp;
		tinfo.dptr=NULL;
		timeout_manager->timernotify(&tinfo);
	}
	
	if(timeout_change)  // timeout_change set to 0 in main loop 
	{  last_timeout=__GetTimeout();  }
	
	#if TESTING_CHECK
	if(!timeout_change)
	{
		long nto=__GetTimeout();
		if(last_timeout!=nto)
		{  fprintf(stderr,"BUG!! in timer code: timeout changed "
			"(%ld -> %ld) but timeout_change=0\n",last_timeout,nto);  }
		last_timeout=nto;
	}
	
	// Check if the timeout returned by __GetTimeout() is really the 
	// correct one: 
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next)
	{
		if(!fb->fdb || fb->fdb->deleted)  continue;
		for(FDManager::TimerNode *i=fb->fdb->timers; i; i=i->next)
		{
			if(i->msec_val<0L)  continue;  /* disabled timer */
			if(i->msec_left<last_timeout)
			{  fprintf(stderr,"FD: BUG!! in timer code: "
				"__GetTimeout() returns %ld msec timer but %ld is shorter.\n",
				last_timeout,i->msec_left);  }
		}
	}
	#endif  /* TESTING_CHECK */
	
	return(last_timeout);
}


/************************* FD Stuff **************************/

inline void FDManager::_AssignFDArrElem(FDManager::FDNode *n)
{
	// Fast case: just add FD to the list and to the array which does 
	// not require a re-allocation of the array as there are free 
	// entries left (due to alloc ahead). As the array keeps in sync 
	// with pfd[], we need not ++fdlist_change_serial and rebuild 
	// the array. 
	n->idx=npfds++;  // increase npfds counter 
	pollfd *p=&pfd[n->idx];
	p->fd=n->fd;
	p->events=n->events;
	p->revents=0;   // important or we might deliver non-existant revents 
	pdebug("FastFDAdd\n");
}


int FDManager::_PollFD(FDBase * /*fdb*/,FDManager::FDNode *j,
	short events,const void **dptr)
{
	// Just update events & dptr: 
	if(dptr)
	{  j->dptr=*dptr;  }
	// else: let j->dptr unchanged 
	//    (Yes!, because it's a pointer to the passed dptr pointer val.)
	if(j->events==events)  return(0);  // done
	// Events have to be changed: 
	if(j->events)  {  --pollnodes;  }
	j->events=events;
	if(j->events)  {  ++pollnodes;  }
	// If fdlist_change_serial>0, then the fd array has to be 
	// updated aynway, so we just quit here and do the rest 
	// later. 
	if(fdlist_change_serial)
	{  ++fdlist_change_serial;  return(0);  }
	if(events!=0)
	{
		// If the fd node already has an associated fd array element 
		// (and the new events are !=0), update it (things are 
		// consistent as fdlist_change_serial=0). 
		if(j->idx>=0)
		{
			// Fast case: just change FD events in pfd[] so the list 
			// keeps in sync with the array and we need not 
			// ++fdlist_change_serial and need not re-build pfd[]. 
			pfd[j->idx].events=events;
			#if TESTING
			// If this message is written then it means that pfd[] is 
			// not in sync with the list although !fdlist_change_serial 
			// OR that j->idx is invalid. 
			if(pfd[j->idx].fd!=j->fd)
			{  fprintf(stderr,"BUG:%d: j->idx=%d (n=%d); pfd.fd=%d; fd=%d ***\n",
				__LINE__,j->idx,npfds,pfd[j->idx].fd,j->fd);  }
			#endif
			pdebug("FastFDChange\n");
		}
		// If the node has no associated fd array element, we may 
		// just assign one to it if there is one left (due to alloc 
		// ahead): 
		else if(j->idx==-1 && npfds<pfd_dim)
		{  _AssignFDArrElem(j);  }
		// Otherwise, the fd array has to be updated. 
		else
		{  ++fdlist_change_serial;  }
		return(0);
	}
	// If we come here, events==0. 
	// We actually changed from events!=0 to events=0. 
	// If there is an associated fd array elem, then the array 
	// has to be updated. 
	if(j->idx>=0)
	{  ++fdlist_change_serial;  }
	// Otherwiese there is nothing to do. 
	return(0);
}

// Never removes an entry from the list; that is only done by Unpoll(). 
int FDManager::PollFD(FDBase *fdb,int fd,short events,
	const void **dptr,PollID *ret_id)
{
	if(!fdb)
	{  return(0);  }
	
	if(fd<0)
	{  return(-2);  }
	
#if TESTING
if(events & ~(POLLIN | POLLOUT))
{  fprintf(stderr,"\n******!!!!!!FD-NOTE[events=%d, fd=%d]!!!!!!****** (BUG-II!! %d)\a\n",
	events,fd,__LINE__);  }
#endif
	
	for(FDManager::FDNode *j=fdb->fds; j; j=j->next)
	{
		if(j->fd==fd)
		{
			if(ret_id)
			{  *ret_id=(PollID)j;  }
			return(_PollFD(fdb,j,events,dptr));
		}
	}
	
	// Hmm.. the fd was not found, so add a new fd entry. 
	FDManager::FDNode *n=fdb->AllocFDNode(fd,events,dptr ? (*dptr) : NULL);
	if(!n)
	{  return(-1);  }
	fdb->AddFDNode(n);
	++fd_nnodes;
	if(n->events)
	{
		++pollnodes;
		if(!fdlist_change_serial && npfds<pfd_dim)
		{  _AssignFDArrElem(n);  }
		else
		{  ++fdlist_change_serial;  }
	}
	if(ret_id)
	{  *ret_id=(PollID)n;  }
	return(0);
}


// Internally: unpoll FD node (dequeue & free). 
inline int FDManager::_UnpollFD(FDBase *fdb,FDManager::FDNode *fdn)
{
	if(fdn->events)
	{  --pollnodes;  }
	// In case there is no array elem associated with that fd 
	// node, the array keeps in sync. 
	// Hmmm... no because that gives trouble if PollFD() gets 
	// called right after UnpollFD() in an environment where 
	// the fdlist is locked (-> fdn->idx=-2 by DeleteFDNode()). 
	//if(fdn->idx>=0)
	{  ++fdlist_change_serial;  }
	--fd_nnodes;
	fdb->DeleteFDNode(fdn);
	return(0);
}

// deletes fd entry from list (if existing)
int FDManager::UnpollFD(FDBase *fdb,int fd)
{
	if(!fdb)
	{  return(0);  }
	
	for(FDManager::FDNode *j=fdb->fds; j; j=j->next)
	{
		if(j->fd==fd)
		{  return(_UnpollFD(fdb,j));  }
	}
	
	return(1);
}

int FDManager::UnpollFD(FDBase *fdb,PollID pollid)
{
	if(!pollid)  return(1);
	
	return(_UnpollFD(fdb,(FDManager::FDNode*)pollid));
}
