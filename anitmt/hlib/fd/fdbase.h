/*
 * fdbase.h
 * 
 * Header containing class FDBase, a base class for 
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

#ifndef _HLIB_FDBase_H_
#define _HLIB_FDBase_H_ 1

#include <hlib/prototypes.h>

#if HAVE_SYS_POLL_H
# include <sys/poll.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>   /* shutdown() */
#endif
#if HAVE_SIGNAL_H
# include <signal.h>
#endif

/*
 * CHANGES / WHAT TO CHECK ON TROUBLE: 
 * - fdnotify(int fd,short events,short revents) replaced by 
 *   fdnotify(FDInfo *)
 * - timeralarm -> timerNOTIFY
 * - timeralarm(int idx,const HTime *current) replaced by 
 *   timernotify(TimerInfo *)
 * - InstallTimer() and KillTimer() changed. 
 * - less virtual signal handlers (aka sigchild); use signotify()
 * - virtual function may take const HTime* instead of HTime*
 *   HINT: try -Woverloaded-virtual
 * - check SA_RESTART (normally don't use)
 */

/*
 * NOTE: All FDBase (and derived) classes must be allocated via 
 *       operator new or using LMalloc() and a call to the constructor 
 *       as they get automatically deleted when the FDManager 
 *       is destroyed. 
 * NOTE: BEFORE CREATING THE FIRST FDBase object, you must have 
 *       created a FDManager object (e.g. be in main()) which 
 *       may be allocated on the stack or on the heap. 
 *       FDBase classes need this object to register there. 
 * NOTE: constructors should take the argument int *failflag 
 *       and decrement it if it is non-NULL and something fails. 
 *       If it is NULL, the constructor should call 
 *       ConstructorFailedExit() to quit. 
 *    If you call the constructor, pass an integer initialized to 0 
 *    and if it is non-zero after construction, it failed. 
 * EXAMPLE: 
int derivedclass::derivedclass(int *failflag=NULL) : 
	FDBase(failflag)
{
	int retval=0;
	if( init_something() == failed )
	{  --retval;  }
	
	if(failflag)
	{  *failflag+=retval;  }  // descrases as retval<0. 
	else if(retval)
	{  ConstructorFailedExit();  }
}
 */


// Values for align argument at timer calls. 
#define FDAT_PercentMask     0x0fff
#define FDAT_FirstLater      0x1000
#define FDAT_FirstEarlier    0x2000
#define FDAT_AlignToShorter  0x4000

class FDManager;
class FDManager::FDBList;
struct FDManager::TimerNode;

class HTime;

class FDBase
{
	friend class FDManager;
	friend class FDManager::FDBList;
	friend class FDCopyBase;
	public:
		typedef FDManager::SigInfo SigInfo;
		typedef FDManager::FDInfo FDInfo;
		typedef FDManager::TimerInfo TimerInfo;
		typedef FDManager::TimerID TimerID;
		typedef FDManager::PollID PollID;
	private:
		
		short int deleted;   // NEVER CHANGE THIS. 
		short int manager_type;   // NEVER CHANGE THIS. 
		
		// Not locked: -1; else locked and number of `deleted' entries. 
		int timerlock,fdslock;
		
		// Called by FDManager, our friend: 
		FDManager::TimerNode *AllocTimerNode(long msec,const void *dptr);
		FDManager::FDNode *AllocFDNode(int fd,short events,const void *dptr);
		
		inline void AddTimerNode(FDManager::TimerNode *n); 
		inline void AddFDNode(FDManager::FDNode *n);
		
		// Dequeue and free (if not locked): 
		void DeleteTimerNode(FDManager::TimerNode *n);
		void DeleteFDNode(FDManager::FDNode *n);
		
		// Return number of deleted nodes: 
		// DO NOT CLEAR WHEN LOCKED!
		int ClearTimers();
		int _ClearFDs(int close_them,int *pollfds);  // FDBase internal use only 
		int ClearFDs(int close_them)
			{  int dummy; return(_ClearFDs(close_them,&dummy));  }
		
