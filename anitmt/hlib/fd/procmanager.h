/*
 * procmanager.h
 * 
 * Header containing class ProcessManager, a class for 
 * process (task) management which works in cooperation 
 * with classes derived from class ProcessBase. 
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

#ifndef _HLIB_ProcessManager_H_
#define _HLIB_ProcessManager_H_ 1

// This also includes prototypes.h: 
#include "intprocbase.h"
#include <hlib/linkedlist.h>

class ProcessBase;

// NOTE: To be able to use classes derived from ProcessBase, 
//       you have to allocate ONE class of type ProcessManager. 
// NOTE: ProcessManager depends on FDBase, so you need to allocate 
//       an FDManager first. 
// !!NOTE!! DO NOT USE wait3(),wait4(),waitpid() ETC IF YOU ARE USING 
//          ProcessManager UNLESS YOU REALLY KNOW WHAT YOU ARE DOING. 
// !!NOTE!! When using this class, you should MAKE SURE THAT YOU 
//          REGISTER ALL filedescriptors at FDManager which must be 
//          closed when executing a process. FDs not to be closed 
//          should also be registered unless they were just opened 
//          just for the purpose of using them for the new process. 

class ProcessManager : 
	private FDBase, 
	public  InternalProcessBase 
{
	public:
		/* static global manager */
		static ProcessManager *manager;
	private:
		struct Node : LinkedListBase<Node>  // one node corresponds to one process 
		{  _CPP_OPERATORS_FF
			pid_t pid;
			ProcessBase *pb;  // or NULL if unregistered
			HTime starttime;  // actually the time fork() returns in parent thread
			int flags;
			int pipe_fd;   // exec failure pipe fd or -1 if execution 
			               // succeeded (initially -2). 
			int errnoval;  // exec failure errno value
			inline Node(int *failflag=NULL);
			inline ~Node();
		};
		LinkedList<Node> procs;
		int nproc;  // number of processes in list
		
		// This contains all ProcessBases with a special_flags!=0. 
		LinkedList<ProcessBase> specialpb;
		
		// TERM-KILL-scheduling: 
		TimerID tid;
		int tktimer_running;
		long term_kill_msec;  // defaults to 4000
		
		// Environment: 
		char *const *envp;
		
		// Zombie check timer: 
		TimerID zmbchk;
		inline void _ScheduleZombieCheck()
			{  UpdateTimer(zmbchk,0,0);  }
		
		// Limits: 
		int limitproc;   // -1 -> no limit
		
		inline void _DeleteNode(Node *n,int may_have_pipefd_set=0);  // dequeue and free
		Node *_FindNode(pid_t pid);
		void _Clear();
		
		int _ReadPipeCode(Node *n);
		void _CheckExecFailure(Node *n,ProcStatus *ps,int estatus);
		void _AddToProcStatus(ProcStatus *ps,struct rusage *rus);
		void _FillProcStatus(Node *n,ProcStatus *ps,int pid,int status,struct rusage *rus);
		void _FillProcStatus(Node *n,ProcStatus *ps,const SigInfo *sig);
		int _CheckSigInfo(const SigInfo *sig);
		
		void _ZombieCheck();
		int _TermProcess(Node *n);
		void _ScheduleKill(Node *n);
		void _SuccessfulExecution(Node *n);
		void _TellProcBase(Node *n,ProcStatus *ps);
		
		// overriding vitual: 
		int signotify(const SigInfo *);
		int timernotify(TimerInfo *);
		int fdnotify(FDInfo *);
	public:  _CPP_OPERATORS_FF
		// Pass the environment as passed to main here so that 
		// this environment can be passed on to the started process. 
		// NOTE: setenv() will probably not change envp[], so if 
		// you want to call a process with a different environment, 
		// tweak the environment value as passed to StartProcess(). 
		ProcessManager(char *const envp[],int *failflag=NULL);
		~ProcessManager();
		
		// Set the TERM-KILL-delay in msec: 
		// Affects (=resets) currently running timer. 
		void TermKillDelay(long msec);
		
		// Limit the number of simultanious running processes started 
		// by ProcessManager. A limit of -1 means `no limit'. 
		void LimitProcesses(int limit=-1);
		
		// Returns number of process nodes (that is number of currently 
		// running processes started by us). 
		int NProc() const  {  return(nproc);  }
		
		char *const *GetEnv()  {  return(envp);  }
		
		// See class ProcessBase (procbase.h) for a description: 
		pid_t StartProcess(ProcessBase *pb,
			const ProcPath &path,
			const ProcArgs &args,
			const ProcMisc &misc,
			const ProcFDs &pfds,
			const ProcEnv &env);
		
		// See class ProcessBase (procbase.h) for a description: 
		int TermProcess(ProcessBase *pb,pid_t pid);
		
		// Called by ProcessBase-classes upon desctruction: 
		void Unregister(ProcessBase *pb);
		
		// Special register function. 
		// If *pb is already registered, only the special flags will be 
		// updated. 
		// special_flags: OR'ed value: 
		//    AlwaysNotify -> get notified for each terminated process
		// Return value: 
		//   0 -> OK
		int SpecialRegister(ProcessBase *pb,int special_flags);
		
		// Does nothing; just returns specified value: 
		int noop(int);
};

#endif /* _HLIB_ProcessManager_H */
