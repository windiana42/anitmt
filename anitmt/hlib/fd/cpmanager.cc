/*
 * cpmanager.cc
 * 
 * Implementation of class FDCopyManager, a class for 
 * copying from and to file descriptors which works in 
 * cooperation with classes derived from class FDCopyBase. 
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

// FIXME: Add Timeout stuff. 

// TODO: Generally, only POLLIN, POLLOUT is supported . 
//       (Especially for CPPollFD)). 

#include <hlib/htime.h>
#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/cpmanager.h>
#include <hlib/cpbase.h>

#include <string.h>

#if HAVE_SYS_UIO_H
#  include <sys/uio.h>
#endif


#ifndef TESTING
#define TESTING 1
#endif

// 1 -> Print each time we change POLLIN/POLLOUT for fd->fd copying. 
//      Used to seee if the algorithm works fine (= lots of "[rw]"). 
#define PRINT_SWITCH_DUMP 0

#if TESTING
#warning TESTING switched on. 
#warning Using assert()
#include <assert.h>
#else
#define assert(x)
#endif


#warning Check poll()s return value on SIGPIPE / EPIPE? - POLLERR??

// NOTE on thresh vals: 
// high_read_thresh: stop reading if bufuse>=high_read_thresh. 
//   high_read_thresh = bufsize -> stop reading if buffer is completely full. 
// low_read_thresh: start reading if bufuse<=low_read_thresh. 
//   low_read_thresh = 0 -> start reading when buffer is completely empty. 
// ==> constraint: low_read_thresh<high_read_thresh (may not be >=)
// high_write_thresh: start writing if bufuse>=high_write_thresh. 
//   high_write_thresh = bufsize -> start writing if buffer is completely full. 
// low_write_thresh: stop writing if bufuse<=low_write_thresh. 
//   low_write_thresh = 0 -> stop writing when buffer is completely empty. 

// Static global manager: 
FDCopyManager *FDCopyManager::manager=NULL;

// Set defaults: 
size_t FDCopyManager::default_iobufsize=16384;


inline void FDCopyManager::_SendCpNotify(MCopyNode *cpn,ProgressInfo *pgi)
{
	// These vars are held in cpn to prevent modification by client: 
	cpn->req.iobufsize=cpn->iobufsize;
	cpn->client->cpnotify(pgi);
}


void FDCopyManager::_FillProgressInfoStruct(ProgressInfo *pgi,
	MCopyNode *cpn,HTime *fdtime,ProgressAction act)
{
	pgi->cpid=(CopyID)cpn;
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
	if(!(act & cpn->req.progress_mask))
	{  return;  }
	
	ProgressInfo pgi;
	_FillProgressInfoStruct(&pgi,cpn,fdtime,act);
	_SendCpNotify(cpn,&pgi);
}


inline void FDCopyManager::_SendCpNotify(MCopyNode *cpn,CopyInfo *cpi)
{
	// These vars are held in cpn to prevent modification by client: 
	cpn->req.iobufsize=cpn->iobufsize;
	cpn->client->cpnotify(cpi);
}


// Basic poll flags modification: The flags in set_flags are set, then those 
// in clear_flags cleared. Then, PollFD() is called if something changed. 
// These are inline for better optimization (usually one of the args is 0). 
inline void FDCopyManager::_ChangePollFDSrc(MCopyNode *cpn,
	short set_flags,short clear_flags)
{
	register short newflags=cpn->srcfd_ev;
	newflags|=set_flags;
	newflags&=~clear_flags;
	if(cpn->srcfd_ev!=newflags)
	{
		cpn->srcfd_ev=newflags;
		PollFD(cpn->psrcid,newflags);
	}
}
inline void FDCopyManager::_ChangePollFDDest(MCopyNode *cpn,
	short set_flags,short clear_flags)
{
	register short newflags=cpn->destfd_ev;
	newflags|=set_flags;
	newflags&=~clear_flags;
	if(cpn->destfd_ev!=newflags)
	{
		cpn->destfd_ev=newflags;
		PollFD(cpn->pdestid,newflags);
	}
}
// This is to be used if psrcif==pdestid (OM_FdBuf2FdBuf): 
inline void FDCopyManager::_ChangePollFDSD(MCopyNode *cpn,
	short set_flags,short clear_flags)
{
	assert(cpn->opmode==OM_FdBuf2FdBuf && cpn->srcfd_ev==cpn->destfd_ev);
	register short newflags=cpn->srcfd_ev;
	newflags|=set_flags;
	newflags&=~clear_flags;
	if(cpn->srcfd_ev!=newflags)
	{
		cpn->srcfd_ev=newflags;
		cpn->destfd_ev=newflags;
		PollFD(cpn->psrcid,newflags);
	}
}


// Set up cpn->controlled_src_ev, cpn->controlled_dest_ev based 
// on cpn->opmode. 
void FDCopyManager::_SetControlledEvents(MCopyNode *cpn)
{
	if(!cpn->recv_fdnotify)
	{  cpn->controlled_src_ev=cpn->controlled_dest_ev=0x7fff;  }
	else switch(cpn->opmode)
	{
		case OM_Buf2Fd:
			cpn->controlled_src_ev=0;
			cpn->controlled_dest_ev=POLLOUT;
			break;
		case OM_Fd2Fd:
			cpn->controlled_src_ev = (cpn->cpstate & CPSFlushing) ? 0 : POLLIN;
			cpn->controlled_dest_ev=POLLOUT;
			break;
		case OM_Fd2Buf:
			cpn->controlled_src_ev=POLLIN;
			cpn->controlled_dest_ev=0;
			break;
		case OM_FdBuf2FdBuf:
			cpn->controlled_src_ev=(POLLIN | POLLOUT);
			cpn->controlled_dest_ev=(POLLOUT | POLLOUT);
			break;
		default:  assert(0);  break;
	}
}


int FDCopyManager::_SendFDNotify(MCopyNode *cpn,FDInfo *_fdi,int which)
{
	assert(cpn->fdb);
	
	// Only send it if we do not completely control this FD: 
	if(cpn->opmode==OM_FdBuf2FdBuf)
	{  return(0);  }
	
	FDInfo fdi=*_fdi;
	fdi.dptr=NULL;
	if(cpn->recv_fdnotify==1)
	{
		// Adjust events: 
		// which=0 only in case OM_FdBuf2FdBuf; but as there is 
		// controlled_src_ev=controlled_dest_ev it does not matter which one 
		// we choose. 
		short mask=(which<0 ? cpn->controlled_src_ev : cpn->controlled_dest_ev);
		fdi.events&=~mask;
		fdi.revents&=~mask;
	}
	int rv=0;
	if(fdi.revents)
	{
		current_cpid=(CopyID)cpn;
		rv=cpn->fdb->fdnotify(&fdi);
		current_cpid=NULL;
	}
	return(rv);
}


int FDCopyManager::fdnotify(FDInfo *fdi)
{
	_LockCPN();
	
	// Get associated copy node: 
	MCopyNode *cpn=(MCopyNode *)(fdi->dptr);
	assert(cpn);
	
	// NOTE; fdi->current may be modified by us. 
	//       So, it's no problem passing it to the clients. 
	
	// which_fd: +1 -> write fd; -1 -> read fd; 0 -> OM_FdBuf2FdBuf
	int which_fd=0;
	if(cpn->opmode==OM_FdBuf2FdBuf)
	{  assert(fdi->pollid==cpn->psrcid && fdi->pollid==cpn->pdestid);  }
	else if(fdi->fd==FDfd(cpn->psrcid))
	{
		which_fd=-1;
		assert(fdi->pollid==cpn->psrcid);
	}
	else if(fdi->fd==FDfd(cpn->pdestid))
	{
		which_fd=+1;
		assert(fdi->pollid==cpn->pdestid);
	}
	else
	{  assert(0);  }
	
	// ** First, check state stuff: 
	#if TESTING
	if(cpn->cpstate & CPSStopped)
	{
		// If we're stopped, no events which we control may be set: 
		assert(!(cpn->srcfd_ev & cpn->controlled_src_ev));
		assert(!(cpn->destfd_ev & cpn->controlled_dest_ev));
	}
	#endif
	
	// NOTE: Data check is done before POLLERR/POLLHUP check because 
	//       if we emulate poll(2) via select(2) we cannot even get 
	//       them at all. Furthermore, POLLERR is quite unspecific 
	//       but if we get the error using read(2) or write(2), 
	//       then errno is set apropriately. We only do that if 
	//       POLLIN/POLLOUT is set, though. 
	
	// ** Then, check for data: 
	// _ReadInData_Buf() -> read in data into dest buffer 
	// _WriteOutData_Buf() -> write out data from src buffer
	// _ReadInData_FD() -> read in data into buffer io buffer
	// _WriteOutData_FD() -> write out data from buffer io buffer
	int must_redecide=(cpn->opmode==OM_Fd2Fd);
	if(cpn->opmode==OM_FdBuf2FdBuf)
	{
		// All events are for us. Note also: input fd = output fd. 
		do {
			if(fdi->revents & POLLIN)
			{  if(_ReadInData_Buf(cpn,fdi->current))  break;  }
			if(fdi->revents & POLLOUT)
			{  _WriteOutData_Buf(cpn,fdi->current);  }
		} while(0);
	}
	else if(which_fd<0)  // input fd
	{
		if(fdi->revents & POLLIN)  // Can read data...
		{
			if(cpn->opmode==OM_Fd2Fd)
			{
				if(_ReadInData_FD(cpn,fdi->current))
				{  must_redecide=0;  }
			}
			else  // OM_Fd2Buf
			{  _ReadInData_Buf(cpn,fdi->current);  }
		}
		#if TESTING
		else if(!(fdi->revents & (POLLERR | POLLNVAL | POLLHUP)))
		{  fprintf(stderr,"Oops: CP:%d: fdi->revents=%d for fd=%d (cannot read)\n",
			__LINE__,fdi->revents,fdi->fd);  }
		#endif
	}
	else if(which_fd>0)  // output fd
	{
		if(fdi->revents & POLLOUT)  // Can write data...
		{
			if(cpn->opmode==OM_Fd2Fd)
			{
				if(_WriteOutData_FD(cpn,fdi->current))
				{  must_redecide=0;  }
			}
			else  // OM_Buf2Fd
			{  _WriteOutData_Buf(cpn,fdi->current);  }
		}
		#if TESTING
		else if(!(fdi->revents & (POLLERR | POLLNVAL | POLLHUP)))
		{  fprintf(stderr,"Oops: CP:%d: fdi->revents=%d for fd=%d (cannot write)\n",
			__LINE__,fdi->revents,fdi->fd);  }
		#endif
	}
	if(must_redecide)
	{  _ReDecidePollEvents(cpn);  }
	
	// ** Then, check for error and hangup: 
	if((fdi->revents & (POLLERR | POLLNVAL | POLLHUP)) && !cpn->is_dead)
	{
		// See if the condition was already handeled: 
		if( (!which_fd) ? (cpn->opmode!=OM_FdBuf2FdBuf) : 
			((which_fd<0 ? cpn->psrcid : cpn->pdestid) != NULL) )
		{
			// Now we have a problem. The job is still there, i.e. no error 
			// or EOF caught using read() or write(). 
			StatusCode scode=SCNone;
			// This works correct for which_fd=0. 
			if(which_fd>=0)
			{
				if(fdi->revents & POLLHUP)  (int)scode|=SCOutHup;
				if(fdi->revents & POLLERR)  (int)scode|=SCErrPollO;
			}
			if(which_fd<=0)
			{
				if(fdi->revents & POLLHUP)  (int)scode|=SCInHup;
				if(fdi->revents & POLLERR)  (int)scode|=SCErrPollI;
			}
			CopyInfo cpi(cpn,scode);
			if(fdi->revents & (POLLNVAL | POLLHUP))
			{  cpi.err_no=fdi->revents;  }
			cpi.fdtime=fdi->current;  // may be modified (FDManager copies it)
			// Okay, we kill it: 
			_FinishRequest(cpn,&cpi,which_fd);
		}
	}
	
	// ** Inform the client FDBase if requested: 
	if(cpn->recv_fdnotify)
	{  _SendFDNotify(cpn,fdi,which_fd);  }
	
	_UnlockCPN();
	return(0);
}

#warning "***************************************************"
#warning ERROR!!!! REMOVE THAT _SetWriteStart() CRAP! YES, IT IS CRAP! \
   THE SOLUTION IS TO SET low_read_thresh>low_write_thresh AND \
   high_read_thresh>high_write_thresh. 
#warning "***************************************************"

/*
**Solution:**
        low_write_thresh         high_write_thresh
	           |   low_read_thresh       |   high_read_thresh
               v       v                 v        v
       .-----------------------------------------------------.
read:  |               ############################          |
write: |       ###########################                   |
       `-----------------------------------------------------´
                       |<---operation--->|
                            in RW mode
### COPY THIS NOTE TO cpmanager.h ###

So, make sure: 
  low_read_thresh  > low_write_thresh
  high_read_thresh > high_write_thresh
  low_read_thresh << high_write_thresh
AND NOT THE OTHER WAY ROUND (because then, nothing will work.

probably best way: 
	In case max_read_len / max_write_len is SMALL but not 0 (0 = unlimited)
		low_write_thresh=max_write_len-1
	    high_read_thresh=buflen-max_read_len+1
    (Think of these 2 values as "min read/write length".)
*/

