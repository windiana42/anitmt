/*
 * procmanager.cc
 * 
 * Implementation of class ProcessManager, a class for 
 * process (task) management which works in cooperation 
 * with classes derived from class ProcessBase. 
 *
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/poll.h>
#include <sys/time.h>
#include <sys/socket.h>   /* shutdown() */
#include <sys/resource.h>
#include <sys/wait.h>

#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/procmanager.h>
#include <hlib/procbase.h>
#include <hlib/refstrlist.h>

#define TermKillAlign  0   /* align value for term-kill-timer */

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. 
#endif

#if DEBUG
  #if !TESTING
    #define DEBUG 0
  #else
    #warning DEBUG switched on. 
    #define pdebug(fmt...)   fprintf(stdout, "FD:" fmt);
  #endif
#else
  #define pdebug(fmt...)
#endif


/* static global manager */
class ProcessManager *ProcessManager::manager=NULL;

// THERE MAY BE ONLY ONE ProcessManager. 

struct PipeCode
{
	int errnoval;
	short int exitcode;
};


// Be sure... a close checking for EINTR (if that will ever happen...)
static inline int fd_close(int fd)
{
	int rv;
	do
	{  rv=close(fd);  }
	while(rv<0 && errno==EINTR);
	return(rv);
}
static inline int fd_dup2(int ofd,int nfd)
{
	int rv;
	do
	{  rv=dup2(ofd,nfd);  }
	while(rv<0 && errno==EINTR);
	return(rv);
}


#define FailEStatus 100
static void _ChildExit(int pipefd,InternalProcessBase::PSDetail exitval,int errnoval)
{
	PipeCode pc;
	pc.errnoval=errnoval;
	pc.exitcode=(short int)(exitval);
	int wr;
	do
	{  wr=write(pipefd,&pc,sizeof(pc));  }
	while(wr<0 && errno==EINTR);
	#if TESTING
	if(wr!=sizeof(pc))
	{  fprintf(stderr,"**BAD** ProcessManager: _ChildExit: Writing to pipe "
		"failed %d, %s\n",wr,strerror(errno));  }
	#endif
	fd_close(pipefd);
	_exit(FailEStatus);  // Nothing should depend on this value (only 
	                     // some warning helping in debugging). 
}


// Highly internal routine for search path check: 
static int _SPathCheckAccess(char *tmp_binpath,
	const char *searchdir,const char *binname)
{
	// These strcpy()s will not overwrite the buffer due to 
	// allocation above. 
	char *dst=tmp_binpath;
	strcpy(dst,searchdir);
	dst+=strlen(searchdir);
	if(*dst!='/')  *(dst++)='/';
	strcpy(dst,binname);
	if(!access(tmp_binpath,X_OK))
	{  return(1);  }
	return(0);
}


