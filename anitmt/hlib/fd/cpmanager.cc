/*
 * cpmanager.cc
 * 
 * Header containing class FDCopyManager, a class for 
 * copying from and to file descriptors which works in 
 * cooperation with classes derived from class FDCopyBase. 
 *
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <sys/poll.h>
#include <sys/time.h>
#include <sys/socket.h>   /* shutdown() */

#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/cpmanager.h>
#include <hlib/cpbase.h>

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. 
#warning Using assert()
#include <assert.h>
#else
#define assert(x)
#endif


#error Check poll()s return value on SIGPIPE / EPIPE? - POLLERR??


// Static global manager: 
FDCopyManager *FDCopyManager::manager=NULL;

// Set defaults: 
size_t FDCopyManager::default_iobufsize=16384;


// If it is an EINTR or an EWOULDBLOCK error, do nothing. 
// If it is an EPIPE send final pipe notify. 
// Otherwise send read error notify and finish input or request: 
void FDCopyManager::_ReadError(MCopyNode *cpn,HTime *fdtime)
{
	int errn=errno;
	if(errn==EINTR || errn==EWOULDBLOCK)
	{
		#if TESTING
		if(errn==EWOULDBLOCK)
		{  fprintf(stderr,"Oops: CP: reading would block (%d)\n",
			cpn->req.srcfd);  }
		#endif
		// Okay, we'll try again later. 
		return;
	}
	
	#if TESTING
	if(errn==EPIPE)
	{  fprintf(stderr,"Ah... got an EPIPE on (input) fd %d\n",
		cpn->req.srcfd);  }
	#endif
	
	// Okay, read returns an error. Finish input: 
	CopyInfo cpi(cpn,(errn==EPIPE) ? SCInPipe : SCErrRead);
	cpi.err_no=errn;
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	_FinishInput(cpn,&cpi);  // Does FinishRequest() if OM_Fd2Buf. 
}


// Similar to _ReadError but for the write side: 
void FDCopyManager::_WriteError(MCopyNode *cpn,HTime *fdtime)
{
	int errn=errno;
	if(errn==EINTR || errn==EWOULDBLOCK)
	{
		#if TESTING
		if(errn==EWOULDBLOCK)
		{  fprintf(stderr,"Oops: CP: writing would block (%d)\n",
			cpn->req.destfd);  }
		#endif
		// Okay, we'll try again later. 
		return;
	}
	
	#if TESTING
	if(errn==EPIPE)
	{  fprintf(stderr,"Ah... got an EPIPE on (output) fd %d\n",
		cpn->req.destfd);  }
	#endif
	
	// Okay, write returns an error. That is fatal...
	CopyInfo cpi(cpn,(errn==EPIPE) ? SCOutPipe : SCErrWrite);
	cpi.err_no=errn;
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	_FinishRequest(cpn,&cpi);
}


// Read in data for fd -> buf copying: 
void FDCopyManager::_ReadInData_Buf(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Fd2Buf);
	assert(!(cpn->cpstate & CPSFlusing));  // may not happen with OM_Fd2Buf
	
	// Read data from fd and write to buffer. Easy case. 
	size_t need=cpn->bufend-cpn->bufheadW;
	if(cpn->req.max_read_len && need>cpn->req.max_read_len)
	{  need=cpn->req.max_read_len;  }
	
	ssize_t rd=read(cpn->req.srcfd,cpn->bufheadW,need);
	if(rd<0)
	{
		_ReadError(cpn,fdi->current);
		return;
	}
	if(rd>0)
	{
		// Update buffer: 
		cpn->bufheadW+=rd;
		
		// Update statistics: 
		cpn->read_bytes+=copylen_t(rd);
		
		// Send progress info: 
		if(cpn->bufheadW<cpn->bufend)
		{
			_SendProgressInfo(cpn,fdi->current,PARead);
			return;
		}
		// If we reach here: reading done; so request is done. 
	}
	// rd==0 or rd>0 and reading done. 
	CopyInfo cpi(cpn,rd ? SCLimit : SCRead0);
	cpi.fdtime=fdi->current;  // may be modified (FDManager copies it)
	_FinishRequest(cpn,&cpi);
}


