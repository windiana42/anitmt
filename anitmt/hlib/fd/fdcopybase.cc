/*
 * fdcopybase.cc
 * 
 * Implementation of class FDCopyBase and other minor classes which 
 * are always needed if FDCopyBase is used. 
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

// TESTING_CHECK is does more internal control tests which will 
// probably slow down things quite much. Only enable for debug 
// reasons. 
#define TESTING_CHECK 1


#if TESTING
#  warning TESTING switched on (using assert())
#  include <assert.h>
#else
#  define assert(x)  do{}while(0)
#  undef TESTING_CHECK
#  define TESTING_CHECK 0
#endif


/******************************************************************************/
/**** FDCopyBase                                                           ****/


int FDCopyBase::fdnotify(FDManager::FDInfo *fdi)
{
	FDDataHook *h=(FDDataHook*)(fdi->dptr);
	assert(h);   // It MUST be set. 
	assert(!h->gets_deleted);
	
	#if TESTING
	if((h->in.ctrl_ev && !h->in.pump) || 
	   (h->out.ctrl_ev && !h->out.pump) )
	{
		fprintf(stderr,"OOPS: BUG! ctrl_ev=0x%x,0x%x; pump=%p,%p (fdcpbase:%d)\n",
			h->in.ctrl_ev,h->out.ctrl_ev,h->in.pump,h->out.pump,__LINE__);
		abort();
	}
	#endif
	
	short remove_flags = (h->in.ctrl_ev | h->out.ctrl_ev);
	
	if(h->in.pump)
	{  _DoHandleFDNotify(fdi,h,h->in.pump);  }
	if(h->out.pump)
	{  _DoHandleFDNotify(fdi,h,h->out.pump);  }
	
	// Forward the call: 
	remove_flags=~remove_flags;
	fdi->revents&=remove_flags;
	// If events rest...
	// (NOTE: I do not want to enter here for POLLWRNORM or such jokes.) 
	// NOTE: POLLERR and POLLHUP are NOT passed if neither 
	//       POLLIN or POLLOUT are still in the events. 
	if((fdi->revents & (POLLIN | POLLOUT | POLLNVAL) ))
	{
		fdi->events&=remove_flags;
		fdi->dptr=h->orig_dptr;
		int rv=fdnotify2(fdi);
		return(rv);
	}
	return(0);
}


// NOTE: pump = h->in_pump or h->out_pump; pump may NOT be NULL. 
void FDCopyBase::_DoHandleFDNotify(FDManager::FDInfo *fdi,
	FDDataHook * /*h*/,FDCopyPump *pump)
{
	if(pump->is_dead)
	{
		#if TESTING
		fprintf(stderr,"Check that: _DoHandleFDNotify(fd=%d) on dead pump.\n",
			fdi->fd);
		#endif
		return;
	}
	
	// Internal error if failing. 
	// Only actine pumps may ne attached to FDDataHook. 
	assert(pump->IsActive());
	
	int rv=pump->HandleFDNotify(fdi);
	if(rv)
	{
		assert(pump->is_dead);
		// Okay, then delete it: 
		pump->_DoSuicide();  // will just "reset" if persistent
	}
}


// Return value: 0 -> OK ("yes"); else error
int FDCopyBase::_Can_AddPump2DataHook(FDCopyPump *pump,PollID pollid,int dir)
{
	FDDataHook *h=(FDDataHook*)FDBase::_FDDPtrNN(pollid);
	assert(h);   // It MUST be set. 
	
	if(h->gets_deleted)  return(1);  // "no"
	
	FDDataHook::HIO *hio = dir<0 ? &(h->in) : &(h->out);
	if(!hio->pump || hio->pump==pump)  return(0);  // "yes"
	#if TESTING
	// This probably means that there is a but in your application. 
	fprintf(stderr,"OOPS: Adding pump (fd=%d, dir=%d) to hook(%p,%p): "
		"other pump (state=0x%x) already egistered.\n",
		FDBase::FDfd(pollid),dir,h->in.pump,h->out.pump,hio->pump->state);
	if(!hio->pump && hio->ctrl_ev!=0)
	{  fprintf(stderr,"OOPS: BUG! fd=%d: events 0x%x set (dir=%d) but "
		"pump=NULL\n",FDBase::FDfd(pollid),hio->ctrl_ev,dir);  abort();  }
	#endif
	return(1);  // "no"
}

