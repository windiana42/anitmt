/*
 * check_fd.cc
 * 
 * This is not a partof libhlib.a but a part of hlib distribution. 
 * This code is used to test some functionality in hlib. 
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

// NOTE: This test suite does NOT test the complete functionality of 
//       FDManager and ProcessManager buf the main features. The primary 
//       purpose of this test program is when you're compiling hlib on 
//       some arch different to the developer's arch to see if things 
//       can be assumes to behave as expected. 
// HOWEVER, currently those features in ProcMisc which need root perms 
//       are NOT tested. 

#define HLIB_IN_HLIB 1
#include <hlib/prototypes.h>

#include "htime.h"
#include "fdmanager.h"
#include "fdbase.h"

#include "procmanager.h"
#include "procbase.h"

#include <fcntl.h>

#include <assert.h>


class FDTester : FDBase
{
	private:
		TimerID stop_test_tid;
		TimerID little_timer;
		TimerID tid;
		
		HTime ht;
		int action;
		int little_timer_running;
		int caught_little_timer;
		int fail;
		int fd;
		
		// Overriding virtuals: 
		int timernotify(TimerInfo *);
		int fdnotify(FDInfo *);
		int signotify(const SigInfo *);
	public:  _CPP_OPERATORS_FF
		FDTester(int *failflag=NULL);
		~FDTester();
};


int FDTester::timernotify(TimerInfo *ti)
{
	if(ti->tid==stop_test_tid)
	{
		fprintf(stderr,"*** Test timed out.\n");
		fdmanager()->Quit(fail=1);
		DeleteMe();
		return(0);
	}
	
	if(ti->tid==little_timer)
	{
		++caught_little_timer;
		if(little_timer_running)
		{
			if(caught_little_timer==5)
			{
				fprintf(stderr,"OK; little timer caught 5 times.\n");
				UpdateTimer(little_timer,-1,0);  // disable it
				little_timer_running=0;
			}
		}
		else
		{
			fprintf(stderr,"*** OOPS: caught little timer although it should "
				"not be running\n");
			++fail;
		}
		return(0);
	}
	
	assert(ti->tid==tid);
	
	switch(action)
	{
		case 0:
			// First, start little timer: 
			little_timer=InstallTimer(20,0);
			assert(little_timer);
			little_timer_running=1;
			// Let's send us a SIGCHLD.
			action=1;
			ht.SetCurr();
			raise(SIGCHLD);
			UpdateTimer(tid,-1,0);
			fprintf(stderr,"Sent SIGCHLD...");
			break;
		case 2:
		{
			// Want to read from /dev/null: 
			action=3;
			fd=open("/dev/zero",O_RDONLY);
			if(fd<0)
			{  fprintf(stderr,"*** Failed to open /dev/null: %s\n",
				strerror(errno));  ++fail;  DeleteMe();  }
			int rv=PollFD(fd,POLLIN);
			assert(rv==0);
			UpdateTimer(tid,-1,0);
			fprintf(stderr,"Waiting for data from /dev/null... ");
		}  break;
		case 6:
			// Okay, data from /dev/null was read. 
			
			// MORE TESTS...
			
			// Finally wait for the little timers to come: 
			UpdateTimer(tid,600,0);
			action=1000;
			break;
		case 1000:
			fprintf(stderr,"Timer: tests completed (fail=%d)\n",fail);
			DeleteMe();
			break;
		default:
			fprintf(stderr,"*** OOPS: illegal action %d\n",action);
			abort();
	}
	
	return(0);
}

int FDTester::fdnotify(FDInfo *fdi)
{
	if(action<3 || action>5)
	{
		static int nerr=0;
		fprintf(stderr,"*** OOPS: fdnotify(%d,%d,%d) but we did not "
			"poll for any fd (action=%d)",
			fdi->fd,fdi->events,fdi->revents,action);
		++fail;
		++nerr;
		if(nerr>=4)
		{  DeleteMe();  }
		return(0);
	}
	
	assert(fdi->fd==fd);
	assert(fdi->events==POLLIN);
	if(fdi->revents & POLLIN)
	{
		if(action==3)
		{
			fprintf(stderr,"OK\n");
			action=4;
			return(0);
		}
		else if(action==4 || action==5)
		{
			fprintf(stderr,"Reading 1024 bytes [%d]: ",action-3);
			char buf[1024];
			ssize_t rd=read(fd,buf,1024);
			if(rd<0)
			{  fprintf(stderr,"%s ***\n",strerror(errno));  ++fail;  }
			else
			{  fprintf(stderr,"OK\n");  }
			if(action==4)  action=5;
			else
			{
				PollFD(fd,0);
				action=6;
				UpdateTimer(tid,10,0);
			}
		}
	}
	else
	{  fprintf(stderr,"*** Unexpected revents=%d (events=%d)\n",
		fdi->revents,fdi->events);  ++fail;  }
	return(0);
}

int FDTester::signotify(const SigInfo *si)
{
	if(si->info.si_signo==SIGINT)
	{
		fprintf(stderr,"*** INTERRUPT\n");
		exit(1);
	}
	if(action==1)
	{
		assert(si->info.si_signo==SIGCHLD);
		fprintf(stderr,"OK; caught signal (%ld msec)\n",
			ht.MsecElapsedR());
		action=2;
		UpdateTimer(tid,0,0);
	}
	return(0);
}


FDTester::FDTester(int *failflag) : 
	FDBase(failflag),
	ht()
{
	stop_test_tid=InstallTimer(5000,0);
	tid=InstallTimer(0,0);
	little_timer=NULL;
	action=0;
	little_timer_running=0;
	caught_little_timer=0;
	fail=0;
	fd=-1;
	if(!tid || !stop_test_tid)
	{  --(*failflag);  }
}

FDTester::~FDTester()
{
	CloseFD(fd);
	
	if(fail)  return;
	
	if(caught_little_timer!=5)
	{  fprintf(stderr,"*** caught little timer %d/5 times (error)\n",
		caught_little_timer);  ++fail;  }
	
	if(fail)
	{  fdmanager()->Quit(fail);  }
}


static int RunFDTests()
{
	int failed=0;
	
	// Okay, the FDManager is up and running. We need an FDBase-derived 
	// class. 
	FDTester *tst=NEW<FDTester>();
	if(!tst)
	{  fprintf(stderr,"*** FD tester class init failed\n");  ++failed;  }
	
	if(!failed)
	{  failed=FDManager::manager->MainLoop();  }
	
	// tst deletes itself. 
	
	return(failed ? 1 : 0);
}


/******************************************************************************/

