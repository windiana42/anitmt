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

class FDCopyBase : 
	#if __GNUG__ < 3
	public LinkedListBase<FDCopyBase>
	#else
	private LinkedListBase<FDCopyBase>
	#endif
{
	friend class LinkedList<FDCopyBase>;
	friend class FDCopyManager;
	public:
		typedef FDCopyManager::CopyInfo CopyInfo;
		typedef FDCopyManager::ProgressAction ProgressAction;
		typedef FDCopyManager::ProgressInfo ProgressInfo;
		typedef FDCopyManager::StatusCode StatusCode;
		typedef FDCopyManager::CopyID CopyID;
		typedef FDCopyManager::CopyRequest CopyRequest;
		typedef FDCopyManager::ControlCommand ControlCommand;
		typedef FDCopyManager::copylen_t copylen_t;
	private:
		typedef FDCopyManager::MCopyNode MCopyNode;
		// List of copy requests, managed by FDCopyManager: 
		LinkedList<struct MCopyNode> rlist;
	protected:
		// Called upon variuos conditions (completion of copy 
		//  request / error condition): 
		// See struct CopyInfo (cpmanager.h) for details. 
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
		
		// Central routine: Give FDCopyManager a copy job: 
		// CopyRequest: stores all the needed information and tuning 
		//    parameters for the copy request. 
		//    NOTE: *req is modified: The iobufsize/thresholds are 
		//          set to proper values if you did not and the errval 
		//          is set. If all that went throuh without an error, 
		//          the CopyRequest structure is copied and stored in 
		//          a list maintained by FDCopyManager. 
		// FDBase: Pass a pointer to *this if your class is derived 
		//    from FDBase. This simply does a PollFD(srcfd/destfd,0) 
		//    to make sure that the fd is not polled by the calling 
		//    FDBase-derived class. Make sure that you do NOT poll 
		//    for the fd you are currently copying! As a further 
		//    feature, the original fd events are restored before 
		//    the last call to cpnotify(). 
		CopyID CopyFD(CopyRequest *req,FDBase *fdb)
			{  return(cpmanager()->CopyFD(this,req,fdb));  }
		
		// CCTerm -> terminate the request: no more input is read, but the 
		//           buffer is written; cpnotify() [SCTerm] gets called 
		//           immediately by CopyControl() 
		// CCKill -> kill request (cpnotify() [SCKilled] gets called)
		//           immediately stops request, buffer gets lost 
		// CCStop -> stop request; must be started again using CCCont
		// CCCont -> start a stopped request
		//           Note that the timeout timer(s) (if any) is not affected 
		//           by CCStop/CCCont. If you stop a request it can time 
		//           out just as a running one. 
		// Return value: 
		//   0 -> success
		//   1 -> already stopped/running (CCStop/CCCont)
		//  -2 -> cpid illegal (i.e. NULL) 
		//  -3 -> illegal ControlCommand
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