		// Lock/unlock lists (affects deleting only) 
		void _UnlockTimers();
		void _UnlockFDs();
		// Inlined ones: 
		void LockTimers()  {  timerlock=0;  }
		void UnlockTimers()
			{  if(timerlock>0) _UnlockTimers(); else timerlock=-1;  }
		void LockFDs()  {  fdslock=0;  }
		void UnlockFDs()
			{  if(fdslock>0) _UnlockFDs(); else fdslock=-1;  }
		
		// Get shortest timer (or rechecks the list if needed): 
		// NULL -> no timer enabled 
		struct FDManager::TimerNode *ShortestTimer()
			{  return(sh_timer_dirty ? _GetShortestTimer() : sh_timer);  }
		// Internally used: 
		struct FDManager::TimerNode *_GetShortestTimer();
		
		// Important to call these to keep sh_timer up to date: 
		inline void _DoResetTimer(FDManager::TimerNode *i);
		void _MsecLeftChanged(FDManager::TimerNode *i,long old_msec_left);
		
		inline int _UnpollFD(PollID pollid)
			{  return(fdmanager()->UnpollFD(this,pollid));  }
		inline int _CloseFD(int fd,PollID pollid)
		{
			_UnpollFD(pollid);
			int rv;  do {  rv=close(fd);  }  while(rv<0 && errno==EINTR);
			return(rv);
		}
		inline int _ShutdownFD(int fd,PollID pollid)
		{
			int rv;  do {  rv=shutdown(fd,2);  }  while(rv<0 && errno==EINTR);
			return(_CloseFD(fd,pollid));
		}
		
		// Non-Null variants: pollid MAY NOT BE NULL: 
		inline int _FDfdNN(PollID pollid) const
			{  return(((FDManager::FDNode*)pollid)->fd);  }
		inline const void *_FDDPtrNN(PollID pollid) const
			{  return(((FDManager::FDNode*)pollid)->dptr);  }
		inline short _FDEventsNN(PollID pollid) const
			{  return(((FDManager::FDNode*)pollid)->events);  }
		
		// The actual `data' fields: 
		struct FDManager::TimerNode *timers;  // timer list 
		struct FDManager::FDNode *fds;        // poll list
		// Never access sh_timer directly, use ShortestTimer()
		struct FDManager::TimerNode *sh_timer;  // shortest timer or NULL
		int sh_timer_dirty;   // sh_timer NULL but has to be re-checked
	protected:
		// Returns a value >0 if DeleteMe() was called. 
		int DeletePending()  const  {  return(deleted);  }
		
		// A timer expired. 
		// TimerInfo memers: 
		// - tid: timer ID of timer which expired 
		// - current: current time (semi-current actually, the same 
		//            for all calls done at once); use that for 
		//            timeouts. 
		// - dptr: custom data pointer associated with the timer 
		// - pollid: PollID of that fd. See FDPollID() below for more 
		//           info. 
		// NOTE: This function MUST NOT delete other 
		//       FDBase classes which may receive a timernotify(). 
		//       It may however call DeleteMe() on other FDBases. 
		// The content of current may be changed without affecting 
		// the time management (FDManager). 
		// TimerInfo is non-const to allow to return data in future 
		// revisons. 
		// Return value: 0 for now; held for future expansion. 
		virtual int timernotify(TimerInfo *)  {  return(0);  }
		
		// poll() returned something for fd. 
		// Members of FDInfo: 
		// - fd: file descriptor it's all about
		// - events,revents: see poll(2) (POLLIN,POLLOUT,...)
		// - dptr: custom data pointer associated with the fd (if set 
		//         using PollFD()) 
		// - current: semi-current time;it's the time the last call to 
		//         poll() returned; use it for timeouts. 
		// NOTE: This function may add/change/delete timers and FDs 
		//       (using PollFD()). It MUST NOT delete other 
		//       FDBase classes which may receive a fdnotify(). 
		//       It may however call DeleteMe() on other FDBases. 
		// FDInfo is non-const and may be modified without side effects 
		// (to allow to return data in future revisons). 
		// Return value: 0 for now; held for future expansion. 
		virtual int fdnotify(FDInfo *)  {  return(0);  }
		
		// Caught signal (called synchroniously): 
		// Called for each signal a handler was installed for 
		// (including HUP, INT, USR1, USR2, PIPE, TERM, CHLD, WINCH). 
		// If this function returns 1 and the signal was one of 
		// SIG{HUP,INT,TERM}, then one of the older virtual functions 
		// below is also called. 
		virtual int signotify(const SigInfo *)  {  return(1);  }
		
