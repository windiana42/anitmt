#define HLIB_IN_HLIB 1
#include <hlib/fdcopybase.h>

#ifndef TESTING
#define TESTING 1
#endif

// 1 -> Print each time we change POLLIN/POLLOUT for fd->fd copying. 
//      Used to seee if the algorithm works fine (= lots of "[rw]"). 
#define PRINT_SWITCH_DUMP 0

#if TESTING
#  warning TESTING switched on (using assert())
#  include <assert.h>
#else
#  define assert(x)  do{}while(0)
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


int FDCopyPump_FD2FD::_ReadInData(int fd,FDCopyIO_FD *cpio,HTime *fdtime)
{
	assert(fd==fcb->FDfd(cpio->pollid));   // otherwise great internal error
	
	if(!fifo.BufFree())
	{
		#if TESTING
		fprintf(stderr,"OOPS?!?!: FD2FD pump: 0 bytes free in fifo\n");
		#endif
		// No need to act here; when re-deciding events we will 
		// stop polling for input. 
		return(0);   
	}
	
	size_t want_max=cpio->max_iolen;
	int reached_limit=0;
	if(limit && cpio->transferred+copylen_t(want_max)>=limit)  // >= REALLY!
	{
		assert(limit>=cpio->transferred);   // Else we may not be here to read data!!
		want_max=size_t(limit-cpio->transferred);
		reached_limit=1;  // That's why we need `>=' above. 
	}
	
	ssize_t rd=fifo.ReadFD(fd,want_max);
	if(rd<0)
	{
		if(errno==EINTR || errno==EWOULDBLOCK)
		{
			#if TESTING
			if(errno==EWOULDBLOCK)
			{  fprintf(stderr,"Oops: FD2FD pump: reading would block (%d)\n",fd);  }
			#endif
			// Okay, we'll try again later. 
			return(0);
		}
		return(_FinishJob(-1,CopyInfo(this,errno==EPIPE ? SCInPipe : SCErrRead,
			errno,fdtime)));
	}
	if(rd>0)
	{
		// Update statistics: 
		cpio->transferred+=(copylen_t)rd;
		
		if(!reached_limit || size_t(rd)<want_max)  // limit not reached
		{
			//#### send progress info
			return(0);
		}
		// If we're here -> (read) limit reached. 
	}
	// rd==0 or read limit reached. 
	return(_FinishJob(-1,CopyInfo(this,rd ? SCLimit : SCRead0,0,fdtime)));
}


int FDCopyPump_FD2FD::_WriteOutData(int fd,FDCopyIO_FD *cpio,HTime *fdtime)
{
	assert(fd==fcb->FDfd(cpio->pollid));   // otherwise great internal error
	
	if(!fifo.BufUse())
	{
		#if TESTING
		fprintf(stderr,"OOPS?!?!: FD2FD pump: 0 bytes available in fifo\n");
		#endif
		// No need to act here; when re-deciding events we will 
		// stop polling for output. 
		return(0);   
	}
	
	size_t want_max=cpio->max_iolen;
	
	ssize_t wr=fifo.WriteFD(fd,want_max);
	if(wr<0)
	{
		if(errno==EINTR || errno==EWOULDBLOCK)
		{
			#if TESTING
			if(errno==EWOULDBLOCK)
			{  fprintf(stderr,"Oops: FD2FD pump: writing would block (%d)\n",fd);  }
			#endif
			// Okay, we'll try again later. 
			return(0);
		}
		return(_FinishJob(+1,CopyInfo(this,errno==EPIPE ? SCOutPipe : SCErrWrite,
			errno,fdtime)));
	}
	if(wr>0)
	{
		// Update statistics: 
		cpio->transferred+=(copylen_t)wr;
		
		if(!flushing || fifo.BufUse())  // limit not reached
		{
			// Surely not the last write. 
			//#### send progress info
			return(0);
		}
		// If we're here -> writing done.
	}
	// wr==0 or everything written. 
	#if TESTING
	if(wr==0)  // Hmmm... write returns 0. 
	{  fprintf(stderr,"OOPS: FD2FD pump: wrote 0/%u bytes to fd %d\n",
		want_max,fd);  }
	#endif
	return(_FinishJob(+1,CopyInfo(this,wr ? SCLimit : SCWrite0,0,fdtime)));
}


