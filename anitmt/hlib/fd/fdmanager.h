/*
 * fdmanager.h
 * 
 * Header containing class FDManager, a class for 
 * file descriptor and time management which works in 
 * cooperation with classes derived from class FDBase. 
 *
 * Copyright (c) 2000--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_FDManager_H_
#define _HLIB_FDManager_H_ 1

// requires <unistd.h>, <signal.h>, 
//          <sys/socket.h> (for shutdown())

#include <hlib/prototypes.h>
#include "htime.h"

class FDBase;
class HTime;

class FDManager
{
	friend class FDBase;
	public:
		/* static global manager */
		static FDManager *manager;
	public:
		struct TimerNode
		{
			TimerNode *prev,*next;   // sorted by FDBase 
			long msec_val;    // timer interval
			long msec_left;   // msec left till next alarm
			const void *dptr;  // custom data pointer associated with timer 
		};
		struct _TimerID {};  typedef _TimerID (*TimerID);
		struct _PollID {};   typedef _PollID  (*PollID);
		struct TimerInfo
		{
			TimerID tid;    // ID of the timer being triggered 
			HTime *current;  // may be modified in timernotify()
			const void *dptr;  // custom data pointer associated with timer (or NULL)
		};
		struct FDNode
		{
			FDNode *prev,*next;
			int fd;   // file descriptor
			short events;  // what to poll for. 
			int idx;  // index in pfd array or -1 or -2 -> delete me. 
			const void *dptr;  // custom data pointer associated with fd (or NULL)
		};
		struct FDInfo
		{
			PollID pollid;   // PollID of that poll node 
			int fd;   // file descriptor
			short events;  // what to poll for 
			short revents;  // what poll returned 
			HTime *current;  // may be modified in fdnotify()
			const void *dptr;  // custom data pointer associated with fd (or NULL)
		};
		struct SigInfo
		{
			HTime time;       // time when signal was caught
			siginfo_t info;   // see sigaction(2)
		};
	private:
		// LIST OF ALL FDBase CLASSES: 
		// NOTE: The prev/next queuing cannot be put into the FDBase because 
		//       stack-allocated FDBase classes would then interfere with 
		//       delayed deletion. (Note that the list may not be modified 
		//       (deletion) while we traverse it.) 
		struct FDBNode
		{
			FDBNode *prev,*next;
			FDBase *fdb;
			//int deleted;  // 1 -> waiting to get tidied up. 
		};
		// Needed to be able to delete (=move to dead list) FDBNodes 
		// while iterating through the list. 
		FDBNode *iterator_next,*iterator_next_ds;
		struct FDBList
		{  _CPP_OPERATORS
			FDBNode *first;
			FDBNode *fdead;  // first `dead' (deleted) FDBase 
			int n_no_managers;  // number of registered non-manager classes 
			void _DequeueNode(FDBNode *n,int deadlist);
			void DeleteNode(FDBNode *n,int deadlist);  // dequeue & free
			void DeadNode(FDBNode *n);  // move normal -> dead list
			int Add(FDBase *fdb);            // normal list 
			FDBNode *FindNode(FDBase *fdb);  // normal list
			void Clear();                    // both lists 
			FDBList()   {  first=NULL;  fdead=NULL;  n_no_managers=0;  }
			~FDBList()  {  Clear();  }
		} fdblist;
		struct SigNode
		{
			SigNode *next;
			SigInfo info;
		} *first_pending_sig,*last_pending_sig;
		int pending_signum;
		// List of free SigNodes to prevent allocation in signal handler. 
		SigNode *first_free_signode;
		int free_signodes;   // number of free signodes in free_signodes list
		int min_free_signodes;   // min. number of free signal nodes
		int max_free_signodes;   // max. number of free sig nodes 
		int extra_signodes;      // number extra signal nodes (e.g. 
				// one per child process)
		
		int __EnsureFreeSigNodes(int n);
		inline int _EnsureFreeSigNodes(int n)
			{  return((free_signodes<n) ? __EnsureFreeSigNodes(n) : 0);  }
		inline int _EnsureFreeSigNodes()
			{  return(_EnsureFreeSigNodes(min_free_signodes+extra_signodes));  }
		SigNode *_AllocSigNode();        // "Alloc" signode
		void _FreeSigNode(SigNode *sn);  // "Free" signode
		
		int timeout_change;  // incremented when timeout (is likely to) change
		long last_timeout;
	public:
		inline void TimeoutChange()   // used by FDBase. 
			{  ++timeout_change;  }
	private:
		
		void AlignTimer(TimerNode *n,int align);
		long __GetTimeout();
		inline long _GetTimeout();  // or -1 if no timers 
		inline void _DeliverTimers(long elapsed,HTime *currtv);
		
		int fd_nnodes;  // number of nodes in fd list. 
		int pollnodes;  // number of nodes in fd list with events!=0. 
		
		// (Incremented when fdlist is changed; must rebuild pfd array then)
		// Indicates weather the pfd array is in sync with the fd list or not. 
		int fdlist_change_serial;  
		struct pollfd *pfd;  // poll fd array [NOTE: allocated at &pfd[-1];  
		                     //    pfd[-1] internally used fot sig_pipe_fd.]
		int pfd_dim,npfds;   // dim and no of elements of pfd[]
		int sig_pipe_fd_r;   // Pipe fd agaist signal / poll race condition; 
		int sig_pipe_fd_w;   // _r: read end; _w: write end of pipe. 
		struct pollfd *pipe_pfd;  // pollfd for pipe fd only. 
		int sig_pipe_bytes;  // number of bytes expected in pipe 
		                     // or -1 -> do not write to pipe. 
		inline void _ClearSigPipe();
		
		inline void _DeliverFDNotify(const HTime *fdtime);
		
		// Internally: unpoll FD node (dequeue & free). 
		inline int _UnpollFD(FDBase *fdb,FDManager::FDNode *fdn);
		int _PollFD(FDBase *fdb,FDManager::FDNode *j,short events,const void **dptr);
		
		// Exit status value or -1. 
		int quitval;
		
		inline void _AssignFDArrElem(FDManager::FDNode *n);
		void __UpdateFDArray();
		inline void _UpdateFDArray()
			{  if(fdlist_change_serial || pollnodes!=npfds)  __UpdateFDArray();  }
		void __TidyUp();
		inline void _TidyUp()  // tidy up garbage left over by deleted FDBase. 
			{  if(fdblist.fdead)  __TidyUp();  }
		
		void _DeliverSignal(SigNode *sn);  // to all FDBase classes
		void __DeliverPendingSignals(int tidy_up);
		inline bool _HavePendingSignals()
			{  return(pending_signum);  }
		inline void _DeliverPendingSignals(int tidy_up=1)
			{  if(_HavePendingSignals())  __DeliverPendingSignals(tidy_up);  }
		void _ZapSignals();
	public:  _CPP_OPERATORS_FF
		FDManager(int *failflag=NULL);
		~FDManager();
		
		// FDBase classes (un)register here. 
		// IF Register() RETURNS != 0, THE CLASS *fdb MUST BE 
		// DELETED. 
		int Register(FDBase *fdb);
		// flag: 1 -> just unregister
		//       2 -> unregister and destruct 
		//       3 -> unregister, destruct and free(fdb)
		void Unregister(FDBase *fdb,int flag);
		
		// Called by FDBase to keep counters up to date. 
		// NEVER CALL DIRECTLY. 
		void DestructionDone(FDBase *,int nfds,int npollfds);
		
		// Gets called if a signal is caught. 
		void CaughtSignal(siginfo_t *info);
		
		// Checks if that signal is queued for delivery: 
		// Returns number of queued signals of type sig. 
		// (See FDBase::SigPending())
		int SigPending(int sig);
		
		// Make sure that the number of free sig nodes is increased by 
		// delta (decreasing it if delta<0).
		void ExtraSigNodes(int delta);
		
		/* TIMER functions; see class FDBase */
		TimerID InstallTimer(FDBase *fdb,long msec,int align,const void *dptr);
		int UpdateTimer(FDBase *fdb,TimerID tid,long msec,int align);
		int ResetTimer(FDBase *fdb,TimerID tid,int align);
		int KillTimer(FDBase *fdb,TimerID tid);
		void KillAllTimers(FDBase *fdb);
		
		/* FILE DESCRIPTOR functions; see class FDBase */
		int PollFD(FDBase *fdb,int fd,short events,const void **dptr,PollID *ret_id=NULL);
		int PollFD(FDBase *fdb,PollID pollid,short events,const void **dptr)
			{  return(pollid ? _PollFD(fdb,(FDManager::FDNode*)pollid,
				events,dptr) : (-2));  }
		int UnpollFD(FDBase *fdb,int fd);
		int UnpollFD(FDBase *fdb,PollID pollid);   // faster, of course...
		
		/* (Un)Set manager flag: */
		int SetManager(FDBase *fdb,int flag);
		
		// Close all FDs except those in the list `exclude' of size n. 
		// Called by ProcessManager before executing a process. 
		int CloseAllFDs(int *exclude,int n);
		
		// Run main loop. 
		// Return value: -1 -> severe error (poll() failed etc.)
		//                0 -> OK
		//              >=0 -> value specified with Quit(). 
		// MainLoop() returns, if no FDBase classes of non-manager 
		// type are left. (See FDBase::SetManager())
		int MainLoop();
		
		// Quit the main loop started with MainLoop(); 
		// val MUST BE >=0 and is returned 
		void Quit(int val)
			{  quitval=val;  }
		
		// Does nothing; just returns specified value: 
		int noop(int);
};


#endif /* _HLIB_FDManager_H */
