/*
 * adminconn.cpp
 * 
 * Implementation of admin connection class (RendView side). 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "adminport.hpp"

#include <lib/netiobase.hpp>

#include <assert.h>

using namespace RVAP;


inline void RendViewAdminConnection::schedule_outpump_start()
{
	if(!outpump_scheduled && !exec_cmd_scheduled)
	{  UpdateTimer(schedule_tid,0,0);  }
	outpump_scheduled=1;
}

inline void RendViewAdminConnection::schedule_exec_cmd()
{
	if(!exec_cmd_scheduled && !outpump_scheduled)
	{  UpdateTimer(schedule_tid,0,0);  }
	exec_cmd_scheduled=1;
}


inline void RendViewAdminConnection::_ResetIdleTimeout(const HTime * /*curr*/)
{
	if(AP()->idle_timeout>=0)
	{  ResetTimer(idle_tid,FDAT_FirstLater | FDAT_AlignToShorter | 10);  }
}


void RendViewAdminConnection::_DoChangeEvents_Error(int rv)
{
	fprintf(stderr,"OOPS(admin): FDChangeEvents(sock=%d,%p) returned %d "
		"[abort?]\n",sock_fd,pollid,rv);
}


void RendViewAdminConnection::ExecuteCommand(RefString &command)
{
	// Okay, how it works: 
	// The command is stored in the RefString (scheduled_cmd) as long 
	// as it is active, i.e. until the response is sent. 
	// We arrive here after one schedule timer. The command is 
	// now passed to AP() which takes care to exectue it and store 
	// the result in the passed buffer. We then start the outpump to 
	// send the result and if that is done, call scheduled_cmd.deref(). 
	// Now, we are ready for the next command. 
	// Note that the exection of a command must be fast. I.e. you 
	// may not have to wait for FDs or timers or even wait for a 
	// re-sched during cmd execution because the complete exec happens 
	// on the stack here. 
	{
		RVAPHeader resp;
		resp.length=0;  // unknown now;
		resp.ptype=RVAP_CommandResp;
		cmd_resp_buf.set((char*)&resp,sizeof(resp));  // cannot fail
	}
	
	AP()->ExecuteCommand(command,&cmd_resp_buf);
	
	// Update length: 
	RVAPHeader *resp=(RVAPHeader*)cmd_resp_buf.str();
	resp->length=cmd_resp_buf.len();
	
	// Begin sending the buffer: 
	send_cmd_resp=1;
	cpnotify_outpump_start();
}