int FDCopyPump_FD2FD::HandleFDNotify(FDManager::FDInfo *fdi)
{
	assert(!is_dead);
	assert(IsActive());   // Otherwise we may not be here. 
	
	// Check which FDCopyIO it is: 
	FDCopyIO_FD *cpio=NULL;
	short our_ev=0;
	int dir=0;
	if(fdi->pollid==((FDCopyIO_FD*)src)->pollid)
	{  cpio=(FDCopyIO_FD*)src;  dir=-1;  our_ev=POLLIN;  }
	if(fdi->pollid==((FDCopyIO_FD*)dest)->pollid)
	{  cpio=(FDCopyIO_FD*)dest;  dir=+1;  our_ev=POLLOUT;  }
	// If this assert fails, then there is some intrnal error. 
	// We are getting fdnotify() for a PolLFD which we do not know. 
	// Happy bug hunting!!
	assert(cpio);
	if(!cpio)  return(0);  // Wwill never kick in becasue of assert above. 
	
	if(IsStopped(dir))
	{
		if((fdi->revents & our_ev))
		{
			#if TESTING
			fprintf(stderr,"HMMM: FDCopyPump_Simple stopped(%d) but got "
				"fdnotify 0x%x (curr_r/w=%s)\n",dir,fdi->revents,
				(dir<0 ? curr_reading : curr_writing) ? "YES" : "no");
			#endif
			// Make sure that we really stopped...
			IChangeEvents(fdi->pollid,0,our_ev);
			if(dir<0)  curr_reading=0;
			else  curr_writing=0;
		}
		return(0);
	}
	
	// Okay, let's see what we can do...
	if((fdi->revents & our_ev))
	{
		int rv = (dir<0) ? 
			_ReadInData(fdi->fd,cpio,fdi->current) : 
			_WriteOutData(fdi->fd,cpio,fdi->current);
		if(rv)
		{
			// We will no longer be alive. 
			assert(is_dead);
			return(0);
		}
		_ReDecidePollEvents();
	}
	
	// If we're here, the job is still active. Check for errors, etc. 
	if((fdi->revents & (POLLERR | POLLNVAL | POLLHUP) ))
	{
		StatusCode scode;
		int err_no=0;
		if(fdi->revents & POLLHUP)
		{  scode = (dir<0) ? SCInHup : SCOutHup;  }
		else if(fdi->revents & (POLLERR | POLLNVAL))
		{
			scode = (dir<0) ? SCErrPollI : SCErrPollO;
			err_no=fdi->revents;
		}
		return(_FinishJob(dir,CopyInfo(this,scode,err_no,fdi->current)));
	}
	
	return(0);
}


void FDCopyPump_FD2FD::_ReDecidePollEvents()
{
	assert(IsActive() && !is_dead);
	
	#if PRINT_SWITCH_DUMP
	short old_reading=curr_reading,
	      old_writing=curr_writing;
	#endif
	
	/*---- INPUT ----*/
	if(curr_reading)
	{
		// Stop if high thresh reached or if flushing or input stopped: 
		if(fifo.BufUse()>=size_t(high_read_thresh) || 
		   flushing || 
		   (state & PS_StoppedIn) )
		{
			IChangeEvents(((FDCopyIO_FD *)src)->pollid,0,POLLIN);
			curr_reading=0;
		}
	}
	else 
	{
		// Start if low thresh reached and not stopped and not flushing: 
		if(!flushing && 
		   fifo.BufUse()<=size_t(low_read_thresh) && 
		   !(state & PS_StoppedIn))
		{
			IChangeEvents(((FDCopyIO_FD *)src)->pollid,POLLIN,0);
			curr_reading=1;
		}
	}
	
	/*---- OUTPUT ----*/
	if(!curr_writing)
	{
		// Start if high thresh reached or if flushing and not stopped. 
		if((fifo.BufUse()>=size_t(high_write_thresh) || flushing) && 
		   !(state & PS_StoppedOut) )
		{
			IChangeEvents(((FDCopyIO_FD *)dest)->pollid,POLLOUT,0);
			curr_writing=1;
		}
	}
	else
	{
		// Stop if low thresh reached and not flusing or stopped: 
		if((fifo.BufUse()<=size_t(low_write_thresh) && !flushing) || 
		   (state & PS_StoppedOut) )
		{
			IChangeEvents(((FDCopyIO_FD *)dest)->pollid,0,POLLOUT);
			curr_writing=0;
		}
	}
	
	#if TESTING
	if(curr_writing)  assert(!(state & PS_StoppedOut));
	if((state & PS_StoppedOut))  assert(!curr_writing);
	if(curr_reading)  assert(!(state & PS_StoppedIn));
	if((state & PS_StoppedIn))  assert(!curr_reading);
	#endif
	
	#if PRINT_SWITCH_DUMP
	if(old_reading!=curr_reading || old_writing!=curr_writing)
	{
		fprintf(stderr,"IO SWITCH: [%c%c] (bufuse=%u)\n",
			curr_reading ? 'r' : '-',
			curr_writing ? 'w' : '-',
			fifo.BufUse());
	}
	#endif
}