// NOTE: ASSUMES THAT _Can_AddPump2DataHook() WAS CALLED AND RETURNED 0. 
void FDCopyBase::_Do_AddPump2DataHook(FDCopyPump *pump,PollID pollid,int dir)
{
	FDDataHook *h=(FDDataHook*)FDBase::_FDDPtrNN(pollid);
	
	FDDataHook::HIO *hio = dir<0 ? &(h->in) : &(h->out);
	hio->pump = pump;
}


int FDCopyBase::Want2Start(FDCopyPump *pump,PollID in_id,PollID out_id)
{
	if(!pump)  return(0);
	
	if(pump->is_dead)
	{
		#if TESTING
		fprintf(stderr,"OOPS: Want2Start(%p,fd=%d,%d) for dead pump\n",
			pump,FDBase::FDfd(in_id),FDBase::FDfd(out_id));
		#endif
		return(-2);
	}
	
	if(!in_id && !out_id)  return(0);  // nothing to do
	
	int error=0;
	if(in_id)   error|=_Can_AddPump2DataHook(pump,in_id,-1);
	if(out_id)  error|=_Can_AddPump2DataHook(pump,out_id,+1);
	if(error)
	{  return(-1);  }
	
	// Okay, actually do it: 
	if(in_id)   _Do_AddPump2DataHook(pump,in_id,-1);
	if(out_id)  _Do_AddPump2DataHook(pump,out_id,+1);
	
	return(0);
}


// Returns 0 or 1 (error)
int FDCopyBase::_DelPumpFromDataHook(FDCopyPump *pump,PollID pollid,int dir)
{
	FDDataHook *h=(FDDataHook*)FDBase::_FDDPtrNN(pollid);
	// This is of course allowed if h->gets_deleted is set. 
	
	FDDataHook::HIO *hio = dir<0 ? &(h->in) : &(h->out);
	if(hio->pump==pump)
	{
		hio->pump=NULL;
		hio->ctrl_ev=0;
		return(0);
	}
	if(hio->pump)  return(1);
	return(0);
}

int FDCopyBase::DoneJob(FDCopyPump *pump,PollID in_id,PollID out_id)
{
	if(!pump)  return(0);
	if(!in_id && !out_id)  return(0);  // nothing to do
	
	int error=0;
	if(in_id)   error|=_DelPumpFromDataHook(pump,in_id,-1);
	if(out_id)  error|=_DelPumpFromDataHook(pump,out_id,+1);
	
	return(error ? -1 : 0);
}


int FDCopyBase::PollFD(int fd,short events,const void *dptr,PollID *ret_id)
{
	if(ret_id)
	{  *ret_id=NULL;  }
	
	// First, see if the poll node already exists: 
	PollID pollid = FDBase::FDPollID(fd);
	if(pollid)
	{
		// In this case, this is illegal. The user has to 
		// call FDChangeEvents(). 
		#if TESTING
		fprintf(stderr,"OOPS: BUG in application: "
			"Use FDChangeEvents() !! (fdcopybase.cc:%d)\n",
			__LINE__);
		#endif
		return(-1);
	}
	
	// Okay, a new poll node shall be allocated. 
	int rv=FDBase::PollFD(fd,events,dptr,&pollid);
	assert(rv!=1);    // 1 -> entry updated
	if(rv)  return(rv);
	
	if(_AttachToFD(pollid))
	{  return(-1);  }
	
	if(ret_id)
	{  *ret_id=pollid;  }
	return(0);
}


int FDCopyBase::FDChangeEvents(PollID pollid,short set_ev,short clear_ev)
{
	if(!pollid)  return(-2);
	FDDataHook *h=(FDDataHook*)FDBase::_FDDPtrNN(pollid);
	assert(h);   // It MUST be set. 
	
	// Okay, this is the function to the user level, so only allow 
	// events to be changed which are not controlled by us. 
	short controlled = (h->in.ctrl_ev | h->out.ctrl_ev);
	
	#if TESTING
	short o_set_ev=set_ev,o_clear_ev=clear_ev;
	#endif
	
	int changed=0;
	if((set_ev & controlled))    {  set_ev&=~controlled;    ++changed;  }
	if((clear_ev & controlled))  {  clear_ev&=~controlled;  ++changed;  }
	#if TESTING
	if(changed)
	{  fprintf(stderr,"NOTE: FDChangeEvents(fd=%d,set=0x%x,clear=0x%x): "
		"conflict 0x%x,0x%x with controlled events (0x%x)\n",
		FDBase::FDfd(pollid),o_set_ev,o_clear_ev,
		o_set_ev & controlled,o_clear_ev & controlled,controlled);  }
	#endif
	int rv=FDBase::FDChangeEvents(pollid,set_ev,clear_ev);
	return(changed ? 2 : rv);
}