pid_t ProcessManager::StartProcess(ProcessBase *pb,
	const ProcPath &path,
	const ProcArgs &args,
	const ProcMisc &misc,
	const ProcFDs &_pfds,
	const ProcEnv &env)
{
	if(!pb)  return(0);
	
	if(limitproc>=0 && nproc>=limitproc)
	{  return(-8);  }
	
	int flags=misc.pflags;
	{
		int oflags=flags;
		flags&=(
			ProcessBase::PF_OnlyKillOwner | 
			ProcessBase::PF_TermOnDestroy );
		if(flags!=oflags)
		{  return(-2);  }
	}
	
	// Check args and envp and path allocation: 
	if(path.freearray==2)  return(-3);
	if(args.freearray==2)  return(-4);
	if(args.freearray==3)  return(-12);
	if(env.freearray==2)   return(-5);
	
	
	// Search for binary: 
	char *binpath=NULL;
	if(!path.path)
	{  return(-6);  }
	#warning CHROOT NOT SUPPORTED HERE!!!
	if(path.path[0]!='/' &&  // not an absolute path 
	   (path.searchpath || path.plist))
	{
		size_t maxlen=0;
		// Find longest string in search path: 
		if(path.searchpath)
			for(int i=0; path.searchpath[i]; i++)
			{
				size_t tmp=strlen(path.searchpath[i]);
				if(maxlen<tmp)  maxlen=tmp;
			}
		if(path.plist)
			for(const RefStrList::Node *n=path.plist->first(); n; n=n->next)
			{
				if(n->stype()!=0)  return(-11);  
				size_t tmp=n->len();
				if(maxlen<tmp)  maxlen=tmp;
			}
		
		// We need 2 bytes more: one for a `/' and one for a \0. 
		binpath=(char*)LMalloc(strlen(path.path)+maxlen+2);
		if(!binpath)  return(-1);
		if(path.searchpath)
			for(int i=0; path.searchpath[i]; i++)
			{
				if(_SPathCheckAccess(binpath,path.searchpath[i],path.path))
				{  goto found;  }
			}
		if(path.plist)
			for(const RefStrList::Node *n=path.plist->first(); n; n=n->next)
			{
				if(_SPathCheckAccess(binpath,n->str(),path.path))
				{  goto found;  }
			}
		
		LFree(binpath);
		return(-6);
		found:;
	}
	else 
	{
		if(access(path.path,X_OK))
		{  return(-6);  }
		binpath=(char*)path.path;
	}
	
	// Binary now at binpath. 
	pid_t retval=0;
	Node *n=NULL;
	int pipefds[2]={-1,-1};
	
	{
		// Alloc a process node: 
		{
			n=NEW<Node>();
			if(!n)
			{  retval=-1;  goto doreturn1;  }
			n->pb=pb;
			n->flags=flags;
		}
		
		// Copy _pfds: 
		ProcFDs pfds;
		if(pfds.Copy(_pfds))
		{  retval=-1;  goto doreturn2;  }
		
		// Now, do the hard part: 
		// Aquire pipe for execution failure detection. 
		{
			int rv;
			do
			{  rv=pipe(pipefds);  }
			while(rv<0 && errno==EINTR);
			if(rv)
			{  retval=-9;  goto doreturn2;  }
		}
		// Write end of pipe must close on exec: 
		if(fcntl(pipefds[1],F_SETFD,FD_CLOEXEC))
		{  retval=-10;  goto doreturn3;  }
		// Set non-blocking flag on read side: 
		if(SetNonblocking(pipefds[0]))
		{  retval=-10;  goto doreturn3;  }
		// read on pipefds[0], write on pipefds[1] 
		// Allocate poll node: 
		if(PollFD(pipefds[0],0))  // return value >0 should never happen here
		{  retval=-1;  goto doreturn3;  }
		
		// Really fork...
		pid_t pid;
		do
		{  pid=fork();  }    // ``Point of no return'' 
		while(pid<0 && errno==EINTR);
		if(pid<0)
		{  retval=-7;  goto doreturn3;  }
		if(pid>0)  // parent thread
		{
			// Close write end of pipe:
			fd_close(pipefds[1]);  pipefds[1]=-1;
			// Set up process node data: 
			retval=pid;
			n->pid=pid;
			n->starttime=HTime(HTime::Curr);
			// Must poll for read end of pipe: 
			n->pipe_fd=pipefds[0];
			PollFD(n->pipe_fd,POLLIN,n);  // will not fail
			pipefds[0]=-1;  // so that we do not close it now
			// Queue the node: 
			procs.append(n);
			n=NULL;  // ...so that we will not delete it 
			++nproc;
			// Tell the manager to allocate an extra signal node 
			// for that child: 
			fdmanager()->ExtraSigNodes(+1);
			goto doreturn3;
		}
		
		/* Child's thread only here. */
		
		// Make sure SIGPIPE never causes any trouble: 
		signal(SIGPIPE,SIG_IGN);
		
		// Close read end of pipe: 
		// (Also unpoll it because we inherit poll node from parent; 
		// otherwiese CloseAllFDs() complains.) 
		CloseFD(pipefds[0]);  pipefds[0]=-1;
		
		#if TESTING
		// This warns us that the write end of the pipe is non-blocking. 
		// Actually never happend...
		if(fcntl(pipefds[1],F_GETFL) & O_NONBLOCK)
		{  fprintf(stderr,"OOPS: ProcMan: write end of pipe NONBLOCKING!!!\n");  }
		#endif
		
		// Deal with the FDs...
		
		int dup_errors=0;
		int close_errors=0;
		int dup_errno=0;
		
		// First, we got to close all our FDs so that the executed 
		// program will not get them. 
		// However, we have to leave those FDs open which we need. 
		// We may only close them; we may NOT shut them down. 
		close_errors+=fdmanager()->CloseAllFDs(pfds.ourfd,pfds.n);
		
		// Now, we bring all source FDs out of the way. 
		// There are probably algorithms with a better runtime...
		for(int i=0; i<pfds.n; i++)
		{
			int ofd=pfds.ourfd[i];
			if(pfds._Find(pfds.destfd,pfds.n,ofd)>=0)
			{
				// pfds.ourfd[i] is in the way
				pfds.ourfd[i]=-1;
				// We have to find a unique FD for it. 
				for(int newfd=0; ; newfd++)
				{
					if(pfds._Find(pfds.destfd,pfds.n,newfd)>=0)  continue;
					if(pfds._Find(pfds.ourfd,pfds.n,newfd)>=0)  continue;
					if(fd_dup2(ofd,newfd)<0)
					{
						if(!dup_errors)  dup_errno=errno;
						newfd=-1;
						++dup_errors;
					}
					if(fd_close(ofd))
					{  ++close_errors;  }
					pfds.ourfd[i]=newfd;
					break;
				}
			}
		}
			
		// Now, we dup all the FDs 
		for(int i=0; i<pfds.n; i++)
		{
			int ofd=pfds.ourfd[i];
			int dfd=pfds.destfd[i];
			if(ofd<0 || dfd<0)  continue;
			if(fd_dup2(ofd,dfd)<0)
			{  ++dup_errors;  }
		}
		
		// Now, we close all our (old) FDs 
		for(int i=0; i<pfds.n; i++)
		{
			int ofd=pfds.ourfd[i];
			if(ofd<0)  continue;
			if(fd_close(ofd))
			{  ++close_errors;  }
		}
		
		// We currently ignore close errors. 
		#warning ...should we?
		
		if(dup_errors)
		{  _ChildExit(pipefds[1],PSDupFailed,dup_errno);  }  // exit child's thread 
		
		// Okay, now we gonna do some special stuff before actually 
		// executing the program: 
		if(misc.pcrdir)
		{
			// As chroot(2) does not change the cwd, I first do 
			// a chdir(): 
			int failed=1;
			if(!chdir(misc.pcrdir))
			{
				if(!chroot("."))
				{  failed=0;  }
			}
			if(failed)
			{  _ChildExit(pipefds[1],PSChrootFailed,errno);  }  // exit child's thread 
		}
		if(misc.pwdir)
		{
			if(chdir(misc.pwdir))
			{  _ChildExit(pipefds[1],PSChdirFailed,errno);  }  // exit child's thread 
		}
		if((misc.useflags & ProcMisc::UseNice))
		{
			#warning ignore nice errors?
			if(nice(misc.pniceval))
			{  _ChildExit(pipefds[1],PSNiceFailed,errno);  }  // exit child's thread 
		}
		if((misc.useflags & ProcMisc::UseGID))
		{
			if(setgid(misc.pgid))
			{  _ChildExit(pipefds[1],PSSetGidFailed,errno);  }
		}
		if((misc.useflags & ProcMisc::UseUID))
		{
			if(setuid(misc.puid))
			{  _ChildExit(pipefds[1],PSSetUidFailed,errno);  }
		}
		if(misc.pfuncptr)
		{
			int frv=((*misc.pfuncptr)(misc.pfuncarg));
			if(frv)
			{  _ChildExit(pipefds[1],PSFunctionFailed,frv);  }
		}
		
		// Finally execute the program: 
		int rv;
		do
		{  rv=execve(binpath, args.args, env.env);  }
		while(rv<0 && errno==EINTR);
		_ChildExit(pipefds[1],PSExecFailed,errno);  // exit child's thread 
	}
	
doreturn3:;
	if(pipefds[0]>=0)  CloseFD(pipefds[0]);   // unpoll & close
	if(pipefds[1]>=0)  fd_close(pipefds[1]);
doreturn2:;
	delete n;
doreturn1:;
	if(binpath!=path.path)
	{  LFree(binpath);  }
	
	return(retval);
}