// This needs some explanation: 
// Let's say the low read/write thresh is 1024 and the hight read/write thresh 
// is 8192. Then, the algorithm will read until high thresh, stop reading there 
// (because of high thresh) and then start writing (same reason). Then, writing 
// goes on until low read/write thresh is reached. Result: We never do 
// POLLIN|POLLOUT (i,e. read and write "simultaniously") which is bad because 
// that's why we do all that thresh stuff in the first place. 
// So, if we're not writing but reading and the buffer is full with at least 
// write_start bytes (middle between low and high write thresh), then we start 
// writing. This function sets the value write_start which holds the threshold. 
// This is only set if the current buffer content is < low_write_thresh and 
// we're not currently writing. 
void FDCopyManager::_SetWriteStart(MCopyNode *cpn)
{
	assert(cpn->opmode==OM_Fd2Fd);
	/*if(cpn->bufuse<=cpn->req.low_write_thresh && 
	   !(cpn->destfd_ev & POLLOUT))
	{
		cpn->write_start=
			(size_t(cpn->req.low_write_thresh)+
			size_t(cpn->req.high_write_thresh))/2;
	}*/
}


// Re-decide on poll events for input fd (fd -> fd only): 
void FDCopyManager::_ReDecidePollEvents(MCopyNode *cpn)
{
	assert(cpn->opmode==OM_Fd2Fd);
	
	// Current events in cpn->srcfd_ev, cpn->destfd_ev. 
	
	#if PRINT_SWITCH_DUMP
	short old_srcev=cpn->srcfd_ev,
	      old_destev=cpn->destfd_ev;
	#endif
	
	/*---- INPUT ----*/
	if(cpn->srcfd_ev & POLLIN)  // currently reading
	{
		// Stop if high thresh reached or if flushing: 
		if(cpn->bufuse>=size_t(cpn->req.high_read_thresh) || 
		   (cpn->cpstate & CPSFlushing) )
		{  _ChangePollFDSrc(cpn,0,POLLIN);  }
	}
	else if(!(cpn->cpstate & CPSFlushing)) // currently not reading & not flusing 
	{
		// Start if low thresh reached: 
		if(cpn->bufuse<=size_t(cpn->req.low_read_thresh))
		{  _ChangePollFDSrc(cpn,POLLIN,0);  }
	}
	
	/*---- OUTPUT ----*/
	if(!(cpn->destfd_ev & POLLOUT))  // currently not writing
	{
		// Start if high thresh reached or if flushing. 
		if(cpn->bufuse>=size_t(cpn->req.high_write_thresh) || 
		   (cpn->cpstate & CPSFlushing) )
		{  _ChangePollFDDest(cpn,POLLOUT,0);  }
		// Or if we did not yet write anything and the buffer is 
		// half way full (that's the write_start thingy): 
		else if( cpn->write_start && cpn->bufuse>=cpn->write_start && 
		   		(cpn->srcfd_ev & POLLIN) )
		{
			cpn->write_start=0;
			_ChangePollFDDest(cpn,POLLOUT,0);
		}
	}
	else if(!(cpn->cpstate & CPSFlushing))  // currently writing & not flusing 
	{
		// Stop if low thresh reached: 
		if(cpn->bufuse<=size_t(cpn->req.low_write_thresh))
		{  _ChangePollFDDest(cpn,0,POLLOUT);  }
	}
	
	#if PRINT_SWITCH_DUMP
	if(old_srcev!=cpn->srcfd_ev || old_destev!=cpn->destfd_ev)
	{
		fprintf(stderr,"IO SWITCH: [%c%c] (bufuse=%u)\n",
			cpn->srcfd_ev & POLLIN ? 'r' : '-',
			cpn->destfd_ev & POLLOUT ? 'w' : '-',
			cpn->bufuse);
	}
	#endif
}


