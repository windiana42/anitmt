/*
 * cpbase.h
 * 
 * Header containing class FDCopyBase, a base class for copying 
 * from and to file descriptors which works in cooperation 
 * with class FDCopyManager. 
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

#ifndef _HLIB_FDCopyBase_H_
#define _HLIB_FDCopyBase_H_ 1

#include <hlib/prototypes.h>


class HTime;

class FDCopyBase : public _FDCopyNamespace, 
	#if __GNUG__ < 3
	public LinkedListBase<FDCopyBase>
	#else
	private LinkedListBase<FDCopyBase>
	#endif
{
	friend class LinkedList<FDCopyBase>;
	friend class FDCopyManager;
	private:
		typedef FDCopyManager::MCopyNode MCopyNode;
		// List of copy requests, managed by FDCopyManager: 
		LinkedList<struct MCopyNode> rlist;
	protected:
		// Called upon variuos conditions (completion of copy 
		//  request / error condition): 
		// See struct CopyInfo (cpmanager.h) for details. 
		// NOTE: Flags are only restored if recv_fdnotify=0 AND fdb!=NULL. 
		// If CSFinal is set in CopyInfo::cstate, and the poll events 
		//  of the src/dest fd were saved when calling CopyFD() 
		//  (this is the case when the FDBase pointer was specified), 
		//  then the original events were restored _before_ calling 
		//  cpnotify() so that you can work with them again. 
		// Note that you get notified when input ends (i.e. SCRead0, 
		//  SCErrRead, SCLimit, ...) and when writing is finally done. 
		//  So, the request is not done when input ends (and thus this 
		//  notification will not have SCFinal set). However, while 
		//  the buffer is written, the input end flag stays set 
		//  (i.e. you may receive SCRead0|SCOutPipe|SCFinal). 
		//  Note that at the time cpnotify() is called as reading is 
		//  stopped (check with SCEOI), the original flags of the 
		//  input fd are already restored in case they were saved. 
		// Note also that the errno value holds the correct error 
		//  code only the fist time, SCErrRead is set. Subsequent 
		//  calls to cpnotify() (while flusing the buffer) still have 
		//  SCErrRead set (as said above), but errno is 0 unless 
		//  some other error (SCWriteErr) is set. 
		// For OM_FdBuf2FdBuf, the behavior is slightly different: 
		//  You get notified once about the ending input and once 
		//  about the ending output but the flags are not persistent. 
		//  The first of these notifications does not have CSFinal 
		//  set but the last one has. 
		//  Upon receiving input error/end, the request in changed 
		//   into an OM_Buf2Fd and behaves like such one. 
		//  Upon receiving output error/end, the request is changed 
		//   info an OM_Fd2Buf and behaves like such one. 
		//  Especially note that in this case those flags which are 
		//  no longer controlled by FDCopyManager can be modified by 
		//  the client USING CPPollFD() AND NOT USING FDBase::PollFD(). 
		//  (In case you are using recv_fdnotify.) 
		// Return value: currently ignored; use 0. 
		virtual int cpnotify(CopyInfo *)  {  return(0);  }
		
		// Called to notify client on progress made: 
		// Return value: currently ignored; use 0. 
		virtual int cpnotify(ProgressInfo *)  {  return(0);  }
	public:  _CPP_OPERATORS_FF
		FDCopyBase(int *failflag);
		virtual ~FDCopyBase();
		
		// Use this to get the FDCopyManager: 
		inline FDCopyManager *cpmanager()  {  return(FDCopyManager::manager);  }
		
		// Get dptr asociated with passed CopyID: 
		inline void *FDCPDPtr(CopyID cpid)
		{  return((!cpid || ((MCopyNode*)cpid)->is_dead) ? NULL : ((MCopyNode*)cpid)->req.dptr);  }
		
		// Central routine: Give FDCopyManager a copy job: 
		// CopyRequest: stores all the needed information and tuning 
		//    parameters for the copy request. 
		//    NOTE: *req is modified: The iobufsize/thresholds are 
		//          set to proper values if you did not and the errval 
		//          is set. If all that went throuh without an error, 
		//          the CopyRequest structure is copied and stored in 
		//          a list maintained by FDCopyManager. 
		// FDBase: Consider passing a pointer to *this if your class is 
		//    derived from FDBase. 
		//    This does the following: 
		//    If recv_fdnotify=0, then simply PollFD(srcfd/destfd,0) 
		//      is called to make sure that the fd is not polled by the 
		//      calling FDBase-derived class. The original events are 
		//      restored before the last call to cpnotify(). 
		//    If recv_fdnotify>0, then only the apropriate flags 
		//      controlled by FDCopyManager are changed (i.e. if you 
		//      do FD -> buf copying, srcfd's POLLIN is controlled and 
		//      srcfd's POLLOUT stays untouched.) The original flags 
		//      are NOT saved and restored. 
		// NOTE: Make sure that you do NOT poll for the fd(s) you are 
		//    currently copying! When using recv_fdnotify>0, you must 
		//    use FDCopyBase's CPPollFD(). 
		// NOTE!!: You may NOT change the FDInfo::dptr when fdnotify() 
		//    gets called by FDCopyManager. You can chech this by calling 
		//    FDCopyBase::CurrentCopyID(): It returns NULL if not called 
		//    by FDCopyManager and the CopyID of the corresponding copy 
		//    job if called by FDCopyManager. 
		//    DO NO OPERATION ON THE PASSED FD/POLLID. 
		CopyID CopyFD(CopyRequest *req,FDBase *fdb)
			{  return(cpmanager()->CopyFD(this,req,fdb));  }
		
		// May be called during fdnotify() to check if the call to 
		// fdnotify() came from FDManager (NULL) or from FDCopyManager 
		// (corresponding CopyID or NULL): 
		// To really check if called from FDCopyManager, use the 
		// flag called_from_cpman: set to 1 if called from FDCopyManager
		// NOTE: The calls to fdnotify() are made at the end of the 
		//       fdnotify() in FDCopyManager ad thus AFTER corresponging 
		//       calls to cpnotify(). Note that after SCFinal was set, 
		//       the CopyID is now loger valis AND MAY NOT BE USED. 
		//       [Actually, CurrentCopyID() will return NULL if it no 
		//       longer exists
		CopyID CurrentCopyID(int *called_from_cpman=NULL)
			{  return(cpmanager()->CurrentCopyID(called_from_cpman));  }
		
		// CCTerm -> terminate request: 
		//    cpnotify() [SCKilled] gets called 
		//    Fd2Fd: no more input is read, but the buffer is written 
		//    Fd2Buf, Buf2Fd -> like CCKill 
		//    FdBuf2FdBuf: invalid (retval -4)
		// CCKill, CCKillI, CCKillO -> kill the request: 
		//    cpnotify() [SCKilled] gets called immediately by CopyControl() 
		//    Fd2Fd: CCKill -> kill request, buffer gets lost
		//           CCKillI, CCKillO -> invalid (retval -4)
		//    Fd2Buf: CCKill, CCKillI -> kill request
		//            CCKillO: invalid (retval -4)
		//    Buf2Fd: CCKill, CCKillO -> kill request
		//            CCKillI -> invalid (retval -4)
		//    FdBuf2FdBuf: 
		//           CCKill -> kill request in both directions
		//           CCKillO -> stop write part, mutate task into Buf2Fd 
		//           CCKillI -> stop read part mutate request into Fd2Buf 
		// CCStop -> stop request; must be started again using CCCont
		// CCCont -> start a stopped request
		//           Note that the timeout timer(s) (if any) is not affected 
		//           by CCStop/CCCont. If you stop a request it can time 
		//           out just as a running one. 
		// NOTE: If you use recv_fdnotify, then this will only stop the 
		//       part of the communication controlled by the FDCopyManager. 
		//       Use CPPollFD() to stop the other part too, if desired. 
		// Return value: 
		//   0 -> success
		//   1 -> already stopped/running (CCStop/CCCont)
		//  -2 -> cpid illegal (i.e. NULL) 
		//  -3 -> illegal ControlCommand
		//  -4 -> control command cannot be used for this request
		//        (e.g. CCTermI for opmode Buf2Fd)
		int CopyControl(CopyID cpid,ControlCommand cc)
			{  return(cpmanager()->CopyControl(cpid,cc));  }
		
		// Add CopyControl(CopyID, etc..) to
		// - change timeouts
		// - change len (for fd2fd)
		// - ...
		
		// Query progress; may always be called. 
		// Progress info is saved under the passed pointer. 
		// ProgressInfo::fdtime is set to NULL. 
		// Return value: 
		//   0 -> success (ProgressInfo stored)
		//  -2 -> cpid illegal (i.e. NULL)
		int QueryProgress(CopyID cpid,ProgressInfo *save_here)
			{  return(cpmanager()->QueryProgress(cpid,save_here));  }
};

#endif  /* _HLIB_FDCopyBase_H_ */