		virtual void sighup()    {  }
		virtual void sigint()    {  }
		virtual void sigterm()   {  }
	public:  _CPP_OPERATORS_FF
		// IF USING THIS FUNCTION: NOTE: RETURNED failflag != 0 
		// INDICATES AN ERROR. IMMEDIATELY DELETE THE CLASS. 
		FDBase(int *failflag=NULL);
		virtual ~FDBase();
		
		// Use this to get the FDManager: 
		inline FDManager *fdmanager() const  {  return(FDManager::manager);  }
		
		/* TIMERS */
		// Use InstallTimer() to install a new timer, 
		// UpdateTimer() to update (and reset) an existing timer, 
		// ResetTimer() to just reset it. 
		// If you install a new timer, you get back a TimerID (actually 
		// a pointer, so doing call by value on TimerIDs is okay). 
		// This TimerID is a handle for the timer which has to be 
		// specified whenever you do something with the timer. 
		// A TimerID of NULL (0) normally means `error'. 
		// You may set msec=-1 to disable a timer or install a disabled 
		// timer. This has the advantage that enabling with a specified 
		// time period and disabling of timers will not fail because 
		// of malloc() failure. 
		//   [ FOR DISABLED TIMERS, USE msec=-1 AND NO DIFFERENT VALUE. ]
		// align specifies if the first timer call (timernotify()) 
		//    may come earlier or later in order to synchronize the 
		//    timer with other timers. A value of 0 switches alignment off. 
		// align is one or more of the FDAT_* flags below OR'ed with a 
		//    value in range 0...99 (specifying how many percent of the 
		//    timer interval the first timer may be later or earlier; 
		//    100% later means: first call after two intervals; 
		//    100% earlier means: first call immediately)
		//    FDAT_FirstLater: first call may come later, but not more than 
		//        (align & FDAT_PercentMask) percent later. 
		//    FDAT_FirstEarlier: first call may come earlier, but not more 
		//        than (align & FDAT_PercentMask) percent earlier. 
		//    FDAT_FirstLater | FDAT_FirstEarlier: first call may come later 
		//        or earlier, but may not more than (align & FDAT_PercentMask) 
		//        percent later/earlier. 
		//    FDAT_AlignToShorter: if set: only align timer to timers that 
		//        have less time left than the interval time of this one. 
		//        This is useful for timers that get called only once or 
		//        twice. It makes no sense aligning a 200ms timer to a 
		//        a timer having 30 minutes left until it expires if the 
		//        200ms timer gets just called several times. 
		//    The timer to which the new timer is aligned is always the 
		//    one that results in minimum length change of the first interval. 
		// Timers with interval msec=0 or -1 cannot be aligned so the align 
		// value is ignored. 
		// InstallTimer: Return value: 
		//     NULL -> failed to allocate timer node
		//   !=NULL -> TimerID of new timer
		// UpdateTimer: Return value:
		//     0 -> timer updated
		//    -2 -> specified timer (TimerID) not valid (NULL)
		//   Timer update for an existing (valid) TimerID never fails. 
		TimerID InstallTimer(long msec,int align,const void *dptr=NULL)
			{  return(fdmanager()->InstallTimer(this,msec,align,dptr));  }
		// Update timer interval (automatically does reset). 
		int UpdateTimer(TimerID tid,long msec,int align)
			{  return(fdmanager()->UpdateTimer(this,tid,msec,align));  }
		// Change data pointer associated with timer: 
		int UpdateTimer(TimerID tid,void *dptr)
			{  if(!tid)  return(-2);  ((FDManager::TimerNode*)tid)->dptr=dptr;  return(0);  }
		// Re-set timer; next timernotify() will occure in msec 
		// milliseconds (as specified with InstallTimer()/UpdateTimer()).
		// Return: 0 -> OK; -2 -> TimerID invalid; 1 -> timer inerval<=0. 
		int ResetTimer(TimerID tid,int align)
			{  return(fdmanager()->ResetTimer(this,tid,align));  }
		// Kill timer/kill all timers 
		// Return: 0 -> OK; -2 -> TimerID invalid
		int KillTimer(TimerID tid)
			{  return(fdmanager()->KillTimer(this,tid));  }
		void KillAllTimers()
			{  fdmanager()->KillAllTimers(this);  }
		