// Write out data for buf -> fd copying: 
void FDCopyManager::_WriteOutData_Buf(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Buf2Fd);
	assert(!(cpm->cpstate & CPSFlusing));  // may not happen with OM_Buf2Fd
	
	// Read data from buf and write to fd. Easy case. 
	size_t avail=cpn->bufend-cpn->bufheadR;
	if(cpn->req.max_write_len && avail>cpn->req.max_write_len)
	{  avail=cpn->req.max_write_len;  }
	
	ssize_t rw=write(cpn->req.destfd,cpn->bufheadR,avail);
	if(wr<0)
	{
		_WriteError(cpn,fdi->current);
		return;
	}
	if(wr>0)
	{
		// Update buffer: 
		cpn->bufheadR+=wr;
		
		// Update statistics: 
		cpn->stat.written_bytes+=copylen_t(wr);
		
		// Send progress info: 
		if(cpn->bufheadR<cpn->bufend)  // otherwise: done
		{
			_SendProgressInfo(cpn,fdi->current,PAWrite);
			return;
		}
		// If we reach here we're done. 
	}
	// wr==0 or all data written. 
	#if TESTING
	if(wr==0)  // Hmm.. wrote 0 bytes. 
	{  fprintf(stderr,"OOPS: CP: wrote 0/%u bytes on fd %d\n",
		avail,cpn->req.destfd);  }
	#endif
	// Done. 
	CopyInfo cpi(cpn,wr ? SCLimit : SCWrite0);
	cpi.fdtime=fdi->current;  // may be modified (FDManager copies it)
	_FinishRequest(cpn,&cpi);
}


// Read in data for fd -> fd copying: 
// Only call if cpn->buf is non-NULL (copy fd to fd). 
void FDCopyManager::_ReadInData_FD(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Fd2Fd);
	
	assert(cpn->iobufsize>=cpn->bufuse);
	if(cpn->bufuse>cpn->req.high_read_thresh)
	#warning check '>'
	{
		#if TESTING
		// There is not enough free space in the buffer; we should 
		// not be here at all. 
		fprintf(stderr,"Oops: CP:%d: why are we here? (%u,%u)\n",
			__LINE__,cpn->bufuse,cpn->req.high_read_thresh);
		#endif
		return;
	}
	
	// See how much free space is in the buffer: 
	size_t avail=cpn->iobufsize-cpn->bufuse;
	
	// Okay, see how much we want to read: 
	if(cpn->req.max_read_len && avail>cpn->req.max_read_len)
	{  avail=cpn->req.max_read_len;  }
	int read_limit=0;
	if(cpn->req.len)  // length limit
	{
		copylen_t max_read=cpn->req.len-cpn->read_bytes;
		if(copylen_t(avail)>=max_read)  // >= REALLY!
		{
			avail=size_t(max_read);
			read_limit=1;  // that's why we need `>=' above. 
		}
	}
	
	// Now, set up read io vector: 
	struct iovec rio[2];  // never need more than 2
	int need_readv=0;
	rio->iov_base=cpn->bufheadW;
	rio->iov_len=cpn->bufend-cpn->bufheadW;
	if(rio->iov_len>=avail)   // >= is CORRECT. 
	{  rio->iov_len=avail;  }
	else
	{
		need_readv=1:
		rio[1].iov_base=cpn->buf;
		rio[1].iov_len=avail-rio->iov_len;
	}
	
	// Actually read the stuff in: 
	ssize_t rd = need_readv ? 
		readv(cpn->req.srcfd,rio,2) : 
		read(cpn->req.srcfd,rio->iov_base,rio->iov_len);
	if(rd<0)
	{
		// Error condition. 
		_ReadError(cpn,fdtime);
		return;
	}
	if(rd>0)
	{
		// Read in data; update buffer stuff: 
		size_t rrd=rd;
		size_t to_end=cpn->bufend-cpn->bufheadW;
		if(rrd<to_end)
		{  cpn->bufheadW+=rrd;  }
		else
		{  cpn->bufheadW=cpn->buf+(rrd-to_end);  }
		cpn->bufuse+=rrd;
		assert(cpn->bufuse<cpn->iobufsize);
		
		// Update statistics: 
		cpn->read_bytes+=copylen_t(rrd);
		
		// Send progress info: 
		if(!read_limit || rrd<avail)  // read limit not reached
		{
			_SendProgressInfo(cpn,fdime,PARead);
			return;
		}
		// If we're here -> read limit reached. 
	}
	// rd==0 or read limit reached. 
	CopyInfo cpi(cpn,rd ? SCLimit | SCRead0);
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	_FinishInput(cpn,&cpi);
}


