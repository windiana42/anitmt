/*
 * timeoutmanager.cc
 * 
 * Implementation of class TimeoutManager, a manager class for 
 * handling (greater numbers of) timeouts. It works in cooperation 
 * with TimeoutBase and requires FDManager. 
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

#include "fdmanager.h"
#include "fdbase.h"
#include "timeoutmanager.h"
#include "timeoutbase.h"

#ifndef TESTING
#define TESTING 1
#endif

// TESTING_CHECK is does more internal control tests which will 
// probably slow down things quite much. Only enable for debug 
// reasons. 
#define TESTING_CHECK 1


#if TESTING
#  warning TESTING switched on (using assert())
#  include <assert.h>
#else
#  define assert(x)
#  undef TESTING_CHECK
#  define TESTING_CHECK 0
#endif

// Init static global manager: 
TimeoutManager *TimeoutManager::manager=NULL;


// Update the FDBase timer. 
void TimeoutManager::SetFDBaseTimer(const HTime *current)
{
	TNode *n=_GetShortest();
	long to_val=-1;
	if(n)
	{
		if(n->timeout<=(*current))
		{  to_val=0;  }
		else
		{
			HTime diff=n->timeout-(*current);
			assert(diff>HTime(HTime::Null));
			// Protect against integer overflow: 
			// If the timeout is at least 10 days, use 10 days: 
			// Note: I use Get(), NOT GetR(), so the real timeout 
			//       is never smaller! (no rounding)
			long days=diff.Get(HTime::days);
			if(days>=10)
			{  to_val=864000000;  }
			else
			{  to_val=diff.Get(HTime::msec);  }
		}
	}
	#warning THIS SHOULD BE THE FIRST ONE...
	long tleft=TimerLeft(tid);
	if(tleft!=to_val)
	{
		// Must update. 
		// Okay, we ignore a +/- 1 msec difference if the value is 
		// greater than 20 msec (i.e. precision >5%). 
		if(labs(tleft-to_val)>1 || to_val<20 /*YES!*/ || tleft<0 /*YES!*/)
		{
			#if TESTING
			// It is normal to get this message if the shortest timeout 
			// was changed. But otherwise not. 
			//if(to_val>=0 && TimerLeft(tid)>=0)
			{  fprintf(stderr,"Timeoutmanager: UPDATE %ld -> %ld (msec)\n",
				tleft,to_val);  }
			#endif
			UpdateTimer(tid,to_val,0);
		}
	}
}


int TimeoutManager::timernotify(TimerInfo *ti)
{
	if(ti->tid==NULL)
	{
		// Special case. Called by FDManager to update the timer: 
		SetFDBaseTimer(ti->current);
		return(0);
	}
	
	assert(ti->tid==tid);
	
	// Okay, look for expired timers: 
	TNode *n=_GetShortest();
	if(!n)
	{  return(0);  }   // Surely no expired timers. 
	
	// First, lock reordering: 
	assert(lock_reorder<0);
	lock_reorder=0;  // YES.
	
	TimeoutInfo tinfo;
	HTime to_tmp;
	assert(curr_tb_iter==NULL);
	curr_tb_iter=tb_list.first();
	for(;;)
	{
		TimeoutBase *tb=curr_tb_iter;
		if(!tb)  break;
		
		// NOTE! The issue is what happens with TNodes which get 
		//       modified / deleted during the call to timeoutnotify(). 
		// -> They are disabled (and thus deleted from the to_list) 
		//    before the call to timeoutnotify(), so re-installing 
		//    them is no problem (tb list reorder lock is held). 
		//    NOTE however, that a re-installed timeout which has an 
		//    expire time before or equal *current will _IMMEDIATELY_ 
		//    (right after return from timeoutnotify()) trigger a 
		//    new call to timeoutnotify(). 
		for(;;)
		{
			TNode *n=tb->to_list.first();
			if(!n)  break;
			// As the list is sorted, things can be speeded up. 
			if(n->timeout>(*ti->current))  break;
			
			// Timeout happens. 
			tinfo.tid=(TimeoutID)n;
			tinfo.dptr=n->dptr;
			to_tmp=n->timeout;
			tinfo.timeout=&to_tmp;
			tinfo.current=ti->current;
			
			// Disable the timer before calling timeoutnotify(): 
			tb->to_list.popfirst();  // as n==first
			n->timeout.SetInvalid();
			tb->dis_list.append(n);
			++lock_reorder;
			
			// Virtual function call: 
			tb->timeoutnotify(&tinfo);
			
			if(curr_tb_iter!=tb)  goto go_on_here;  // *tb was deleted. 
		}
		assert(curr_tb_iter==tb);   // *tb was not deleted. 
		curr_tb_iter=tb->next; 
		go_on_here:;
	}
	
	// Finally, unlock reordering: 
	if(lock_reorder>0)
	{
		lock_reorder=-1;
		_ReOrderTBList();
		assert(sh_timeout_change);
	}
	else
	{  lock_reorder=-1;  }
	
	return(0);
}