short FDCopyBase::FDEvents(PollID pollid)
{
	if(!pollid)  return(-1);
	FDDataHook *h=(FDDataHook*)FDBase::_FDDPtrNN(pollid);
	assert(h);   // It MUST be set. 
	
	// Events as present at FDManager: 
	short events = FDBase::_FDEventsNN(pollid);
	
	return(events & ~(h->in.ctrl_ev | h->out.ctrl_ev));
}


// Must be called for every newly allocated poll node to 
// attach the FDDataHook to it. 
// Return value: 
//   0 -> OK
//  -1 -> alloc failure
int FDCopyBase::_AttachToFD(PollID &pollid)
{
	if(!pollid)  return(-1);
	FDDataHook *h=NEW<FDDataHook>();
	if(!h)
	{
		FDBase::UnpollFD(pollid);
		return(-1);
	}
	h->orig_dptr=FDBase::_FDDPtrNN(pollid);
	FDBase::PollFDDPtr(pollid,h);
	return(0);
}


// This is what UnpollFD() and CloseFD(), etc. are doing. 
// THIS HAS TO BE CALLED BEFORE A POLLID DIES in order to 
// de-allocate the FDDataHook, kill associated jobs, etc. 
void FDCopyBase::_DetachFromFD(PollID pollid)
{
	FDDataHook *h=(FDDataHook*)FDBase::FDDPtr(pollid);
	// NOTE: h may be NULL in case 
	//  - we unpoll it because h alloc failed
	//  - h=NULL becuase pollid=NULL 
	if(!h)  return;
	
	assert(!h->gets_deleted);
	h->gets_deleted=1;
	
	if(h->in.pump || h->out.pump)
	{
		// Must instantly kill the jobs. 
		#if TESTING
		fprintf(stderr,"NOTE: Killing pumps (%p,%p; ctrl_ev=0x%x,0x%x) as "
			"PollID(fd=%d) gets deleted.\n",
			h->in.pump,h->out.pump,h->in.ctrl_ev,h->out.ctrl_ev,
			FDBase::FDfd(pollid));
		#endif
		h->in.pump->Control(FDCopyPump::CC_Kill);
		h->out.pump->Control(FDCopyPump::CC_Kill);
		// ...now cpnotify() and DoneJob() are called, then 
		// the pumps must be NULL: 
		// If this fails, then the used pump(s) most probably 
		// failed to react properly to CC_Kill. 
		assert(!h->in.pump && !h->out.pump);
		assert(!h->in.ctrl_ev && !h->out.ctrl_ev);
	}
	
	// Cancel dptr and delete FDDataHook: 
	FDBase::PollFDDPtr(pollid,NULL);
	delete h;
}


int FDCopyBase::CloseFD(int fd)
{
	PollID pollid=FDBase::FDPollID(fd);
	_DetachFromFD(pollid);
	return(FDBase::_CloseFD(fd,pollid));
}

int FDCopyBase::ShutdownFD(int fd)
{
	PollID pollid=FDBase::FDPollID(fd);
	_DetachFromFD(pollid);
	return(FDBase::_ShutdownFD(fd,pollid));
}