// Actually send data; only do if you received POLLOUT. 
// NOTE: This will get the data length from d->length, then 
//       translate d->length to network order. 
// Gives up if the write is not atomically. 
// HEADER IN HOST ORDER. 
// Retun value: 
//  0 -> OK, written
// -1 -> error; must call _ConnClose(). 
// -2 -> short write; must call _ConnClose(). 
int RendViewAdminConnection::_AtomicSendData(RVAPHeader *d)
{
	// Convert to network order: 
	size_t len=d->length;
	d->length=htonl(len);
	d->ptype=htons(d->ptype);
	
	ssize_t wr;
	do
	{  wr=write(sock_fd,(char*)d,len);  }
	while(wr<0 && errno==EINTR);
	if(wr<0)
	{
		Error("Admin: Failed to send %u bytes to %s: %s\n",
			len,addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	out.tot_transferred+=u_int64_t(wr);
	if(size_t(wr)<len)
	{
		Error("Admin: Short write (sent %u/%u bytes) to %s.\n",
			size_t(wr),len,addr.GetAddress().str());
		return(-2);
	}
	return(0);
}

// RETURNS HEADER IN HOST ORDER. 
// Return value: 0 -> OK; -1 -> error 
int RendViewAdminConnection::_AtomicRecvData(RVAPHeader *d,size_t len)
{
	ssize_t rd;
	do
	{  rd=read(sock_fd,(char*)d,len);  }
	while(rd<0 && errno==EINTR);
	if(rd<0)
	{
		Error("Admin: %s: While reading: %s\n",
			addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	in.tot_transferred+=u_int64_t(rd);
	if(!rd)
	{
		assert(len);  // We may not attempt zero-sized reads. 
		return(-1);
	}
	if(size_t(rd)!=len)
	{
		Error("Admin: Short read from %s at challenge (%d/%u bytes)\n",
			addr.GetAddress().str(),rd,len);
		return(-1);
	}
	if(size_t(rd)>=sizeof(RVAPHeader))
	{
		// Translate to host order: 
		d->length=ntohl(d->length);
		d->ptype=ntohs(d->ptype);
		if(d->length<sizeof(RVAPHeader))
		{
			Error("Admin: %s: Packet size entry too short: %u<%u\n",
				addr.GetAddress().str(),d->length,sizeof(RVAPHeader));
			return(-1);
		}
	}
	return(0);
}


int RendViewAdminConnection::_SendChallengeRequest()
{
	assert(auth_state==AS_SendChReq);
	
	RVAPChallengeRequest d;
	d.length=sizeof(RVAPChallengeRequest);  // STILL IN HOST ORDER
	d.ptype=RVAP_ChallengeRequest;   // STILL IN HOST ORDER
	uchar *cr=((uchar*)&d)+sizeof(RVAPHeader);
	uchar *ce=((uchar*)&d)+sizeof(RVAPChallengeRequest);
	memset(cr,0,ce-cr);
	RVAPSetIDString((char*)d.id_string,RVAPIDStringLength);
	d.protocol_vers=htons(RVAPProtocolVersion);
	d.idle_msec=LDR::LDR_timeout_hton(AP()->idle_timeout);
	// Fill in random challenge: 
	RVAPGetChallengeData(d.challenge);
	
	// That was the packet. Send it. 
	// I assume the challenge request is so small (64 bytes or so) 
	// that it can be sent atomically. Otherwise, we give up. 
	if(_AtomicSendData(&d))  return(-1);
	
	// Fill in what we expect: 
	RVAPComputeCallengeResponse(d.challenge,expect_chresp,
		AP()->password.str());
	
	auth_state=AS_WaitChResp;
	_DoPollFD(POLLIN,POLLOUT);
	
	return(0);
}


int RendViewAdminConnection::_RecvChallengeResponse()
{
	assert(auth_state==AS_WaitChResp);
	
	RVAPChallengeResponse d;
	int rv=_AtomicRecvData(&d,sizeof(d));
	if(rv)
	{  _ConnClose(0);  return(1);  }
	
	// Check what we got... The actually read amount of bytes is okay 
	// because _AtomicRecvData() will return error for short reads. 
	// length and ptype are already translated to host order. 
	if(d.length!=sizeof(d) || d.ptype!=RVAP_ChallengeResponse)
	{
		Error("Admin: Illegal challenge response packet: len=%u/%d, type=%u.\n",
			d.length,sizeof(d),d.ptype);
		_ConnClose(2);  // "auth failure"
		return(1);
	}
	
	// See if the response is okay: 
	if(memcmp(expect_chresp,d.response,RVAPChallengeRespLength))
	{
		Error("Admin: Illegal challenge response from %s.\n",
			addr.GetAddress().str());
		_ConnClose(2);  // "auth failure"
		return(1);
	}
	
	// Valid challenge response. 
	auth_state=AS_SendOK;
	memset(expect_chresp,0,RVAPChallengeRespLength);
	
	// Okay, correct challenge and no other authenticated server. 
	Verbose(MiscInfo,"Admin: Now connected to %s (%.*s).\n",
		addr.GetAddress().str(),
		RVAPIDStringLength,d.id_string);
	
	// Must sill send the now connected packet. 
	_DoPollFD(/*set=*/POLLOUT,/*clear=*/POLLIN);
	
	// Enable idle timeout if desired (and kill auth timeout): 
	if(AP()->idle_timeout<0)
	{  UpdateTimer(idle_tid,-1,0);  }
	else
	{  UpdateTimer(idle_tid,AP()->idle_timeout,
		FDAT_FirstLater | FDAT_AlignToShorter | 10);  }
	
	return(0);
}


int RendViewAdminConnection::_SendNowConnected()
{
	assert(auth_state==AS_SendOK);
	
	RVAPNowConnected d;
	d.length=sizeof(RVAPNowConnected);  // STILL IN HOST ORDER
	d.ptype=RVAP_NowConnected;   // STILL IN HOST ORDER
	
	if(_AtomicSendData(&d))  return(-1);
	
	// We now switch to FDCopy facility. 
	// No need to poll for writing (in case we do): 
	_DoPollFD(/*set=*/POLLIN,/*clear=*/POLLOUT);
	
	auth_state=AS_Passed;
	in.ioaction=IOA_None;
	out.ioaction=IOA_None;
	
	return(0);
}


// Return value: 
//   0 -> not handeled
//   1 -> handeled
//  -1 -> called _ConnClose(). 
int RendViewAdminConnection::_AuthSConnFDNotify(FDInfo *fdi)
{
	assert(auth_state==AS_Passed);
	
	if(fdi->revents & POLLIN)
	{
		// Read protocol header: 
		// We want to get it atomically. 
		RVAPHeader hdr;
		int rv=_AtomicRecvData(&hdr,sizeof(hdr));
		if(rv)
		{  _ConnClose(1);  return(-1);  }
		
		rv=_HandleReceivedHeader(&hdr);
		if(rv<0)
		{  _ConnClose(0);  }
		
		// Update response time: 
		_ResetIdleTimeout(fdi->current);
		
		return(rv);
	}
	
	// _AuthSConnFDNotify() is (currently) only called for POLLIN. 
	// We may NOT reach here. 
	assert(0);  // If caught: SIMPLE PROGRAMMING ERROR. 
	_ConnClose(0);
	return(-1);
}


int RendViewAdminConnection::_FDCopyStartRecvBuf(char *buf,size_t len)
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
	
	in.ioaction=IOA_Buf;
	
	return(0);
}


int RendViewAdminConnection::_FDCopyStartSendBuf(RVAPHeader *hdr)
{
	assert(!out_active_cmd);
	
	// Translate into network order: 
	size_t len=hdr->length;
	int ptype=hdr->ptype;
	hdr->length=htonl(len);
	hdr->ptype=htons(ptype);
	
	int rv=0;
	do {
		if(!pollid)  break;  // Cannot do anything; already closed socket. 
		
		if(out.pump_s->is_dead)
		{  out.pump_s->PumpReuseNow();  }
		
		assert(out.ioaction==IOA_None);
		
		out.io_buf->buf=(char*)hdr;
		out.io_buf->buflen=len;
		out.io_buf->bufdone=0;
		
		assert(out.io_sock->pollid==pollid);
		
		rv=out.pump_s->SetIO(out.io_buf,out.io_sock);
		if(rv!=0 && rv!=-1)
		{
			// If rv=-5, you probably need FDCopyPump::PumpReuseNow(). 
			fprintf(stderr,"OOPS(admin): out pump->SetIO(buf,fd) "
				"returned %d\n",rv);
			abort();
		}
		if(rv)  break;
		
		rv=out.pump_s->Control(FDCopyPump::CC_Start);
		if(rv!=0)
		{
			fprintf(stderr,"OOPS(admin): out pump(buf,fd)->CC_Start "
				"returned %d\n",rv);
			abort();  break;
		}
		
		out.ioaction=IOA_Buf;
		out_active_cmd=ptype;
	} while(0);
	
	return(rv);
}


int RendViewAdminConnection::_StartReadingCommandBody(IOBuf *dest,
	RVAPHeader *hdr)
{
	// NOTE: hdr in HOST order. 
	size_t len=hdr->length;
	assert(len>=sizeof(RVAPHeader));  // Already checked that in _AtomicRecvData(). 
	
	if(dest->Resize(len))
	{  return(-1);  }
	
	memcpy(dest->data,hdr,sizeof(RVAPHeader));
	
	int rv=_FDCopyStartRecvBuf(dest->data+sizeof(RVAPHeader),
		len-sizeof(RVAPHeader));
	if(!rv)
	{
		dest->content=hdr->ptype;
		in_active_cmd=hdr->ptype;
	}
	return(rv);
}


int RendViewAdminConnection::_HandleReceivedHeader(RVAPHeader *hdr)
{
	switch(hdr->ptype)
	{
		case RVAP_NoOp:
			return(1);
		case RVAP_CommandString:
		{
			// Okay, then let's get the body. 
			assert(!recv_buf.content);
			int rv=_StartReadingCommandBody(&recv_buf,hdr);
			if(rv)
			{
				if(rv==-1)
				{  Error("Admin: Resp buf alloc failure (%u bytes)!\n",
					hdr->length);  }
				return(-1);
			}
			
			return(1);
		}
		default:;
	}
	
	Error("Admin: Received unexpected/unknown command header "
		"(cmd=%u, length=%u).\n",
		hdr->ptype,hdr->length);
	
	return(-1);  // -->  _ConnClose()
}


int RendViewAdminConnection::fdnotify2(FDBase::FDInfo *fdi)
{
	Verbose(DBG,"--<Admin:fdnotify2>--<fd=%d, ev=0x%x, rev=0x%x>--\n",
		fdi->fd,int(fdi->events),int(fdi->revents));
	
	#if 0
	assert(pollid==fdi->pollid && fdi->fd==sock_fd);
	#else
	if(pollid!=fdi->pollid || sock_fd!=fdi->fd)
	{
		fprintf(stderr,
			"OOPS(admin): pollid=%p, fdi->pollid=%p, sock_fd=%d, fdi->fd=%d\n",
			pollid,fdi->pollid,sock_fd,fdi ? fdi->fd : -9999);
		if(!pollid && sock_fd<0)
		{
			// We should not be here. HLib's fault, probably. 
			return(0);
		}
		assert(0);
	}
	#endif
	
	int handeled=0;
	
	// See what we must do...
	if(fdi->revents & POLLHUP)
	{
		_ConnClose(1);  // "hangup"
		return(0);
	}
	if(fdi->revents & POLLIN)
	{
		if(auth_state==AS_WaitChResp)
		{
			int rv=_RecvChallengeResponse();
			if(rv)  // _ConnClose() was called. 
			{  return(0);  }
			handeled=1;
		}
		else if(auth_state==AS_Passed)
		{
			handeled=_AuthSConnFDNotify(fdi);
			if(handeled<0)  return(0);
		}
	}
	if(fdi->revents & POLLOUT)
	{
		if(auth_state==AS_SendChReq)
		{
			if(_SendChallengeRequest())
			{  _ConnClose(0);  return(0);  }
			handeled=1;
		}
		else if(auth_state==AS_SendOK)
		{
			if(_SendNowConnected())
			{  _ConnClose(0);  return(0);  }
			handeled=1;
		}
		// The rest is sent via FDCopy facility. 
		//else -> !handeled below
	}
	
	if(!handeled || (fdi->revents & (POLLPRI | POLLERR | POLLNVAL)) )
	{
		// See if there is an error: 
		int errval=GetSocketError(sock_fd);
		if(errval)
		{
			// We do not deal with the case of errval<0 here. 
			// GetSocketError() did not fail for initial connect; 
			// why sould it fail here. 
			// If you get "Unknown error" then proably getspockopt() 
			// failed...
			Error("Admin: %s: connection error: %s\n",
				addr.GetAddress().str(),strerror(errval));
		}
		else
		{
			Error("Admin: %s: unexpected revents=%d. Quitting.\n",
				addr.GetAddress().str(),fdi->revents);
		}
		_ConnClose(0);
	}
	
	return(0);
}


int RendViewAdminConnection::cpnotify_handle_errors(FDCopyBase::CopyInfo *cpi)
{
	// Check some status info. 
	// SCLimit <- Always set here. 
	// SCTimeout <- not implemented
	// SCKilled <- see above
	// SCTerm <- not used
	// SCInHup,SCOutHup,SCInPipe,SCOutPipe,SCRead0,SCWrite0
	//   SCEOI=(SCInHup|SCInPipe|SCRead0)
	//   SCEOO=(SCOutHup|SCOutPipe|SCWrite0)
	// SCErrPollI,SCErrPollO,SCErrRead,SCErrWrite,SCErrCopyIO
	//   SCError=(SCErrPollI|SCErrPollO|SCErrRead|SCErrWrite|SCErrCopyIO)
	assert(!(cpi->scode & (FDCopyPump::SCTimeout | FDCopyPump::SCTerm)));
	
	int illegal=0;
	
	if(cpi->scode & FDCopyPump::SCError)
	{
		// Well, this HAD to be done at some time...
		
		// Remove final flag for error message...
		cpi->scode=(FDCopyPump::StatusCode)(cpi->scode&~FDCopyPump::SCFinal);
		Error("Admin: Filesys/network IO error: %s\n",
			NetworkIOBase::CPNotifyStatusString(cpi));
		cpi->scode=(FDCopyPump::StatusCode)(cpi->scode|FDCopyPump::SCFinal);
		int errval=GetSocketError(sock_fd);
		if(errval)
		{  Error("     network socket: %s%s (%d)\n",
			errval==-1 ? "getsockopt failed: " : strerror(errval),
			errval==-1 ? strerror(errno) : "",errval);  }
		_ConnClose(0);  return(1);
	}
	else
	{
		if(cpi->scode & FDCopyPump::SCEOI)
		{
			// End of input...
			if(cpi->pump==in.pump_s)
			{
				// This means, the admin shell disconnected. 
				_ConnClose(1);  return(1);
			}
			else ++illegal;
		}
		if(cpi->scode & FDCopyPump::SCEOO)
		{
			// Cannot send any more...
			if(cpi->pump==out.pump_s)
			{
				// This means, the admin shell disconnected. 
				_ConnClose(1);  return(1);
			}
			else ++illegal;
		} // if(EOO)
	}
	
	if(illegal)
	{
		const char *pumpstr="???";
		if(cpi->pump==in.pump_s)  pumpstr="in.pump_s";
		else if(cpi->pump==out.pump_s)  pumpstr="out.pump_s";
		Error("Admin: Internal (?) error: Received illegal cpnotify.\n"
			"  pump: %s, src=%d, dest=%d; code: %s\n",
			pumpstr,
			cpi->pump->Src() ? cpi->pump->Src()->Type() : (-1),
			cpi->pump->Dest() ? cpi->pump->Dest()->Type() : (-1),
			NetworkIOBase::CPNotifyStatusString(cpi));
		int errval=GetSocketError(sock_fd);
		if(errval)
		{  Error("  network socket: %s%s (%d)\n",
			errval==-1 ? "getsockopt failed: " : strerror(errval),
			errval==-1 ? strerror(errno) : "",errval);  }
		// For now, I do a hack_assert(0), because I want to catch these 
		// in case they ever happen: 
		hack_assert(0);
		_ConnClose(0);  return(1);
	}
	
	return(0);
}


int RendViewAdminConnection::cpnotify(FDCopyBase::CopyInfo *cpi)
{
	Verbose(DBG,"--<cpnotify>--<%s: %s>--\n",
		cpi->pump==in.pump_s ? "IN" : "OUT",
		NetworkIOBase::CPNotifyStatusString(cpi));
	
	// We are only interested in FINAL codes. 
	if(!(cpi->scode & FDCopyPump::SCFinal))
	{  return(0);  }
	
	// Update statistics: 
	if(cpi->pump==in.pump_s)
	{  in.tot_transferred+=u_int64_t(in.io_sock->transferred);  }
	else if(cpi->pump==out.pump_s)
	{  out.tot_transferred+=u_int64_t(out.io_sock->transferred);  }
	else assert(0);  // ?!

	if((cpi->scode & FDCopyPump::SCKilled))
	{
		// Killed? Then that was probably necessary. 
		if(pollid && (cpi->pump==in.pump_s))
		{  _DoPollFD(POLLIN,0);  }
		return(0);
	}
	
	// This is only used when auth was passed. Hence...
	if(auth_state!=AS_Passed)
	{  assert(0);  return(0);  }
	
	if(cpnotify_handle_errors(cpi))
	{  return(0);  }
	
	do {
		if(cpi->pump==in.pump_s)
		{
			if(cpnotify_inpump(cpi))  break;  // _ConnClose() was called. 
			_ResetIdleTimeout(cpi->fdtime);
			
			// We're always listening to the auth shell. 
			if(!in_active_cmd)
			{  _DoPollFD(POLLIN,0);  }
		}
		else if(cpi->pump==out.pump_s)
		{
			if(cpnotify_outpump_done(cpi))  break;  // _ConnClose() was called. 
			
			assert(out.ioaction==IOA_None);  // If FDCopyPump is running, we may not be here. 
			if(cpnotify_outpump_start())  break;  // _ConnClose() was called. 
		}
		else assert(0);
	} while(0);
	
	return(0);
}


int RendViewAdminConnection::cpprogress(FDCopyBase::ProgressInfo *pi)
{
	// Currently ignore progress information from the FD pumps. 
	_ResetIdleTimeout(pi->fdtime);
	return(0);
}


int RendViewAdminConnection::timernotify(FDBase::TimerInfo *ti)
{
	Verbose(DBGV,"--<Admin::timernotify>--\n");
	
	assert(!DeletePending());
	
	if(ti->tid==schedule_tid)
	{
		UpdateTimer(schedule_tid,-1,0);
		
		if(outpump_scheduled)
		{
			outpump_scheduled=0;
			
			// Okay, do it: 
			cpnotify_outpump_start();
			// NOTE: Above function returns 1 if _ConnClose() was called. 
		}
		if(exec_cmd_scheduled)
		{
			exec_cmd_scheduled=0;
			
			ExecuteCommand(scheduled_cmd);
		}
	}
	else if(ti->tid==idle_tid)
	{
		if(auth_state!=AS_Passed)
		{
			Error("Admin connection %s timed out during auth.\n",
				addr.GetAddress().str());
			
			_ConnClose(2);  // "auth failure"
		}
		else
		{
			Warning("Admin connection %s idle timeout passed (%ld msec).\n",
				addr.GetAddress().str(),AP()->idle_timeout);
			
			_ConnClose(3);  // "timeout"
		}
	}
	else assert(0);
	
	return(0);
}


// Return value: 1 -> called _ConnClose(); 0 -> normal
int RendViewAdminConnection::cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi)
{
	assert(out_active_cmd!=0);
	
	Verbose(DBGV,"Admin: Sending ptype=%d done.\n",out_active_cmd);
	
	switch(out_active_cmd)
	{
		case RVAP_CommandResp:
		{
			send_cmd_resp=0;
			
			cmd_resp_buf.clear(/*free_it=*/0);
			if(cmd_resp_buf.size()>4096)
			{  cmd_resp_buf.trunc(4096,/*free_rest=*/1);  }
			
			scheduled_cmd.deref();
			
			out.ioaction=IOA_None;
			out_active_cmd=0;
		} break;
		default: assert(0);
	}
	
	return(0);
}


int RendViewAdminConnection::cpnotify_outpump_start()
{
	// FDCopy facility only used when auth was passed. 
	if(auth_state!=AS_Passed)
	{  assert(0);  return(0);  }
	
	// See if another command is currently active: 
	if(out_active_cmd)  return(0);
	
	if(send_cmd_resp)
	{
		assert(scheduled_cmd);
		
		int fail=_FDCopyStartSendBuf((RVAPHeader*)cmd_resp_buf.str());
		if(fail)
		{
			const char *msg=NULL;
			switch(fail)
			{
				case -1: msg=cstrings.allocfail;  break;
				default: assert(0);
			}
			Error("Admin: %s: Sending command response: %s.\n",
				addr.GetAddress().str(),msg);
			_ConnClose(0);  return(1);
		}
	}
	// else: Nothing to do (we're waiting). 
	
	return(0);
}


int RendViewAdminConnection::cpnotify_inpump(FDCopyBase::CopyInfo *cpi)
{
	switch(in_active_cmd)
	{
		case RVAP_NoOp:  assert(0);  break;  // we may not be here. 
		case RVAP_CommandString:
		{
			in_active_cmd=0;
			in.ioaction=IOA_None;
			
			int rv=_ParseCommandString(&recv_buf);
			if(rv<0)
			{  _ConnClose(0);  return(1);  }
			assert(rv==0);    // else: unknown error code
			
			recv_buf.content=0;
		} break;
		default:
			// This is an internal error. Only known packets may be accepted 
			// in _HandleReceivedHeader(). 
			assert(0);
	}
	
	return(cpnotify_outpump_start()==1 ? 1 : 0);
}


int RendViewAdminConnection::_ParseCommandString(IOBuf *buf)
{
	RVAPCommandString *pack=(RVAPCommandString*)(buf->data);
	if(pack->length<sizeof(RVAPCommandString))
	{  Error("Admin: %s: Illegal cmd string paket.\n",
		addr.GetAddress().str());  return(-1);  }
	assert(buf->len==pack->length);  // buf->alloc_len may be larger
	
	// Okay, here we have the command: 
	RefString cmd_str;
	size_t cmd_len=buf->data+pack->length-(char*)pack->command_str;
	if(cmd_str.set0((char*)pack->command_str,cmd_len))
	{  Warning("Admin: %s: Command allocation (%u bytes) failed.\n",
		addr.GetAddress().str(),cmd_len);  }
	
	// Commands are ignored if the previous one is still active. 
	bool ignore_command=0;
	if(!!scheduled_cmd)
	{  ignore_command=1;  }
	
	// Put it into the logs...
	if(ignore_command)
	{  Warning("Ignoring admin cmd from %s: \"%s\"\n",
		addr.GetAddress().str(),cmd_str.str());  }
	else
	{  Verbose(MiscInfo,"Admin command from %s: \"%s\"\n",
		addr.GetAddress().str(),cmd_str.str());  }
	
	if(!ignore_command)
	{
		// The command is not executed directly but scheduled for execution. 
		scheduled_cmd=cmd_str;
		schedule_exec_cmd();
	}
	
	return(0);
}


int RendViewAdminConnection::Setup(int sock,MyAddrInfo *_addr)
{
	if(PollFD(sock,POLLOUT,NULL,&pollid)<0)
	{  return(-1);  }
	
	assert(pollid);  // Otherwise PollFD() should have returned error. 
	
	addr=*_addr;  // implicit copy
	sock_fd=sock;
	
	auth_state=AS_SendChReq;
	
	in.io_sock->pollid=pollid;
	in.io_sock->max_iolen=4096;
	out.io_sock->pollid=pollid;
	out.io_sock->max_iolen=4096;
	
	// Set auth timeout: 
	if(AP()->auth_timeout>=0)
	{  UpdateTimer(idle_tid,AP()->auth_timeout,
		FDAT_FirstLater | FDAT_AlignToShorter | 10);  }
	
	connected_since=HTime::Curr;
	
	return(0);
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

void RendViewAdminConnection::_ConnClose(int reason)
{
	// We should not call ConnClose() twice. 
	// This assert is the bug trap for that: 
	assert(!DeletePending());
	
	if(outpump_scheduled)
	{
		outpump_scheduled=0;
		if(!exec_cmd_scheduled)
		{  UpdateTimer(schedule_tid,-1,0);  }
	}
	
	if(sock_fd>=0)  // Otherwise: Not connected or already disconnected. 
	{
		// Make sure we close down. 
		if(pollid)
		{
			// First, cancel all IO jobs...
			_KillControl(in.pump_s);
			_KillControl(out.pump_s);
			
			// ...then actually shut down the connection. 
			PollFDDPtr(pollid,NULL);
			ShutdownFD(pollid);  // sets pollid=NULL
			sock_fd=-1;
		}
	}
	
	AP()->ConnClose(this,reason);
}


RendViewAdminConnection::RendViewAdminConnection(ComponentDataBase *_cdb,
	int *failflag) : 
	FDCopyBase(failflag),
	//TimeoutBase(failflag),
	LinkedListBase<RendViewAdminConnection>(),
	addr(failflag),
	connected_since(HTime::Invalid),
	in(),
	out(),
	recv_buf(),
	scheduled_cmd(failflag),
	cmd_resp_buf(/*reserve_len=*/1024,failflag)
{
	int failed=0;
	
	cdb=_cdb;
	
	pollid=NULL;
	sock_fd=-1;
	auth_state=AS_None;
	memset(expect_chresp,0,RVAPChallengeRespLength);
	
	in_active_cmd=0;
	out_active_cmd=0;
	
	outpump_scheduled=0;
	exec_cmd_scheduled=0;
	
	send_cmd_resp=0;
	
	idle_tid=InstallTimer(-1,0);
	if(!idle_tid)  ++failed;
	
	schedule_tid=InstallTimer(-1,0);
	if(!schedule_tid)  ++failed;
	
	if(in.Setup(this) || out.Setup(this))  ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("RendViewAdminConnection");  }
}

RendViewAdminConnection::~RendViewAdminConnection()
{
	if(sock_fd>=0)
	{
		Warning("Still connected to admin shell %s; disconnecting.\n",
			addr.GetAddress().str());
		ShutdownFD(sock_fd);  pollid=NULL;
	}
	
	memset(expect_chresp,0,RVAPChallengeRespLength);
	
	in.Cleanup();
	out.Cleanup();
}


//------------------------------------------------------------------------------

int RendViewAdminConnection::FDCopy::Setup(FDCopyBase *fcb)
{
	// All values NULL as of FDCopy::FDCopy() constructor. 
	do {
		if(!(io_sock=NEW<FDCopyIO_FD>()))  break;
		if(!(io_buf=NEW<FDCopyIO_Buf>()))  break;
		if(!(pump_s=NEW1<FDCopyPump_Simple>(fcb)))  break;
		goto okay;
	} while(0);
	return(-1);
okay:;
	io_sock->persistent=1;
	io_buf->persistent=1;
	pump_s->persistent=1;
	return(0);
}


void RendViewAdminConnection::FDCopy::Cleanup()
{
	// This is tricky...
	// First, we must make sure that the pumps do no longer reference the 
	// CopyIOs. 
	if(pump_s)
	{
		assert(!pump_s->IsActive());  // _ShutdownConnection() must do that
		pump_s->SetIO(NULL,NULL);  // May be done even if pump->is_dead. 
	}
	
	// Then, we delete the pump: 
	if(pump_s)
	{  pump_s->persistent=0;   delete pump_s;   pump_s=NULL;   }
	
	// And finally, we delete the CopyIOs. 
	if(io_sock)
	{  io_sock->persistent=0;  delete io_sock;  io_sock=NULL;  }
	if(io_buf)
	{  io_buf->persistent=0;   delete io_buf;   io_buf=NULL;   }
}


RendViewAdminConnection::FDCopy::FDCopy()
{
	io_sock=NULL;
	io_buf=NULL;
	pump_s=NULL;
	ioaction=IOA_Locked;
	
	tot_transferred=0;
}


//------------------------------------------------------------------------------

int RendViewAdminConnection::IOBuf::Resize(size_t _new_size)
{
	size_t new_size=(_new_size<256 ? 256 : _new_size);
	if(alloc_len<new_size || alloc_len>2*new_size)
	{
		char *odata=data;
		data=(char*)LRealloc(data,new_size);
		if(!data)
		{  data=odata;  return(-1);  }
		alloc_len=new_size;
	}
	len=_new_size;
	return(0);
}
