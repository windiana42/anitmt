/*
 * intprocbase.h
 * 
 * Internal header containing class InternalProcessBase 
 * which is the base class for ProcessBase and ProcessManager. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_InternalProcessBase_H_
#define _HLIB_InternalProcessBase_H_ 1

#include <hlib/prototypes.h>

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>  /* for struct rusage */
#endif


class ProcessBase;
class RefStrList;


// This just server as some sort of namespace. 
struct InternalProcessBase
{
	enum  // See StartProcess() for a description. 
	{
		PF_KillPending1=0x001,  // INTRNAL
		PF_KillPending2=0x002,  // INTERAL
		PF_KillPending=(PF_KillPending1|PF_KillPending2),
		PF_OnlyKillOwner=0x004,
		PF_TermOnDestroy=0x008
		// Must update ProcessManager::StartProcess() when adding 
		// entries here. 
	};
	
	enum  // See SpecialRegister(); must be OR'ed. 
	{
		AlwaysNotify=0x01
	};
	
	enum PSAction
	{
		PSFailed=0,  // child execution failed
		PSDead,      // child no longer exists
		PSStopCont,  // child stopped/continued
		PSExecuted   // child was successfully executed
	};
	enum PSDetail
	{
		// DO NOT CHANGE THE PS*FAILED VALUES/ORDER; add new 
		// ones before _PSLASTFailed. 
		// *** detail for PSFailed: ***
		PSExecFailed=101,  // execve() failed
		PSDupFailed,       // dup2() failed
		PSChrootFailed,    // chdir(); chroot(".") failed [chroot dir]
		PSChdirFailed,     // chdor() failed [working dir]
		PSNiceFailed,      // nice() failed
		PSSetSidFailed,    // setsid() failed (and failure not ignored)
		PSSetGidFailed,    // setgid() failed
		PSSetUidFailed,    // setuid() failed
		PSFunctionFailed,  // user-supplied function returned error
		PSUnknownError,    // should never happen
		_PSLASTFailed,     // internal use only
		// *** detail for PSDead: ***
		PSExited=1,  // normal exit
		PSKilled,    // killed by signal
		PSDumped,    // killed and dumped core
		// *** detail for PSStopCont: ***
		PSStop,    // stopped
		PSCont,    // continued
		// *** detail for PSExecuted: ***
		PSSuccess  // success
	};
	struct ProcStatus
	{  _CPP_OPERATORS
		PSAction action;  // see above
		PSDetail detail;  // see above
		HTime starttime;  // when process was started
		HTime endtime;    // when SIGCHLD was caught
		HTime utime;   // time spent in user mode
		HTime stime;   // time spend in system mode
		int estatus;   // more info... 
			// estatus holds: 
			//  - process exit status if PSDead/PSExited
			//  - signal number if killed, i.e. 
			//    PSDead/{PSKilled,PSDumped}, PSStopCont 
			//  - custom function's return value if PSFailed/PSFunctionFailed
			//  - errno value for all other PSFailed details. 
		pid_t pid;     // PID of process                | wait3()/wait4(): 
		uid_t uid;     // UID of process                | (-1 -> unknown**)
		int sigcode;   // signal code (CLD_EXITED, ...) | (-1 -> unknown)
		struct rusage rus;  // for further info...
		ProcessBase *started_by;  // ProcessBase which started the process 
		               // or NULL if that ProcessBase was already destroyed. 
		               // (Only !=this if AlwaysNotify is set.)
		// ** NOTE that uid_t is unsigned so that you must check 
		//    if(uid!=uid_t(-1)) and NOT if(uid<0). 
	};
	
	struct ProcTimeUsage
	{  _CPP_OPERATORS
		HTime starttime;  // when started
		HTime uptime;  // time since startup
		HTime utime;   // time spent in user mode
		HTime stime;   // time spend in system mode
	};
	
