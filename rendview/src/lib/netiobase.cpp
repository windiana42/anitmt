/*
 * netiobase.cpp
 * 
 * Network IO (e.g. for LDR client and sever) as a layer on top of
 * FDCopyBase. 
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "netiobase.hpp"

#include <ctype.h>
#include <assert.h>

#include <fcntl.h>


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


char *NetworkIOBase::CPNotifyStatusString(const FDCopyBase::CopyInfo *cpi)
{
	static char tmp[256];   // Don't make it too small -- I use strcpy(). 
	char *str=tmp,*end=tmp+256;
	
	strcpy(str,"status:");  str+=strlen(str);
	
	if(cpi->scode & FDCopyPump::SCFinal)
	{  strcpy(str," Final");  str+=strlen(str);  }
	if(cpi->scode & FDCopyPump::SCLimit)
	{  strcpy(str," Limit");  str+=strlen(str);  }
	if(cpi->scode & FDCopyPump::SCKilled)
	{  strcpy(str," Killed");  str+=strlen(str);  }
	if(cpi->scode & FDCopyPump::SCTerm)
	{  strcpy(str," Terminated");  str+=strlen(str);  }
	if(cpi->scode & FDCopyPump::SCTimeout)
	{
		snprintf(str,end-str," Timeout[%d]",cpi->err_no);
		str+=strlen(str);
	}
	if(cpi->scode & FDCopyPump::SCEOI)
	{
		snprintf(str,end-str," InputEOF (%s%s%s)",
			(cpi->scode & FDCopyPump::SCInHup) ? "Hup" : "",
			(cpi->scode & FDCopyPump::SCInPipe) ? "Pipe" : "",
			(cpi->scode & FDCopyPump::SCRead0) ? "Read" : "");
		str+=strlen(str);
	}
	if(cpi->scode & FDCopyPump::SCEOO)
	{
		snprintf(str,end-str," OutputEOF (%s%s%s)",
			(cpi->scode & FDCopyPump::SCOutHup) ? "Hup" : "",
			(cpi->scode & FDCopyPump::SCOutPipe) ? "Pipe" : "",
			(cpi->scode & FDCopyPump::SCWrite0) ? "Write" : "");
		str+=strlen(str);
	}
	if(cpi->scode & FDCopyPump::SCError)
	{
		snprintf(str,end-str," Error (");  str+=strlen(str);
		if(cpi->scode & (FDCopyPump::SCErrPollI | FDCopyPump::SCErrPollO))
		{
			snprintf(str,end-str,"Poll%s: rev=0x%x",
				((cpi->scode & FDCopyPump::SCErrPollI) && (cpi->scode & FDCopyPump::SCErrPollO)) ? "IO" : 
					(cpi->scode & FDCopyPump::SCErrPollI) ? "In" : "Out",
				cpi->err_no);
			str+=strlen(str);
		}
		if(cpi->scode & (FDCopyPump::SCErrRead | FDCopyPump::SCErrWrite))
		{
			snprintf(str,end-str,"%s: %s",
				((cpi->scode & FDCopyPump::SCErrRead) && (cpi->scode & FDCopyPump::SCErrWrite)) ? "RW" : 
					(cpi->scode & FDCopyPump::SCErrRead) ? "Read" : "Write",
				strerror(cpi->err_no));
			str+=strlen(str);
		}
		if(cpi->scode & FDCopyPump::SCErrCopyIO)
		{
			snprintf(str,end-str,"CopyIO: rv=%d",cpi->err_no);
			str+=strlen(str);
		}
		snprintf(str,end-str,")");  str+=strlen(str);
	}
	return(tmp);
}


void NetworkIOBase::_DoChangeEvents_Error(int rv)
{
	fprintf(stderr,"OOPS: FDChangeEvents(sock=%d,%p) returned %d [abort?]\n",
		sock_fd,pollid,rv);
}


char *NetworkIOBase::CopyStrList2Data(char *dest,const RefStrList *l)
{
	for(const RefStrList::Node *i=l->first(); i; i=i->next)
	{
		const char *s=i->str();
		while( (*(dest++)=*(s++)) );  // The routine that made C famous - ;)
	}
	return(dest);
}


char *NetworkIOBase::CopyData2StrList(RefStrList *dest,const char *data,
	int n,size_t data_len)
{
	const char *dataend=data+data_len;
	while(n>0)
	{
		for(const char *str=data; data<dataend; data++)
		{
			if(*data=='\0')
			{
				++data;
				if(dest->append(str))
				{  return(NULL);  }
				--n;
				goto done;
			}
		}
		// Reached data end. 
		return((char*)(dataend-data_len-1));  // -> original data[-1]
		done:;
	}
	return((char*)data);
}

int NetworkIOBase::CopyData2StrList(RefStrList *dest,const char *data,
	size_t data_len)
{
	if(data_len && data[data_len-1]!='\0')
	{  return(-2);  }
	
	const char *dataend=data+data_len;
	const char *str=data;
	int n=0;
	for(const char *c=data; c<dataend; c++)
	{
		if(*c=='\0')
		{
			if(dest->append(str))
			{  return(-1);  }
			str=c+1;
			++n;
		}
	}
	
	return(n);
}


// Start sending passed buffer using FDCopyBase etc. 
int NetworkIOBase::_FDCopyStartSendBuf(char *buf,size_t len)
{
	if(!pollid)  return(0);  // Cannot do anything; already closed socket. 
	
	if(out.pump_s->is_dead)
	{  out.pump_s->PumpReuseNow();  }
	
	assert(out.ioaction==IOA_None);
	
	out.io_buf->buf=buf;
	out.io_buf->buflen=len;
	out.io_buf->bufdone=0;
	
	assert(out.io_sock->pollid==pollid);
	
	int rv=out.pump_s->SetIO(out.io_buf,out.io_sock);
	if(rv!=0 && rv!=-1)
	{
		// If rv=-5, you probably need FDCopyPump::PumpReuseNow(). 
		fprintf(stderr,"OOPS: out pump->SetIO(buf,fd) returned %d\n",rv);
		abort();
	}
	if(rv)  return(rv);
	
	rv=out.pump_s->Control(FDCopyPump::CC_Start);
	if(rv!=0)
	{
		fprintf(stderr,"OOPS: out pump(buf,fd)->CC_Start returned %d\n",rv);
		abort();
	}
	
	out.ioaction=IOA_SendingBuf;
	
	return(0);
}

// See _FDCopyStartSendBuf()
int NetworkIOBase::_FDCopyStartRecvBuf(char *buf,size_t len)
{
	if(!pollid)  return(0);  // Cannot do anything; already closed socket. 
	
	if(in.pump_s->is_dead)
	{  in.pump_s->PumpReuseNow();  }
	
	assert(in.ioaction==IOA_None);
	
	in.io_buf->buf=buf;
	in.io_buf->buflen=len;
	in.io_buf->bufdone=0;
	
	assert(in.io_sock->pollid==pollid);
	
	int rv=in.pump_s->SetIO(in.io_sock,in.io_buf);
	if(rv!=0 && rv!=-1)
	{
		// If rv=-5, you probably need FDCopyPump::PumpReuseNow(). 
		fprintf(stderr,"OOPS: in pump->SetIO(fd,buf) returned %d\n",rv);
		abort();
	}
	if(rv)  return(rv);
	
	rv=in.pump_s->Control(FDCopyPump::CC_Start);
	if(rv!=0)
	{
		fprintf(stderr,"OOPS: in pump(fd,buf)->CC_Start returned %d\n",rv);
		abort();
	}
	
	in.ioaction=IOA_SendingBuf;
	
	return(0);
}


// Start sending passed file using FDCopyBase etc. 
int NetworkIOBase::_FDCopyStartSendFile(const char *path,int64_t filelen)
{
	if(!pollid)  return(0);  // Cannot do anything; already closed socket. 
	
	assert(out.ioaction==IOA_None);
	
	// This is special: We cannot send a file with size 0 with limit 
	// because limit=0 is "unlimited". So, for simplicity, we simply 
	// send a buffer of size 1: (See recv-side for explanation on that.) 
	if(filelen==0)
	{
		static char _dummybuf='\0';
		return(_FDCopyStartSendBuf(&_dummybuf,1));  // YES, size 1
	}
	
	if(filelen<0)
	{  return(-3);  }
	
	// Open the file: 
	int fd;
	do
	{  fd=::open(path,O_RDONLY);  }
	while(fd<0 && errno==EINTR);
	if(fd<0)  return(-2);
	
	PollID send_pollid=NULL;
	int rv=PollFD(fd,0,NULL,&send_pollid);
	if(rv==-1)
	{
		do
		{  rv=::close(fd);  }
		while(rv<0 && errno==EINTR);
		assert(rv>=0);  // Actually never fails, right?
		return(-1);
	}
	if(rv || !send_pollid)
	{
		fprintf(stderr,"OOPS: PollFD failed: %d,%d\n",rv,fd);
		abort();
	}
	
	if(out.pump_fd->is_dead)
	{  out.pump_fd->PumpReuseNow();  }
	
	out.io_fd->pollid=send_pollid;
	assert(!out.io_fd->transferred);  // io params must be reset by copy facility
	out.pump_fd->limit=filelen;
	out.pump_fd->io_bufsize=16384;
	// Reset thresh values: This is necessary so that they are re-calculated 
	// again because thresh values are never larger than copy length limit. 
	out.pump_fd->low_read_thresh=-1;
	out.pump_fd->high_read_thresh=-1;
	out.pump_fd->low_write_thresh=-1;
	out.pump_fd->high_write_thresh=-1;
	
	assert(out.io_sock->pollid==pollid);
	
	rv=out.pump_fd->SetIO(out.io_fd,out.io_sock);
	if(rv!=0 && rv!=-1)
	{
		// If rv=-5, you probably need FDCopyPump::PumpReuseNow(). 
		fprintf(stderr,"OOPS: out pump->SetIO(fd,fd) returned %d\n",rv);
		abort();
	}
	if(rv)  return(rv);
	
	rv=out.pump_fd->Control(FDCopyPump::CC_Start);
	if(rv!=0)
	{
		fprintf(stderr,"OOPS: out pump(fd,fd)->CC_Start returned %d\n",rv);
		fprintf(stderr,"      limit=%ld; iobs=%u; thresh=%u,%u,%u,%u\n",
			long(out.pump_fd->limit),out.pump_fd->io_bufsize,
			out.pump_fd->low_read_thresh,out.pump_fd->high_read_thresh,
			out.pump_fd->low_write_thresh,out.pump_fd->high_write_thresh);
		abort();
	}
	
	out.ioaction=IOA_SendingFD;
	
	return(0);
}

// Start receiving passed file using FDCopyBase etc. 
int NetworkIOBase::_FDCopyStartRecvFile(const char *path,int64_t filelen)
{
	if(!pollid)  return(0);  // Cannot do anything; already closed socket. 
	
	assert(in.ioaction==IOA_None);
	
	if(filelen<0)
	{  return(-3);  }
	
	#if 0  /* Enable to get warning when an existring file is getting overwritten. */
	if(!::access(path,F_OK))
	{  Warning("HMMM: Overwriting existing file \"%s\".\n\a",path);  }
	#endif
	
	// Open the file: 
	int fd;
	do
	{  fd=::open(path,O_WRONLY | O_CREAT | O_TRUNC,0666);  }
	while(fd<0 && errno==EINTR);
	if(fd<0)  return(-2);
	
	// This is special: We cannot receive a file with size 0 with limit 
	// because limit=0 is "unlimited". So, for simplicity, we simply 
	// receive a buffer of size 1 and create a file of size 0. 
	// (YES, buffer size 1 is necessary so that the server can send one 
	// byte and POLLIN will occur. Otherwise POLLIN will not occur because 
	// of lack of data and deadlock occurs instead. 
	if(filelen==0)
	{
		// File already created; close it again: 
		int rv;
		do
		{  rv=::close(fd);  }
		while(rv<0 && errno==EINTR);
		assert(rv>=0);  // Actually never fails, right?
		
		static char _dummybuf='\0';
		return(_FDCopyStartRecvBuf(&_dummybuf,1));   // YES, size 1
	}
	
	PollID recv_pollid=NULL;
	int rv=PollFD(fd,0,NULL,&recv_pollid);
	if(rv==-1)
	{
		do
		{  rv=::close(fd);  }
		while(rv<0 && errno==EINTR);
		assert(rv>=0);  // Actually never fails, right?
		return(-1);
	}
	if(rv || !recv_pollid)
	{
		fprintf(stderr,"OOPS: PollFD failed: %d,%d\n",rv,fd);
		abort();
	}
	
	if(in.pump_fd->is_dead)
	{  in.pump_fd->PumpReuseNow();  }
	
	in.io_fd->pollid=recv_pollid;
	assert(!in.io_fd->transferred);  // io params must be reset by copy facility
	in.pump_fd->limit=filelen;
	in.pump_fd->io_bufsize=16384;
	// Reset thresh values: This is necessary so that they are re-calculated 
	// again because thresh values are never larger than copy length limit. 
	in.pump_fd->low_read_thresh=-1;
	in.pump_fd->high_read_thresh=-1;
	in.pump_fd->low_write_thresh=-1;
	in.pump_fd->high_write_thresh=-1;
	
	assert(in.io_sock->pollid==pollid);
	
	rv=in.pump_fd->SetIO(in.io_sock,in.io_fd);
	if(rv!=0 && rv!=-1)
	{
		// If rv=-5, you probably need FDCopyPump::PumpReuseNow(). 
		fprintf(stderr,"OOPS: in pump->SetIO(fd,fd) returned %d\n",rv);
		abort();
	}
	if(rv)  return(rv);
	
	rv=in.pump_fd->Control(FDCopyPump::CC_Start);
	if(rv!=0)
	{
		fprintf(stderr,"OOPS: in pump(fd,fd)->CC_Start returned %d\n",rv);
		abort();
	}
	
	in.ioaction=IOA_SendingFD;
	
	return(0);
}


