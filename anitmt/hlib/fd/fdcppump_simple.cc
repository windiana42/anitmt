/*
 * fdcppump_simple.cc
 * 
 * Implementation of class FDCopyPump_Simple which works in 
 * cooperation with class FDCopyBase (and thus FDManager). 
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

#define HLIB_IN_HLIB 1
#include <hlib/fdcopybase.h>

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#  warning TESTING switched on (using assert())
#  include <assert.h>
#else
#  define assert(x)  do{}while(0)
#endif


// Return value: 
//   0 -> still going on
//   1 -> EOF or buffer end
//  -1 -> error
int FDCopyPump_Simple::_ReadData(int fd,HTime *fdtime)
{
	// Assert: Otherwise we cannot read(2) data. 
	assert(fd_dir<0 && src->Type()==FDCopyIO::CPT_FD);
	
	FDCopyIO_FD *s = (FDCopyIO_FD*)src;
	
	FDCopyIO::DataPtr dptr;
	int rv=dest->dataptr(&dptr);
	if(rv)
	{  return(_EndJob(CopyInfo(this,SCErrCopyIO,rv,fdtime)));  }
	
	size_t need=dptr.buflen;
	if(s->max_iolen && need>s->max_iolen)
	{  need=s->max_iolen;  }
	
	ssize_t rd = need ? ::read(fd,dptr.buf,need) : 0;
	if(rd<0)
	{
		if(errno==EINTR || errno==EWOULDBLOCK)
		{
			#if TESTING
			if(errno==EWOULDBLOCK)
			{  fprintf(stderr,"Oops: simple pump: reading would block (%d)\n",fd);  }
			#endif
			// Okay, we'll try again later. 
			return(0);
		}
		return(_EndJob(CopyInfo(this,errno==EPIPE ? SCInPipe : SCErrRead,
			errno,fdtime)));
	}
	if(rd>0)
	{
		// Tell FDCopyIO: 
		FDCopyIO::DataDone ddone;
		ddone.buf=dptr.buf;
		ddone.donelen=rd;
		dest->datadone(&ddone);
		
		// Update statistics: 
		s->transferred+=(copylen_t)rd;
		
		if(dptr.more || ddone.donelen<dptr.buflen)
		{
			//#### send progress info
			return(0);
		}
		// If we reach here: reading done; so job is done. 
	}
	// rd==0 here, i.e. EOF -OR- rd>0 and end of buffer
	return(_EndJob(CopyInfo(this,(!rd && need) ? SCRead0 : SCLimit,0,fdtime)));
}


// Return value: 
//   0 -> still going on
//   1 -> write() returned 0 or buffer end
//  -1 -> error
int FDCopyPump_Simple::_WriteData(int fd,HTime *fdtime)
{
	// Assert: Otherwise we cannot write(2) data. 
	assert(fd_dir>0 && dest->Type()==FDCopyIO::CPT_FD);
	
	FDCopyIO_FD *d = (FDCopyIO_FD*)dest;
	
	FDCopyIO::DataPtr dptr;
	int rv=src->dataptr(&dptr);
	if(rv)
	{
		if(errno==EINTR || errno==EWOULDBLOCK)
		{
			#if TESTING
			if(errno==EWOULDBLOCK)
			{  fprintf(stderr,"Oops: simple pump: writing would block (%d)\n",fd);  }
			#endif
			// Okay, we'll try again later. 
			return(0);
		}
		return(_EndJob(CopyInfo(this,SCErrCopyIO,rv,fdtime)));
	}
	
	size_t want=dptr.buflen;
	if(d->max_iolen && want>d->max_iolen)
	{  want=d->max_iolen;  }
	
	ssize_t wr = want ? ::write(fd,dptr.buf,want) : 0;
	if(wr<0)
	{  return(_EndJob(CopyInfo(this,errno==EPIPE ? SCOutPipe : SCErrWrite,
		errno,fdtime)));  }
	if(wr>0)
	{
		// Tell FDCopyIO: 
		FDCopyIO::DataDone ddone;
		ddone.buf=dptr.buf;
		ddone.donelen=wr;
		src->datadone(&ddone);
		
		// Update statistics: 
		d->transferred+=(copylen_t)wr;
		
		if(dptr.more || ddone.donelen<dptr.buflen)
		{
			//#### send progress info
			return(0);
		}
		// If we reach here: writing done; so job is done. 
	}
	// wr==0 here, i.e. EOF -OR- wr>0 and end of buffer
	return(_EndJob(CopyInfo(this,(!wr && want) ? SCWrite0 : SCLimit,0,fdtime)));
}


int FDCopyPump_Simple::HandleFDNotify(FDManager::FDInfo *fdi)
{
	assert(!is_dead);
	assert(IsActive());   // Otherwise we may not be here. 
	assert(fdi->pollid==((FDCopyIO_FD*)(fd_dir<0 ? src : dest))->pollid);
	
	short our_ev=(fd_dir<0 ? POLLIN : POLLOUT);
	
	if((state & PS_Stopped))
	{
		if((fdi->revents & our_ev))
		{
			#if TESTING
			fprintf(stderr,"HMMM: FDCopyPump_Simple stopped but got fdnotify 0x%x\n",
				fdi->revents);
			#endif
			// Make sure that we really stopped...
			IChangeEvents(fdi->pollid,0,our_ev);
		}
		return(0);
	}
	
	// Okay, let's see what we can do...
	if((fdi->revents & our_ev))
	{
		int rv = (fd_dir<0) ? 
			_ReadData(fdi->fd,fdi->current) : 
			_WriteData(fdi->fd,fdi->current);
		if(rv)
		{
			// In this case, the job got finished, client was informed. 
			return(rv);
		}
	}
	
	// If we're here, the job is still active. Check for errors, etc. 
	if((fdi->revents & (POLLERR | POLLNVAL | POLLHUP) ))
	{
		StatusCode scode;
		int err_no=0;
		if(fdi->revents & POLLHUP)
		{  scode = (fd_dir<0) ? SCInHup : SCOutHup;  }
		else /* if(fdi->revents & (POLLERR | POLLNVAL)) */
		{
			scode = (fd_dir<0) ? SCErrPollI : SCErrPollO;
			err_no=fdi->revents;
		}
		return(_EndJob(CopyInfo(this,scode,err_no,fdi->current)));
	}
	
	return(0);
}