// NOTE: FDCopyPump::Control() may delete us (doing delete *this). 
int FDCopyPump_FD2FD::VControl(ControlCommand cc)
{
	if(is_dead || (!IsActive() && cc!=CC_Start) )
	{  return(-2);  }  // Must be active for this command
	
	switch(cc)
	{
		case CC_Start:  return(_StartJob());
		case CC_Term:
			_FinishJob(-1,CopyInfo(this,SCTerm,0,NULL));
			// We may get deleted by FDCopyBase::Control(). 
			return(0);
		case CC_Kill:
			_FinishJob(+1,CopyInfo(this,SCKilled,0,NULL));
			// We get deleted by FDCopyBase::Control(). 
			return(0);
		case CC_Stop:
		case CC_StopI:
		case CC_StopO:  return(_StopContJob(cc,1));
		case CC_Cont:
		case CC_ContI:
		case CC_ContO:  return(_StopContJob(cc,0));
		//default: fall through to function body end
	}
	
	return(-2);
}


int FDCopyPump_FD2FD::_StartJob()
{
	if(IsActive())
	{  return(1);  }  // already started
	
	FDCopyIO_FD *src=(FDCopyIO_FD *)(this->src);
	FDCopyIO_FD *dest=(FDCopyIO_FD *)(this->dest);
	
	// See if we CAN start: 
	if(!src || !dest || !src->pollid || !dest->pollid)
	{  return(-1);  }
	
	assert(!src->active && !dest->active);
	
	// Okay, do the necessary setup and checks. 
	int rv=_StartSetup();
	if(rv)                        // <<<"Point of no return">>>
	{  return(rv);  }
	
	// Tell FDCopyBase that we want start. 
	// It's not actually just that we _want_; FDCopyBase also 
	// treats us as running now. 
	// ** So, if sth fails below, DoneJob() must be called. **
	rv=Want2Start(src->pollid,dest->pollid);
	if(rv)  // We may not start. 
	{  return(-3);  }
	
	// Okay, then let's do it: 
	if(req_timeout>=0)
	{
		#warning missing: start timeout
		// on failure: goto ret_error;
	}
	
	if(ISetControlledEvents(src->pollid,POLLIN))  goto ret_error;
	if(ISetControlledEvents(dest->pollid,POLLOUT))  goto ret_error;
	
	// Actually start...
	IChangeEvents(src->pollid,POLLIN,0);    // start input 
	curr_reading=1;
	IChangeEvents(dest->pollid,0,POLLOUT);  // and stop output in case it is active
	curr_writing=0;
	
	// Mark us and the FDCopIO classes active: 
	(int)state|=PS_Active;
	is_dead=0;  // be sure...
	flushing=0;
	persistent_sc=FDCopyPump::SCNone;
	src->active=1;
	dest->active=1;
	
	return(0);
	
ret_error:
	_Cleanup(/*go_dead=*/0);
	return(-1);
}


int FDCopyPump_FD2FD::_StopContJob(ControlCommand cc,int do_stop)
{
	PumpState flag=PS_Inactive;
	switch(cc)
	{
		case CC_Stop:
		case CC_Cont:   flag=PS_Stopped;  break;
		case CC_StopI:
		case CC_ContI:  flag=PS_StoppedIn;  break;
		case CC_StopO:
		case CC_ContO:  flag=PS_StoppedOut;  break;
		default: assert(0);  // Internal error (illegal command passed).
	}
	
	// Be careful: One might call CC_Stop on a CC_StopI'd job. 
	int already=((state&flag)==flag);
	if((already && do_stop) || (!already && !do_stop))
	{  return(1);  }  // already the case
	
	if(do_stop)
	{  (int)state|=flag;  }
	else
	{  (int)state&=~flag;  }
	
	// This will do all the events stuff for us: 
	_ReDecidePollEvents();
	
	return(0);
}