void NetworkIOBase::DumpNetIOStatistics(int vlevel,
	const NetIOStatistics *dn,const NetIOStatistics *up,
	const HTime *starttime,const char *title,const char *indent)
{
	if(!IsVerboseR(vlevel))  return;
	
	unsigned long long upbytes=up->tot_transferred;
	unsigned long long dnbytes=dn->tot_transferred;
	char up_bps_str[32];
	char dn_bps_str[32];
	HTime elapsed(HTime::Invalid);
	if(!starttime || starttime->IsInvalid())
	{  strcpy(up_bps_str,"???");  strcpy(dn_bps_str,up_bps_str);  }
	else
	{
		elapsed=HTime(HTime::Curr)-(*starttime);
		double sec=elapsed.GetD(HTime::seconds);
		if((sec<0.0 ? -sec : sec)<0.001)
		{  strcpy(up_bps_str,"[too short]");  strcpy(dn_bps_str,up_bps_str);  }
		else
		{
			double up_bps=double(upbytes)/sec;
			double dn_bps=double(dnbytes)/sec;
			if(up_bps<100.0)   snprintf(up_bps_str,32,"%.3f",up_bps);
			else  snprintf(up_bps_str,32,"%.0f",up_bps);
			if(dn_bps<100.0)   snprintf(dn_bps_str,32,"%.3f",dn_bps);
			else  snprintf(dn_bps_str,32,"%.0f",dn_bps);
		}
	}
	VerboseR(vlevel,"%s%s (duration: %s)\n",indent,title,
		elapsed.IsInvalid() ? "[unknown]" : elapsed.PrintElapsed(/*with_msec=*/1));
	VerboseR(vlevel,
		"%s  Network stat:  transferred   bytes/sec    hdrs  files\n"
		"%s    Downstream: %12llu  %10s  %6u  %5u\n"
		"%s    Upstream:   %12llu  %10s  %6u  %5u\n",
		indent,
		indent,dnbytes,dn_bps_str,dn->count_headers,dn->count_files,
		indent,upbytes,up_bps_str,up->count_headers,up->count_files);
}