// Write out data for fd -> fd copying: 
// Only call if cpn->buf is non-NULL (copy fd to fd). 
void FDCopyManager::_WriteOutData_FD(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Fd2Fd);
	
	assert(cpn->iobufsize>=cpn->bufuse);
	if(cpn->bufuse<cpn->req.low_write_thresh && 
	#warning check '<'
	   !(cpn->cpstate & CPSFlushing))
	{
		#if TESTING
		// There is not enough data in the buffer; we should 
		// not be here at all. 
		fprintf(stderr,"Oops: CP:%d: why are we here? (%u,%u)\n",
			__LINE__,cpn->bufuse,cpn->req.low_write_thresh);
		#endif
		return;
	}
	
	// See how much data is available in the buffer
	size_t avail=cpn->bufuse;
	
	// Okay, see how much we may write: 
	if(cpn->max_write_len && avail>cpn->max_write_len)
	{  avail=cpn->max_write_len;  }
	
	// Now, set up write io vector: 
	struct iovec wio[2];  // never need more than 2
	int need_writev=0;
	wio->iov_base=cpn->bufheadR;
	wio->iov_len=cpn->bufend-cpn->bufheadR;
	if(wio->iov_len>=avail)   // >= is CORRECT. 
	{  wio->iov_len=avail;  }
	else
	{
		need_writev=1:
		wio[1].iov_base=cpn->buf;
		wio[1].iov_len=avail-wio->iov_len;
	}
	
	// Actually write the stuff out: 
	ssize_t wr = need_writev ? 
		writev(cpn->req.destfd,wio,2) : 
		write(cpn->req.destfd,wio->iov_base,wio->iov_len);
	if(wr<0)
	{
		// Error condition. 
		_WriteError(cpn,fdtime);
		return;
	}
	if(wr>0)
	{
		// Wrote out data; update buffer stuff: 
		size_t wwd=wd;
		size_t to_end=cpn->bufend-cpn->bufheadR;
		if(wwd<to_end)
		{  cpn->bufheadR+=wwd;  }
		else
		{  cpn->bufheadR=cpn->buf+(wwd-to_end);  }
		assert(cpn->bufuse>=wwd);
		cpn->bufuse-=wwd;
		
		// Update statistics: 
		cpn->stat.written_bytes+=copylen_t(wwd);
		
		// Send progress info: 
		if(!(cpn->cpstate & CPSFlushing) || cpn->bufuse)
		{
			// Surely not the last write. 
			_SendProgressInfo(cpn,fdtime,PAWrite);
			return;
		}
		// When er're here -> writing done. 
	}
	// Here: wr==0 or everything written. 
	#if TESTING
	if(wr==0)  // Hmmm... write returns 0. 
	{  fprintf(stderr,"OOPS: CP: wrote 0/%u bytes on fd %d\n",
		avail,cpn->req.destfd);  }
	#endif
	CopyInfo cpi(cpn,wr ? 0 : SCWrite0);
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	_FinishRequest(cpn,&cpi);
}


// Called to initially start the copy request. 
void FDCopyManager::_StartCopyRequest(MCopyNode *cpn)
{
	assert(cpn->cpstate & CPSStopped);
	assert(cpm->opmode);
	
	// Set start time: 
	cpn->stat.starttime.SetCurr();
	
	if(cpn->opmode==OM_Buf2Fd)
	{
		// src buffer and dest fd: 
		PollFD(cpn->pdestid,cpn->destfd_ev=POLLOUT);
	}
	else  // OM_Fd2Buf and OM_Fd2Fd: 
	{
		// Source (and maybe dest) are file descriptors. 
		PollFD(cpn->psrcid,cpn->srcfd_ev=POLLIN);
	}
	
	(int)cpn->cpstate&=~CPSStopped;
}

// Called to continue a stopped request: 
void FDCopyManager::_ContCopyRequest(MCopyNode *cpn)
{
	if(!(cpn->cpstate & CPSStopped))
	{  return;  }
	
	switch(cpn->opmode)
	{
		case OM_Buf2Fd:
			PollFD(cpn->pdestid,cpn->destfd_ev=POLLOUT);
			break;
		case OM_Fd2Fd:
			_ReDecidePollEvents_In(cpn);
			_ReDecidePollEvents_Out(cpn);
			break;
		case OM_Fd2Buf:
			PollFD(cpn->psrcid,cpn->srcfd_ev=POLLIN);
			break;
		#if TESTING
		default:  assert(0);
		#endif
	}
	
	(int)cpn->cpstate&=~CPSStopped;
}