int FDCopyBase::IChangeEvents(FDCopyPump *pump,PollID pollid,
	short set_ev,short clear_ev)
{
	#if TESTING
	// See if the pump controls the events it is changing. 
	FDDataHook *h=(FDDataHook*)FDBase::FDDPtr(pollid);
	if(h)
	{
		static const char *_err_mgs=
			"OOPS: IChangeEvents(%p,fd=%d,set_ev=0x%x,clear_ev=0x%x): %s\n";
		const char *msg=NULL;
		do {
			short *evptr;
			if(h->in.pump==pump)  evptr=&h->in.ctrl_ev;
			else if(h->out.pump==pump)  evptr=&h->out.ctrl_ev;
			else
			{  msg="pump not registered at PollID";  break;  }
			if((set_ev &   (*evptr))!=set_ev || 
			   (clear_ev & (*evptr))!=clear_ev )
			{  msg="pump sets/clears events which are not "
				"marked as controlled";  break;  }
		} while(0);
		if(msg)
		{
			fprintf(stderr,_err_mgs,pump,FDBase::FDfd(pollid),set_ev,clear_ev,msg);
			fprintf(stderr,"     FDDataHook: pumps=%p,%p; ctrl_ev=0x%x,0x%x; del=%s\n",
				h->in.pump,h->out.pump,h->in.ctrl_ev,h->out.ctrl_ev,
				h->gets_deleted ? "*YES*" : "no");
		}
		if(h->in.pump==pump && h->out.pump==pump)
		{  assert(h->in.ctrl_ev==h->out.ctrl_ev);  }
	}
	#endif
	
	return(FDBase::FDChangeEvents(pollid,set_ev,clear_ev));
}


int FDCopyBase::ISetControlledEvents(FDCopyPump *pump,PollID pollid,short events)
{
	if(!pump)  return(0);
	if(!pollid)  return(-3);
	
	FDDataHook *h=(FDDataHook*)FDBase::FDDPtr(pollid);
	assert(h);
	
	int done=0;
	if(h->in.pump==pump)
	{
		h->in.ctrl_ev=events;
		++done;  // NOT =1
	}
	// NO ELSE
	if(h->out.pump==pump)
	{
		h->out.ctrl_ev=events;
		++done;  // NOT =1
	}
	
	if(!done)
	{
		#if TESTING
		fprintf(stderr,"OOPS: ISetCtrlEv(%p,fd=%d,0x%x): pump wants to set "
			"controlled events of PollID it is not registered at (%p,%p).\n",
			pump,FDBase::FDfd(pollid),events,h->in.pump,h->out.pump);
		#endif
		return(-2);
	}
	
	#if TESTING
	if(done==1)  // and NOT 2
	{
		short *evptr = (h->in.pump==pump) ? 
			&h->in.ctrl_ev : &h->out.ctrl_ev;
		short *other_evptr = (h->in.pump==pump) ? 
			&h->out.ctrl_ev : &h->in.ctrl_ev;
		if((events & (*other_evptr)))
		{
			fprintf(stderr,"NOTE: ISetCtrlEv(%p,fd=%d,0x%x): pump sets controlled "
				"events which are also controlled by other pump "
				"(current=0x%x; other=0x%x; &=ßx%x)\n",
				pump,FDBase::FDfd(pollid),events,*evptr,*other_evptr,
				(events & (*other_evptr)));
		}
	}
	#endif
	
	return(0);
}


short FDCopyBase::IGetControlledEvents(FDCopyPump *pump,PollID pollid)
{
	if(!pump || !pollid)  return(0);
	
	FDDataHook *h=(FDDataHook*)FDBase::FDDPtr(pollid);
	if(!h)  return(0);
	
	#if TESTING
	if(h->in.pump==pump && h->out.pump==pump)
	{  assert(h->in.ctrl_ev==h->out.ctrl_ev);  }
	#endif
	
	if(h->in.pump==pump)   return(h->in.ctrl_ev);
	if(h->out.pump==pump)  return(h->out.ctrl_ev);
	
	return(0);
}


// Internally used by NEW_CopyPump(): 
FDCopyPump *FDCopyBase::_NEW_CopyPump_DoRest(FDCopyPump *pump,
	FDCopyIO *src,FDCopyIO *dest)
{
	if(pump)
	{
		if(!pump->SetIO(src,dest))
		{  return(pump);  }
		delete pump;
	}
	if(src)  src->DoSuicide();
	if(dest)  dest->DoSuicide();
	return(NULL);
}


void FDCopyBase::RegisterPump(FDCopyPump *p)
{
	if(!p)  return;
	
	// If this assert fails, the pump is already queued. 
	assert(!p->next && cplist.last()!=p);
	
	cplist.append(p);
}

void FDCopyBase::UnregisterPump(FDCopyPump *p)
{
	if(!p)  return;
	
	if(p->next || cplist.last()==p)
	{  cplist.dequeue(p);  }
}