void NetworkIOBase::DumpNetIOStatistics(int vlevel,const HTime *starttime,
	int side,const char *peer_name)
{
	if(!IsVerboseR(vlevel))  return;
	
	char tmp[128];
	snprintf(tmp,128,"Connection to %s %s",
		side<0 ? "server" : "client",peer_name);
	DumpNetIOStatistics(vlevel,
		/*downstream=*/side<0 ? &in.stat : &out.stat,
		/*upstream=*/side<0 ? &out.stat : &in.stat,
		starttime,tmp);
}


static inline void _KillControl(FDCopyPump *p)
{
	if(p && p->IsActive())
	{
		// This does happen e.g. if we shut down while file download 
		// is in progress. 
		p->Control(FDCopyPump::CC_Kill);
		// NOTE that this calls cpnotify(SCKilled) on the stack (ignored). 
	}
}

void NetworkIOBase::_ShutdownConnection()
{
	if(!pollid)  return;
	
	// First, cancel all IO jobs...
	_KillControl(in.pump_s);
	_KillControl(in.pump_fd);
	_KillControl(out.pump_s);
	_KillControl(out.pump_fd);
	
	// ...then actually shut down the connection. 
	PollFDDPtr(pollid,NULL);
	ShutdownFD(pollid);  // sets pollid=NULL
	sock_fd=-1;
}