// Read in data for fd -> buf copying: 
int FDCopyManager::_ReadInData_Buf(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Fd2Buf || cpn->opmode==OM_FdBuf2FdBuf);
	assert(!(cpn->cpstate & CPSFlushing));  // May not happen with OM_Fd2Buf, OM_FdBuf2FdBuf. 
	
	// Read data from fd and write to buffer. Easy case. 
	size_t need=cpn->Wbufend-cpn->bufheadW;
	if(cpn->req.max_read_len && need>cpn->req.max_read_len)
	{  need=cpn->req.max_read_len;  }
	
	ssize_t rd=read(FDfd(cpn->psrcid),cpn->bufheadW,need);
	if(rd<0)
	{  return(_ReadError(cpn,fdtime));  }
	if(rd>0)
	{
		// Update buffer: 
		cpn->bufheadW+=rd;
		
		// Update statistics: 
		cpn->stat.read_bytes+=copylen_t(rd);
		cpn->read_bytes+=copylen_t(rd);
		
		// Send progress info: 
		if(cpn->bufheadW<cpn->Wbufend)
		{
			_SendProgressInfo(cpn,fdtime,PARead);
			return(0);
		}
		// If we reach here: reading done; so request is done. 
	}
	// rd==0 or rd>0 and reading done. 
	CopyInfo cpi(cpn,rd ? SCLimit : SCRead0);
	cpi.fdtime=fdtime;  // May be modified (FDManager copies it). 
	return(_FinishRequest(cpn,&cpi,-1));
}


// Write out data for buf -> fd copying: 
int FDCopyManager::_WriteOutData_Buf(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Buf2Fd || cpn->opmode==OM_FdBuf2FdBuf);
	assert(!(cpn->cpstate & CPSFlushing));  // may not happen with OM_Buf2Fd
	
	// Read data from buf and write to fd. Easy case. 
	size_t avail=cpn->Rbufend-cpn->bufheadR;
	if(cpn->req.max_write_len && avail>cpn->req.max_write_len)
	{  avail=cpn->req.max_write_len;  }
	
	ssize_t wr=write(FDfd(cpn->pdestid),cpn->bufheadR,avail);
	if(wr<0)
	{  return(_WriteError(cpn,fdtime));  }
	if(wr>0)
	{
		// Update buffer: 
		cpn->bufheadR+=wr;
		
		// Update statistics: 
		cpn->stat.written_bytes+=copylen_t(wr);
		
		// Send progress info: 
		if(cpn->bufheadR<cpn->Rbufend)  // otherwise: done
		{
			_SendProgressInfo(cpn,fdtime,PAWrite);
			return(0);
		}
		// If we reach here we're done. 
	}
	// wr==0 or all data written. 
	#if TESTING
	if(wr==0)  // Hmm.. wrote 0 bytes. 
	{  fprintf(stderr,"OOPS: CP: wrote 0/%u bytes on fd %d\n",
		avail,FDfd(cpn->pdestid));  }
	#endif
	// Done. 
	CopyInfo cpi(cpn,wr ? SCLimit : SCWrite0);
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	return(_FinishRequest(cpn,&cpi,+1));
}