// NOTE: FDCopyPump::Control() may delete us (doing delete *this). 
int FDCopyPump_Simple::VControl(ControlCommand cc)
{
	if(is_dead || (!IsActive() && cc!=CC_Start) )
	{  return(-2);  }  // Must be active for this command
	
	switch(cc)
	{
		case CC_Start:  return(_StartJob());
		case CC_Term:
		case CC_Kill:
			_EndJob(CopyInfo(this,(cc==CC_Term ? SCTerm : SCKilled),0,NULL));
			// We get deleted by FDCopyBase::Control(). 
			return(0);
		case CC_Stop:
		case CC_StopI:
		case CC_StopO:  return(_StopContJob(1));
		case CC_Cont:
		case CC_ContI:
		case CC_ContO:  return(_StopContJob(0));
		//default: fall through to function body end
	}
	
	return(-2);
}


int FDCopyPump_Simple::_StartJob()
{
	if(IsActive())
	{  return(1);  }   // already started
	
	// See if we CAN start: 
	if(!src || !dest || !fd_dir)
	{  return(-1);  }
	
	assert(!src->active && !dest->active);
	
	// Tell FDCopyBase that we want start. 
	// It's not actually just that we _want_; FDCopyBase also 
	// treats us as running now. 
	// ** So, if sth fails below, DoneJob() must be called. **
	int rv=Want2Start(
		/*in_pollid=*/  fd_dir<0 ? ((FDCopyIO_FD*)src)->pollid :  NULL,
		/*out_pollid=*/ fd_dir>0 ? ((FDCopyIO_FD*)dest)->pollid : NULL );
	if(rv)  // We may not start. 
	{  return(-3);  }
	
	// Okay, then let's do it: 
	if(req_timeout>=0)
	{
		#warning missing: start timeout
		// on failure: goto ret_error;
	}
	
	if(fd_dir<0)
	{
		FDCopyIO_FD *s = (FDCopyIO_FD*)src;
		// ISetControlledEvents also checks for pollid==NULL: 
		if(ISetControlledEvents(s->pollid,POLLIN))  goto ret_error;
		IChangeEvents(s->pollid,POLLIN,0);
	}
	else  // fd_dir>0
	{
		FDCopyIO_FD *d = (FDCopyIO_FD*)dest;
		// ISetControlledEvents also checks for pollid==NULL: 
		if(ISetControlledEvents(d->pollid,POLLOUT))  goto ret_error;
		IChangeEvents(d->pollid,POLLOUT,0);
	}
	
	// Mark us and the FDCopIO classes active: 
	(int)state|=PS_Active;
	is_dead=0;  // be sure...
	src->active=1;
	dest->active=1;
	
	return(0);
	
ret_error:
	_Cleanup(/*go_dead=*/0);
	return(-1);
}


void FDCopyPump_Simple::_Cleanup(int go_dead)
{
	FDBase::PollID in_pollid= 
		(fd_dir<0 && src) ?  ((FDCopyIO_FD*)src)->pollid :  NULL;
	FDBase::PollID out_pollid=
		(fd_dir>0 && dest) ? ((FDCopyIO_FD*)dest)->pollid : NULL;
	
	// This has to be done BFORE informing the client: 
	// FIRST; Reset the events. 
	if(in_pollid)   IChangeEvents(in_pollid,0,IGetControlledEvents(in_pollid));
	if(out_pollid)  IChangeEvents(out_pollid,0,IGetControlledEvents(out_pollid));
	
	// Detach from the FDDataHook: 
	// (Also removes controlled events.) 
	int rv=DoneJob(in_pollid,out_pollid);  // -> FDCopyBase
	#if TESTING
	if(rv)
	{  fprintf(stderr,"Hm... DoneJob() failed (%d) (simple pump; fd_dir=%d)\n",
		rv,fd_dir);  }
	#endif
	
	// Then, kill the timeouts: 
	//#### kill timeouts
	
	// De-activate:
	state=PS_Inactive;
	if(src)   src->active=0;
	if(dest)  dest->active=0;
	// src/dest will be deleted in _DoSuicide() or destructor. 
	
	// NOTE: 
	// DO NOT GENERALLY SET fd_dir=0 and is_dead=1;
	// (becuase we're also called from _StartJob())
	if(go_dead)
	{
		fd_dir=0;
		is_dead=1;  // <- We will get deleted [if !persistent]
	}
}