// Used to temporarily stop a copy request: 
void FDCopyManager::_StopCopyRequest(MCopyNode *cpn)
{
	if(cpn->cpstate & CPSStopped)
	{  return;  }
	
	if(cpn->srcfd_ev)   PollFD(cpn->psrcid, cpn->srcfd_ev=0);
	if(cpn->destfd_ev)  PollFD(cpn->pdestid,cpn->destfd_ev=0);
	
	(int)cpn->cpstate|=CPSStopped;
}


// Re-decide on poll events for input fd (fd -> fd only): 
void FDCopyManager::_ReDecidePollEvents_In(MCopyNode *cpn)
{
	assert(cpn->opmode==OM_Fd2Fd);
	assert(!(cpn->cpstate & CPSStopped));
	
	// Current events in cpn->srcfd_ev, cpn->destfd_ev
	if(cpn->cpstate & CPSFlushing)
	{
		if(cpn->srcfd_ev & POLLIN)
		{  PollFD(cpn->psrcid, cpn->srcfd_ev=0);  }
		return;
	}
	
	#error check > and >= here!!
	if(cpn->srcfd_ev & POLLIN)  // currently reading
	{
		// Stop if high thresh reached: 
		if(cpn->bufuse>cpn->req->high_read_thresh)
		{  PollFD(cpn->psrcid, cpn->srcfd_ev=0);  }
	}
	else  // currently not reading
	{
		// Start if low thresh reached: 
		if(cpn->bufuse<cpn->req->low_read_thresh)
		{  PollFD(cpn->psrcid, cpn->srcfd_ev=POLLIN);  }
	}
}

// Re-decide on poll events for output fd (fd -> fd only): 
void FDCopyManager::_ReDecidePollEvents_Out(MCopyNode *cpn)
{
	assert(cpn->opmode==OM_Fd2Fd);
	assert(!(cpn->cpstate & CPSStopped));  ###//könnte Probleme machen...
	
	// Current events in cpn->srcfd_ev, cpn->destfd_ev
	if(cpn->cpstate & CPSFlushing)
	{
		if(!(cpn->destfd_ev & POLLOUT))
		{  PollFD(cpn->pdestid, cpn->destfd_ev=POLLOUT);  }
		return;
	}
	
	#error check > and >= here!!
	if(cpn->destfd_ev & POLLOUT)  // currently writing
	{
		// Stop if low thresh reached: 
		if(cpn->bufuse<cpn->req->low_write_thresh)
		{  PollFD(cpn->pdestid, cpn->destfd_ev=0);  }
	}
	else  // currently not writing
	{
		// Start if high thresh reached: 
		if(cpn->bufuse>cpn->req->high_write_thresh)
		{  PollFD(cpn->pdestid, cpn->destfd_ev=POLLOUT);  }
	}
}