// Read in data for fd -> fd copying: 
// Only call if cpn->buf is non-NULL (copy fd to fd). 
int FDCopyManager::_ReadInData_FD(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Fd2Fd);
	
	assert(cpn->iobufsize>=cpn->bufuse);
	if(cpn->bufuse>=size_t(cpn->req.high_read_thresh))
	{
		#if TESTING
		// There is not enough free space in the buffer; we should 
		// not be here at all. 
		fprintf(stderr,"Oops: CP:%d: why are we here? (%u,%u)\n",
			__LINE__,cpn->bufuse,cpn->req.high_read_thresh);
		#endif
		return(0);
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
		need_readv=1;
		rio[1].iov_base=cpn->buf;
		rio[1].iov_len=avail-rio->iov_len;
	}
	
//fprintf(stderr,"RD: pos=%d, len=%d  ",(char*)rio->iov_base-cpn->buf,rio->iov_len);
//if(need_readv)
//fprintf(stderr,"pos=%d, len=%d\n",(char*)rio[1].iov_base-cpn->buf,rio[1].iov_len);
//else
//fprintf(stderr,"\n");
	
	// Actually read the stuff in: 
	ssize_t rd = need_readv ? 
		readv(FDfd(cpn->psrcid),rio,2) : 
		read(FDfd(cpn->psrcid),rio->iov_base,rio->iov_len);
//fprintf(stderr,"rd=%d; bufsize=%d; bufuse=%d; avail=%d\n",rd,
//cpn->iobufsize,cpn->bufuse,cpn->iobufsize-cpn->bufuse);
	if(rd<0)
	{
		// Error condition. 
		return(_ReadError(cpn,fdtime));
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
		assert(cpn->bufuse<=cpn->iobufsize);
		
		// Update statistics: 
		cpn->stat.read_bytes+=copylen_t(rrd);
		cpn->read_bytes+=copylen_t(rrd);
		
		// Send progress info: 
		if(!read_limit || rrd<avail)  // read limit not reached
		{
			_SendProgressInfo(cpn,fdtime,PARead);
			return(0);
		}
		// If we're here -> read limit reached. 
	}
	// rd==0 or read limit reached. 
	CopyInfo cpi(cpn,rd ? SCLimit : SCRead0);
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	return(_FinishRequest(cpn,&cpi,-1));
}


// Write out data for fd -> fd copying: 
// Only call if cpn->buf is non-NULL (copy fd to fd). 
int FDCopyManager::_WriteOutData_FD(MCopyNode *cpn,HTime *fdtime)
{
	assert(cpn->opmode==OM_Fd2Fd);
	
	assert(cpn->iobufsize>=cpn->bufuse);
	if(cpn->bufuse<=size_t(cpn->req.low_write_thresh) && 
	   !(cpn->cpstate & CPSFlushing))
	{
		#if TESTING
		// There is not enough data in the buffer; we should 
		// not be here at all. 
		fprintf(stderr,"Oops: CP:%d: why are we here? (%u,%u)\n",
			__LINE__,cpn->bufuse,cpn->req.low_write_thresh);
		#endif
		return(0);
	}
	
	// See how much data is available in the buffer
	size_t avail=cpn->bufuse;
	
	// Okay, see how much we may write: 
	if(cpn->req.max_write_len && avail>cpn->req.max_write_len)
	{  avail=cpn->req.max_write_len;  }
	
	// Now, set up write io vector: 
	struct iovec wio[2];  // never need more than 2
	int need_writev=0;
	wio->iov_base=cpn->bufheadR;
	wio->iov_len=cpn->bufend-cpn->bufheadR;
	if(wio->iov_len>=avail)   // >= is CORRECT. 
	{  wio->iov_len=avail;  }
	else
	{
		need_writev=1;
		wio[1].iov_base=cpn->buf;
		wio[1].iov_len=avail-wio->iov_len;
	}
	
	// Actually write the stuff out: 
	ssize_t wr = need_writev ? 
		writev(FDfd(cpn->pdestid),wio,2) : 
		write(FDfd(cpn->pdestid),wio->iov_base,wio->iov_len);
	if(wr<0)
	{
		// Error condition. 
		return(_WriteError(cpn,fdtime));
	}
	if(wr>0)
	{
		// Wrote out data; update buffer stuff: 
		size_t wwd=wr;
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
			return(0);
		}
		// When er're here -> writing done. 
	}
	// Here: wr==0 or everything written. 
	#if TESTING
	if(wr==0)  // Hmmm... write returns 0. 
	{  fprintf(stderr,"OOPS: CP: wrote 0/%u bytes on fd %d\n",
		avail,FDfd(cpn->pdestid));  }
	#endif
	CopyInfo cpi(cpn,wr ? SCNone : SCWrite0);
	cpi.fdtime=fdtime;  // may be modified (FDManager copies it)
	return(_FinishRequest(cpn,&cpi,+1));
}