class ProcTester : 
	FDBase,
	ProcessBase
{
	private:
		TimerID tid;
		TimerID timeout_tid;
		
		int fail;
		int action;
		int devnull_fd;
		pid_t pid;
		
		const char *acion_str(PSAction a);
		const char *detail_str(PSDetail d);
		
		// Overriding virtual from FDBase: 
		int timernotify(TimerInfo *);
		void sigint()
		{
			fprintf(stderr,"*** INTERRUPTED\n");
			exit(1);
		}
		// Overriding virtual from ProcessBase: 
		void procnotify(const ProcStatus *ps);
	public:  _CPP_OPERATORS_FF
		ProcTester(int *failflag);
		~ProcTester();
};


int ProcTester::timernotify(TimerInfo *ti)
{
	if(ti->tid==timeout_tid)
	{
		fprintf(stderr,"*** TEST TIMED OUT!\n");
		++fail;
		DeleteMe();
		return(0);
	}
	
	assert(ti->tid==tid);
	
	UpdateTimer(tid,-1,0);
	
	// Okay, launch a process: 
	switch(action)
	{
		case 0:
		{
			ProcMisc pmisc;
			pmisc.wdir("..");
			pmisc.niceval(19);
			ProcFDs pfd;
			int arv=pfd.Add(devnull_fd,1);
			assert(arv==0);
			fprintf(stderr,"Forking for \"ls -l . > /dev/null\" in "
				"parent dir, nice 19... ");
			pid_t rv=StartProcess(
				ProcPath("ls","/bin","/usr/bin",NULL),
				ProcArgs("ls","-l",".",NULL),
				pmisc,
				pfd);
			if(rv>0)
			{
				fprintf(stderr,"(pid=%ld)\n",long(pid=rv));
				action=1;
			}
			else
			{
				fprintf(stderr,"*** FAILED\n");
				++fail;
				DeleteMe();
			}
		}  break;
		case 2:
		case 5:
		{
			fprintf(stderr,"Forking for \"sleep 1\"... ");
			pid_t rv=StartProcess(
				ProcPath("sleep","/bin","/usr/bin","/usr/local/bin",NULL),
				ProcArgs("sleep","1",NULL));
			if(rv>0)
			{
				fprintf(stderr,"(pid=%ld)\n",long(pid=rv));
				++action;
			}
			else
			{
				fprintf(stderr,"*** FAILED\n");
				++fail;
				DeleteMe();
			}
		}  break;
		case 7:
		{
			// Kill process. 
			fprintf(stderr,"  Scheduling TERM/KILL on [%ld]... ",long(pid));
			errno=0;
			int rv=TermProcess(pid);
			if(rv)
			{
				fprintf(stderr,"  *** Failed: %d, %s\n",rv,strerror(errno));
				++fail;
				action=9;  // skip...
				UpdateTimer(tid,0);
				break;
			}
			else
			{  fprintf(stderr,"OK\n");  }
			action=8;
		}  break;
		case 9:
		{
			// Start non-existing process: 
			fprintf(stderr,"Forking for \"re41ly/n0n3xisTing/diR/fazz\"... ");
			pid_t rv=StartProcess(
				ProcPath("re41ly/n0n3xisTing/diR/fazz"),
				ProcArgs("fazz",NULL));
			if(rv>0)
			{
				fprintf(stderr,"*** WUH?! pid=%ld (FAILURE!)\n",
					long(pid=rv));
				++fail;
				DeleteMe();
			}
			else
			{
				fprintf(stderr,"failed as expected\n");
				++action;
				UpdateTimer(tid,0,0);
			}
		}  break;
		case 10:
		{
			// Start failing process: 
			fprintf(stderr,"Forking for \"test 10 -gt 20\"... ");
			pid_t rv=StartProcess(
				ProcPath("test","/bin","/usr/bin","/usr/local/bin",NULL),
				ProcArgs("test","10","-gt","20",NULL));
			if(rv>0)
			{
				fprintf(stderr,"(pid=%ld)\n",long(pid=rv));
				++action;
			}
			else
			{
				fprintf(stderr,"*** FAILED\n");
				++fail;
				DeleteMe();
			}
		}  break;
		case 12:
		{
			fprintf(stderr,"%sAll done: fail=%d\n",fail ? "*** " : "",fail);
			DeleteMe();
		}  break;
		default: assert(0);
	}
	
	return(0);
}