int ProcessManager::TermProcess(ProcessBase *pb,pid_t pid)
{
	Node *n=_FindNode(pid);
	if(!n)  return(-3);
	if((n->flags & ProcessBase::PF_OnlyKillOwner) && 
	   n->pb != pb)
	{  return(-4);  }
	return(_TermProcess(n));
}


// This is inline as it is called only from one place. 
inline void ProcessManager::_SuccessfulExecution(Node *n)
{
	// Tell the associated ProcessBase (and special ones): 
	ProcStatus ps;
	ps.action=PSExecuted;
	ps.detail=PSSuccess;
	ps.starttime=n->starttime;
	ps.pid=n->pid;
	ps.started_by=n->pb;
	ps.uid=uid_t(-1);
	ps.sigcode=-1;
	ps.estatus=n->errnoval;  // ...which should be 0
	_TellProcBase(n,&ps);
}


// Return value: 
// PSExecFailed..._PSLASTFailed-1 -> pipe code
//  0 -> no pipe code read (-> exec success)
// -1 -> reading pipe code failed (bad..) [currently never happens due to abort]
int ProcessManager::_ReadPipeCode(Node *n)
{
	PipeCode pc;
	int rd;
	do
	{  rd=read(n->pipe_fd,&pc,sizeof(pc));  }
	while(rd<0 && errno==EINTR);
	if(rd<0)
	{
		#if TESTING
		fprintf(stderr,"Oops: ProcMan[%d]: reading from pipe failed: %s\n",
			n->pid,strerror(errno));
		#endif
		// Hmm... what shall we do?
		// (NOTE THAT we have O_NONBLOCK set on the pipe.)
		// Do best possible tidy-up...
		/* ps->action=PSDead;
		   ps->detail=PSUnknownError;
		   CloseFD(n->pipe_fd);  n->pipe_fd=-1;
		*/
		#warning what shall we do? (currently aborting)
		abort();
		return(-1);
	}
	else if(rd!=0 && rd!=(int)sizeof(pc))
	{
		#if TESTING
		fprintf(stderr,"Oops: ProcMan[%d]: Short read from pipe %d/%d bytes\n",
			n->pid,rd,int(sizeof(pc)));
		#endif
		#warning what shall we do? (currently aborting)
		abort();
		return(-1);
	}
	
	/* rd=0 or rd=sizeof(pc) here. */
	
	// Got failure/success info. Don't need pipe any more. 
	// procnotify() will be called. 
	// (CANNOT call _SuccessfulExecution() here because child 
	// is exiting.) 
	CloseFD(n->pipe_fd);
	n->pipe_fd=-1;
	
	if(!rd)  return(0);  // nothing read
	
	// Store errno value: 
	n->errnoval=int(pc.errnoval);
	int pcd=int(pc.exitcode);
	if(pcd<PSExecFailed || pcd>=_PSLASTFailed)
	{
		// This case should never happen. 
		#if TESTING
		fprintf(stderr,"Oops: ProcMan: read unknown pipe code %d\n",pcd);
		#endif
		pcd=PSUnknownError;
	}
	
	return(pcd);
}