int FDCopyPump_FD2FD::_StartSetup()
{
	// Be sure that the limit is sane: 
	if(limit<0)
	{  return(-5);  }
	
	// Force minimum for iobufsize (may only be smaller than that if 
	// the limit is smaller. 
	if(iobufsize<32)
	{  iobufsize=32;  }
	
	if(limit<copylen_t(iobufsize))
	{  iobufsize=(size_t)limit;  }
	
	// Okay, the limit and iobufsize are set. 
	// Now check the thresholds: 
	// -------------------------------------------------------------------------
	// [Illustratuion see fdcopybase.h]
	// 
	// So, make sure: 
	//   low_read_thresh  > low_write_thresh
	//   high_read_thresh > high_write_thresh
	//   low_read_thresh << high_write_thresh
	// AND NOT THE OTHER WAY ROUND (because then, nothing will work).
	//
	// probably best way: 
	// In case max_read_len / max_write_len is SMALL but not 0 (0 = unlimited)
	//      low_write_thresh=max_write_len-1
	//      high_read_thresh=buflen-max_read_len+1
	//     (Think of these 2 values as "min read/write length".)
	// -------------------------------------------------------------------------
	size_t iobs=iobufsize;
	if(iobufsize<32)
	{
		// The calculations below will not work for too small buffers. 
		// As we use such small buffers if the length limit is such 
		// small, this case has to be dealt with. We simply assume 
		// that IO will be atomic: 
		// [Check: this also works for iobufsize=1: YES, DONE.]
		low_write_thresh=0;
		low_read_thresh=iobs/8;
		high_write_thresh=iobs-iobs/8;
		high_read_thresh=iobs;
	}
	else
	{
		// See if the user wants defaults to be set. 
		// Currently you may EITHER specify ALL values yourself 
		// OR use defaults for ALL 4 values (by passing -1). 
		// Also note the checks below which will not pass if one 
		// of the values is <0. 
		if(low_read_thresh<0 && low_write_thresh<0 && 
		   high_read_thresh<0 && high_write_thresh<0 )
		{
			// Okay, use defaults: 
			// Of course, any amount of time could be spent on 
			// tweaking these for various circumstances...
			high_read_thresh= iobs-iobs/8;
			high_write_thresh=iobs-iobs/4;
			low_read_thresh=  iobs/4;
			low_write_thresh= iobs/8;
		}
		
		// This one is important or the algorithms won't work...
		// (The difference, however, should be much larger than 4.)
		if(low_read_thresh+4 >= high_write_thresh)
		{  return(-5);  }
	}
	
	// NOTE: These comparations are CORRECT. ALL. 
	if(low_read_thresh>=high_read_thresh || 
	   low_write_thresh>=high_write_thresh || 
	   low_read_thresh<0 || 
	   low_write_thresh<0 || 
	   high_read_thresh>iobs ||
	   high_write_thresh>iobs )
	{  return(-5);  }
	
	// Okay, seems that the thresholds are also okay now. 
	
	fifo.Clear();  // be sure...
	int rv=fifo.ResizeBuf(iobufsize);
	if(rv)
	{
		fifo.ResizeBuf(0);
		return(-4);
	}
	
	return(0);
}


// Automagically sets SCFinal in cpi.scode if needed. 
// dir: -1 -> input; +1 -> output
// Returns 
//   -1 -> SCError is set and job finished
//   +1 -> job finished "normally"
//    0 -> job not finished (now flushing)
int FDCopyPump_FD2FD::_FinishJob(int dir,CopyInfo cpi)
{
	// See what happened: 
	if(dir<0 && fifo.BufUse())
	{
		// Okay, only stop input but go on flushing the output: 
		flushing=1;
		
		// This means, we MUST NOT control the input PollID any longer. 
		// (Do this BEFORE notifying the client.) 
		FDCopyIO_FD *src=(FDCopyIO_FD *)(this->src);
		if(curr_reading)
		{
			IChangeEvents(src->pollid,0,POLLIN);
			curr_reading=0;
		}
		int rv=DoneJob(src->pollid,/*dest_id=*/NULL);  // -> FDCopyBase
		#if TESTING
		if(rv)
		{
			FDCopyIO_FD *dest=(FDCopyIO_FD *)(this->dest);
			fprintf(stderr,"Hm... DoneJob() for flushing failed (%d) "
				"(FD2FD pump; fd=%d,%d)\n",
				rv,fcb->FDfd(src ? src->pollid : NULL),
				fcb->FDfd(dest ? dest->pollid : NULL));
		}
		#endif
		
		(int)persistent_sc|=cpi.scode;
		cpi.scode=persistent_sc;
		assert(!(cpi.scode & SCFinal));
		
		_CallCPNotify(&cpi);
		return(0);
	}
	
	// Okay, if we are here, then we must completely end the job, 
	// not just end input. 
	_Cleanup(/*go_dead=*/1);
	
	(int)cpi.scode|=SCFinal;
	(int)cpi.scode|=persistent_sc;
	_CallCPNotify(&cpi);
	
	// We get deleted (unless persistent) as soon as we're 
	// back in fdnotify() / Control() (we're called on it's stack). 
	//is_dead already set. 
	assert(is_dead);
	
	return((cpi.scode & SCError) ? (-1) : (+1));
}