		// Query timer parameters: 
		// Note for hackers: NEVER CHANGE THE VALUE -3. IT IS MAGIC!!!
		// Timer interval in msec; -1 for disabled, -3 if tid==NULL. 
		long TimerInterval(TimerID tid) const
			{  return(tid ? ((FDManager::TimerNode*)tid)->msec_val : -3);  }
		// msec until next call to timernotify() 
		// or -1 if disabled, -3 if tid==NULL. 
		long TimerLeft(TimerID tid) const
			{  return(tid ? ((FDManager::TimerNode*)tid)->msec_left : -3);  }
		// Get data pointer of timer (attached custom data; void *dptr). 
		const void *TimerDPtr(TimerID tid) const
			{  return(tid ? ((FDManager::TimerNode*)tid)->dptr : NULL);  }
		
		/* FILE DESCRIPTORS */
		// Specified fd should be polled: 
		// events is OR-combination of POLLIN, POLLOUT, etc. 
		//    (see <sys/poll.h>); 
		//  NOTE: PollFD(fd,0) means: do not poll() for fd but keep the 
		//        fd entry in memory so that a following PollFD() call on 
		//        this fd will be faster and will not fail (no malloc() 
		//        necessary). 
		// Again: PollFD() is guaranteed NOT TO FAIL if you use it to 
		//        change the events entry of an already-existing fd list 
		//        entry (created by the first successful call to PollFD 
		//        for the specified FDBase (=*this) and file descriptor fd). 
		// Call UnpollFD(fd) to remove the fd entry from the poll list. 
		// ** Be sure you shutdown(), close() and UnpollFD(fd) 
		//    all opened fds in the desctructor. 
		// dptr is a custom data pointer associated with this fd and 
		//   also passed back by fdnotify. If you do not pass a dptr, it 
		//   is not changed (and if a new node is allocated, it defaults to 
		//   NULL). 
		// ret_id: if non-NULL: store PollID of newly created/updated fd at 
		//   passed pointer. NOTE: *ret_id is only valid if the return value 
		//   is 0 or 1 (no error), otherwise unchanged. 
		// Return value: 
		//     1 -> OK, fd entry updated. 
		//     0 -> OK, added new fd entry. 
		//    -1 -> malloc() failed. (BAD.)
		//    -2 -> fd<0
		int PollFD(int fd,short events=0)
			{  return(fdmanager()->PollFD(this,fd,events,NULL,NULL));  }
		int PollFD(int fd,short events,const void *dptr,PollID *ret_id=NULL)
			{  return(fdmanager()->PollFD(this,fd,events,&dptr,ret_id));  }
		// Same as PollFD() above but with PollID argument instead of 
		// PollFD. Thus, you can only use this version to CHANGE poll 
		// settings, NOT to allocate new ones. 
		// Return value: 
		//    1 -> OK, fd entry updated. 
		//    0 -> nothing to be done (or just dptr updated) 
		//   -2 -> pollid==NULL 
		int PollFD(PollID pollid,short events=0)
			{  return(fdmanager()->PollFD(this,pollid,events,NULL));  }
		int PollFD(PollID pollid,short events,const void *dptr)
			{  return(fdmanager()->PollFD(this,pollid,events,&dptr));  }
		int PollFDDPtr(PollID pollid,const void *dptr)
			{  if(!pollid)  return(-2);
				((FDManager::FDNode*)pollid)->dptr=dptr;  return(1);  }
		// Special version of PollFD(PollID,events): 
		// Sets those events set in set_events and then clears those in 
		// clear_events. 
		// Return value: like PollFD() 
		// (-2 -> pollid==NULL; 1 -> events updated; 0 -> OK (unchanged))
		int FDChangeEvents(PollID pollid,short set_ev,short clear_ev)
			{  return(fdmanager()->FDChangeEvents(this,pollid,set_ev,clear_ev));  }
		// Return value: 
		//    0 -> OK; 
		//    1 -> nothing to un-poll: no such fd for *this. 
		int UnpollFD(int fd)
			{  return(_UnpollFD(FDPollID(fd)));  }
		int UnpollFD(PollID &pollid)
			{  int rv=_UnpollFD(pollid);  pollid=NULL;  return(rv);  }
		// Return value: that of close(fd). 
		int CloseFD(int fd)
			{  return(fd<0 ? 0 : _CloseFD(fd,FDPollID(fd)));  }
		int CloseFD(PollID &pollid)
			{  int rv=_CloseFD(FDfd(pollid),pollid);  pollid=NULL;  return(rv);  }
		int ShutdownFD(int fd)  // shutdown & close
			{  return(fd<0 ? 0 : _ShutdownFD(fd,FDPollID(fd)));  }
		int ShutdownFD(PollID &pollid)
			{  int rv=_ShutdownFD(FDfd(pollid),pollid);  pollid=NULL;  return(rv);  }
		