// Return value: 0 -> OK; -1 -> alloc failure 
int NetworkIOBase::_Construct_FDCopy(NetworkIOBase::FDCopy *fdc)
{
	// All values NULL as of FDCopy::FDCopy() constructor. 
	do {
		if(!(fdc->io_sock=NEW<FDCopyIO_FD>()))  break;
		if(!(fdc->io_buf=NEW<FDCopyIO_Buf>()))  break;
		if(!(fdc->io_fd=NEW<FDCopyIO_FD>()))  break;
		if(!(fdc->pump_s=NEW1<FDCopyPump_Simple>(this)))  break;
		if(!(fdc->pump_fd=NEW1<FDCopyPump_FD2FD>(this)))  break;
		goto okay;
	} while(0);
	return(-1);
okay:;
	fdc->io_sock->persistent=1;
	fdc->io_buf->persistent=1;
	fdc->io_fd->persistent=1;
	fdc->pump_s->persistent=1;
	fdc->pump_fd->persistent=1;
	return(0);
}

void NetworkIOBase::_Destroy_FDCopy(NetworkIOBase::FDCopy *fdc)
{
	// This is smart becuase it can handle NULL correctly and sets 
	// the passed parameter to NULL (takes ref to ptr). 
	
	// This is tricky...
	// First, we must make sure that the pumps do no longer reference the 
	// CopyIOs. 
	if(fdc->pump_s)
	{
		assert(!fdc->pump_s->IsActive());  // _ShutdownConnection() must do that
		fdc->pump_s->SetIO(NULL,NULL);  // May be done even if pump->is_dead. 
	}
	if(fdc->pump_fd)
	{
		assert(!fdc->pump_fd->IsActive());  // _ShutdownConnection() must do that
		fdc->pump_fd->SetIO(NULL,NULL);  // May be done even if pump->is_dead. 
	}
	
	// Then, we delete the pumps: 
	if(fdc->pump_s)
	{  fdc->pump_s->persistent=0;   delete fdc->pump_s;   fdc->pump_s=NULL;   }
	if(fdc->pump_fd)
	{  fdc->pump_fd->persistent=0;  delete fdc->pump_fd;  fdc->pump_fd=NULL;  }
	
	// And finally, we delete the CopyIOs. 
	if(fdc->io_sock)
	{  fdc->io_sock->persistent=0;  delete fdc->io_sock;  fdc->io_sock=NULL;  }
	if(fdc->io_buf)
	{  fdc->io_buf->persistent=0;   delete fdc->io_buf;   fdc->io_buf=NULL;   }
	if(fdc->io_fd)
	{  fdc->io_fd->persistent=0;    delete fdc->io_fd;    fdc->io_fd=NULL;    }
}