	// Error return values of StartProcess(): 
	enum // StartProcessStatus
	{
		// VALUES MUST ALL BE NEGATIVE!
		SPS_LMallocFailed=-1,
		SPS_IllegalFlags=-2,
		SPS_PathAllocFailed=-3,
		SPS_ArgAllocFailed=-4,
		SPS_EvnAllocFailed=-5,
		SPS_AccessFailed=-6,   // access(X_OK) failed; program not found
		SPS_ForkFailed=-7,
		SPS_ProcessLimitExceeded=-8,
		SPS_PipeFailed=-9,   // pipe(2) failed
		SPS_FcntlPipeFailed=-10,  // fcntl(2) on pipe failed
		SPS_SearchPathError=-11,
		SPS_ArgListError=-12
	};
	
	// Used as arguments to StartProcess: 
	class ProcEnv
	{
		friend class ProcessManager;
		private:
			char *const *env;
			int freearray;
			ProcEnv(const ProcEnv &)  { }  // don't copy!
			void operator=(const ProcEnv &)  { }
		public:  _CPP_OPERATORS
			// NOTE: The args to the constructor are NEITHER COPIED nor 
			//       freed by the destructor. Make sure the pointers are 
			//       valid as long as the class exists. 
			ProcEnv();  // use environment as passed to ProcessManager(). 
			ProcEnv(char *const *_env)  // last: NULL-pointer!
				{  env=_env;  freearray=0;  }
			ProcEnv(const char *env0,...);  // NULL-terminated!
			~ProcEnv()
				{  if(freearray)  env=(char**)LFree((void*)env);  }
	};
	class ProcArgs
	{
		friend class ProcessManager;
		private:
			char *const *args;
			int freearray;
			ProcArgs(const ProcArgs &)  { }  // don't copy!
			void operator=(const ProcArgs &)  { }
		public:  _CPP_OPERATORS
			// NOTE: The args to the constructor are NEITHER COPIED nor 
			//       freed by the destructor. Make sure the pointers are 
			//       valid as long as the class exists. 
			ProcArgs(char *const *_args)  // last: NULL-pointer!
				{  args=_args;  freearray=0;  }
			ProcArgs(const char *args0,...);  // NULL-terminated!
			// Note that the passed arglist must exist as long as the 
			// ProcArgs exist (i.e. normally until StartProcess() returns).
			// (arglist may only contain '\0'-terminated RefStrings.)
			ProcArgs(const RefStrList *arglist);
			~ProcArgs()
				{  if(freearray)  args=(char**)LFree((void*)args);  }
	};
	class ProcPath
	{
		friend class ProcessManager;
		private:
			const char *path;
			char *const *searchpath;
			int freearray;
			const RefStrList *plist;
			ProcPath(const ProcPath &)  { }  // don't copy!
			void operator=(const ProcPath &)  { }
		public:  _CPP_OPERATORS
			// NOTE: The args to the constructor are NEITHER COPIED nor 
			//       freed by the destructor. Make sure the pointers are 
			//       valid as long as the class exists. 
			ProcPath(const char *_path)   // binary located at _path
				{  path=_path;  freearray=0;  searchpath=NULL;  plist=NULL; }
			// Probably the best way; The passed pathlist is not copied; 
			// it must exist as long as ProcPath exists (i.e. normally 
			// until StartProcess() returns).
			// (pathlist may only contain '\0'-terminated RefStrings.)
			ProcPath(const char *name,const RefStrList *pathlist)
				{  path=name;  freearray=0;  searchpath=NULL;  plist=pathlist;  }
			ProcPath(const char *name,char *const *Searchpath)  // last: NULL-pointer!
				{  path=name;  freearray=0;  searchpath=Searchpath;  plist=NULL;  }
			ProcPath(const char *name,const char *path0,...);  // NULL-terminated!
			~ProcPath()
				{  if(freearray)  searchpath=(char**)LFree((char*)searchpath);  plist=NULL;  }
	};
	class ProcFDs
	{
		friend class ProcessManager;
		private:
			int n,dim;
			int *ourfd,*destfd;  // size: n
			inline int _Find(const int *array,int n,int tofind) const;  // -1 or index
			ProcFDs(const ProcFDs &)  { }  // use Copy() to copy
			void operator=(const ProcFDs &)  { }
		public:  _CPP_OPERATORS
			ProcFDs()  {  n=dim=0; ourfd=NULL; destfd=NULL;  }
			~ProcFDs()  {  ourfd=(int*)LFree(ourfd); /* NOT FREE destfd = ourfd+dim */ }