FDCopyBase::FDCopyBase(int *failflag) : 
	FDBase(failflag),
	cplist(failflag)
{
	
}

FDCopyBase::~FDCopyBase()
{
	// Delete all the pumps: 
	for(FDCopyPump *p;;)
	{
		p=cplist.popfirst();
		if(!p)  break;
		#if TESTING
		if(p->persistent)
		{  fprintf(stderr,"OOPS?!: ~FDCopyBase: deleting persistent "
			"FDCopyPump\n");  }
		#endif
		p->persistent=0;
		delete p;
	}
}


FDCopyBase::FDDataHook::FDDataHook(int * /*failflag*/)
{
	in.pump=NULL;
	out.pump=NULL;
	in.ctrl_ev=0;
	out.ctrl_ev=0;
	gets_deleted=0;
	orig_dptr=NULL;
}

FDCopyBase::FDDataHook::~FDDataHook()
{
	in.pump=NULL;
	out.pump=NULL;  // be sure...
}

// FDCopyPump and FDCopyIO are here, too, because anyone who uses 
// FDCopyBase also needs these objects. (linker issue in case you wonder)

/******************************************************************************/
/**** FDCopyPump                                                           ****/

// This does a "delete this;" if we're not persistent 
// Or resets the is_dead flag and the class if persistent. 
void FDCopyPump::_DoSuicide()
{
	assert(state==PS_Inactive);
	
	if(persistent)
	{
		is_dead=0;
		
		// Timeout already killed. 
		// Events already reset. 
		
		// Get rid of src and dest: 
		if(src)
		{  src->DoSuicide();  src=NULL;  }
		if(dest)
		{  dest->DoSuicide();  dest=NULL;  }
		return;
	}
	
	assert(is_dead);
	delete this;  // Will delete src/dest in destructor. 
}


int FDCopyPump::Control(FDCopyBase::ControlCommand cc)
{
	if(is_dead)
	{  return(-2);  }
	
	int rv=VControl(cc);
	
	// See if the FDCopyPump has to be deleted: 
	if(is_dead)
	{  _DoSuicide();  }
	
	return(rv);
}


FDCopyPump::FDCopyPump(FDCopyBase *_fcb,int *failflag) : 
	LinkedListBase<FDCopyPump>()
{
	int failed=0;
	
	fcb=_fcb;
	
	src=NULL;
	dest=NULL;
	
	state=PS_Inactive;
	
	req_timeout=-1;
	persistent=0;
	is_dead=0;
	dptr=NULL;
	
	if(fcb)
	{  fcb->RegisterPump(this);  }
	else
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit();  }
}


FDCopyPump::~FDCopyPump()
{
	fcb->UnregisterPump(this);
	
	#if TESTING
	if(state & PS_Active)
	{  fprintf(stderr,"Aiee! Deleting active FDCopyPump\n");  }
	#endif
	
	static const char *_warn_persist_fdcopyio=
		"OOPS? ~FDCopyPump: deleting persistent FDCopyIO (type=%d)\n";
	if(src)
	{
		#if TESTING
		if(src->persistent)
		{  fprintf(stderr,_warn_persist_fdcopyio,src->Type());  }
		#endif
		src->persistent=0;
		src->DoSuicide();  src=NULL;
	}
	if(dest)
	{
		#if TESTING
		if(dest->persistent)
		{  fprintf(stderr,_warn_persist_fdcopyio,dest->Type());  }
		#endif
		dest->persistent=0;
		dest->DoSuicide();  dest=NULL;
	}
	
	fcb=NULL;
	dptr=NULL;
}


/******************************************************************************/
/**** FDCopyIO                                                             ****/

int FDCopyIO::DoSuicide()
{
	if(!persistent)
	{  delete this;  return(1);  }
	reset();
	active=0;
	return(0);
}


FDCopyIO::FDCopyIO(CPType t,int * /*failflag*/) : 
	starttime(HTime::Invalid)
{
	type=t;
	
	io_timeout=-1;
	dptr=NULL;
	
	persistent=0;
	active=0;
}

// Is virtual; won't be inlined anyway. 
FDCopyIO::~FDCopyIO()
{
	#if TESTING
	if(active)
	{  fprintf(stderr,"OOPS? Deleting active FDCopyIO (type=%d)\n",type);  }
	#endif
}