void ProcessManager::_CheckExecFailure(Node *n,ProcStatus *ps,int estatus)
{
	ps->estatus=estatus;
	
	// If pipe_fd>=0, then it is the file descriptor of the 
	//   diagnosis pipe. We do not yet know if execution failed. 
	// If pipe_fd==-1, then execution did not fail and 
	//   we already know that. 
	// If pipe_fd==-2, then pipe_fd was not set up during 
	//   StartProcess. This may not happen. 
	// If pipe_fd<=-PSExecFailed, then execution failed but 
	//   we did not tell the ProcessBase (becuase we read it 
	//   from fdnotify()). 
	
	// Get pipe code: 
	int pcd;
	if(n->pipe_fd>=0)
	{  pcd=_ReadPipeCode(n);  }  // <- Calls CloseFD() on pipe. 
	else if(n->pipe_fd==-1)
	{  pcd=0;  }
	else
	{  pcd=-n->pipe_fd;  }
	
	if(!pcd)  // pipe was empty
	{
		// Okay, child execution did not fail. 
		// (Nothing written to pipe and pipe closed by close 
		// on exec.)
		
		ps->action=PSDead;
		ps->detail=PSExited;
		
		#if TESTING
		// If this warning appears, then either the process really 
		// exited with code FailEStatus or something in the pipe 
		// mechanism failed. 
		if(estatus==FailEStatus)
		{  fprintf(stderr,"Hmmm... proc %d exits with status %d %s.\n",
			n->pid,estatus,
			(n->pipe_fd==-1) ? 
				"and we assume that execution was successful" : 
				"but could not read fail code from pipe");  }
		#endif
	}
	else if(pcd>0)
	{
		// Child execution failed. 
		ps->action=PSFailed;
		ps->detail=(PSDetail)pcd;
		// Save errno value in estatus to pass the info to ProcessBase: 
		ps->estatus=n->errnoval;
	}
	// pcd<0 currently never happens due to abort() in _ReadPipeCode(). 
	#if TESTING
	if(pcd<0 || n->pipe_fd==-2)
	{  fprintf(stderr,"** OOPS: ProcMan: BUG: pcd=%d; pipe_fd=%d\n",
		pcd,n->pipe_fd);  abort();  }
	#endif
}


// Fills in everything except starttime/started_by from status and 
// rus as returned by wait3() or wait4(). 
void ProcessManager::_FillProcStatus(Node *n,ProcStatus *ps,
	int pid,int status,struct rusage *rus)
{
	ps->action=(PSAction)-1;
	ps->detail=(PSDetail)-1;
	if(WIFEXITED(status))
	{  _CheckExecFailure(n,ps,WEXITSTATUS(status));  }
	else if(WIFSIGNALED(status))
	{
		ps->action=PSDead;
		ps->estatus=WTERMSIG(status);
		ps->detail=PSKilled;
		switch(ps->estatus)
		{
			case SIGQUIT:  case SIGILL:  case SIGTRAP:
			case SIGABRT:  case SIGFPE:  case SIGSEGV:
			case SIGBUS:   case SIGSYS:  case SIGXCPU:
			case SIGXFSZ:  ps->detail=PSDumped;
		}
	}
	else if(WIFSTOPPED(status))
	{
		ps->action=PSStopCont;
		ps->estatus=WSTOPSIG(status);
		if(ps->estatus==SIGSTOP)
		{  ps->detail=PSStop;  }
		else if(ps->estatus==SIGCONT)
		{  ps->detail=PSCont;  }
		#if TESTING
		else
		{  fprintf(stderr,"HE?! process %d stopped by signal %d\n",
			pid,ps->detail);  abort();  }
		#endif
	}
	#if TESTING
	else
	{  fprintf(stderr,"Oops: why did this child die?!\n");  abort();  }
	#endif
	ps->endtime=HTime(HTime::Curr);  // better than nothing...
	ps->pid=pid;
	ps->uid=uid_t(-1);      // damn!
	ps->sigcode=-1;  // also damn
	_AddToProcStatus(ps,rus);  // stime, utime, rus
}

