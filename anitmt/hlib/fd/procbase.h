/*
 * procbase.h
 * 
 * Header containing class ProcessBase, a base class for 
 * process (task) management which works in cooperation 
 * with class ProcessManager. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_ProcessBase_H_
#define _HLIB_ProcessBase_H_ 1

#include <sys/poll.h>

// This also includes prototypes.h: 
#include "intprocbase.h"

// NOTE: ProcessBase needs ProcessManager which depends on FDManager; 
//       see procmanager.h for details. 

class ProcessBase : 
	public InternalProcessBase,
	#warning should be private; try with gcc-3: 
	public LinkedListBase<ProcessBase>
{
	friend class ProcessManager;
	private:
		// Maintained by ProcessManager; stores special flags 
		// set via SpecialRegister(). 
		int special_flags;
	protected:
		// Gets called when a process started by this class exited 
		// (or got stopped/killed). See procmanager.h for ProcStatus. 
		// Note: procmanager()->NProc() is decremented AFTER procnotify() 
		//       returns. 
		// NOTE: If a child is executed successfully and running, 
		//       procnotify with action PSExecuted (detail PSSuccess) is 
		//       called. Only the fields starttime,pid,started_by will 
		//       be set correctly (apart from action and detail). 
		//       NOTE that if the child was executed successfully but 
		//       exits very quickly, then PSExecuted may be omitted and 
		//       you immediately receive PSDead/PSExited. The same issue 
		//       may happen if the child is executed and immediately killed 
		//       afterwards. 
		//       SO: NEVER relay on getting action=PSExecuted. 
		//   ... It may even get worse and you receive a procnotity telling 
		//       you that the process was stopped before you actually get 
		//       PSExecuted. This is the case if something happens to our 
		//       child fork() before execve() was called. 
		virtual void procnotify(const ProcStatus * /*pstat*/) { }
	public:  _CPP_OPERATORS_FF
		ProcessBase(int * /*failflag*/=NULL) : 
			LinkedListBase<ProcessBase>()
			{  special_flags=0;  }
		virtual ~ProcessBase()
			{  procmanager()->Unregister(this);  }
		
		// Never use ProcessManager::manager if you can avoid it. 
		// Use this function instead. 
		inline ProcessManager *procmanager()  {  return(ProcessManager::manager);  }
		
		// This is the function to call if you want to start a process. 
		// path: Path to the file to execute; may also specify name and 
		//       search path. 
		//       (name is treated as path if it begins with `/')
		// args: Arguments to be passed to the program. You should set 
		//       the zero-arg to the program name. 
		// misc: flags, chroot dir, working dir, nice value, UID, GID; 
		//       see class ProcMisc in procmanager.h. 
		// pfds: File descriptors to be `passed' (redirected) to the 
		//       process. 
		// env: environment to be passed to the process. 
		// Return value: 
		//    >0 -> PID of the started process 
		//    -1 -> LMalloc() failed 
		//    -2 -> illegal flags set 
		//    -3 -> path allocation failed (during ProcPath::ProcPath(..))
		//    -4 -> args allocation failed (during ProcArgs::ProcArgs(..))
		//    -5 -> env allocation failed (during ProcEnv::ProcEnv(..))
		//    -6 -> access(X_OK) not successful for (search)path: 
		//          program to execute not found. 
		//    -7 -> fork() failed (errno is set) 
		//    -8 -> process limit exceeded (see ProcManager::LimitProcesses())
		//    -9 -> pipe() failed (errno is set)
		//   -10 -> fcntl)() on pipe failed (errno is set)
		// Note: Other errors (FD stuff failed / execution failed) cannot 
		//       be returned here; you get them via procnotify(). 
		pid_t StartProcess(
			const ProcPath &path,
			const ProcArgs &args,
			const ProcMisc &misc=ProcMisc(),
			const ProcFDs &pfds=ProcFDs(),
			const ProcEnv &env=ProcEnv())
		{  return(procmanager()->StartProcess(this,path,args,misc,pfds,env));  }
		
		// TERM and then KILL a process. 
		// The delay between TERM and KILL is in range [tkd...2*tkd] with 
		// tkd being the value set with ProcessManager::TermKillDelay(). 
		// Return value: 
		//   0 -> OK, SIGTERM sent (errors when sending SIGKILL cannot be 
		//        detected but there is no reason why SIGKILL should fail 
		//        if SIGTERM did not other than that the process exited 
		//        (or executed an SUID binary). 
		//        procnotify() gets called if the process exits or 
		//        is killed, of course. 
		//  -2 -> error sending SIGTERM (see errno)
		//  -3 -> pid not found in process list; pid was not started by 
		//        ProcessManager or already exited. 
		//  -4 -> The process has the PF_OnlyKillOwner flag set and you 
		//        are not the owner. 
		int TermProcess(pid_t pid)
			{  return(procmanager()->TermProcess(this,pid));  }
		
		// Special registration function. 
		// See ProcessManager for details. 
		int SpecialRegister(int spec_flags)
			{  return(procmanager()->SpecialRegister(this,spec_flags));  }
};

#endif /* _HLIB_ProcessBase_H_ */