NetworkIOBase::NetworkIOBase(int *failflag) : 
	FDCopyBase(failflag),
	in(),
	out()
{
	int failed=0;
	
	sock_fd=-1;
	pollid=NULL;
	
	if(_Construct_FDCopy(&in))   ++failed;
	if(_Construct_FDCopy(&out))  ++failed;
	
	// Hehe... little test suite for htonll and ntohll: 
	//assert(htonll(0x123456789abcdef0LL)!=0x123456789abcdef0LL);
	//assert(htonll(ntohll(0x123456789abcdef0LL))==0x123456789abcdef0LL);
	//assert(0);
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("NetworkIOBase");  }
}

NetworkIOBase::~NetworkIOBase()
{
	ShutdownFD(pollid);  // be sure...
	
	_Destroy_FDCopy(&in);
	_Destroy_FDCopy(&out);
}


/******************************************************************************/

NetworkIOBase::FDCopy::FDCopy() : stat()
{
	// NOTE: ONLY NULL-INITIALISATION
	ioaction=IOA_Locked;
	io_sock=NULL;
	io_buf=NULL;
	io_fd=NULL;
	pump_s=NULL;
	pump_fd=NULL;
}


/******************************************************************************/

void NetworkIOBase::NetIOStatistics::Add(
	const NetworkIOBase::NetIOStatistics *s)
{
	tot_transferred+=s->tot_transferred;
	count_headers+=s->count_headers;
	count_files+=s->count_files;
}


void NetworkIOBase::NetIOStatistics::Reset()
{
	tot_transferred=0;
	count_headers=0;
	count_files=0;
}