const char *ProcTester::acion_str(PSAction a)
{
	switch(a)
	{
		case PSFailed: return("PSFailed");
		case PSDead:  return("PSDead");
		case PSStopCont:  return("PSStopCont");
		case PSExecuted:  return("PSExecuted");
	}
	assert(0);
}

const char *ProcTester::detail_str(PSDetail d)
{
	if(d>=PSExecFailed && d<_PSLASTFailed)
	{  return(ProcessBase::PSDetail_SyscallString(d));  }
	switch(d)
	{
		case PSExited:  return("exited");
		case PSKilled:  return("killed");
		case PSDumped:  return("dumped");
		case PSStop:  return("stop");
		case PSCont:  return("cont");
		case PSSuccess:  return("success");
	}
	assert(0);
}

void ProcTester::procnotify(const ProcStatus *ps)
{
	fprintf(stderr,"  Notify[%ld]: %s: %s (estatus=%d)\n",
		long(ps->pid),
		acion_str(ps->action),detail_str(ps->detail),
		ps->estatus);
	int valid=0;
	switch(action)
	{
		case 1: valid = 
			(ps->action==PSDead && 
				ps->detail==PSExited && ps->estatus==0) ||
			(ps->action==PSExecuted && 
				(ps->detail==PSSuccess) );
			break;
		case 3:
			valid=ps->action==PSExecuted && ps->detail==PSSuccess;
			action=4;
			break;
		case 4:
			valid=ps->action==PSDead && 
				ps->detail==PSExited && ps->estatus==0;
			break;
		case 6:
			// sleep running, Wanna kill it. 
			valid=ps->action==PSExecuted && ps->detail==PSSuccess;
			action=7;
			UpdateTimer(tid,200,0);  // schedule kill
			break;
		case 8:
			valid=ps->action==PSDead && ps->detail==PSKilled && 
				(ps->estatus==SIGTERM || ps->estatus==SIGKILL);
			break;
		case 11:
			valid=(ps->action==PSDead && 
				(ps->detail==PSExited && ps->estatus!=0) ) ||
			(ps->action==PSExecuted && 
				(ps->detail==PSSuccess) );
			break;
		default: assert(0);
	}
	
	if(pid!=ps->pid)
	{
		fprintf(stderr,"  *** PID unexpected (expect %ld).\n",long(pid));
		++fail;
	}
	if(!valid)
	{
		fprintf(stderr,"  *** Unexpected. FAILURE (%d)\n",action);
		++fail;
	}
	
	if(ps->action==PSDead)
	{
		if(action==4 || action==8)
		{  fprintf(stderr,"    (Elapsed: %s)\n",
			(ps->endtime-ps->starttime).PrintElapsed());  }
		
		// Schedule starting of new process: 
		++action;
		UpdateTimer(tid,0,0);
		return;
	}
}


ProcTester::ProcTester(int *failflag)
{
	action=0;
	fail=0;
	pid=-1;
	
	devnull_fd=open("/dev/null",O_WRONLY);
	if(devnull_fd<0)
	{
		fprintf(stderr,"opening /dev/null: %s\n",strerror(errno));
		--(*failflag);
	}
	
	if(PollFD(devnull_fd,0))
	{  --(*failflag);  }
	
	tid=InstallTimer(0,0);
	if(!tid)
	{  --(*failflag);  }
	
	timeout_tid=InstallTimer(10000,0);  // 10 seconds
	if(!timeout_tid)
	{  --(*failflag);  }
}