// Adds the rusage info to the ProcStat
void ProcessManager::_AddToProcStatus(ProcStatus *ps,
	struct rusage *rus)
{
	ps->utime.SetTimeval(&rus->ru_utime);
	ps->stime.SetTimeval(&rus->ru_stime);
	ps->rus=*rus;
}


// Return value: 1 -> ignore signal
int ProcessManager::_CheckSigInfo(const SigInfo *sig)
{
	switch(sig->info.si_code)
	{
		case CLD_EXITED:
		case CLD_KILLED:
		case CLD_DUMPED:
		case CLD_STOPPED:
		case CLD_CONTINUED:  /* handeled by _FillProcStatus() */ break;
		case SI_USER:
			#if TESTING
			fprintf(stderr,"ProcMan: ignoring SIGCHILD (%d)\n",
				sig->info.si_code);
			#endif
			return(1);
		case CLD_TRAPPED:   // <- seems to be sth like `stopped'. 
		default: 
			#if TESTING
			#warning unclean solution (currently aborting)
			fprintf(stderr,"ProcMan: strange si_code %d\n",
				sig->info.si_code);
			abort();
			// (what to do? 
			#else
			#error this has to be fixed. 
			#endif
			return(1);
	}
	return(0);
}

// Fills in everything except starttime/started_by, rus, utime and stime 
// from SigInfo. 
void ProcessManager::_FillProcStatus(Node *n,ProcStatus *ps,
	const SigInfo *sig)
{
	// Ignored valued si_code do not reach here (see _CheckSigInfo()). 
	
	ps->action=(PSAction)-1;
	ps->detail=(PSDetail)-1;
	// estatus has to be set here so that _CheckExecFailure() can 
	// overwrite it with errno val if exec failed. 
	ps->estatus=sig->info.si_status;
	
	// Check what happened: 
	switch(sig->info.si_code)
	{
		// case SI_USER:  <-- trapped by _CheckSigInfo()
		case CLD_EXITED:
		{
			// Check for failed execution: 
			_CheckExecFailure(n,ps,sig->info.si_status);
		}	break;
		case CLD_KILLED:
			ps->action=PSDead;
			ps->detail=PSKilled;
			break;
		case CLD_DUMPED:
			ps->action=PSDead;
			ps->detail=PSDumped;
			break;
		case CLD_STOPPED:
			ps->action=PSStopCont;
			ps->detail=PSStop;
			break;
		case CLD_CONTINUED:
			ps->action=PSStopCont;
			ps->detail=PSCont;
			break;
		
		//case CLD_TRAPPED:  BOTH trapped by...
		//default:           ..._CheckSigInfo()
		
		#if TESTING
		default:
			fprintf(stderr,"*** OOPS!!! unhandeled si_code %d not rejected "
				"by _CheckSigInfo()!!!\n",sig->info.si_code);
			abort();
		#endif
	}
	
	ps->endtime=sig->time;   // when the signal was caught
	// Set according to rusage info which is also more exactly. 
	//ps->utime_seconds=double(sig->info.si_stime)/double(CLOCKS_PER_SEC);
	//ps->stime_seconds=double(sig->info.si_utime)/double(CLOCKS_PER_SEC);
	ps->pid=sig->info.si_pid;
	ps->uid=sig->info.si_uid;
	ps->sigcode=sig->info.si_code;
}


// Calls a wait3() to check for all exited children. 
void ProcessManager::_ZombieCheck()
{
	ProcStatus ps;
	for(;;)
	{
		int status;
		struct rusage rus;
		pid_t pid;
		do
		{  pid=::wait3(&status,WNOHANG | WUNTRACED,&rus);  }
		while(pid==-1 && errno==EINTR);
		//fprintf(stderr,"SIGCHILD: %d (%s)\n",pid,strerror(errno));
		if(pid<=0)  break;  // pid=0 means: no child exited. 
		
		#if TESTING
		// This means that the found process was not reported by a 
		// SIGCHILD or that the signal was not yet delivered to us. 
		fprintf(stderr,"Hep.. _ZombieCheck() found child %d.\n",pid);
		#endif
		
		Node *n=_FindNode(pid);
		if(!n)
		{
			// (In my testing program this happens when I re-start the 
			// ProcessManager and it receives a SIGCHLD from a process 
			// started by this ProcessManager.)
			// If that happens, it is normally a bad sign. Probably some 
			// child was considered dead while it is actually still running. 
			#if TESTING
			fprintf(stderr,"He?! Not aware of child %d\n",int(pid));
			#endif
			continue;
		}
		
		// We must do that to see if the child really exited or if 
		// it just stopped. 
		_FillProcStatus(n,&ps,pid,status,&rus);
		//Set missing fields in ps (starttime, started_by): 
		ps.starttime=n->starttime;
		ps.started_by=n->pb;
		
		_TellProcBase(n,&ps);
		
		// It can be the case that the pipe_fd is still set here. 
		// This happens if we did not yet read the pipe but 
		// _ZombieCheck() was called for some reason. In this case 
		// ps.action!=PSExited (and thus !=PSFailed). [In case of 
		// PSExited, _FillProcStatus() checks the pipe and closes it.] 
		// What could have happened: 
		//  The child process executed successfully and was killed 
		//  immediately afterwards or the child fork() thread was 
		//  killed before executing anything. 
		// We simply go on or close the pipe if the process is dead. 
		if(ps.action==PSDead || 
		   ps.action==PSFailed )
		{  _DeleteNode(n,/* may have pipe_fd set*/ps.action==PSDead);  }
	}
}


