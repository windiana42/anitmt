/*
 * check_misc.cc
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

#include "prototypes.h"

#include <assert.h>
#include <signal.h>
#include <fcntl.h>

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <netinet/in.h>


int sig_caught=0;
void sig_handler(int /*sh*/)
{
	++sig_caught;
}


int constructed=0,destroyed=0;
int constr_will_fail=0;
class MyObj
{
	private:
		char data[100];
	public:  _CPP_OPERATORS_FF
		MyObj(int *failflag=NULL)
		{
			data[0]=data[99]=~data[49];  ++constructed;
			if(constr_will_fail)
			{  --(*failflag);  }
		}
		~MyObj()
		{  data[1]=data[98]=~data[50];  ++destroyed;  }
};


char *prg_name="check_misc";

int main()
{
	int failed=0;
	
	// Check size types: 
	fprintf(stderr,"sizeof(size_t)=%u; sizeof(ssize_t)=%u; sizeof(char *)=%u; ",
		sizeof(size_t),sizeof(ssize_t),sizeof(char *));
	if(sizeof(size_t)!=sizeof(ssize_t))
	{  fprintf(stderr,"***\nssize_t is illegal type!\n");  ++failed;  }
	else if(sizeof(char*)>sizeof(ssize_t))
	{  fprintf(stderr,"***\nOOPS: size_t smaller than pointer!\n");  ++failed;  }
	else
	{  fprintf(stderr,"OK\n");  }
	
	// Print some other things:
	fprintf(stderr,"sizeof(char)=%u; sizeof(short int)=%u; "
		"sizeof(int)=%u; sizeof(long int)=%u\n",
		sizeof(char),sizeof(short int),sizeof(int),sizeof(long int));
	fprintf(stderr,"sizeof(u_int16_t)=%u; sizeof(u_int32_t)=%u; sizeof(u_int64_t)=%u ",
		sizeof(u_int16_t),sizeof(u_int32_t),sizeof(u_int64_t));
	if(sizeof(u_int16_t)!=2 || sizeof(u_int32_t)!=4 || sizeof(u_int64_t)!=8)
	{  fprintf(stderr,"***ERROR\n");  ++failed;  }
	else
	{  fprintf(stderr,"OK\n");  }
	fprintf(stderr,"sizeof(int16_t)=%u; sizeof(int32_t)=%u; sizeof(int64_t)=%u ",
		sizeof(int16_t),sizeof(int32_t),sizeof(int64_t));
	if(sizeof(int16_t)!=2 || sizeof(int32_t)!=4 || sizeof(int64_t)!=8)
	{  fprintf(stderr,"***ERROR\n");  ++failed;  }
	else
	{  fprintf(stderr,"OK\n");  }
	
	// LMalloc statistics: 
	void *ptr=LMalloc(1024);
	assert(ptr);
	struct LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"Allocated 1024 bytes: curr_used=%u in %u chunks; ",
		lmu.curr_used,lmu.used_chunks);
	if(lmu.curr_used>=1024 && lmu.used_chunks==1)
	{  fprintf(stderr,"OK\n");  }
	else
	{  fprintf(stderr,"***HMMM\n");  }
	if(lmu.curr_used<1024)
	{
		fprintf(stderr,"  Byte counting does not work ");
		#ifdef HLIB_DONT_USE_MALLOC_USABLE_SIZE
		fprintf(stderr,"but we already know that.\n");
		#else
		fprintf(stderr,"*** which seems to be a bug! Please check that.\n");
		++failed;
		#endif
	}
	ptr=LFree(ptr);
	assert(!ptr);
	LMallocGetUsage(&lmu);
	fprintf(stderr,"  Freed mem: curr_used=%u in %u chunks; ",
		lmu.curr_used,lmu.used_chunks);
	if(lmu.curr_used || lmu.used_chunks)
	{  fprintf(stderr,"***\nOOPS: statistics wrong!\n");  ++failed;  }
	else
	{  fprintf(stderr,"OK\n");  }
	
	// Object allocation: 
	constructed=0; destroyed=0; constr_will_fail=0;
	fprintf(stderr,"Object allocation (ok): NEW<>: ");
	MyObj *myobj=NEW<MyObj>();
	if(myobj && constructed==1)
	{
		LMallocGetUsage(&lmu);
		if(lmu.used_chunks)
		{  fprintf(stderr,"OK (%u)",lmu.used_chunks);  }
		else
		{  fprintf(stderr,"***NOT USING LMALLOC!!");  ++failed;  }
		fprintf(stderr,"; delete: ");
		delete myobj;
		LMallocGetUsage(&lmu);
		if(destroyed==1)
		{
			if(!lmu.used_chunks)
			{  fprintf(stderr,"OK\n");  }
			else
			{  fprintf(stderr,"***NOT USING LFREE!! (%d)\n",
				lmu.used_chunks);  ++failed;  }
		}
		else
		{  fprintf(stderr,"***NOT DONE\n");  ++failed;  }
	}
	else if(myobj)
	{  fprintf(stderr,"***NOT CONSTRUCTED\n");  ++failed;  }
	else
	{  fprintf(stderr,"***ALLOC FAILURE\n");  ++failed;  }
	//----
	constructed=0; destroyed=0; constr_will_fail=1;
	fprintf(stderr,"Object allocation (fail): NEW<>: ");
	myobj=NEW<MyObj>();
	LMallocGetUsage(&lmu);
	if(myobj)
	{  fprintf(stderr,"***OOPS: failure not detected!\n");  ++failed;  }
	else if(constructed==1 && destroyed==1)
	{
		if(lmu.used_chunks==0)
		{  fprintf(stderr,"OK\n");  }
		else
		{  fprintf(stderr,"***MEM NOT DEALLOCATED.\n");  ++failed;  }
	}
	else
	{  fprintf(stderr,"***FAILED: constructed=%d; destroyed=%d; used chunks=%u\n",
		constructed,destroyed,lmu.used_chunks);  ++failed;  }
	
	// Get terminal size call: 
	int nrows=-1,ncols=-1;
	fprintf(stderr,"GetTerminalSize: using ");
	int fd=open("/dev/tty",O_RDONLY);
	if(fd>=0)  fprintf(stderr,"/dev/tty: ");
	else
	{  fprintf(stderr,"stdin: ");  fd=0;  }
	errno=0;
	switch(GetTerminalSize(fd,&nrows,&ncols))
	{
		case 0:
			fprintf(stderr,"OKAY: %d x %d\n",nrows,ncols);
			if(nrows<1 || ncols<1)
			{  fprintf(stderr,"*** but these are illegal values!\n");  ++failed;  }
			break;
		case 1:
			fprintf(stderr,"*UNSUPPORTED*\n");
			break;
		case -1:
			fprintf(stderr,"Not a tty! [check failed]\n");
			++failed;
			break;
		case -2:
			fprintf(stderr,"(%dx%d) ioctl failed: %s ***\n",
				nrows,ncols,strerror(errno));
			++failed;
			break;
		default:
			fprintf(stderr,"ILLEGAL RETVAL! ***\n");
			++failed;
			break;
	}
	
	// Install signal handler: 
	fprintf(stderr,"InstallSignalHandler: ");
	errno=0;
	if(InstallSignalHandler(SIGUSR1,&sig_handler,0))
	{  fprintf(stderr,"FAILED: %s ***\n",strerror(errno));  ++failed;  }
	else
	{
		fprintf(stderr,"OK\n");
		for(int i=0; i<10; i++)
		{
			kill(getpid(),SIGUSR1);
			usleep(10000);
		}
		fprintf(stderr,"  Caught %d/10 sigs",sig_caught);
		if(sig_caught<10)
		{  fprintf(stderr," ***\n");  ++failed;  }
		else
		{  fprintf(stderr," OK\n");  }
	}
	
	#if defined(HLIB_CRIPPLED_SIGINFO_T) || defined(HLIB_PROCMAN_USE_LESS_SIGINFO_T)
	 fprintf(stderr,"  Note: defined:");
	  #ifdef HLIB_PROCMAN_USE_LESS_SIGINFO_T
	  fprintf(stderr," HLIB_PROCMAN_USE_LESS_SIGINFO_T");
	  #endif
	  #ifdef HLIB_CRIPPLED_SIGINFO_T
	  fprintf(stderr," HLIB_CRIPPLED_SIGINFO_T");
	  #endif
	 fprintf(stderr,"\n");
	#endif
	
	// Set fd nonblocking: 
	fprintf(stderr,"SetNonblocking: ");
	fd=0;  // We're not using stdin below here. 
	if(SetNonblocking(fd))
	{  fprintf(stderr,"FAILED: %s\n",strerror(errno));  ++failed;  }
	else
	{  fprintf(stderr,"OK\n");
		// Okay, check if it really works: 
		fprintf(stderr,"  NOTE: If the program hangs here, non-blocking IO "
			"does NOT work...");
		int okay=-1;
		for(;;)
		{
			char buf[256];
			ssize_t rv=read(fd,buf,256);
			if(rv<0)
			{
				if(errno==EWOULDBLOCK)
				{
					// Great, it works. 
					okay=1;  break;
				}
				else
				{  fprintf(stderr,"\n  ***Read error: %s\n",strerror(errno));  
					++failed;  okay=0;  break;  }
			}
			else if(rv==0)
			{  fprintf(stderr,"\n  Reached EOF; could not test if reading blocks. ***\n");
				++failed;  okay=0;  break;  }
		}
		if(okay>0)
		{  fprintf(stderr,"\n  Okay, read() reported EWOULDBLOCK\n");  }
		else if(okay<0)
		{  fprintf(stderr,"\n  *** nonblocking IO does not seem to work\n");
			++failed;  }
	}
	
	// Disable nagle algorithm: 
	fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(fd<0)
	{  fprintf(stderr,"Failed to create TCP socket: %s\n",
		strerror(errno));  ++failed;  }
	else
	{
		if(SetTcpNoDelay(fd))
		{  fprintf(stderr,"SetTcpNoDelay: failed: %s ***\n",
			strerror(errno));  ++failed;  }
		else
		{  fprintf(stderr,"SetTcpNoDelay: OK\n");  }
		close(fd);
	}
	
	// Get system load: 
	fprintf(stderr,"System load average: ");
	errno=0;
	int loadval=GetLoadAverage();
	if(loadval>=0)
	{  fprintf(stderr,"OK, %.2f\n",double(loadval/100.0));  }
	else
	{
		#if HAVE_GETLOADAVG
		fprintf(stderr,"***FAILED: %s\n",strerror(errno));  ++failed;
		#else
		fprintf(stderr,"FAILED but we don't have getloadavg()\n");
		// ++failed NOT increased here. 
		#endif
	}
	
	fprintf(stderr,"REPORT: %d tests failed.\n",failed);
	return(failed ? 1 : 0);
}