// Insert passed node at correct position in tb->to_list: 
void TimeoutManager::_InsertActiveTimer(TimeoutBase *tb,TNode *ti)
{
	for(TNode *i=tb->to_list.first(); i; i=i->next)
	{
		if(i->timeout>ti->timeout)
		{  tb->to_list.queuebefore(ti,i);  return;  }
	}
	tb->to_list.append(ti);
}


// Find TimeoutBase with shortest timer and bring it to the beginning of 
// the tb_list: 
void TimeoutManager::_ReOrderTBList()
{
	if(lock_reorder>=0)
	{  ++lock_reorder;  return;  }
	
	TimeoutBase *shortest=NULL;
	HTime *sh_timeout=NULL;
	
	// First, find first class which has a non-disabled timeout: 
	for(TimeoutBase *tb=tb_list.first(); tb; tb=tb->next)
	{
		TNode *f=tb->to_list.first();
		if(f)
		{
			shortest=tb;
			sh_timeout=&f->timeout;
			goto found_first;
		}
	}
	++sh_timeout_change;  // YES!
	return;
	found_first:;
	
	// Find the class holding the shortest timeout: 
	for(TimeoutBase *tb=shortest; tb; tb=tb->next)
	{
		TNode *f=tb->to_list.first();
		if(!f)  continue;
		if(f->timeout<(*sh_timeout))
		{
			shortest=tb;
			sh_timeout=&f->timeout;
		}
	}
	
	// Okay, re-order them if needed: 
	if(shortest!=tb_list.first())
	{  _TBListToFront(shortest);  }
}


TimeoutManager::TimeoutID TimeoutManager::IstallTimeout(TimeoutBase *tb,
	const HTime &timeout,void *dptr)
{
	if(!tb)  return(NULL);
	
	TNode *n=NEW<TNode>();
	if(!n)  return(NULL);
	
	n->dptr=dptr;
	
	if(timeout.IsInvalid())  // disabled
	{
		n->timeout.SetInvalid();
		tb->dis_list.append(n);
	}
	else  // enabled
	{
		TNode *sh0=_GetShortest();
		
		n->timeout=timeout;
		_InsertActiveTimer(tb,n);
		
		// See if the shortest timeout changed: 
		if(!sh0 || n->timeout<sh0->timeout)
		{
			// Shortest timeout changed. *n is now the shortest. 
			// Re-order tb_list: 
			_TBListToFront(tb);
			
			assert(_GetShortest()==n);
		}
	}
	
	return((TimeoutID)n);
}


#if TESTING_CHECK
#warning !! ESTING_CHECK switched on. This will slow down performance. !!
#define _CheckTimeoutID(tb,tid) \
	for(TNode *n=tb->to_list.first(); n; n=n->next) \
	{  if((TimeoutID)n==tid)  goto _tc_found;  } \
	for(TNode *n=tb->dis_list.first(); n; n=n->next) \
	{  if((TimeoutID)n==tid)  goto _tc_found;  } \
	fprintf(stderr,"TOM: OOPS: Illegal TimeoutID %p\n",tid);  \
	return(-3);  _tc_found:;
#endif  /* TESTING_CHECK */