ProcTester::~ProcTester()
{
	CloseFD(devnull_fd);
	
	if(fail)
	{  fdmanager()->Quit(1);  }
}


static int RunProcTests()
{
	fprintf(stderr,"Testing ProcessManager...\n");
	
	int failed=0;
	
	// Okay, the ProcManager is up and running. We need an ProcBase-derived 
	// class. 
	ProcTester *tst=NEW<ProcTester>();
	if(!tst)
	{  fprintf(stderr,"*** Proc rester class init failed\n");  ++failed;  }
	
	if(!failed)
	{  failed=FDManager::manager->MainLoop();  }
	
	// tst deletes itself. 
	
	return(failed ? 1 : 0);
}


/******************************************************************************/

static inline timeval *randtime()
{
	long msec=random();
	int s=msec%2;
	msec/=4;  // aginst long overflow in Get(HTime::msec)
	static timeval x;
	x.tv_sec=(s ? -1 : +1)*msec/1000;
	x.tv_usec=1000*(msec%1000);
	return(&x);
}


static int nfail=0;

void check_sub(const HTime &a,const HTime &b,const HTime &r)
{
	long av=a.Get(HTime::msec);
	long bv=b.Get(HTime::msec);
	long rv=r.Get(HTime::msec);
	if(av-bv != rv)  ++nfail;
}
void check_add(const HTime &a,const HTime &b,const HTime &r)
{
	long av=a.Get(HTime::msec);
	long bv=b.Get(HTime::msec);
	long rv=r.Get(HTime::msec);
	if(av+bv != rv)  ++nfail;
}

void check_add2(const HTime &a,const HTime &r,long dm  /* <- delta_msec*/)
{
	long av=a.Get(HTime::msec);
	long rv=r.Get(HTime::msec);
	if(av+dm != rv)  ++nfail;
}


static int htime_test_diff()
{
	fprintf(stderr,"  Testing HTime add/sub... ");
	for(int i=0; i<10000; i++)
	{
		HTime a,b;
		a.SetTimeval(randtime());
		b.SetTimeval(randtime());
		
		HTime add=a+b;   check_add(a,b,add);
		HTime sub=a-b;   check_sub(a,b,sub);
		
		HTime aa(a);  aa+=b;   if(aa!=add)  ++nfail;
		aa=a;  aa-=b;   if(aa!=sub)  ++nfail;
		
		aa=a;  aa.Add(+1499,HTime::msec);  check_add2(a,aa,+1499);
		aa=a;  aa.Add(-1499,HTime::msec);  check_add2(a,aa,-1499);
		aa=a;  aa.Sub(+1499,HTime::msec);  check_add2(a,aa,-1499);
		aa=a;  aa.Sub(-1499,HTime::msec);  check_add2(a,aa,+1499);
	}
	
	if(nfail)
	{
		fprintf(stderr,"*** FAILED (%d errors)\n",nfail);
		return(1);
	}
	fprintf(stderr,"passed\n");
	return(0);
}

static int check_htime()
{
	int fail=0;
	fprintf(stderr,"Testing HTime:\n");
	
	// Okay, first, the add / sub stress test
	fail+=htime_test_diff();
	
	// Other things are cuurently not tested. 
	
	if(fail)
	{  fprintf(stderr,"*** HTime test FAILED\n");  }
	else
	{  fprintf(stderr,"HTime tests passed\n");  }
	
	return(fail ? 1 : 0);
}


/******************************************************************************/

char *prg_name="check_fd";

int main(int,char**,char**envp)
{
	fprintf(stderr,"Running FD test program...\n");
	srandom(getpid());
	
	/* ---HTime--- */
	if(check_htime())
	{
		fprintf(stderr,"*** OOPS: bug in HTime. "
			"Please submit a bug report NOW. ***\n");
		return(1);
	}
	
	FDManager *fdm=NEW<FDManager>();
	if(!fdm)
	{  fprintf(stderr,"*** FDManager initialisation failed.\n");  return(1);  }
	
	/*  ---FD--- */
	int rv=RunFDTests();
	
	/* ---PROC--- */
	if(!rv)
	{
		ProcessManager *procman=NEW1<ProcessManager>(envp);
		if(!procman)
		{  fprintf(stderr,"*** ProcManager init failed.\n");  return(1);  }
		
		// Set short term-kill-delay: 
		procman->TermKillDelay(200);
		
		rv=RunProcTests();
		
		delete procman;
	}
	
	delete fdm;
	
	struct LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	if(lmu.used_chunks)
	{
		fprintf(stderr,
			"*** Exiting with %u bytes non-freed memory in %u chunks\n"
			"    This is a memory leak in hlib (or the test proggy...)\n",
		lmu.curr_used,lmu.used_chunks);
		rv=1;
	}
	return(rv);
}