		// Get PollID associated with specified fd: 
		// Note: one PollID corresponds to one fd. A PollID is created when 
		// you call PollFD() and is deleted when you call UnpollFD() (or the 
		// class gets destroyed). Internally, a PollID is just a pointer to 
		// the apropriate FDNode, so use it with care; illegal PollIDs 
		// (dangling pointers) can hardly be detected. 
		PollID FDPollID(int fd) const;
		// The other way round: get fd from PollID (or -1): 
		int FDfd(PollID pollid) const
			{  return(pollid ? _FDfdNN(pollid) : (-1));  }
		// Query FD data (only fds that this FDBase is polling for): 
		// Get custon data pointer associated with fd/PollID or NULL: 
		const void *FDDPtr(int fd) const
			{  return(FDDPtr(FDPollID(fd)));  }
		const void *FDDPtr(PollID pollid) const
			{  return(pollid ? _FDDPtrNN(pollid) : NULL);  }
		// Get events mask for this fd/PollID or -1 in case of invalid fd: 
		short FDEvents(int fd) const
			{  return(FDEvents(FDPollID(fd)));  }
		short FDEvents(PollID pollid) const
			{  return(pollid ? _FDEventsNN(pollid) : (-1));  }
		
		// Like exit() for this FDBase: *this gets 
		// destructed and if free_me==1 also LFree()'ed. 
		// NOTE THAT ALL FDBase classes should be allocated 
		// via new (using LMalloc()/constructor), so that 
		// free_me should always be 1. 
		void DeleteMe(int free_me=1)
			{  fdmanager()->Unregister(this,free_me ? 3 : 2);  }
		
		// Tell the FDManager that we are a manager class 
		// (mtype!=FDManager::MT_None) or no manager class (MT_None; default). 
		// FDManager quits the main loop if no FDBases of non-manager 
		// type are left. 
		// Please use FDManager::MT_Other unless you know what you're doing. 
		// Returns: 0 -> success; -1 -> failed (not registered) 
		int SetManager(FDManager::ManagerType mtype)
			{  return(fdmanager()->SetManager(this,mtype));  }
		
		// Checks if that signal is queued for delivery: 
		// Returns number of queued signals of type sig. 
		int SigPending(int sig)
			{  return(fdmanager()->SigPending(sig));  }
};

// These functions are here because when compiling with gcc-2.95 you get 
// problems when linking (unresolved symbols) if these are put into 
// fdmanager.cc. 
inline void FDBase::AddFDNode(FDManager::FDNode *n)
{
	n->next=fds;
	if(fds)
	{  fds->prev=n;  }
	fds=n;
}

inline void FDBase::_DoResetTimer(FDManager::TimerNode *i)
{
	i->msec_left=i->msec_val;  // reset timer
	sh_timer_dirty=1;
	fdmanager()->TimeoutChange();
}			

inline void FDBase::AddTimerNode(FDManager::TimerNode *n)
{
	n->next=timers;
	if(timers)
	{  timers->prev=n;  }
	timers=n;
	
	// Keep sh_timer up to date: 
	// This does not have to be done here as the calling function will 
	// call fdb->_MsecLeftChanged() after AddTimerNode() [and possibly 
	// AlignTimer()]. 
	// NOTE: InstallTimer() depends on that we do NOT check sh_timer here. 
	/*if(!sh_timer_dirty)
	{
		if(!sh_timer || sh_timer->msec_left>n->msec_left)
		{  sh_timer=n;  fdmanager()->TimeoutChange();  }
	}*/
}

#endif /* _HLIB_FDBase_H_ */