int TimeoutManager::UpdateTimeout(TimeoutBase *tb,TimeoutID tid,
	const HTime &timeout)
{
	if(!tb)  return(0);
	
	TNode *n=(TNode*)tid;
	if(!n)  return(-3);
	
	#if TESTING_CHECK
	_CheckTimeoutID(tb,tid)
	#endif
	
	assert(n->next || tb->to_list.last()==n || tb->dis_list.last()==n);
	
	// Okay, let's see what changes: 
	if(timeout.IsInvalid())
	{
		// Timeout shall be disabled. 
		if(n->timeout.IsInvalid())  return(1);  // already disabled. 
		
		// Disable enabled timeout: 
		TNode *sh0=_GetShortest();
		tb->to_list.dequeue(n);
		n->timeout.SetInvalid();
		tb->dis_list.append(n);
		if(n==sh0)  // Just disabled shortest timer. 
		{  _ReOrderTBList();  }
		
		return(0);
	}
	
	// Shortest timeout before we touch anyhing: 
	TNode *sh0=_GetShortest();
	
	if(n->timeout.IsInvalid())
	{
		// Enable disabled timeout: 
		tb->dis_list.dequeue(n);
		n->timeout=timeout;
		_InsertActiveTimer(tb,n);
		
		// New timeout shorter than shortest? 
		if(!sh0 || n->timeout<sh0->timeout)
		{
			// Shortest timeout changed. *n is now the shortest. 
			// Re-order tb_list: 
			_TBListToFront(tb);
		}
		return(0);
	}
	
	// Change timeout: 
	// Now, if the new timeout is shorter than the old timeout 
	// and this is the shortest timeout, OR it is not the shortest 
	// and the timeout is made longer, then no re-ordering is 
	// needed. 
	int dont_reorder = 
		( (n==sh0 && timeout<=n->timeout) || 
		  (n!=sh0 && timeout>=n->timeout) );
	if(n==sh0)  ++sh_timeout_change;
	tb->to_list.dequeue(n);
	n->timeout=timeout;
	_InsertActiveTimer(tb,n);
	
	if(!dont_reorder)
	{  _ReOrderTBList();  }
	
	return(0);
}


int TimeoutManager::KillTimeout(TimeoutBase *tb,TimeoutID tid)
{
	if(!tb)  return(0);
	
	TNode *n=(TNode*)tid;
	if(!n)  return(-3);
	
	#if TESTING_CHECK
	_CheckTimeoutID(tb,tid)
	#endif
	
	assert(n->next || tb->to_list.last()==n || tb->dis_list.last()==n);
	
	if(n->timeout.IsInvalid())
	{
		// Killing disabled timer. No problem. 
		tb->dis_list.dequeue(n);
		delete n;
	}
	else
	{
		// Killing enabled timer. 
		// See if we kill the shortest one: 
		int must_reorder=(_GetShortest()==n);
		tb->to_list.dequeue(n);
		delete n;
		if(must_reorder)
		{  _ReOrderTBList();  }
		assert(!must_reorder || sh_timeout_change);
	}
	
	return(0);
}


void TimeoutManager::Register(TimeoutBase *tb)
{
	if(!tb)  return;
	if(tb->next || tb_list.last()==tb)  return;
	// Append at the _end_ of the list: 
	tb_list.append(tb);
}

void TimeoutManager::Unregister(TimeoutBase *tb)
{
	if(!tb)  return;
	assert(tb->next || tb_list.last()==tb);  // tb queued in tb_list
	
	// First, the easy part: delete all disabled timeouts: 
	while(!tb->dis_list.is_empty())
	{  delete tb->dis_list.popfirst();  }
	
	// See if there are enabled timers: 
	TNode *f=tb->to_list.first();
	int must_reorder=0;
	if(f)
	{
		// Okay, there are enabled timers. Then handle them: 
		must_reorder=(tb_list.first()==tb);
		
		// Delete all active timeout nodes of that class: 
		do  // We know that the list is initially not empty. 
		{  delete tb->to_list.popfirst();  }
		while(!tb->to_list.is_empty());
	}
	
	// Finally, delete tb from tb_list: 
	tb_list.dequeue(tb);
	if(curr_tb_iter==tb)
	{
		// Oh, we're called on the stack of timeoutnotify(). Nice. 
		curr_tb_iter=tb->next;
	}
	
	if(must_reorder)
	{  _ReOrderTBList();  }
	assert(!must_reorder || sh_timeout_change);
}


TimeoutManager::TimeoutManager(int *failflag) : 
	FDBase(failflag),
	tb_list(failflag)
{
	int failed=0;
	
	sh_timeout_change=0;
	curr_tb_iter=NULL;
	lock_reorder=-1;
	
	if(SetManager(FDManager::MT_Timeout))
	{  --failed;  }
	if(!(tid=InstallTimer(-1,0)))  // alloc timer node 
	{  --failed;  }
	
	if(failed)
	{
		if(failflag)
		{  *failflag+=failed;  return;  }
		ConstructorFailedExit("TOMan");
	}
	
	// Init global manager: 
	#if TESTING
	if(manager)
	{  fprintf(stderr,"%s: more than one TimeoutManager.\n",prg_name);  abort();  }
	#endif
	
	manager=this;
}

TimeoutManager::~TimeoutManager()
{
	// Make sure there are no TimeoutBases left: 
	assert(tb_list.is_empty());
	
	// Cleanup: 
	manager=NULL;
}