void FDCopyPump_FD2FD::_Cleanup(int go_dead)
{
	FDCopyIO_FD *src=(FDCopyIO_FD *)(this->src);
	FDCopyIO_FD *dest=(FDCopyIO_FD *)(this->dest);
	
	// This has to be done BFORE informing the client: 
	// FIRST; Reset the events. 
	if(src)   IChangeEvents(src->pollid,0,IGetControlledEvents(src->pollid));
	if(dest)  IChangeEvents(dest->pollid,0,IGetControlledEvents(dest->pollid));
	curr_reading=0;
	curr_writing=0;
	
	// Detach from the FDDataHook: 
	// (Also removes controlled events.) 
	int rv=DoneJob(src->pollid,dest->pollid);  // -> FDCopyBase
	#if TESTING
	if(rv)
	{  fprintf(stderr,"Hm... DoneJob() failed (%d) (FD2FD pump; fd=%d,%d)\n",
		rv,fcb->FDfd(src ? src->pollid : NULL),
		fcb->FDfd(dest ? dest->pollid : NULL));  }
	#endif
	
	// Then, kill the timeouts: 
	//#### kill timeouts
	
	// De-activate:
	state=PS_Inactive;
	if(src)   src->active=0;
	if(dest)  dest->active=0;
	// src/dest will be deleted in _DoSuicide() or destructor. 
	
	// NOTE: 
	// DO NOT GENERALLY SET is_dead=1;
	// (becuase we're also called from _StartJob())
	if(go_dead)
	{
		is_dead=1;  // <- We will get deleted [if !persistent]
	}
}


// UNLESS 0 is returned, the source and dest stored in *this 
// were not modified. 
int FDCopyPump_FD2FD::SetIO(FDCopyIO *nsrc,FDCopyIO *ndest)
{
	// Are we active? If so, we refuse: 
	if(IsActive() || is_dead)
	{  return(-5);  }
	
	// First, check if the passed FDCopyIO's are active: 
	if((nsrc  && nsrc->active) ||
	   (ndest && ndest->active) )
	{  return(-3);  }
	
	// See if we support that: 
	int retval=-2;
	if(nsrc && ndest)
	{
		if(nsrc->Type()!=FDCopyIO::CPT_FD || 
		   ndest->Type()==FDCopyIO::CPT_FD )
		{  goto delret;  }
		
		if(!((FDCopyIO_FD*)nsrc)->pollid ||
		   !((FDCopyIO_FD*)ndest)->pollid )
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
	
	src=nsrc;
	dest=ndest;
	flushing=0;
	curr_reading=0;
	curr_writing=0;
	persistent_sc=FDCopyPump::SCNone;
	return(0);
	
delret:
	if(nsrc)  nsrc->DoSuicide();
	if(ndest)  ndest->DoSuicide();
	return(retval);
}


FDCopyPump_FD2FD::FDCopyPump_FD2FD(FDCopyBase *_fcb,int *failflag) : 
	FDCopyPump(_fcb,failflag),
	fifo(/*size=*/0,failflag)
{
	limit=(copylen_t)0;
	
	iobufsize=16384;
	
	flushing=0;
	curr_reading=0;
	curr_writing=0;
	persistent_sc=FDCopyPump::SCNone;
	
	low_read_thresh=-1;
	high_read_thresh=-1;
	low_write_thresh=-1;
	high_write_thresh=-1;
}

FDCopyPump_FD2FD::~FDCopyPump_FD2FD()
{
	
}