int ProcessManager::signotify(const SigInfo *sig)
{
	if(sig->info.si_signo!=SIGCHLD)
	{  return(0);  }
	
	pdebug("SIGNOTIFY(pid=%d)\n",sig->info.si_pid);
	
	if(_CheckSigInfo(sig))
	{  return(0);  }
	
	// See if we can locate the child in our list: 
	Node *n=_FindNode(sig->info.si_pid);
	if(!n)
	{
		// This can happen if _ZombieCheck() detects the child 
		// before the signal is actually delivered. 
		#if TESTING
		fprintf(stderr,"He?! Not aware of child %d (%d)\n",
			int(sig->info.si_pid),sig->info.si_code);
		#endif
		// (At least prevent zombies from laying around...)
		pid_t _p;
		do
		{  _p=waitpid(sig->info.si_pid,NULL,WNOHANG);  }
		while(_p==-1 && errno==EINTR);
		return(0);
	}
	
	ProcStatus ps;
	_FillProcStatus(n,&ps,sig);
	
	// Set missing fields in ps (starttime, started_by, u/stime rus): 
	ps.starttime=n->starttime;  // when the fork returned to parent thread 
	ps.started_by=n->pb;
	
	// Wait for the child so that the zombie can be freed. 
	int w4stat;
	struct rusage rus;
	pid_t rv;
	do
	{  rv=::wait4(ps.pid,&w4stat,WNOHANG | WUNTRACED,&rus);  }
	while(rv==-1 && errno==EINTR);
	if(rv<=0)
	{
		#if TESTING
		fprintf(stderr,"Hep?! caught SIGCHLD but wait4() returns error %d\n",
			errno);
		#endif
		return(0);
	}
	
	_AddToProcStatus(&ps,&rus);  // set utime, stime, rus
	
	_TellProcBase(n,&ps);
	
	// Only delete the node if the child actually died. 
	// It can be the case that the pipe_fd is still set here. 
	// .... [read on in _ZombieCheck()]
	if(ps.action==PSDead || 
	   ps.action==PSFailed )
	{  _DeleteNode(n,/* may have pipe_fd set*/ps.action==PSDead);  }
	
	// No further SIGCHLD queued at FDManager? 
	// I do NOT call _ZombieCheck() here but give signals some time 
	// to arrive by starting a timer and then calling ZombieCheck(). 
	// This helps preventing the ``not aware of child'' message in case 
	// several processes quit at the same time. 
	// (_ZombieCheck() MUST be called sooner or later as there might 
	// be more dead processes than signals.)
	if(!SigPending(SIGCHLD))
	{  _ScheduleZombieCheck();  }
	
	return(0);
}


void ProcessManager::_TellProcBase(Node *n,ProcStatus *ps)
{
	// First, tell the class which started the process if it 
	// still exists: 
	ProcessBase *st=n->pb;
	if(st)
	{  st->procnotify(ps);  }
	
	// Now, tell all special ProcessBases with AlwaysNotify set: 
	// (The class which started the process will not be informed 
	// twice.)
	for(ProcessBase *i=specialpb.first(); i; i=i->next)
	{
		if(i!=st && (i->special_flags & AlwaysNotify))
		{  i->procnotify(ps);  }
	}
}


int ProcessManager::fdnotify(FDInfo *fdi)
{
	if(fdi->fd<0)  return(0);   // just to be really sure...
	
	#if TESTING
	if(!fdi->dptr)
	{  fprintf(stderr,"Oops: ProcessManager::fdnotify(%d,%d,%d): dptr=NULL.\n",
		fdi->fd,fdi->events,fdi->revents);  }
	#endif
	
	Node *n=(Node*)(fdi->dptr);
	#if TESTING
	if(n->pipe_fd!=fdi->fd)
	{  fprintf(stderr,"Oops: ProcessManager: fd/pipe_fd inconsistent: %d, %d\n",
		n->pipe_fd,fdi->fd);  }
	#endif
	
	if(fdi->revents & (POLLIN | POLLHUP))
	{
		// When we call this, EWOULDBLOCK may never occur. 
		// This should not be the case as POLLIN is set. 
		int pcd=_ReadPipeCode(n);  // calls CloseFD() 
		// pcd<0 will not happen due to abort() in _ReadPipeCode(). 
		if(!pcd)
		{  _SuccessfulExecution(n);  }
		else if(pcd>0)
		{
			// Okay. Seems as if the child has trouble... 
			// Just stupid that we did not yet catch the signal. 
			// So, we store the exit value as -(n->pipe_fd): 
			// n->pipe_fd is -1 here (set by _ReadPipeCode())
			// pcd is always >100. 
			n->pipe_fd=-pcd;
		}
	}
	
	return(0);
}