int FDCopyManager::fdnotify(FDInfo *fdi)
{
	// Get associated copy node: 
	MCopyNode *cpn=(MCopyNode *)fdi->dptr;
	assert(cpn);
	
	// NOTE; fdi->current may be modified by us. 
	//       So, it's no problem passing it to the clients. 
	
	// which_fd: +1 -> write fd; -1 -> read fd
	int which_fd=0;
	if(fdi->fd==cpn->req.srcfd)
	{
		which_fd=-1;
		assert(fdi->pollid==cpn->psrcid);
	}
	else if(fdi->fd==cpn->req.destfd)
	{
		which_fd=+1;
		assert(fdi->pollid==cpn->pdestid);
	}
	else
	{  assert(0);  }
	
	// ** First, check for error and hangup: 
	if(fdi->revents & (POLLERR | POLLNVAL | POLLHUP))
	{
		CopyInfo cpi(cpn,
			(fdi->revents & POLLHUP) ? 
				((which_fd>0) ? SCOutHup : SCInHup) : 
				((which_fd>0) ? SCErrPollO : SCErrPollI) );
		if(fdi->revents & (POLLNVAL | POLLHUP))
		{  cpi.err_no=fdi->revents;  }
		cpi.fdtime=fdi->current;  // may be modified (FDManager copies it)
		
		if(which_fd>0)  // Oh, it's the output fd. Finish the request. 
		{  _FinishRequest(cpn,&cpi);  }
		else  // Input fd. Okay, finish input. 
		{  _FinishInput(cpn,&cpi);  }
		return(0);
	}
	
	// ** Then, check state stuff: 
	if(cpn->cpstate & CPSStopped)
	{
		// If we're stopped, no events may be set: 
		#if TESTING
		assert(!cpn->destfd_ev);
		assert(!cpn->srcfd_ev);
		fprintf(stderr,"Oops: CP: in fdnotify() although request is stopped.\n");
		#endif
		return(0);
	}
	
	// ** Then, check for data: 
	if(which_fd<0)  // input fd
	{
		if(!(fdi->revents & POLLIN))
		{
			#if TESTING
			fprintf(stderr,"Oops: CP:%d: fdi->revents=%d for fd=%d (cannot read)\n",
				__LINE__,fdi->revents,fdi->fd);
			#endif
			return(0);
		}
		
		// Can read data...
		if(cpn->opmode==OM_Fd2Fd)
		{
			// Okay, read in data into buffer io buffer: 
			_ReadInData_FD(cpn,fdi->current);
			
			// Decide on new poll events: 
			_ReDecidePollEvents_In(cpn);
		}
		else  // OM_Fd2Buf
		{
			// Okay read in data into dest buffer: 
			_ReadInData_Buf(cpn,fdi->current);
		}
	}
	else if(which_fd>0)  // output fd
	{
		if(!(fdi->revents & POLLOUT))
		{
			#if TESTING
			fprintf(stderr,"Oops: CP:%d: fdi->revents=%d for fd=%d (cannot write)\n",
				__LINE__,fdi->revents,fdi->fd);
			#endif
			return(0);
		}
		
		// Can write data...
		if(cpn->opmode==OM_Fd2Fd)
		{
			// Okay, write out data from buffer io buffer: 
			_WriteOutData_FD(cpn,fdi->current);
			
			// Decide on new poll events: 
			_ReDecidePollEvents_Out(cpn);
		}
		else  // OM_Buf2Fd
		{
			// Okay write out data from src buffer: 
			_WriteOutData_Buf(cpn,fdi->current);
		}
	}
	
	return(0);
}


