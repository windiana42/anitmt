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

#include <hlib/prototypes.h>

#include <signal.h>
#include <fcntl.h>

#include "htime.h"
#include "fdmanager.h"
#include "fdbase.h"

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


static int RunTests()
{
	int failed=0;
	
	// Okay, the FDManager is up and running. We need an FDBase-derived 
	// class. 
	FDTester *tst=NEW<FDTester>();
	if(!tst)
	{  fprintf(stderr,"*** Tester class init failed\n");  ++failed;  }
	
	if(!failed)
	{  failed=FDManager::manager->MainLoop();  }
	
	// tst deletes itself. 
	
	return(failed ? 1 : 0);
}


char *prg_name="check_fd";

int main()
{
	fprintf(stderr,"Running FD test program...\n");
	
	FDManager *fdm=NEW<FDManager>();
	if(!fdm)
	{  fprintf(stderr,"*** FDManager initialisation failed.\n");  return(1);  }
	
	int rv=RunTests();
	
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