			// Return value: 0 -> OK; -1 -> LMalloc() failed. 
			int Copy(const ProcFDs &pfds);

			// ourfd: the FD value in the current procss 
			// destfd: the FD value this fd shall have in the 
			//         executed process 
			// Return value: 
			//   0 -> OK
			//  -1 -> malloc failed
			//  -2 -> destfd already in list 
			//  -3 -> ourfd or destds <0
			int Add(int ourfd,int destfd);
	};
	class ProcMisc
	{
		friend class ProcessManager;
		private:
			int pflags;  // Bitwise OR of the following constants:  
			// PF_OnlyKillOwner -> allow TermProcess() only if you are 
			//         the owner. ::kill() will still work, of course. 
			//     PF_TermOnDestroy -> terminate and then kill (after a 
			//         delay; see ProcessManager::TermKillDelay()) the 
			//         process when the ProcessBase() gets destroyed. 
			//     PF_KillPendingX -> INTERNAL USE; MAY NOT BE SET. 
			
			enum { UseNice=0x1, UseUID=0x2, UseGID=0x4 };
			int useflags;  // OR of the enum above 

			// These settings are listed in the order they are set 
			// after fork() and before execve(): 
			const char *pcrdir;  // change root directory (chdir(), chroot()) [not copied]
			const char *pwdir;   // working directory (chdir()) [not copied]
			int call_setsid;     // call setsid() (0 -> no; 1 -> yes; 2 -> allow fail)
			int pniceval;        // nice value (nice())
			gid_t pgid;          // gid (setgid())
			uid_t puid;          // uid (setuid())
			int (*pfuncptr)(void*);  // custom function to be called 
			void *pfuncarg;      // argument for custom function
				// setgroups()/initgroups()/setsid()/setpgdp() can be 
				// called in custom function. 
			
			ProcMisc(const ProcMisc &)  { }  // don't copy
			void operator=(const ProcMisc &)  { }
		public:  _CPP_OPERATORS
			ProcMisc()  {  pflags=0; pcrdir=NULL; pwdir=NULL; call_setsid=0;
				pfuncarg=NULL; pfuncptr=NULL; useflags=0;  }
			~ProcMisc()  { }
			
			// Set flags: 
			ProcMisc *flags(int _flags)  {  pflags=_flags;  return(this);  }
			
			// Set change root directory and working directory. 
			// Strings are NOT copied. 
			// Order: chdir(crdir); chroot("."); chdir(wdir);
			ProcMisc *chroot(const char *_crdir)  {  pcrdir=_crdir;  return(this);  }
			ProcMisc *wdir(const char *_wdir)  {  pwdir=_wdir;  return(this);  }
			
			// Set nice value: 
			ProcMisc *niceval(int _niceval)
				{  pniceval=_niceval;  useflags|=UseNice;  return(this);  }
			
			// Set UID and GID: 
			ProcMisc *uid(uid_t _uid)  {  puid=_uid;  useflags|=UseUID;  return(this);  }
			ProcMisc *gid(gid_t _gid)  {  pgid=_gid;  useflags|=UseGID;  return(this);  }
			
			// call setsid() (0 -> no; 1 -> yes; 2 -> allow fail)
			ProcMisc *setsid(int y)  {  call_setsid=y;  return(this);  }
			
			// Set function to be called before the execve() call in the 
			// child's thread. funcarg is passed to func as an argument. 
			// The function must return 0 on success and !=0 on failure. 
			// You get informed if the function failed via procnotify() 
			// action=PSFailed, detail=PSFunctionFailed; the estatus field 
			// holds the return value of the function. 
			ProcMisc *function(int (*_funcptr)(void*),void *_funcarg)
				{  pfuncptr=_funcptr;  pfuncarg=_funcarg;  return(this);  }
	};
};

#endif  /* _HLIB_InternalProcessBase_H_ */