FDCopyManager::CopyID FDCopyManager::CopyFD(FDCopyBase *client,
	CopyRequest *req,FDBase *fdb)
{
	req->errcode=0;
	
	#if TESTING
	assert(clients.find(client));
	#endif
	
	// Check input: 
	if(((req->srcfd<0)==(!req->srcbuf)) || 
	   ((req->destfd<0)==(!req->destbuf)) || 
	   (req->srcbuf && req->destbuf) || 
	   req->srcfd==req->destfd)
	{
		req->errcode=-3;
		return(NULL);
	}
	
	// This may give a warning if req->len is an unsigned type: 
	if(req->len<0)
	{  req->len=0;  }
	// May not pass buffer of size 0: 
	if(!req->len && (req->srcbuf || req->destbuf))
	{  req->errcode=1;  return(NULL);  }
	
	#warning A more clever algorithm should be invented here: 
	ssize_t iobs=req->iobufsize;
	if(req->low_read_thresh<0 || low_read_thresh>=iobs)
	{  req->low_read_thresh=iobs/8;  }
	if(req->high_read_thresh<0 || req->high_read_thresh>=iobs)
	{  req->low_read_thresh=iobs-iobs/8;  }
	if(req->high_read_thresh<=req->low_read_thresh)
	{
		#error ...
	}
	
	if(req->low_write_thresh<0 || low_write_thresh>=iobs)
	{  req->low_write_thresh=iobs/8;  }
	if(req->high_write_thresh<0 || req->high_write_thresh>=iobs)
	{  req->low_write_thresh=iobs-iobs/8;  }
	if(req->high_write_thresh<=req->low_write_thresh)
	{
		#error ...
	}
	
	// Last chance to quit without much effort...
	if(req->errcode)
	{  return(NULL);  }
	
	// Okay, allocate a copy node: 
	MCopyNode *cpn=NEW<MCopyNode>();
	if(!cpn)  // allocation failure
	{  req->errcode=-1;  return(NULL);  }
	
	// Basic setup: 
	cpn->client=client;  // set back pointer
	cpn->fdb=fdb;
	cpn->req=*req;  // implicit copy 
	
	// Allocate our FDNodes at the FDManager: 
	if(req->srcfd>=0)
	{
		int rv=PollFD(req->srcfd,0,cpn,&cpn->psrcid);
		if(rv!=0)
		{  req->errcode=-2;  goto retunpoll;  }
	}
	if(req->destfd>=0)
	{
		int rv=PollFD(req->destfd,0,cpn,&cpn->pdestid);
		if(rv!=0)
		{  req->errcode=-2;  goto retunpoll;  }
	}
	
	// Allocate copy buffer if needed: 
	if(cpn->psrcid && cpn->pdestid)
	{
		cpn->opmode=OM_Fd2Fd;
		// If iobufsize is 0, set up default: 
		if(!cpn->req.iobufsize)
		{  cpn->req.iobufsize=default_iobufsize;  }
		// If we shall copy less bytes than iobufsize, make buffer smaller: 
		if(cpn->req.len && cpn->req.len<copylen_t(cpn->req.iobufsize))
		{  cpn->req.iobufsize=size_t(cpn->req.len);  }
		// Actually allocate buffer: 
		cpn->buf=(char*)LMalloc(cpn->req.iobufsize);
		if(!cpn->buf)
		{  req->errcode=-1;  goto retfreebuf;  }
		cpn->bufend=cpn->buf+cpn->req.iobufsize;
		cpn->bufheadR=cpn->buf;
		cpn->bufheadW=cpn->buf;
		cpn->bufuse=0;
	}
	else 
	// Set buffer vars (src or dest (not both) is passd as argument 
	// to CopyFD()). 
	if(cpn->req.srcbuf)
	{
		cpm->opmode=OM_Buf2Fd;
		cpn->bufheadR = cpn->req.srcbuf;
		// cpn->bufheadW stays NULL
		cpn->bufend = cpn->bufheadR + cpn->req.len;
		// bufuse must stay 0 as buf=NULL. 
		cpn->req.iobufsize=0;
	}
	else if(cpn->req.destbuf)
	{
		cpm->opmode=OM_Fd2Buf;
		//cpn->bufheadR must stay 0
		cpn->bufheadW = cpn->req.destbuf;
		cpn->bufend = cpn->bufheadW + cpn->req.len;
		// bufuse must stay 0 as buf=NULL. 
		cpn->req.iobufsize=0;
	}
	else
	{  assert(0);  }
	
	// Copy values (for security reason; so that client cannot 
	// modify them when the request gets passed down by cpnotify()): 
	cpn->iobufsize=cpn->req.iobufsize;
	
	// Set poll events of FDs to 0 if FDBase pointer is set: 
	_SavePollEvents(cpn);
	
	// Actually start copy process: 
	_StartCopyRequest(cpn);
	
	return((CopyID)cpn);
	
retfreebuf:;
	cpn->buf=(char*)LFree(cpn->buf);
retunpoll:;
	if(cpn->psrcid)   {  UnpollFD(cpn->psrcid);   cpn->psrcid=NULL;   }
	if(cpn->pdestid)  {  UnpollFD(cpn->pdestid);  cpn->pdestid=NULL;  }
retfree:;
	if(cpn)  delete cpn;
	return(NULL);
}


int FDCopyManager::CopyControl(CopyID cpid,ControlCommand cc)
{
	if(!cpid)
	{  return(-2);  }
	
	MCopyNode *cpn=(MCopyNode*)cpid;
	switch(cc)
	{
		case CCKill:
			CopyInfo cpi(cpn,SCKilled);
			_FinishRequest(cpn,&cpi);
			break;
		case CCStop:
			if(cpn->cpstate & CPSStopped)
			{  return(1);  }
			_StopCopyRequest(cpn);
			break;
		case CCCont:
			if(!(cpn->cpstate & CPSStopped))
			{  return(1);  }
			_ContCopyRequest(cpn);
			break;
		default:  return(-3);
	}
	return(0);
}