// Called to initially start the copy request. 
void FDCopyManager::_StartCopyRequest(MCopyNode *cpn)
{
	assert(cpn->cpstate & CPSStopped);
	assert(cpn->opmode);
	
	// Set start time: 
	cpn->stat.starttime.SetCurr();
	
	if(cpn->opmode==OM_Buf2Fd)
	{  _ChangePollFDDest(cpn,POLLOUT,0);  }
	else if(cpn->opmode==OM_FdBuf2FdBuf)   // cpn->psrcid=cpn->pdestid here. 
	{  _ChangePollFDSD(cpn,POLLIN | POLLOUT,0);  }
	else  // OM_Fd2Buf and OM_Fd2Fd: 
	{
		if(cpn->opmode==OM_Fd2Fd)
		{  _SetWriteStart(cpn);  }
		_ChangePollFDSrc(cpn,POLLIN,0);
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
			_ChangePollFDDest(cpn,POLLOUT,0);
			break;
		case OM_Fd2Fd:
			_SetWriteStart(cpn);
			_ReDecidePollEvents(cpn);
			break;
		case OM_Fd2Buf:
			_ChangePollFDSrc(cpn,POLLIN,0);
			break;
		case OM_FdBuf2FdBuf:
			// In case one of the copy parts is done, OM_FdBuf2FdBuf 
			// gets changed into OM_Fd2Fd or OM_Fd2Buf so there is no 
			// problem. 
			_ChangePollFDSD(cpn,POLLIN | POLLOUT,0);
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
	
	if(cpn->opmode==OM_FdBuf2FdBuf)
	{  _ChangePollFDSD(cpn,0,POLLIN | POLLOUT);  }
	else
	{
		if(cpn->srcfd_ev)   _ChangePollFDSrc(cpn,0,POLLIN);
		if(cpn->destfd_ev)  _ChangePollFDDest(cpn,0,POLLOUT);
	}
	
	(int)cpn->cpstate|=CPSStopped;
}


FDCopyManager::CopyID FDCopyManager::CopyFD(FDCopyBase *client,
	CopyRequest *req,FDBase *fdb)
{
	req->errcode=0;
	
	#if TESTING
	assert(clients.find(client));
	#endif
	
	// Check input: 
	OperationMode opmode=OM_None;
	if(req->srcfd<0 && req->destfd>=0 && 
	   req->srcbuf && !req->destbuf)
	{  opmode=OM_Buf2Fd;  }
	else if(req->srcfd>=0 && req->destfd>=0 && req->srcfd!=req->destfd && 
	   !req->srcbuf && !req->destbuf)
	{  opmode=OM_Fd2Fd;  }
	else if(req->srcfd>=0 && req->destfd<0 && 
	   !req->srcbuf && req->destbuf)
	{  opmode=OM_Fd2Buf;  }
	else if(req->srcfd>=0 &&                   req->srcfd==req->destfd && 
	   req->srcbuf && req->destbuf)
	{  opmode=OM_FdBuf2FdBuf;  }
	
	if(opmode==OM_None)
	{
		req->errcode=-3;
		return(NULL);
	}
	
	// This may give a warning if req->len is an unsigned type: 
	//if(req->len<0)      req->len=0;  <-- union...
	if(req->srclen<0)   req->srclen=0;
	if(req->destlen<0)  req->destlen=0;
	// May not pass buffer of size 0: 
	if( ( (opmode==OM_Buf2Fd || opmode==OM_FdBuf2FdBuf) && !req->srclen) || 
	    ( (opmode==OM_Fd2Buf || opmode==OM_FdBuf2FdBuf) && !req->destlen) )
	{  req->errcode=1;  return(NULL);  }
	
	if(req->recv_fdnotify && !fdb)
	{  req->errcode=-4;  return(NULL);  }
	
	// See some more details: 
	if(opmode==OM_Fd2Fd)  // fd -> fd
	{
		// In this case len must specify the len and destlen must be 0 or <0. 
		if(req->destlen>0 /*|| req->srclen>0*/)  // <-- union...
		{  req->errcode=-3;  return(NULL);  }
		req->destlen=0;
		
		// If iobufsize is 0, set up default: 
		if(!req->iobufsize)
		{  req->iobufsize = default_iobufsize ? default_iobufsize : 1024;  }
		// If we shall copy less bytes than iobufsize, make buffer smaller: 
		if(req->len && req->len<copylen_t(req->iobufsize))
		{  req->iobufsize=size_t(req->len);  }
	}
	else 
	{  req->iobufsize=0;  }
	
	// Set up thresh values: 
	if(opmode==OM_Fd2Fd)  // fd -> fd
	{
		if(req->iobufsize<32)
		{
			// The calculations below will not work for too small buffers. 
			// As we use such small buffers if the length limit is such 
			// small, this case has to be dealt with. We simply assume 
			// that IO will be atomic: 
			req->low_read_thresh=0;
			req->high_read_thresh=req->iobufsize;
			req->low_write_thresh=0;
			req->high_write_thresh=req->iobufsize;
		}
		else
		{
			// First, set high defaults if needed: 
			ssize_t def_high_io_thresh=req->iobufsize-req->iobufsize/8;
			if(req->high_read_thresh<0)
			{  req->high_read_thresh=def_high_io_thresh;  }
			if(req->high_write_thresh<0)
			{  req->high_write_thresh=def_high_io_thresh;  }

			// Then, make sure that the upper limits are okay: 
			ssize_t max_io_thresh=req->iobufsize-req->iobufsize/8;
			if(req->high_read_thresh>max_io_thresh)
			{  req->high_read_thresh=max_io_thresh;  }
			if(req->high_write_thresh>max_io_thresh)
			{  req->high_write_thresh=max_io_thresh;  }

			// Okay, if the low thresh is <0, use defaults: 
			ssize_t def_low_io_thresh=req->iobufsize/8;
			if(req->low_read_thresh<0)
			{  req->low_read_thresh=def_low_io_thresh;  }
			if(req->low_write_thresh<0)
			{  req->low_write_thresh=def_low_io_thresh;  }

			// NOTE: All these calculations expect a buffer which is at least 
			//       16 or 32 bytes long. 
			// Okay, now make sure that the low thresh is below 
			// the high thresh: 
			// Adjust the value which is <50% iobufsize. 
			ssize_t min_io_hysteresis=req->iobufsize/8;
			if(req->high_read_thresh-req->low_read_thresh < min_io_hysteresis)
			{
				if(size_t(req->low_read_thresh)*2>req->iobufsize)
				{  req->low_read_thresh=req->high_read_thresh-min_io_hysteresis;  }
				else if(size_t(req->high_read_thresh)*2<req->iobufsize)
				{  req->high_read_thresh=req->low_read_thresh+min_io_hysteresis;  }
			}
			if(req->high_write_thresh-req->low_write_thresh < min_io_hysteresis)
			{
				if(size_t(req->low_write_thresh)*2>req->iobufsize)
				{  req->low_write_thresh=req->high_write_thresh-min_io_hysteresis;  }
				else if(size_t(req->high_write_thresh)*2<req->iobufsize)
				{  req->high_write_thresh=req->low_write_thresh+min_io_hysteresis;  }
			}
		}
		
		// NOTE: These comparations are CORRECT. ALL. 
		assert(req->low_read_thresh<req->high_read_thresh);
		assert(req->low_write_thresh<req->high_write_thresh);
		assert(req->low_read_thresh>=0 && req->low_write_thresh>=0);
		assert(size_t(req->high_read_thresh)<=req->iobufsize && 
		       size_t(req->high_write_thresh)<=req->iobufsize );
	}
	else
	{
		// Thresh values not needed. 
		// Do not meddle around with them: 
		req->low_read_thresh=-1;
		req->high_read_thresh=-1;
		req->low_write_thresh=-1;
		req->high_write_thresh=-1;
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
	cpn->opmode=opmode;
	
	// Allocate our FDNodes at the FDManager: 
	if(opmode==OM_Fd2Buf || opmode==OM_Fd2Fd || opmode==OM_FdBuf2FdBuf)
	{
		int rv=PollFD(req->srcfd,0,cpn,&cpn->psrcid);
		if(rv!=0)
		{  req->errcode=-2;  goto retunpoll;  }
	}  // NO else 
	if(opmode==OM_Buf2Fd || opmode==OM_Fd2Fd)
	{
		int rv=PollFD(req->destfd,0,cpn,&cpn->pdestid);
		if(rv!=0)
		{  req->errcode=-2;  goto retunpoll;  }
	}
	else if(opmode==OM_FdBuf2FdBuf)
	{  cpn->pdestid=cpn->psrcid;  }
	
	// Allocate copy buffer if needed: 
	switch(opmode)
	{
		case OM_Fd2Fd:
			// Actually allocate buffer: 
			cpn->buf=(char*)LMalloc(cpn->req.iobufsize);
			if(!cpn->buf)
			{  req->errcode=-1;  goto retfreebuf;  }
			cpn->bufend=cpn->buf+cpn->req.iobufsize;
			cpn->bufheadR=cpn->buf;
			cpn->bufheadW=cpn->buf;
			cpn->bufuse=0;
			break;
		// Set buffer vars (src or dest (not both) is passd as argument 
		// to CopyFD()). 
		case OM_Buf2Fd:
			cpn->bufheadR = cpn->req.srcbuf;
			// cpn->bufheadW stays NULL
			cpn->Rbufend = cpn->bufheadR + cpn->req.srclen;
			// bufuse must stay 0 as buf=NULL. 
			//cpn->req.iobufsize stays 0. 
			break;
		case OM_Fd2Buf:
			//cpn->bufheadR must stay 0
			cpn->bufheadW = cpn->req.destbuf;
			cpn->Wbufend = cpn->bufheadW + cpn->req.destlen;
			// bufuse must stay 0 as buf=NULL. 
			//cpn->req.iobufsize stays 0. 
			break;
		case OM_FdBuf2FdBuf:
			cpn->bufheadR = cpn->req.srcbuf;
			cpn->bufheadW = cpn->req.destbuf;
			cpn->Rbufend = cpn->bufheadR + cpn->req.srclen;
			cpn->Wbufend = cpn->bufheadW + cpn->req.destlen;
			// bufuse must stay 0 as buf=NULL. 
			//cpn->req.iobufsize stays 0. 
			break;
		default:  assert(0);  break;
	}
	
	// Copy values (for security reason; so that client cannot 
	// modify them when the request gets passed down by cpnotify()): 
	cpn->iobufsize=cpn->req.iobufsize;
	cpn->recv_fdnotify=cpn->req.recv_fdnotify;
	
	_SetControlledEvents(cpn);
	
	// Set poll events of FDs to 0 if FDBase pointer is set: 
	_SavePollEvents(cpn);
	
	// Actually start copy process: 
	_StartCopyRequest(cpn);
	
	// Finally, queue the request: 
	cpn->client->rlist.append(cpn);
	
	return((CopyID)cpn);
	
retfreebuf:;
	cpn->buf=(char*)LFree(cpn->buf);
retunpoll:;                        UnpollFD(cpn->psrcid);
	if(cpn->psrcid!=cpn->pdestid)  UnpollFD(cpn->pdestid);
	cpn->psrcid=NULL;  cpn->pdestid=NULL;
	delete cpn;
	return(NULL);
}


int FDCopyManager::CopyControl(CopyID cpid,ControlCommand cc)
{
	if(!cpid || ((MCopyNode*)cpid)->is_dead)
	{  return(-2);  }
	
	MCopyNode *cpn=(MCopyNode*)cpid;
	switch(cc)
	{
		case CCKill:
		case CCKillI:
		case CCKillO:
		{
			int dir=0;
			switch(cpn->opmode)
			{
				case OM_Buf2Fd:  if(cc==CCKillI)  return(-4);  break;
				case OM_Fd2Buf:  if(cc==CCKillO)  return(-4);  break;
				case OM_FdBuf2FdBuf:
					     if(cc==CCKillI)  dir=-1;
					else if(cc==CCKillO)  dir=+1;
					//else dir=0 as initialized
					break;
				case OM_Fd2Fd:
					if(cc==CCKillI || cc==CCKillO)  return(-2);
					break;
			}
			CopyInfo cpi(cpn,SCKilled);
			_FinishRequest(cpn,&cpi,dir);
		}  break;
		case CCTerm:
		{
			int dir=-1;
			switch(cpn->opmode)
			{
				case OM_Buf2Fd:  // fall through
				case OM_Fd2Buf:  dir=0;
				case OM_Fd2Fd:   dir=-1;  break;
				case OM_FdBuf2FdBuf:  return(-4);  break;
			}
			CopyInfo cpi(cpn,SCTerm);
			_FinishRequest(cpn,&cpi,dir);
		}  break;
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


int FDCopyManager::CPPollFD(CopyID cpid,int dir,
	short set_events,short clear_events)
{
	if(!cpid || ((MCopyNode*)cpid)->is_dead || !dir)  return(-2);
	MCopyNode *cpn=(MCopyNode*)cpid;
	if(!cpn->recv_fdnotify)  return(-2);
	
	if(!((dir<0) ? cpn->psrcid : cpn->pdestid))  return(1);
	
	// Check if we control the requested flags: 
	short mf=set_events | clear_events;  // modified flags
	if(( (dir<0) ? 
		(cpn->controlled_src_ev & mf) : 
	    (cpn->controlled_dest_ev & mf) ))
	{  return(-1);  }
	
	if(dir<0)  _ChangePollFDSrc(cpn,set_events,clear_events);
	else       _ChangePollFDDest(cpn,set_events,clear_events);
	return(0);
}


// Save ("original") poll events (client side) on passed fds and 
// set them to 0. (Only apropriate ones.)
// Mofifies srcfd_ev,destfd_ev if recv_fdnotify was set. 
void FDCopyManager::_SavePollEvents(MCopyNode *cpn)
{
	FDBase *fdb=cpn->fdb;
	if(!fdb)  return;
	
	PollID pollid;
	if(cpn->req.srcfd==cpn->req.destfd)  // <-- case OM_FdBuf2FdBuf
	{
		pollid=fdb->FDPollID(cpn->req.srcfd);
		if(pollid)
		{
			short flags=fdb->FDEvents(pollid);
			if(flags)  fdb->PollFD(pollid,0);
			if(!cpn->recv_fdnotify)
			{
				cpn->orig_dest_events=cpn->orig_src_events=flags;
				cpn->orig_flags|=(MCopyNode::OFSrcSet | MCopyNode::OFDestSet);
			}
			// All flags are controlled by us. 
		}
	}
	else  // srcfd != destfd
	{
		pollid=fdb->FDPollID(cpn->req.srcfd);
		if(pollid)
		{
			short flags=fdb->FDEvents(pollid);
			if(flags)  fdb->PollFD(pollid,0);
			if(cpn->recv_fdnotify)
			{
				// FDCopyManager now polls the FD, not FDBase *fdb: 
				// And we poll all flags except POLLIN (controlled by us). 
				cpn->srcfd_ev = flags & ~POLLIN;
				if(cpn->srcfd_ev)
				{  PollFD(cpn->psrcid,cpn->srcfd_ev);  }
			}
			else
			{
				cpn->orig_src_events=flags;
				cpn->orig_flags|=MCopyNode::OFSrcSet;
			}
		}
		pollid=fdb->FDPollID(cpn->req.destfd);
		if(pollid)
		{
			short flags=fdb->FDEvents(pollid);
			if(flags)  fdb->PollFD(pollid,0);
			if(cpn->recv_fdnotify)
			{
				// FDCopyManager now polls the FD, not FDBase *fdb: 
				// And we poll all flags except POLLOUT (controlled by us). 
				cpn->destfd_ev = flags & ~POLLOUT;
				if(cpn->destfd_ev)
				{  PollFD(cpn->pdestid,cpn->destfd_ev);  }
			}
			else
			{
				cpn->orig_dest_events=fdb->FDEvents(pollid);
				cpn->orig_flags|=MCopyNode::OFDestSet;
			}
		}
	}
}


// Restore the saved poll events:
// only_dir: 0 -> both directions; +1 -> only dest; -1 -> only src. 
// Must be called before mutating a OM_FdBuf2FdBuf into a Fd2Buf or Buf2Fd. 
void FDCopyManager::_RestorePollEvents(MCopyNode *cpn,int only_dir)
{
	FDBase *fdb=cpn->fdb;
	if(!fdb)  return;
	
	if(cpn->psrcid!=cpn->pdestid)  // <-- NOT OM_FdBuf2FdBuf
	{
		// In case cpn->recv_fdnotify=0 we simply write back the old flags. 
		// If cpn->recv_fdnotify!=0, we use the current flags. 
		if( ( (cpn->orig_flags & MCopyNode::OFSrcSet) || cpn->recv_fdnotify )
		   && only_dir!=+1)  // correct. 
		{
			// First check for the PollID because the poll node might 
			// no longer exist; in this case we should not allocate it. 
			PollID pollid=fdb->FDPollID(FDfd(cpn->psrcid));
			if(pollid)
			{  fdb->PollFD(pollid,  cpn->recv_fdnotify ? 
					cpn->srcfd_ev : cpn->orig_src_events );  }
			// Flags are restored, we shall not do that more than once: 
			cpn->orig_flags&=~MCopyNode::OFSrcSet;
		}
		if( ( (cpn->orig_flags & MCopyNode::OFDestSet) || cpn->recv_fdnotify ) 
		   && only_dir!=-1)  // correct. 
		{
			PollID pollid=fdb->FDPollID(FDfd(cpn->pdestid));
			if(pollid)
			{  fdb->PollFD(pollid,  cpn->recv_fdnotify ? 
					cpn->destfd_ev : cpn->orig_dest_events);  }
			cpn->orig_flags&=~MCopyNode::OFDestSet;
		}
		
		return;
	}
	
	// HERE: OM_FdBuf2FdBuf
	assert(cpn->opmode==OM_FdBuf2FdBuf);
	
	// ONLY CALLED BY _MutateRequest() with only_dir!=0. 
	// only_dir: 
	//  +1 -> restore dest (switch to Fd2Buf)
	//  -1 -> restore src  (switch to Buf2Fd)
	// ONLY CALLED BY _FinishRequest() with only_dir==0. 
	//   0 -> restore both (reason: kill request)
	if(only_dir)
	{
		// If the client does not get fdnotify() from us, we restore nothing. 
		// (Restoring is then done by the next call to this function. However, 
		// we will either cancel OFDestSet or OFDestSet to avoid 
		// double-restore.)
		if(cpn->recv_fdnotify)
		{
			// Simply make sure we no longer poll for things we do not need. 
			assert(cpn->psrcid==cpn->pdestid);
			_ChangePollFDSD(cpn,0,only_dir<0 ? POLLIN : POLLOUT);
		}
		cpn->orig_flags&=
			(only_dir<0) ? (~MCopyNode::OFSrcSet) : (~MCopyNode::OFDestSet);
	}
	else  // only_dir=0
	{
		if(cpn->recv_fdnotify || (cpn->orig_flags & MCopyNode::OFSrcSet) )
		{
			assert(cpn->psrcid==cpn->pdestid);
			// First check for the PollID because the poll node might 
			// no longer exist; in this case we should not allocate it. 
			PollID pollid=fdb->FDPollID(FDfd(cpn->psrcid));
			if(pollid)
			{
				short flags=0;
				if(cpn->recv_fdnotify)
				{  flags=cpn->srcfd_ev;  }  // = cpn->destfd_ev
				else // cpn->orig_flags & MCopyNode::OFSrcSet is set
				{  flags=cpn->orig_src_events;  } // = orig_dest_events
				fdb->PollFD(pollid,flags);
			}
			cpn->orig_flags&=~(MCopyNode::OFSrcSet | MCopyNode::OFDestSet);
		}
	}
}


// If it is an EINTR or an EWOULDBLOCK error, do nothing. 
// If it is an EPIPE send final pipe notify. 
// Otherwise send read error notify and finish input or request: 
int FDCopyManager::_ReadError(MCopyNode *cpn,HTime *fdtime)
{
	int errn=errno;
	if(errn==EINTR || errn==EWOULDBLOCK)
	{
		#if TESTING
		if(errn==EWOULDBLOCK)
		{  fprintf(stderr,"Oops: CP: reading would block (%d)\n",
			FDfd(cpn->psrcid));  }
		#endif
		// Okay, we'll try again later. 
		return(0);
	}
	
	#if TESTING
	if(errn==EPIPE)
	{  fprintf(stderr,"Ah... got an EPIPE on (input) fd %d\n",
		FDfd(cpn->psrcid));  }
	#endif
	
	// Okay, read returns an error. Finish input: 
	CopyInfo cpi(cpn,(errn==EPIPE) ? SCInPipe : SCErrRead);
	cpi.err_no=errn;
	cpi.fdtime=fdtime;  // May be modified (FDManager copies it). 
	return(_FinishRequest(cpn,&cpi,-1));
}

// Similar to _ReadError but for the write side: 
int FDCopyManager::_WriteError(MCopyNode *cpn,HTime *fdtime)
{
	int errn=errno;
	if(errn==EINTR || errn==EWOULDBLOCK)
	{
		#if TESTING
		if(errn==EWOULDBLOCK)
		{  fprintf(stderr,"Oops: CP: writing would block (%d)\n",
			FDfd(cpn->pdestid));  }
		#endif
		// Okay, we'll try again later. 
		return(0);
	}
	
	#if TESTING
	if(errn==EPIPE)
	{  fprintf(stderr,"Ah... got an EPIPE on (output) fd %d\n",
		FDfd(cpn->pdestid));  }
	#endif
	
	// Okay, write returns an error. That is fatal...
	CopyInfo cpi(cpn,(errn==EPIPE) ? SCOutPipe : SCErrWrite);
	cpi.err_no=errn;
	cpi.fdtime=fdtime;  // May be modified (FDManager copies it). 
	return(_FinishRequest(cpn,&cpi,+1));
}


void FDCopyManager::_MutateRequest(MCopyNode *cpn,CopyInfo *cpi,
	OperationMode new_opmode)
{
	assert(cpn->opmode==OM_FdBuf2FdBuf);
	
	assert(!(cpi->scode & SCFinal));  // may not be final here 
	_SendCpNotify(cpn,cpi);
	
	// Switch to new_opmode: 
	if(new_opmode==OM_Buf2Fd)
	{
		_RestorePollEvents(cpn,/*dir_only=*/-1);
		
		cpn->opmode=OM_Buf2Fd;
		cpn->psrcid=NULL;  // YES! Simply NULL because cpn->pdestid=cpn->psrcid [NO UnpollFD!]
		cpn->srcfd_ev=0;
		cpn->Wbufend=NULL;
		cpn->bufheadW=NULL;
	}
	else if(new_opmode==OM_Fd2Buf)
	{
		_RestorePollEvents(cpn,/*dir_only=*/+1);
		
		cpn->opmode=OM_Fd2Buf;
		cpn->pdestid=NULL;  // YES! Simply NULL because cpn->pdestid=cpn->psrcid [NO UnpollFD!]
		cpn->destfd_ev=0;
		cpn->Rbufend=NULL;
		cpn->bufheadR=NULL;
	}
	else assert(0);
	
	// Note: We need not update the poll events (stop polling 
	// for read/write) as that is done by _RestorePollEvents(). 
	// But we have to set controlled_src_ev and controlled_dest_ev: 
	_SetControlledEvents(cpn);
}


// Called by: 
//   Fd2Buf, Buf2Fd -> send cpnotify(), kill request, restore flags 
//                     (dir = <does not matter>)
//   OM_FdBuf2FdBuf -> mutate request into Fd2Buf or Buf2Fd (depends on dir)
//                     or kill it completely (dir=0)
//   Fd2Fd -> call cpnotify and either finish input (dir=-1) or kill request 
//            (dir>=0); restore apropriate flags. 
// (Finishing input means: Set persistent_sc to passed cpn->scode, notify 
// client, set input poll flags to 0 and set CPSFlushing in cpstate.) 
// dir: +1 -> write end; -1 -> read end 
// Return value: 
//   0 -> request changed (MCopyNode still there)
//   1 -> request killed (MCopyNode deleted!) 
int FDCopyManager::_FinishRequest(MCopyNode *cpn,CopyInfo *cpi,int dir)
{
	if(cpn->opmode==OM_FdBuf2FdBuf && dir)
	{
		_MutateRequest(cpn,cpi,dir>0 ? OM_Fd2Buf : OM_Buf2Fd);
		return(0);
	}
	if(cpn->opmode==OM_Fd2Fd && dir<0 && cpn->bufuse)
	{
		// Finish input, go into flushing state. 
		
		// Set flushing flag: 
		(int)cpn->cpstate|=CPSFlushing;
		
		// Restore input events before calling cpnotify(). 
		_RestorePollEvents(cpn,/*dir_only=*/-1);
		_SetControlledEvents(cpn);
		
		(int)cpn->persistent_sc|=cpi->scode;
		cpi->scode=cpn->persistent_sc;
		assert(!(cpi->scode & SCFinal));
		_SendCpNotify(cpn,cpi);
		
		// Don't need any more input: 
		cpn->srcfd_ev=0;
		UnpollFD(cpn->psrcid);
		cpn->psrcid=NULL;
		return(0);
	}
	
	// HERE: Fd2Buf, Buf2Fd or (OM_Fd2Fd and dir>=0) or 
	//       OM_FdBuf2FdBuf and dir==0). 
	
	// Restore events before calling cpnotify(). 
	// (Will not restore input fd events if already done earlier in 
	// _FinshInput().) 
	_RestorePollEvents(cpn,0);
	cpn->controlled_src_ev=cpn->controlled_dest_ev=0;
	
	(int)cpi->scode|=SCFinal;
	(int)cpi->scode|=cpn->persistent_sc;
	_SendCpNotify(cpn,cpi);
	
	_KillRequest(cpn);
	return(1);
}


// Kill request without any side effects (does not notify client, 
// does not restore client's fd flags, etc.)
void FDCopyManager::_KillRequest(MCopyNode *cpn)
{
	                               UnpollFD(cpn->psrcid);
	if(cpn->psrcid!=cpn->pdestid)  UnpollFD(cpn->pdestid);
	cpn->psrcid=NULL;  cpn->pdestid=NULL;
	#warning missing: kill timeout timer
	
	// Finally delete it: 
	cpn->is_dead=1;
	cpn->client->rlist.dequeue(cpn);
	if(cpn_locked)
	{  dead_cpn.append(cpn);  }
	else
	{  delete cpn;  }
}


int FDCopyManager::Register(FDCopyBase *client)
{
	if(client)
	{
		if(client->next || client==clients.last())
		{  return(1);  }   // already registered
		clients.append(client);
	}
	return(0);
}

void FDCopyManager::Unregister(FDCopyBase *client)
{
	if(!client)  return;
	
	// Client registered?
	if(client->next || client==clients.last())
	{
		// Terminate all the client's copy requests without notifying it: 
		while(client->rlist.first())
		{  _KillRequest(client->rlist.first());  }
		
		// Remove client from list: 
		clients.dequeue(client);
	}
}


FDCopyManager::FDCopyManager(int *failflag) : 
	FDBase(failflag),
	clients(failflag)
{
	int failed=0;
	
	current_cpid=NULL;
	cpn_locked=0;
	
	// Tell FDManager that we're a manager: 
	if(SetManager(FDManager::MT_FDCopy))
	{  ++failed;  }
	
	if(failed)
	{
		if(failflag)
		{  *failflag-=failed;  return;  }
		ConstructorFailedExit("CP");
	}
	
	/*--- DO NOT USE >int failed< BELOW HERE. ---*/
	
	// Init global manager: 
	#if TESTING
	if(manager)
	{  fprintf(stderr,"%s: more than one FDCopyManager.\n",
		prg_name);  abort();  }
	#endif
	
	manager=this;
}

FDCopyManager::~FDCopyManager()
{
	_UnlockCPN();  // calls _TidyUp(). 
	
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
	LinkedListBase<MCopyNode>(),
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
	
	controlled_src_ev=0;
	controlled_dest_ev=0;
	
	recv_fdnotify=0;
	is_dead=0;
	
	iobufsize=0;
	buf=NULL;
	Rbufend=NULL;
	Wbufend=NULL;
	bufheadR=NULL;
	bufheadW=NULL;
	bufuse=0;
	
	write_start=0;
	
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
	Rbufend=NULL;
	Wbufend=NULL;
	bufheadR=NULL;
	bufheadW=NULL;
	bufuse=0;
}

/******************************************************************************/

FDCopyManager::CopyStatistics::CopyStatistics(int * /*failflag*/) : 
	starttime()
{
	read_bytes=(copylen_t)0;
	written_bytes=(copylen_t)0;
}

/******************************************************************************/

FDCopyManager::CopyInfo::CopyInfo(
	const FDCopyManager::MCopyNode *cpn,
	FDCopyManager::StatusCode _scode,
	int * /*failflag*/)
{
	cpid=(CopyID)cpn;
	req=&cpn->req;
	stat=&cpn->stat;
	fdtime=NULL;
	
	scode=_scode;
	err_no=0;
}
