/*
 * timeoutmanager.h
 * 
 * Header containing class TimeoutManager, a manager class for 
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

#ifndef _HLIB_TimeoutManager_H_
#define _HLIB_TimeoutManager_H_ 1

#include <hlib/htime.h>
#include <hlib/linkedlist.h>


class TimeoutBase;

class TimeoutManager : private FDBase
{
	friend class TimeoutBase;
	// NOTE!!! If the static manager gets removed: 
	// There should be ONE TimeoutManager per FDManager. 
	static TimeoutManager *manager;
	public:
		struct _TimeoutID {};  typedef _TimeoutID (*TimeoutID);
		
		// Used in calls to virtual timeoutnotify(): 
		struct TimeoutInfo
		{
			TimeoutID tid;         // ID of timeout which occured 
			void *dptr;            // attached data or NULL
			const HTime *timeout;  // when the timeout occured
			const HTime *current;  // current time as passed to timernotify()
		};
	private:
		struct TNode : LinkedListBase<TNode>
		{
			// NOTE: timeout.IsValid() must be false for disabled timers. 
			HTime timeout;
			void *dptr;
			
			_CPP_OPERATORS_FF
			TNode(int * /*failflag*/=NULL) : LinkedListBase<TNode>(), timeout()
				{  dptr=NULL;  }
			~TNode() { }
		};
		
		// NOTE: List unsorted but the class with the shortest 
		//       timeout is the first element. 
		LinkedList<TimeoutBase> tb_list;
		
		// This is the FDManager - timer used for the timeouts: 
		TimerID tid;
		
		// Counts changes of shortest timeout. 
		int sh_timeout_change;
		
		// Lock reordering of tb list. 
		int lock_reorder;
		
		// These are needed if TimeoutBases unregister while we're 
		// running through the list in timernotify(): 
		TimeoutBase *curr_tb_iter;
		
		// Get shortest timeout or NULL: 
		inline TNode *_GetShortest();  // see timeoutbase.h
		
		// Put shortest to the beginning of tb_list. 
		inline void _TBListToFront(TimeoutBase *shortest);  // see timeoutbase.h
		
		// Find TimeoutBase with shortest timer and bring it to the 
		// beginning of the tb_list: 
		void _ReOrderTBList();
		// Insert passed node at correct position in tb->to_list: 
		void _InsertActiveTimer(TimeoutBase *tb,TNode *ti);
		// Update the FDBase timer. 
		void SetFDBaseTimer(const HTime *current);
		
		// Overriding virtual from FDBase: 
		int timernotify(TimerInfo *);
	public:  _CPP_OPERATORS_FF
		TimeoutManager(int *failflag=NULL);
		~TimeoutManager();
		
		// Called upon con/destruction: 
		void Register(TimeoutBase *tb);
		void Unregister(TimeoutBase *tb);
		
		// These are called by TimeoutBase. See there for more info. 
		TimeoutID InstallTimeout(TimeoutBase *tb,const HTime &timeout,void *dptr);
		int UpdateTimeout(TimeoutBase *tb,TimeoutID tid,const HTime &timeout);
		int KillTimeout(TimeoutBase *tb,TimeoutID tid);
};

#endif  /* _HLIB_TimeoutManager_H_ */