// Save ("original") poll events (client side) on passed fds and 
// set them to 0. 
int FDCopyManager::_SavePollEvents(MCopyNode *cpn)
{
	FDBase *fdb=cpn->fdb;
	if(!fdb)  return(0);
	
	PollID pollid=fdb->FDPollID(cpn->req.srcfd);
	if(pollid)
	{
		cpn->orig_src_events=fdb->FDEvents(pollid);
		cpn->orig_flags|=MCopyNode::OFSrcSet;
		fdb->PollFD(pollid,0);
	}
	pollid=fdb->FDPollID(cpn->req.destfd);
	if(pollid)
	{
		cpn->orig_dest_events=fdb->FDEvents(pollid);
		cpn->orig_flags|=MCopyNode::OFDestSet;
		fdb->PollFD(pollid,0);
	}
	return(0);
}

// Restore the saved poll events:
int FDCopyManager::_RestorePollEvents(MCopyNode *cpn,int only_input_ev)
{
	FDBase *fdb=cpn->fdb;
	if(!fdb)  return(0);
	
	if(cpn->orig_flags & MCopyNode::OFSrcSet)
	{
		// First check for the PollID because the poll node might 
		// no longer exist; in this case we should not allocate it. 
		PollID pollid=fdb->FDPollID(cpn->req.srcfd);
		if(pollid)
		{  fdb->PollFD(pollid,cpn->orig_src_events);  }
		// Flags are restored, we shall not do that more than once: 
		(int)cpn->orig_flags&=~MCopyNode::OFSrcSet;
	}
	if(only_input_ev)
	{  return(0);  }
	if(cpn->orig_flags & MCopyNode::OFDestSet)
	{
		PollID pollid=fdb->FDPollID(cpn->req.destfd);
		if(pollid)
		{  fdb->PollFD(pollid,cpn->orig_dest_events);  }
		(int)cpn->orig_flags&=~MCopyNode::OFDestSet;
	}
	
	return(0);
}


inline void FDCopyManager::_SendCpNotify(MCopyNode *cpn,ProgressInfo *pgi)
{
	// These vars are held in cpn to prevent modification by client: 
	cpn->req.iobufsize=cpn->iobufsize;
	cpn->stat.read_bytes=cpn->read_bytes;
	cpn->client->cpnotify(pgi);
}


void FDCopyManager::_FillProgressInfoStruct(ProgressInfo *pgi,
	MCopyNode *cpn,HTime *fdtime,ProgressAction act)
{
	pgi->req=&cpn->req;
	pgi->stat=&cpn->stat;
	pgi->fdtime=fdtime;
	pgi->scode=cpn->persistent_sc;
	pgi->action=act;
}

// Decide whether to send progress info to the client 
// and do so if necessary. 
void FDCopyManager::_SendProgressInfo(MCopyNode *cpn,HTime *fdtime,
	ProgressAction act)
{
	// Check if client is interested in progress info: 
	if(!act & cpn->req.progress_mask)
	{  return;  }
	
	ProgressInfo pgi;
	_FillProgressInfoStruct(&pgi,cpn,fdtime,act);
	_SendCpNotify(cpn,&pgi);
}


inline void FDCopyManager::_SendCpNotify(MCopyNode *cpn,CopyInfo *cpi)
{
	// These vars are held in cpn to prevent modification by client: 
	cpn->req.iobufsize=cpn->iobufsize;
	cpn->stat.read_bytes=cpn->read_bytes;
	cpn->client->cpnotify(cpi);
}

// Kill request without any side effects (does not notify cli
// does not restore client's fd flags, etc.)
void FDCopyManager::_KillRequest(MCopyNode *cpn)
{
	UnpollFD(cpn->psrcid);   cpn->psrcid=NULL;
	UnpollFD(cpn->pdestid);  cpn->pdestid=NULL;
	#warning missing: kill timeout timer
	
	// Finally delete it: 
	cpn->client->rlist.deqeueu(cpn);
	delete cpn;
}


// Set persistent_sc to passed cpn->scode, notify client 
// and set input poll flags to 0. 
// Also sets CPSFlushing in cpstate (unless there is nothing 
// to flush in which case FinishRequest() is called). 
// Actually calls _FinishRequest() if OM_Fd2Buf or if there 
// is nothing to flush. 
void FDCopyManager::_FinishInput(MCopyNode *cpn,CopyInfo *cpi)
{
	assert(!(cpn->cpstate & CPSFlushing));
	if(cpm->opmode==OM_Fd2Buf || !cpn->bufuse)
	{
		// There's nothing to flush; we're done. 
		_FinishRequest(cpn,cpi);
		return;
	}
	// Set flushing flag: 
	(int)cpn->cpstate|=CPSFlushing;
	
	// Restore input events before calling cpnotify(). 
	_RestorePollEvents(cpn,/*input_only=*/1);
	
	(int)cpn->persistent_sc|=cpi->scode;
	cpi->scode=cpn->persistent_sc;
	assert(!(cpn->scode & SCFinal));
	_SendCpNotify(cpn,cpi);
	
	// Don't need any more input: 
	cpn->srcfd_ev=0;
	UnpollFD(cpn->psrcid);
	cpn->psrcid=NULL;
}