int ProcessManager::timernotify(TimerInfo *ti)
{
	if(ti->tid==zmbchk)
	{
		// Zombie check timer. 
		if(!SigPending(SIGCHLD))
		{
			_ZombieCheck();             // check for zombies...
			UpdateTimer(zmbchk,-1,0);   // ...and disable timer
		}
		// else: we run into signotify() just after returning
		return(0);
	}
	
	pdebug("TermKillTimeralarm\n");
	
	int nleft=0;
	for(Node *i=procs.first(); i; )
	{
		Node *n=i;
		i=i->next;
		
		if(!(n->flags & ProcessBase::PF_KillPending))
		{
			#if TESTING
			if(!n->pb)
			{  fprintf(stderr,"(procman) Oops: abandoned Node (pid=%d).\n",
				n->pid);  }
			#endif
			continue;
		}
		
		if((n->flags & ProcessBase::PF_KillPending2))
		{
			n->flags &= ~ProcessBase::PF_KillPending;
			n->flags |= ProcessBase::PF_KillPending1;
			++nleft;
		}
		else if((n->flags & ProcessBase::PF_KillPending1))
		{
			pdebug("KILLing %d\n",n->pid);
			if(::kill(n->pid,SIGKILL))
			{  pdebug("Failed to kill -KILL process %d: %s\n",
				n->pid,strerror(errno));  }
			n->flags &= ~ProcessBase::PF_KillPending;
			// After starting SUID processes, we may not be able to 
			// kill them (EPERM). 
			// This should not happen here, though (but when sending 
			// SIGTERM). 
			// (The process node gets deleted when the SIGCHLD gets 
			// received.) 
			/* FIXME
			if(!n->pb)  // abandoned 
			{  _DeleteNode(n);  } */
		}
	}
	
	if(!nleft)
	{
		UpdateTimer(tid,-1,0);  // disable 
		tktimer_running=0;
	}
	
	return(0);
}


void ProcessManager::_ScheduleKill(Node *n)
{
	if(!n)  return;
	if((n->flags & ProcessBase::PF_KillPending))  return;
	if(n->pid<=0)  return;
	
	if(tktimer_running)
	{  n->flags |= ProcessBase::PF_KillPending2;  }
	else
	{
		n->flags |= ProcessBase::PF_KillPending1;
		UpdateTimer(tid,term_kill_msec,TermKillAlign);
		tktimer_running=1;
	}
}


// NOTE: _TermProcess(n) may delete n. 
// Returns -2 if ::kill(SIGTERM) failed. 
int ProcessManager::_TermProcess(Node *n)
{
	if(!n)  return(0);
	if((n->flags & ProcessBase::PF_KillPending))  return(0);
	if(n->pid<=0)  return(0);
	
	pdebug("TERMing %d\n",n->pid);
	if(!(::kill(n->pid,SIGTERM)))
	{  _ScheduleKill(n);  }
	else
	{
		pdebug("Failed to kill -TERM process %d: %s\n",
			n->pid,strerror(errno));
		/* FIXME
		if(!n->pb)  // process base unregistered
		{  _DeleteNode(n);  } */
		return(-2);  // leave that
	}
	return(0);
}


// Called by ProcessBase-classes upon desctruction: 
void ProcessManager::Unregister(ProcessBase *pb)
{
	if(!pb)  return;
	
	// Must be removed from special list BEFORE we kill the processes 
	// (prevent virtual functions getting called from within destructor). 
	if(specialpb.first()==pb || pb->prev)
	{  specialpb.dequeue(pb);  }
	
	for(Node *_i=procs.first(); _i; )
	{
		Node *n=_i;
		_i=_i->next;
		if(n->pb==pb)
		{
			n->pb=NULL;
			if((n->flags & ProcessBase::PF_TermOnDestroy))
			{  _TermProcess(n);  }
		}
	}
}


int ProcessManager::SpecialRegister(ProcessBase *pb,int special)
{
	if(!pb)  return(0);
	if(specialpb.first()!=pb && !pb->prev)
	{
		// pb not in special list. 
		if(special)
		{  specialpb.append(pb);  }   // add to special list
	}
	else
	{
		// pb in special list
		if(!special)
		{  specialpb.dequeue(pb);  }   // remove from special list
	}
	// Update saved special flags: 
	pb->special_flags=special;
	return(0);
}