// Automagically sets SCFinal in cpi.scode. 
// Always returns -1 if SCError is set, otherwise +1. 
int FDCopyPump_Simple::_EndJob(CopyInfo cpi)
{
	_Cleanup(/*go_dead=*/1);  // Hope setting go_dead=1 here is okay. - [YES!]
	
	int rv=(cpi.scode & SCError) ? (-1) : (+1);
	
	// Inform client: 
	(int)cpi.scode|=SCFinal;
	_CallCPNotify(&cpi);
	
	// We get deleted (unless persistent) as soon as we're 
	// back in fdnotify() / Control() (we're called on it's stack). 
	//is_dead already set. 
	//is_dead=1;  // <- We will get deleted [if !persistent]
	// See fdcopybase.cc for information on the on_stack_of_fdnotify 
	// issue. 
	#if TESTING
	if(on_stack_of_fdnotify==1)  assert(!is_dead);
	else assert(is_dead);
	#endif
	
	return(rv);
}


int FDCopyPump_Simple::_StopContJob(int stop)
{
	if(stop ? (state & PS_Stopped) : !(state & PS_Stopped))
	{  return(1);  }  // already stopped / already running
	
	FDManager::PollID pollid = (fd_dir<0) ? 
		((FDCopyIO_FD*)src)->pollid : ((FDCopyIO_FD*)dest)->pollid;
	if(stop)
	{
		IChangeEvents(pollid,0,(fd_dir<0 ? POLLIN : POLLOUT));
		(int)state|=PS_Stopped;
	}
	else
	{
		IChangeEvents(pollid,(fd_dir<0 ? POLLIN : POLLOUT),0);
		(int)state&=~PS_Stopped;
	}
	
	return(0);
}


// UNLESS 0 is returned, the source and dest stored in *this 
// were not modified. 
int FDCopyPump_Simple::SetIO(FDCopyIO *nsrc,FDCopyIO *ndest)
{
	int _rv=_BasicSetIOLogic(nsrc,ndest);
	if(_rv)  return(_rv);
	
	// See if we support that: 
	int nfd_dir=0;
	int retval=-2;
	if(nsrc && ndest)
	{
		// First, find the fd data io: 
		int cnt=0;
		if(nsrc->Type()==FDCopyIO::CPT_FD)   {  nfd_dir=-1;  ++cnt;  }
		if(ndest->Type()==FDCopyIO::CPT_FD)  {  nfd_dir=+1;  ++cnt;  }
		if(cnt!=1)  goto delret;
		
		// Okay, we have it. The other one then has to be 
		// the one working on a buffer, 
		FDCopyIO *other = (nfd_dir>0) ? nsrc : ndest;
		if(other->dataptr(NULL)!=0)
		{  retval=-20;  goto delret;  }
		
		FDCopyIO_FD *fdio = (FDCopyIO_FD*)((nfd_dir<0) ? nsrc : ndest);
		if(!fdio->pollid)
		{  retval=-21;  goto delret;  }
	}
	else if(nsrc || ndest)
	{  goto delret;  }
	
	// Check the old ones: 
	if((src  && src->active) ||
	   (dest && dest->active) )
	{  retval=-4;  goto delret;  }
	
	// Okay, then get rid of the old ones: 
	if(src)   {  src->DoSuicide();   src=NULL;   }
	if(dest)  {  dest->DoSuicide();  dest=NULL;  }
	
	// Do !!NOT!! set is_dead=0 here. 
	// We may be here in case is_dead=1 and nsrc=ndest=NULL. 
	src=nsrc;
	dest=ndest;
	fd_dir=nfd_dir;
	return(0);
	
delret:
	if(nsrc)   nsrc->DoSuicide();
	if(ndest)  ndest->DoSuicide();
	return(retval);
}


FDCopyPump_Simple::FDCopyPump_Simple(FDCopyBase *_fcb,int *failflag) : 
	FDCopyPump(_fcb,failflag)
{
	fd_dir=0;
}


FDCopyPump_Simple::~FDCopyPump_Simple()
{
	// src/dest cleanup done by FDCopyPump::~FDCopyPump()
	if(!is_dead)
	{  _Cleanup(/*go_dead=*/1);  }
}