// Send an cpnotify() to the client and kill the request. 
void FDCopyManager::_FinishRequest(MCopyNode *cpn,CopyInfo *cpi)
{
	// Restore events before calling cpnotify(). 
	// (Will not restore input fd events if already done earlier in 
	// _FinshInput().) 
	_RestorePollEvents(cpn);
	
	(int)cpi->scode|=SCFinal;
	(int)cpi->scode|=cpn->persistent_sc;
	_SendCpNotify(cpn,cpi);
	
	_KillRequest(cpn);
}


int FDCopyManager::Register(FDCopyBase *client)
{
	if(client)
	{
		if(client.next || client==clients.last())
		{  return(1);  }   // already registered
		clients.append(client);
	}
	return(0);
}

void FDCopyManager::Unregister(FDCopyBase *client)
{
	if(!client)  return;
	
	// Client registered?
	if(client.next || client==clients.last())
	{
		// Terminate all the client's copy requests without notifying it: 
		for(MCopyNode *r=client->rlist.first(); r; r=r->next)
		{  _KillRequest(r);  }
		
		// Remove client from list: 
		clients.dequeue(client);
	}
}


FDCopyManager::FDCopyManger(int *failflag=NULL) : 
	FDBase(failflag),
	clients(failflag)
{
	int failed=0;
	
	// Tell FDManager that we're a manager: 
	if(SetManager(1))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  return;  }
	else if(failed)
	{  ConstructorFailedExit("CP");  }
	
	/*--- DO NOT USE >int failed< BELOW HERE. ---*/
	
	// Init global manager: 
	#if TESTING
	if(manager)
	{  fprintf(stderr,"%s: more than one FDCopyManager.\n",
		prg_name);  exit(1);  }
	#endif
	
	manager=this;
}

FDCopyManager::~FDCopyManger()
{
	#if TESTING
	if(!clients.is_empty())
	{
		// This is bad as this will 
		// 1. SIGSEGV when the client gets destroyed
		// 2. not free the requests of the clients still existing 
		fprintf(stderr,"CPMan: Oops: client list not empty: %d clients left.\n",
			clients.count());
	}
	#endif
	
	manager=NULL;
}


/******************************************************************************/

FDCopyManager::MCopyNode::MCopyNode(int *failflag) : 
	req(failflag),
	stat(failflag)
{
	client=NULL;
	fdb=NULL;
	
	orig_flags=0;
	orig_src_events=0;
	orig_dest_events=0;
	
	psrcid=NULL;
	pdestid=NULL;
	cpstate=CPSStopped;
	srcfd_ev=0;
	destfd_ev=0;
	persistent_sc=SCNone;
	opmode=OM_None;
	
	iobufsize=0;
	buf=NULL;
	bufend=NULL;
	bufheadR=NULL;
	bufheadW=NULL;
	bufuse=0;
	
	read_bytes=0;
}

FDCopyManager::MCopyNode::~MCopyNode()
{
	// Clear only the most important thigs 
	// (make bug tracking easier...)
	client=NULL;
	fdb=NULL;
	assert(!psrcid);
	assert(!pdestid);
	
	opmode=OM_None;
	
	buf=(char*)LFree(buf);
	bufend=NULL;
	bufheadR=NULL;
	bufheadW=NULL;
	bufuse=0;
}

/******************************************************************************/

FDCopyManager::CopyStatistics::CopyStatistics(int * /*failflag*/) : 
	starttime()
{
	read_bytes=0;
	written_bytes=0;
}

/******************************************************************************/

FDCopyManager::CopyInfo::CopyInfo(
	FDCopyManager::StatusCode _scode,
	const FDCopyManager::MCopyNode *cpn,
	int * /*failflag*/)
{
	req=&cpn->req;
	stat=&cpn->stat;
	fdtime=NULL;
	
	scode=_scode;
	err_no=0;
}