void ProcessManager::TermKillDelay(long msec)
{
	term_kill_msec=msec;
	if(tktimer_running)
	{  UpdateTimer(tid,term_kill_msec,TermKillAlign);  }
}


ProcessManager::Node *ProcessManager::_FindNode(pid_t pid)
{
	for(Node *i=procs.first(); i; i=i->next)
	{
		if(i->pid==pid)
		{  return(i);  }
	}
	return(NULL);
}


inline ProcessManager::Node::Node(int * /*failflag*/=NULL) : 
	LinkedListBase<Node>(),
	starttime() 
{
	pid=-1;
	pb=NULL;
	flags=0;
	pipe_fd=-2;  // initial value -2
	errnoval=0;  // success
}

inline ProcessManager::Node::~Node()
{
	#if TESTING
	if(pipe_fd>=0)
	{  fprintf(stderr,"OOPS: valid pipe_fd %d in ~Node(%d)\n",pipe_fd,pid);  }
	#endif
}


inline void ProcessManager::_DeleteNode(Node *n,int may_have_pipefd_set)  // dequeue and free
{
	if(!n)  return;
	procs.dequeue(n);
	
	// Don't need pipe_fd any more: 
	if(n->pipe_fd>=0)
	{
		#if TESTING
		if(!may_have_pipefd_set)
		{
			// This warning means that the Node is deleted but the pipe 
			// was not read. 
			// This should not happen with regular operation and thus 
			// should be investigated when it happens. 
			fprintf(stderr,"warning: ProcMan[%d]: n->pipe_fd=%d still set"
				" [INVESTIGATE THAT!]\n",n->pid,n->pipe_fd);
		}
		#endif
		
		CloseFD(n->pipe_fd);
		n->pipe_fd=-1;
	}
	
	delete n;
	--nproc;
	// Tell the manager that the sig node for that child is not 
	// needed any more: 
	fdmanager()->ExtraSigNodes(-1);
}


void ProcessManager::_Clear()
{
	while(procs.first())
	{  _DeleteNode(procs.first(),/*may_have_pipefd_set=*/1);  }
	#if TESTING
	if(nproc!=0)
	{  fprintf(stderr,"Oops: ProcessManager: nproc=%d != 0 after _Clear()\n",nproc);  }
	#endif
	nproc=0;
}


int ProcessManager::noop(int x)
{  return(x);  }


ProcessManager::ProcessManager(char *const _envp[],int *failflag=NULL) : 
	FDBase(),
	procs(),
	specialpb()
{
	int failed=0;
	
	nproc=0;
	term_kill_msec=4000;
	tktimer_running=0;
	envp=_envp;
	limitproc=-1;
	
	if(SetManager())
	{  --failed;  }
	if(!(tid=InstallTimer(-1,0)))  // alloc timer node 
	{  --failed;  }
	if(!(zmbchk=InstallTimer(-1,0)))  // alloc timer for zombie check
	{  --failed;  }
	
	if(failed)
	{
		if(failflag)
		{  *failflag+=failed;  return;  }
		ConstructorFailedExit("ProcMan");
	}
	
	// init global manager: 
	#if TESTING
	if(manager)
	{  fprintf(stderr,"%s: more than one ProcessManager.\n",prg_name);  exit(1);  }
	#endif
	
	manager=this;
}

ProcessManager::~ProcessManager()
{
	_Clear();
	
	#if TESTING
	if(!specialpb.is_empty())
	{  fprintf(stderr,"Oops: ~ProcessManager: special ProcessBase(s) still "
		"registered\n");  }
	#endif
	
	// cleanup global manager: 
	manager=NULL;
}

/******************************************************************************/

// Close all FDs except those in the list `exclude' of size n. 
int FDManager::CloseAllFDs(int *exclude,int n)
{
	int *excludeend=exclude+n;
	int errors=0;
	for(FDBNode *fb=fdblist.first; fb; fb=fb->next)
	{
		FDBase *fdb=fb->fdb;
		#if TESTING
		if(!fdb->fdslock)  // already locked
		{  fprintf(stderr,"FD: *** OOPS: fds already locked! (CA)\n");  }
		#endif
		fdb->LockFDs();
		for(FDManager::FDNode *i=fdb->fds; i; i=i->next)
		{
			for(int *ex=exclude; ex<excludeend; ex++)
			{
				if(i->fd == (*ex))
				{  goto cont;  }
			}
			// MAY NOT shutdown HERE!
			if(fdb->CloseFD(i->fd))
			{
				if(errno!=EBADF)  // actually, this should never happen...
				{  ++errors;  }
				#if TESTING
				else
				{  fprintf(stderr,"FD: Oops?!: CloseAllFDs: "
					"trapped bad fd %d\n",i->fd);  }
				#endif
			}
			cont:;
		}
		fdb->UnlockFDs();
	}
	return(errors);
}

