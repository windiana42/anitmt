/*
 * timeoutbase.h
 * 
 * Header containing class TimeoutBase, a base class for 
 * handling (greater numbers of) timeouts. It works in cooperation 
 * with TimeoutManager and FDManager. 
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

#ifndef _HLIB_TimeoutBase_H_
#define _HLIB_TimeoutBase_H_ 1

#include <hlib/htime.h>
#include <hlib/linkedlist.h>

// NOTE: requires <hlib/timeoutmanager.h>

// NOTE: TimeoutBase is a class which allows you to install several timeouts. 
//       You first have to create an object of type TimeoutManager if you 
//       use any TimeoutBase-derived classes. Note that TimeoutManager needs 
//       FDManager, so be sure to create an FDManager before the 
//       TimeoutManager
// NOTE: TimeoutBase/TimeoutManager are thought as a mechanism of dealing 
//       with a larger amount of timeouts. If you just have one or two 
//       (or any small number), it is probably best to use a normal FDBase 
//       timer (and use FDAT_AlignToShorter if you want). But manager 
//       classes such as FDCopyManager can have a larger number of 
//       individual timeouts; it is such classes, that TimeoutBase was 
//       written for. TimeoutManager keeps track of all timeouts and uses 
//       just one single FDBase timer (with no alignment) for the timeouts. 
class TimeoutBase : 
	#if __GNUG__ < 3
	public LinkedListBase<TimeoutBase>
	#else
	private LinkedListBase<TimeoutBase>
	#endif
{
	friend class LinkedList<TimeoutBase>;
	friend class TimeoutManager;
	public:
		typedef TimeoutManager::TimeoutInfo TimeoutInfo;
		typedef TimeoutManager::TimeoutID TimeoutID;
	private:
		typedef TimeoutManager::TNode TNode;
		
		// NOTE: List sorted by timeout. 
		// The earlier the timeout, the further at the beginning of the list. 
		LinkedList<TNode> to_list;
		// This list holds all disabled timeout: 
		LinkedList<TNode> dis_list;
		
	protected:
		// Gets called when a timeout actually happens. 
		// [Always called on the stack of fdmanager's timernotify().]
		// Return value: currently unused; use 0. 
		// NOTE: The timeout gets automatically disabled before the call. 
		virtual int timeoutnotify(TimeoutInfo *) HL_PureVirt(0)
	public:  _CPP_OPERATORS_FF
		TimeoutBase(int *failflag=NULL) : 
			LinkedListBase<TimeoutBase>(), to_list(failflag), dis_list(failflag)
			{  timeoutmanager()->Register(this);  }
		virtual ~TimeoutBase()
			{  timeoutmanager()->Unregister(this);  }
		
		// Return pointer to the manager. 
		TimeoutManager *timeoutmanager()
			{  return(TimeoutManager::manager);  }
		
		// Install a new timeout. You may use HTime(HTime::Invalid) to 
		// set up a disabled timeout. 
		// NOTE: A timeout is a complete HTime, i.e. if it is 
		//     Aug 15 12:08:30 2002
		// and you want the timeout to occur in 30 minutes, you must pass 
		//     Aug 15 12:38:30 2002
		// as timeout. To do that, you may use: 
		//     HTime timeout(HTime::Curr);
		//     timeout.Add(30,HTime::minutes);
		//     TimeoutID tid=InstallTimeout(timeout);
		// Return value: TimeoutID or NULL (alloc failed). 
		TimeoutID InstallTimeout(const HTime &timeout,void *dptr=NULL)
			{  return(timeoutmanager()->InstallTimeout(this,timeout,dptr));  }
		// Change timeout time of specified TimeoutID: 
		// Pass an invalid time (HTime::IsInvalid()) to disable timeout. 
		// Return value: 0 -> OK
		//               1 -> disabled already disabled timeout
		//              -3 -> tid invalid (NULL)
		int UpdateTimeout(TimeoutID tid,const HTime &timeout)
			{  return(timeoutmanager()->UpdateTimeout(this,tid,timeout));  }
		// Change data pointer associated with timeout: 
		// Return value: 0 -> OK; 
		//              -3 -> tid invalid (NULL)
		int UpdateTimeout(TimeoutID tid,void *dptr)
			{  if(!tid)  return(-3);  ((TimeoutManager::TNode*)tid)->dptr=dptr;  return(0);  }
		// Kill a timeout destroying the TimeoutID. 
		// Return value: 0 -> OK; -3 -> tid invalid (NULL)
		int KillTimeout(TimeoutID tid)
			{  return(timeoutmanager()->KillTimeout(this,tid));  }
		
		// Query data: 
		// Timeout HTime of timeout or invalid HTime (see HTime::IsInvalid()) if 
		// disabled or NULL if tid==NULL. 
		// DO NOT MODIFY THE RETURNED HTime!
		const HTime *TimeoutTime(TimeoutID tid)
			{  return(tid ? &(((TimeoutManager::TNode*)tid)->timeout) : NULL);  }
		// Get data pointer of timeout (attached custom data; void *dptr). 
		// Returns NULL if tid is invalid (NULL). 
		const void *TimeoutDPtr(TimeoutID tid)
			{  return(tid ? ((TimeoutManager::TNode*)tid)->dptr : NULL);  }
};


// This is here because of include order... grmbl. 
inline TimeoutManager::TNode *TimeoutManager::_GetShortest()
{
	TimeoutBase *tb=tb_list.first();
	return(tb ? tb->to_list.first() : NULL);
}
// This is here because gcc prior to 3.0 need that this way. (hrrm..)
inline void TimeoutManager::_TBListToFront(TimeoutBase *shortest)
{
	if(lock_reorder>=0)
	{  ++lock_reorder;  return;  }
	tb_list.dequeue(shortest);
	tb_list.insert(shortest);
	++sh_timeout_change;
}

#endif  /* _HLIB_TimeoutBase_H_ */
