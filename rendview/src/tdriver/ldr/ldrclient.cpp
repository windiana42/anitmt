/*
 * ldrclient.cpp
 * 
 * LDR task driver stuff: LDR client representation on server side. 
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


#include "../../taskmanager.hpp"
#include "dif_ldr.hpp"
#include "dif_param.hpp"
#include "ldrclient.hpp"

#include <ctype.h>
#include <assert.h>

#include <lib/mymath.hpp>   /* for NAN, etc */


using namespace LDR;


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif

#warning IMPORTANT: Remove potentially dangerous asserts in destructors (like assert(fd<0))


static void _AllocFailure(int fail=-1)
{
	if(fail)
	{  Error("LDR: %s.\n",cstrings.allocfail);  }
}


// Search in supported render/filter descs for passed *desc 
// (component data base object). Uses a 1-entry cache per dtype 
// so successfully looking up the same RF_DescBase several 
// times will be fast. 
// Return value: 
//   1 -> supported
//   0 -> not supported
//  -1 -> dtype out of range or desc==NULL
int LDRClient::IsSupportedDesc(TaskDriverType dtype,const RF_DescBase *desc)
{
	if(dtype<0 || dtype>=_DTLast || !desc)
	{  return(-1);  }
	
	if(_cache_known_supported[dtype]==desc)
	{  return(1);  }
	
	int n=n_supp_descs[dtype];
	for(int i=0; i<n; i++)
	{
		if(supp_desc[dtype][i]==desc)
		{
			_cache_known_supported[dtype]=desc;
			return(1);
		}
	}
	
	return(0);
}


// Return value: 
//   0 -> connecting...
//   1 -> connected successfully without delay
//  -1 -> error
int LDRClient::ConnectTo(ClientParam *cp)
{
	Verbose(TDR,"  Client %s (%s): ",
		cp->name.str(),cp->addr.GetAddress().str());
	
	const char *failure="FAILURE\n";
	
	int sock=cp->addr.socket();
	if(sock<0)
	{
		int errn=errno;
		Verbose(TDR,failure);
		Error("Failed to create inet socket: %s\n",strerror(errn));
		return(-1);
	}
	
	int fail=1;
	int already_connected=0;
	do {
		if(SetNonblocking(sock)<0)
		{
			int errn=errno;
			Verbose(TDR,failure);
			Error("Failed to set socket non-blocking: %s\n",strerror(errn));
			break;
		}
		
		int rv;
		do
		{  rv=cp->addr.connect(sock);  }
		while(rv<0 && errno==EINTR);
		if(rv<0)
		{
			if(errno!=EINPROGRESS)
			{
				int errn=errno;
				Verbose(TDR,failure);
				Error("Failed to connect to %s: %s\n",
					cp->addr.GetAddress().str(),strerror(errn));
				break;
			}
			// Okay, must poll for it to see when connection estblishes. 
			Verbose(TDR,"[in progress]\n");
		}
		else if(!rv)
		{
			// Oh! That was fast. We're already connected. 
			already_connected=1;
			Verbose(TDR,"[connected]\n");
		}
		else assert(rv<=0);
		
		fail=0;
	} while(0);
	
	if(!fail)
	{
		if(PollFD(sock,POLLIN,/*dptr=*/NULL,&pollid)<0)
		{  pollid=NULL;  }
		if(!pollid)
		{  Error("PollFD failed.\n");  fail=1;  }
	}
	
	if(fail)
	{
		close(sock);
		return(-1);
	}
	
	this->sock_fd=sock;
	assert(pollid);  // otherwise great internal error
	
	out.io_sock->pollid=pollid;
	out.io_sock->max_iolen=4096;
	in.io_sock->pollid=pollid;
	in.io_sock->max_iolen=4096;
	
	connected_state=already_connected+1;
	// Okay, now we're doing POLLIN and waiting for answer. 
	
	this->cp=cp;
	
	if(already_connected)
	{  connected_since=HTime::Curr;  }
	
	return(already_connected);
}


// Return value: 
//  1 -> already disconnected
//  0 -> wait for disconnect to happen. 
int LDRClient::Disconnect()
{
	if(connected_state<2)
	{
		Verbose(TDR,"  Nothing to do for disconnect from %s.\n",
			_ClientName().str());
		ShutdownFD(pollid);
		return(1);
	}
	
	// Normal disconnect. Queue LCCC_ClientQuit: 
	if(!client_quit_queued)
	{
		int rv=SendControlRequest(LCCC_ClientQuit);
		if(rv==-2)
		{
			// Not completely connected; cannot send control command. 
			// In this case: Simple quit...
			_KickMe();
		}
		else assert(rv==-1 || client_quit_queued==1);
	}
	
	return(0);
}


// Return value: 0 -> OK; -1 -> error
int LDRClient::_AtomicSendData(LDRHeader *d)
{
	// Translate to network order: 
	size_t len=d->length;
	d->length=htonl(len);
	d->command=htons(d->command);
	
	ssize_t wr;
	do
	{  wr=write(sock_fd,(char*)d,len);  }
	while(wr<0 && errno==EINTR);
	if(wr<0)
	{
		int errn=errno;
		Error("Client %s: Failed to send %u bytes: %s\n",
			_ClientName().str(),len,strerror(errn));
		return(-1);
	}
	out.stat.tot_transferred+=u_int64_t(wr);
	if(size_t(wr)<len)
	{
		Error("Client %s: Short write (sent %u/%u bytes).\n",
			_ClientName().str(),size_t(wr),len);
		return(-1);
	}
	return(0);
}


void LDRClient::_ClientDisconnectMessage()
{
	if(client_quit_queued==3)
	{  Verbose(TDR,"Client %s disconnected due to our quit request.\n",
		_ClientName().str());  }
	else
	{  Error("Client %s: Disconnected unexpectedly.\n",
		_ClientName().str());  }
}

// Return value: len or -1 -> error
ssize_t LDRClient::_AtomicRecvData(LDRHeader *dest,size_t len,size_t min_len)
{
	ssize_t rd;
	do
	{  rd=read(sock_fd,(char*)dest,len);  }
	while(rd<0 && errno==EINTR);
	if(rd<0)
	{
		int errn=errno;
		Error("Client %s: While reading: %s\n",
			_ClientName().str(),strerror(errn));
		return(-1);
	}
	in.stat.tot_transferred+=u_int64_t(rd);
	if(!rd)
	{
		assert(len);  // Passing len=0 to this function is illegal. 
		_ClientDisconnectMessage();
		return(-1);
	}
	else if(size_t(rd)<len)
	{
		Error("Client %s: Short packet \"%s\": %d/%u bytes\n",
			_ClientName().str(),LDRCommandString(expect_cmd),rd,min_len);
		return(-1);
	}
	// Translate to host order: 
	dest->length=ntohl(dest->length);
	dest->command=ntohs(dest->command);
	if(dest->length<min_len)
	{
		Error("Client %s: Packet size entry too short: %u<%u\n",
			_ClientName().str(),dest->length,min_len);
		return(-1);
	}
	return(rd);
}


// Finish up connect(2). 
// Return value: 
//  0 -> OK, now connected. 
//  1 -> failure; remove client
int LDRClient::_DoFinishConnect(FDBase::FDInfo *fdi)
{
	assert(connected_state==1);
	
	// Okay, see if we have POLLIN. (DO THAT FIRST; YES!!)
	if(fdi->revents & (POLLIN | POLLERR | POLLHUP))
	{
		int errval=GetSocketError(sock_fd);
		if(errval)
		{
			Error("Client %s: %s failed: %s\n",
				_ClientName().str(),
				errval<0 ? "getsockopt" : "connect",
				strerror(errval));
			return(1);
		}
	}
	
	// Seems that we may connect without problems. 
	// See if other flags are set: (The ERR/HUP test just in case we did 
	// not return above.) 
	if(fdi->revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		Error("Client %s: Strange poll revents%s%s%s. Removing client.\n",
			_ClientName().str(),
			(fdi->revents & POLLERR) ? " ERR" : "",
			(fdi->revents & POLLHUP) ? " HUP" : "",
			(fdi->revents & POLLNVAL) ? " NVAL" : "");
		return(1);
	}
	
	// Okay, we're connected. 
	connected_state=2;
	Verbose(TDR,"Okay, connected to %s. Waiting for challenge...\n",
		_ClientName().str());
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_ChallengeRequest;
	
	connected_since=HTime::Curr;
	
	return(0);
}


// Compute a response out of a request and store the whole 
// response packet stored in RespBuf *dest. 
// Return value: 0 -> OK. 
int LDRClient::_StoreChallengeResponse(LDRChallengeRequest *d,RespBuf *dest)
{
	if(_ResizeRespBuf(dest,sizeof(LDRChallengeResponse)))
	{  return(1);  }
	
	assert(dest->content==Cmd_NoCommand);
	dest->content=Cmd_ChallengeResponse;
	LDRChallengeResponse *r=(LDRChallengeResponse *)(dest->data);
	r->length=sizeof(LDRChallengeResponse);  // STILL IN HOST ORDER
	r->command=next_send_cmd;
	
	LDRSetIDString((char*)r->id_string,LDRIDStringLength);
	LDRComputeCallengeResponse(d->challenge,(char*)r->response,
		cp->password.str());
	
	r->keepalive_msec=LDR_timeout_hton(tdif->P()->keepalive_interval);
	
	//r->current_time set immediately before sending. 
	
	return(0);
}


// Return value: 0 -> OK
// 1 -> failure; close down conn. 
int LDRClient::_DoAuthHandshake(FDBase::FDInfo *fdi)
{
	assert(connected_state==2 && auth_state==0);  // auth_state=1 uses FDCopy facility
	
	int handeled=0;
	if(fdi->revents & POLLHUP)
	{
		_ClientDisconnectMessage();
		return(1);
	}
	if(fdi->revents & POLLIN)
	{
		if(expect_cmd==Cmd_ChallengeRequest)
		{
			assert(next_send_cmd==Cmd_NoCommand);
			
			// We want to get challenge from client. 
			// Challenge is fixed-length and we expect to get it atomically. 
			
			LDRChallengeRequest d;
			ssize_t rd=_AtomicRecvData(&d,sizeof(d),sizeof(d));
			if(rd<0)
			{  return(1);  }
			
			// Check fields: 
			if(size_t(rd)<sizeof(LDRHeader))
			{
				Error("Client %s: Packet too short (header incomplete: %u bytes)\n",
					_ClientName().str(),size_t(rd));
				return(1);
			}
			++in.stat.count_headers;   // statistics
			LDRCommand last_recv_cmd=LDRCommand(d.command);
			if(last_recv_cmd!=expect_cmd)
			{
				Error("Client %s: Conversation error (expected: %s; received: %s)\n",
					_ClientName().str(),
					LDRCommandString(expect_cmd),
					LDRCommandString(last_recv_cmd));
				return(1);
			}
			if(d.length!=sizeof(LDRChallengeRequest))
			{
				Error("Client %s: Illegal-sized packet (%u/%u bytes)\n",
					_ClientName().str(),d.length,sizeof(LDRChallengeRequest));
				return(1);
			}
			
			u_int16_t clientpv=ntohs(d.protocol_vers);
			if(clientpv!=LDRProtocolVersion)
			{
				Error("Client %s: LDR proto version mismatch: "
					"client: %d; server %d.\n",
					_ClientName().str(),int(clientpv),int(LDRProtocolVersion));
				return(1);
			}
			
			Verbose(TDR,"Client %s: %.*s\n",
				_ClientName().str(),
				LDRIDStringLength,d.id_string);
			
			// Okay, we have a challenge. Compute the response. 
			// The response is computed and stored in send_buf. 
			next_send_cmd=Cmd_ChallengeResponse;    // DO NOT MOVE
			expect_cmd=Cmd_NoCommand;
			_StoreChallengeResponse(&d,&send_buf);  // DO NOT MOVE
			
			_DoPollFD(POLLOUT,POLLIN);
			handeled=1;
		}
		// expect_cmd==Cmd_NowConnected is handeled by FDCopy facility. 
	}
	if(fdi->revents & POLLOUT)
	{
		if(next_send_cmd==Cmd_ChallengeResponse)
		{
			assert(send_buf.data && send_buf.alloc_len>=sizeof(LDRChallengeResponse));
			assert(expect_cmd==Cmd_NoCommand);
			
			assert(send_buf.content==Cmd_ChallengeResponse);
			
			// Set the server time as late as possible for better time 
			// estimation (smaller server time interval). 
			HTime curr(HTime::Curr);
			HTime2LDRTime(&curr,
				&(((LDRChallengeResponse*)(send_buf.data))->current_time));
			
			if(_AtomicSendData((LDRHeader *)(send_buf.data)))
			{  return(1);  }
			
			++out.stat.count_headers;   // statistics
			
			next_send_cmd=Cmd_NoCommand;
			expect_cmd=Cmd_NowConnected;
			send_buf.content=Cmd_NoCommand;
			
			// Now, switch to FDCopy facility: 
			// out.ioaction, in.ioaction currently IOA_Locked: 
			out.ioaction=IOA_None;
			in.ioaction=IOA_None;
			auth_state=1;  // waiting for Cmd_NowConnected
			
			_DoPollFD(POLLIN,POLLOUT);
			handeled=1;
		}
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
			Error("Client %s: socket error: %s\n",
				_ClientName().str(),strerror(errval));
			return(1);
		}
		else
		{
			Error("Client %s: Unexpected revents=0x%x (state %d,%d). "
				"Disconnecting.\n",
				_ClientName().str(),int(fdi->revents),
				expect_cmd,next_send_cmd);
			return(1);
		}
	}
	return(0);
}


// Helper for _Create_TaskRequest_Packet(): 
void LDRClient::_LDRStoreInputFileInfo(TaskDriverType /*dtype*/,
	TaskFile *infile,TaskFile *outfile,
	char *dptr,u_int64_t *save_size,LDRTime *save_mtime)
{
	HTime tmp(HTime::Invalid);
	int64_t fsize=infile->FileLength(&tmp);
	// We ignore errors. If the file is not there (normal case 
	// for filter input in case the client also does rendering), 
	// then we pass invalid htime and size=0. 
	if(fsize<0)
	{  fsize=0;  tmp.SetInvalid();  }
	
	*save_size=htonll(u_int64_t(fsize));
	HTime2LDRTime(&tmp,save_mtime);
	
	// Copy input and output file name: 
	strcpy_no0(
		strcpy_rdst(dptr,infile->BaseNamePtr()),
		outfile->BaseNamePtr());
}

// This formats the whole LDRTaskRequest data into the RespBuf *dest. 
// (Note: Header in HOST oder)
// NOTE: Only sends that part (render/filter/both) which can actually 
//       be done by the client. 
// Return value: 
//   0 -> OK complete packet in resp_buf; ready to send it 
//  -1 -> alloc failure
//  -2 -> failed to convert frame_clock to u_int32_t
int LDRClient::_Create_TaskRequest_Packet(RespBuf *dest)
{
	CompleteTask *ctsk=tri.scheduled_to_send;
	assert(dest->content==Cmd_NoCommand);
	
	// NOTE: AlWAYS USE rt/ft INSTEAD OF ctsk->rt/ctsk->fd BECAUSE 
	// IF THE CLIENT CANNOT DO THE TASK, WE SET rt/ft TO NULL HERE. 
	RenderTask *rt=(ctsk->d.shall_render ? ctsk->rt : NULL);
	FilterTask *ft=(ctsk->d.shall_filter ? ctsk->ft : NULL);
	assert(rt || ft);  // Else intrnl error: may not give a no-op to client. 
	
	// First, compute the size of the "packet": 
	size_t totsize=sizeof(LDRTaskRequest);
	
	// All the additional args have to be passed: 
	size_t r_add_args_size;
	size_t r_desc_slen,r_oformat_slen,r_iofile_slen;
	if(rt)
	{
		r_add_args_size=SumUpSize(&rt->add_args);
		r_desc_slen=rt->rdesc->name.len();
		r_oformat_slen=strlen(rt->oformat->name);
		r_iofile_slen=
			strlen(rt->infile.BaseNamePtr())+
			strlen(rt->outfile.BaseNamePtr())+1;  // 1 -> '\0'-separator
	} else {
		r_add_args_size=0;
		r_desc_slen=0;
		r_oformat_slen=0;
		r_iofile_slen=0;
	}
	totsize+=(r_add_args_size+r_desc_slen+r_oformat_slen+r_iofile_slen);
	
	size_t f_add_args_size;
	size_t f_desc_slen,f_iofile_slen;
	if(ft)
	{
		f_add_args_size=SumUpSize(&ft->add_args);
		f_desc_slen=ft->fdesc->name.len();
		f_iofile_slen=
			strlen(ft->infile.BaseNamePtr())+
			strlen(ft->outfile.BaseNamePtr())+1;  // 1 -> '\0'-separator
	} else {
		f_add_args_size=0;
		f_desc_slen=0;
		f_iofile_slen=0;
	}
	totsize+=(f_add_args_size+f_desc_slen+f_iofile_slen);
	
	// Count required files: 
	// NOTE: Only files with skip_flag NOT set are transferred. 
	//       Thus, r_n_files can be smaller than ctsk->radd.nfiles. 
	int r_n_files=0,f_n_files=0;
	if(rt)
	{
		totsize+=LDRSumFileInfoSize(&ctsk->radd,&r_n_files);
		tri.non_skipped_radd_files=r_n_files;
		//assert(f_n_files<=ctsk->radd.nfiles);
	}
	if(ft)
	{
		totsize+=LDRSumFileInfoSize(&ctsk->fadd,&f_n_files);
		tri.non_skipped_fadd_files=f_n_files;
		//assert(f_n_files<=ctsk->fadd.nfiles);
	}
	
	u_int16_t r_flags=0;
	if(rt)
	{
		if(rt->resume || rt->outfile.IsIncomplete())
		{
			// If resume is set, incomplete must be set and vice versa. 
			assert(rt->resume && rt->outfile.IsIncomplete());
			
			r_flags|=TRRF_Unfinished;
		}
		if(rt->resume_flag_set)
		{  r_flags|=TRRF_ResumeFlagActive;  }
		if(rt->use_frame_clock)
		{  r_flags|=TRRF_UseFrameClock;  }
	}
	
	#if 0   // FrameClockVal: UNUSED
	u_int32_t r_frame_clock=Tnt32Double_NAN;
	if(rt && finite(rt->frame_clock))
	{
		int rv=DoubleToInt32(rt->frame_clock,&r_frame_clock);
		assert(rv!=3);  // ...which is NaN which should not happen here. 
		if(rv)  // frame clock conversion failure (overflow)
		{  return(-2);  }
	}
	#endif
	
	// Okay, these limits are meant to be never exceeded. 
	// But we must be sure: 
	assert(r_desc_slen<=0xffff && r_oformat_slen<=0xffff);
	assert(f_desc_slen<=0xffff);
	assert(r_iofile_slen<=0xffff && f_iofile_slen<=0xffff);
	assert(r_n_files<=0xffff);
	assert(f_n_files<=0xffff);
	assert(totsize<0xffffffff);
	
	if(_ResizeRespBuf(dest,totsize))
	{
		// Alloc failure: 
		return(-1);
	}
	
	// Actually format the packet: 
	// (Header still in HOST oder)
	LDRTaskRequest *pack=(LDRTaskRequest *)(dest->data);
	pack->length=totsize;
	pack->command=Cmd_TaskRequest;
	
	pack->frame_no=htonl(ctsk->frame_no);
	pack->task_id=htonl(ctsk->task_id);
	
	if(rt)
	{
		pack->r_width=htons(rt->width);
		pack->r_height=htons(rt->height);
		pack->r_timeout=LDR_timeout_hton(rt->timeout);
	}
	else
	{  pack->r_width=0;  pack->r_height=0;  pack->r_timeout=0;  }
	pack->r_iofile_slen=htons(r_iofile_slen);
	pack->r_desc_slen=htons(r_desc_slen);
	pack->r_add_args_size=htonl(r_add_args_size);
	pack->r_oformat_slen=htons(r_oformat_slen);
	pack->r_flags=htons(r_flags);
	// FrameClockVal: UNUSED
	//pack->r_frame_clock=r_frame_clock;  // already network order
	
	if(ft)
	{  pack->f_timeout=LDR_timeout_hton(ft->timeout);  }
	else
	{  pack->f_timeout=0;  }
	pack->f_iofile_slen=htons(f_iofile_slen);
	pack->f_desc_slen=htons(f_desc_slen);
	pack->f_add_args_size=htonl(f_add_args_size);
	
	pack->r_n_files=htons(r_n_files);
	pack->f_n_files=htons(f_n_files);
	
	char *dptr=(char*)(pack->data);
	// Assign render/filter primary input file data: 
	if(rt)
	{
		_LDRStoreInputFileInfo(DTRender,&rt->infile,&rt->outfile,dptr,
			&pack->r_in_file_size,&pack->r_in_file_mtime);
		dptr+=r_iofile_slen;
	}
	else
	{  pack->r_in_file_size=0;  HTime2LDRTime(NULL,&pack->r_in_file_mtime);  }
	if(ft)
	{
		_LDRStoreInputFileInfo(DTFilter,&ft->infile,&ft->outfile,dptr,
			&pack->f_in_file_size,&pack->f_in_file_mtime);
		dptr+=f_iofile_slen;
	}
	else
	{  pack->f_in_file_size=0;  HTime2LDRTime(NULL,&pack->f_in_file_mtime);  }
	
	// Store render/filter desc: 
	if(rt)
	{  strcpy_no0(dptr,rt->rdesc->name.str());  dptr+=r_desc_slen;  }
	if(ft)
	{  strcpy_no0(dptr,ft->fdesc->name.str());  dptr+=f_desc_slen;  }
	
	if(rt)
	{
		strcpy_no0(dptr,rt->oformat->name);      dptr+=r_oformat_slen;
		dptr=CopyStrList2Data(dptr,&rt->add_args);
	}
	if(ft)
	{
		dptr=CopyStrList2Data(dptr,&ft->add_args);
	}
	
	{
		char *end=dest->data+totsize;
		if(rt)
		{  dptr=LDRStoreFileInfoEntries(dptr,end,&ctsk->radd);  }
		if(ft)
		{  dptr=LDRStoreFileInfoEntries(dptr,end,&ctsk->fadd);  }
	}
	
	// Now, we must be exactly at the end of the buffer: 
	// If that fails, be sure that LDRTaskRequest::data is on a 32 
	// (or better 64) bit boundary!!
	assert(dptr==dest->data+totsize);
	
	dest->content=Cmd_TaskRequest;
	return(0);
}


int LDRClient::SendTaskToClient(CompleteTask *ctsk)
{
	bool r_ok=(ctsk->d.shall_render && ctsk->rt);
	bool f_ok=(ctsk->d.shall_filter && ctsk->ft);
	if(!ctsk || (!r_ok && !f_ok))  return(-2);
	if(auth_state!=2)  return(-1);
	if(assigned_jobs>=c_task_thresh_high)  return(2);
	if(client_no_more_tasks)  return(3);
	
	// See if there is already a task scheduled to be sent to the client: 
	if(tri.scheduled_to_send || out.ioaction==IOA_Locked)
	{  return(1);  }
	
	tri.scheduled_to_send=ctsk;
	tri.task_request_state=TRC_SendTaskRequest;
	
	// The job is now assigned to us. 
	assert(ctsk->d.ldrc==this);   // Otherwise internal error. 
	++assigned_jobs;
	
	cpnotify_outpump_start();
	
	return(0);
}


int LDRClient::_DoQueueControlRequest(LDRClientControlCommand cccmd,
	char *data,size_t datalen)
{
	if(auth_state!=2)  return(-2);
	
	if(cccmd==LCCC_PingPong && queued_ping_requests>=3)
	{
		Error("Client %s: Attempt to queue more than 3 keepalive "
			"requests. Something seems to be wrong. Kicking...\n",
			_ClientName().str());
		_KickMe();
		return(-1);
	}
	
	// Append command to command queue: 
	int rv=cmd_queue.AddSendEntry(/*pos=*/+1,cccmd,datalen,(uchar*)data);
	if(rv)
	{
		Error("Client %s: Failure to enqueue LDR command (%s, dlen=%u): %s\n",
			_ClientName().str(),LDRClientControlCommandString(cccmd),datalen,
			rv==-1 ? cstrings.allocfail : 
				(rv==-2 ? "queue limit exceeded" : "???"));
		_KickMe();
		return(-1);
	}
	
	if(cccmd==LCCC_PingPong)
	{  ++queued_ping_requests;  }
	else if(cccmd==LCCC_ClientQuit)
	{
		client_quit_queued=1;
		client_no_more_tasks=1;
	}
	
	cpnotify_outpump_start();
	return(0);
}


int LDRClient::SendControlRequest(LDRClientControlCommand cccmd)
{
	return(_DoQueueControlRequest(cccmd,NULL,0));
}


// may_keep: number of not-currently-processed tasks the client may keep. 
int LDRClient::TellClientToGiveBackTasks(int _may_keep)
{
	assert(_may_keep>=0);
	
	// NOTE: special value: may_keep=0x7fffffff: give back all not processed. 
	LDRCCC_GBT_Req_Data rdata;
	rdata.may_keep=htonl(u_int32_t(_may_keep));
	
	// NOTE: We will send the control command to the client. 
	//       The client will put all (but may_keep) tasks from 
	//       the todo queue into the done queue and schedule 
	//       giving them back. But it will send the reponse 
	//       IMMEDIATELY. The server cannot control easily if 
	//       the client actually gives back the tasks. But we 
	//       have to trust the client doing right with the 
	//       tasks anyway...
	
	return(_DoQueueControlRequest(LCCC_GiveBackTasks,
		(char*)(&rdata),sizeof(LDRCCC_GBT_Req_Data)));
}


// Return value: 1 -> handeled; -1 -> must call _KickMe(). 
int LDRClient::_HandleReceivedHeader(LDRHeader *hdr)
{
	// BE CAREFUL!! hdr ALLOCATED ON THE STACK. 
	++in.stat.count_headers;   // statistics
	
	// See what we receive: 
	LDRCommand recv_cmd=LDRCommand(hdr->command);
	
	Verbose(DBGV,"Client %s: Received header: %s (length=%u)\n",
		_ClientName().str(),LDRCommandString(recv_cmd),hdr->length);
	
	if( (auth_state==1 && recv_cmd!=Cmd_NowConnected) ||  // <-- (A)
	    (client_quit_queued==3) )  // <-- (B)
	{
		// (A) We did not yet get the LDR_NowConnected packet. 
		//     Want to see that first. 
		// (B) We already received the quit confirmation. Client shall 
		//     disconnect and not send anything else. 
		Error("Client %s: Sending %s packet instead of %s. Kicking it.\n",
			_ClientName().str(),LDRCommandString(recv_cmd),
			(client_quit_queued==3) ? 
				"disconnecting due to quit" : 
				LDRCommandString(Cmd_NowConnected));
		return(-1);
	}
	
	switch(recv_cmd)
	{
		case Cmd_NowConnected:
		case Cmd_FileRequest:
		case Cmd_TaskResponse:
		case Cmd_TaskDone:
		case Cmd_FileUpload:
		case Cmd_DoneComplete:
		case Cmd_ControlResponse:
		{
			// Okay, then let's get the body. 
			assert(recv_buf.content==Cmd_NoCommand);  // Can that happen?
			int rv=_StartReadingCommandBody(&recv_buf,hdr);
			if(rv)
			{
				if(rv==-2)
				{  Error("Client %s: Too long %s packet "
					"(header reports %u bytes)\n",
					_ClientName().str(),
					LDRCommandString(recv_cmd),hdr->length);  }
				else _AllocFailure();
				return(-1);
			}
		
			return(1);
		}
		default:;
	}
	
	Error("Client %s: Received unexpected command header %s "
		"(cmd=%u, length=%u=0x%x).\n",
		_ClientName().str(),LDRCommandString(recv_cmd),
		recv_cmd,hdr->length,hdr->length);
	
	return(-1);  // -->  _KickMe()
}


// Used by _ParseNowConnected() to read in render/filter descs. 
// Return value: 
//   0 -> OK
//  -1 -> alloc failure
//  -2 -> format error
int LDRClient::_ParseNowConnected_ReadDescs(TaskDriverType dtype,
	const char *src,size_t len)
{
	// This should not be necessary as supp_descs should be NULL: 
	supp_desc[dtype]=(const RF_DescBase**)LFree(supp_desc[dtype]);
	n_supp_descs[dtype]=0;
	_cache_known_supported[dtype]=NULL;
	
	Verbose(TDR,"  %c%s descs:",
		toupper(*DTypeString(dtype)),DTypeString(dtype)+1);
	int ndumped=0;
	int alloc_len=0;
	for(const char *end=src+len; src<end; src++)
	{
		// Get the current string: 
		const char *name=src;
		while(*src && src<end)  ++src;
		if(src>=end)
		{
			Verbose(TDR," [error]\n");
			supp_desc[dtype]=(const RF_DescBase**)LFree(supp_desc[dtype]);
			n_supp_descs[dtype]=0;
			return(-2);
		}
		const RF_DescBase *dsc=component_db()->FindDescByName(name,dtype);
		// If the desc name was not found, then it is of no use for us. 
		// Dump it with brackets around and go on. 
		if(!dsc)
		{  Verbose(TDR," (%s)",name);  ++ndumped;  }
		else
		{
			// Re-alloc array if needed:
			if(n_supp_descs[dtype]>=alloc_len)
			{
				alloc_len+=16;
				const RF_DescBase **old=supp_desc[dtype];
				supp_desc[dtype]=(const RF_DescBase**)LRealloc(supp_desc[dtype],
					alloc_len*sizeof(RF_DescBase*));
				if(!supp_desc[dtype])
				{
					LFree(old);
					n_supp_descs[dtype]=0;
					return(-1);
				}
			}
			// Store it:
			supp_desc[dtype][n_supp_descs[dtype]++]=dsc;
			Verbose(TDR," %s",name);  ++ndumped;
		}
	}
	// Free unneeded array elements:
	if(alloc_len!=n_supp_descs[dtype])
	{
		const RF_DescBase **old=supp_desc[dtype];
		supp_desc[dtype]=(const RF_DescBase**)LRealloc(supp_desc[dtype],
			n_supp_descs[dtype]*sizeof(RF_DescBase*));
		if(!supp_desc[dtype] && n_supp_descs[dtype])
		{
			LFree(old);
			n_supp_descs[dtype]=0;
			return(-1);
		}
	}
	Verbose(TDR,ndumped ? " [%d]\n" : " [none]\n",n_supp_descs[dtype]);
	
	return(0);
}

// Return value: 
//   0 -> Okay, auth done
//  -1 -> must call _KickMe()
//   1 -> failed; do NOT call _KickMe(), just return 
//        (we're getting deleted)
//   2 -> call Disconnect()
int LDRClient::_ParseNowConnected(RespBuf *buf)
{
	//assert(expect_cmd==Cmd_NowConnected);  // True, asserted by caller. 
	assert(buf->content==Cmd_NowConnected);
	
	assert(auth_state!=0);
	if(auth_state!=1)  // i.e. auth_state=2 here. 
	{
		Error("Client %s: Unexpected %s packet. (Kicking it)\n",
			_ClientName().str(),LDRCommandString(Cmd_NowConnected));
		return(-1);
	}
	
	// Last auth packet. Either challenge response was accepted or not. 
	LDRNowConnected *ncon=(LDRNowConnected*)(buf->data);
	
	// Min len for deny packet: 
	size_t min_len=(((char*)&ncon->auth_code)-((char*)ncon)+
		sizeof(ncon->auth_code));
	if(ncon->length<min_len)
	{
		Error("Client %s: Illegal-sized %s packet (%u/>=%u bytes).\n",
			_ClientName().str(),LDRCommandString(Cmd_NowConnected),
			ncon->length,min_len);
		return(-1);
	}
	
	// Get code: 
	int okay=0;
	switch(ntohs(ncon->auth_code))
	{
		case CAC_Success:  okay=1;  break;
		case CAC_AuthFailed:
			Warning("Client %s: Auth failed (wrong password?!).\n",
				_ClientName().str());
			break;
		case CAC_AlreadyConnected:
			Warning("Client %s: Already connected to some other server.\n",
				_ClientName().str());
			break;
		default:
			Error("Client %s: Illegal auth code %d.\n",
				_ClientName().str(),int(ntohs(ncon->auth_code)));
			break;
	}
	if(!okay)
	{
		// This means that we now disconnect. 
		_ShutdownConnection();
		connected_state=0;  // In case it is needed...
		tdif->FailedToConnect(this);  // This will delete us. 
		return(1);  // Really, +++1 here.
	}
	
	// So, we got CAC_Success. Read more in the packet. 
	min_len=sizeof(LDRNowConnected);
	size_t rdesclen=0,fdesclen=0;
	if(ncon->length>=min_len)
	{
		rdesclen=ntohs(ncon->r_descs_size);
		fdesclen=ntohs(ncon->f_descs_size);
		min_len+=rdesclen+fdesclen;
	} // <-- NO ELSE!
	if(ncon->length<min_len)
	{
		Error("Client %s: Illegal-sized %s packet (%u/>=%u bytes).\n",
			_ClientName().str(),LDRCommandString(Cmd_NowConnected),
			ncon->length,min_len);
		return(-1);
	}
	
	c_jobs=ntohs(ncon->njobs);
	c_task_thresh_high=ntohs(ncon->task_thresh_high);
	
	if(c_jobs>tdif->P()->max_jobs_per_client || 
	   c_task_thresh_high>tdif->P()->max_high_thresh_of_client || 
	   c_task_thresh_high<c_jobs )
	{
		int old_j=c_jobs,old_t=c_task_thresh_high;
		
		if(c_task_thresh_high<c_jobs)
		{  c_task_thresh_high=c_jobs;  }
		if(c_jobs>tdif->P()->max_jobs_per_client)
		{  c_jobs=tdif->P()->max_jobs_per_client;  }
		if(c_task_thresh_high>tdif->P()->max_high_thresh_of_client)
		{  c_task_thresh_high=tdif->P()->max_high_thresh_of_client;  }
		
		Warning("Client %s: reports njobs=%d,high-thresh=%d; "
			"using %d,%d.\n",
			_ClientName().str(),
			old_j,old_t,
			c_jobs,c_task_thresh_high);
	}
	
	// Okay, we are now connected. 
	{
		HTime client_curr_time;
		LDRTime2HTime(&ncon->current_time,&client_curr_time);
		
		HTime delta_min,delta_max;
		LDRTime2HTime(&ncon->time_diff_min,&delta_min);
		LDRTime2HTime(&ncon->time_diff_max,&delta_max);
		
		HTime up_since;
		LDRTime2HTime(&ncon->starttime,&up_since);
		
		HTime idle_elapsed;
		LDRTime2HTime(&ncon->idle_elapsed,&idle_elapsed);
		
		char ltmp[12];
		if(ncon->loadval==0xffffU)
		{  strcpy(ltmp,"[unknown]");  }
		else
		{
			int lv=ntohs(ncon->loadval);
			snprintf(ltmp,12,"%d.%02d",lv/100,lv%100);
		}
		Verbose(TDR,"Client %s: Now connected: njobs=%d (parallel jobs)\n"
			"  Task-thresh: high: %d;  Current load avg: %s\n"
			"  Local client time: %s  (local; not exact)\n",
			_ClientName().str(),c_jobs,
			c_task_thresh_high,ltmp,
			client_curr_time.PrintTime(1));
		Verbose(TDR,
			"  Up since:          %s  (local)\n"
			"  Server-client time diff: %s%s",
			up_since.PrintTime(1),
			delta_min<HTime::Null ? "" : "+",delta_min.PrintElapsed(1));
		HTime delta=delta_max-delta_min;
		delta.Sub(1,HTime::msec);
		if(delta>=HTime::Null)
		{  Verbose(TDR," ... %s%s",
			delta_max<HTime::Null ? "" : "+",delta_max.PrintElapsed(1));  }
		Verbose(TDR,"\n  Client work cycle: %d  (idle time: %s)\n",
			int(ntohs(ncon->work_cycle_count)),idle_elapsed.PrintElapsed());
	}
	int rrv=_ParseNowConnected_ReadDescs(DTRender,
		(char*)(ncon->data),rdesclen);
	int frv=_ParseNowConnected_ReadDescs(DTFilter,
		(char*)(ncon->data+rdesclen),fdesclen);
	if(rrv || frv)
	{
		if(rrv==-1 || frv==-1)
		{
			_AllocFailure();
		 	return(-1);
		}
		else
		{
			Error("Client %s: format error in %s desc list.\n",
				_ClientName().str(),rrv ? "render" : "filter");
			return(-1);
		}
	}
	
	auth_state=2;
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	// Tell interface to that we are now connected. 
	tdif->SuccessfullyConnected(this);
	
	if(!n_supp_descs[DTRender] && !n_supp_descs[DTFilter])
	{
		Warning("Client %s: No known render or filter descs. No use...\n",
			_ClientName().str());
		return(2);
	}
	
	return(0);
}


// Return value: 
//   -1 -> call _KickMe()
//   -2 -> _KickMe() and mark task as failed. 
int LDRClient::_ParseFileRequest(RespBuf *buf)
{
	assert(buf->content==Cmd_FileRequest);
	LDRFileRequest *freq=(LDRFileRequest *)(buf->data);
	
	if(!tri.scheduled_to_send || 
	   tri.scheduled_to_send->task_id!=ntohl(freq->task_id) || 
	   tri.task_request_state!=TRC_WaitForResponse )
	{  Error("Client %s: Unexpected file request. (Kicking it)\n",
		_ClientName().str());  return(-1);  }
	
	if(freq->length!=sizeof(LDRFileRequest))
	{  Error("Client %s: Illegal-sized file request (%u/%u bytes).\n",
		_ClientName().str(),freq->length,sizeof(LDRFileRequest));  return(-1);  }
	
	// Okay, parse file request: 
	// Of course, only INPUT files may be requested. 
	CompleteTask *ctsk=tri.scheduled_to_send;
	tri.req_file_type=ntohs(freq->file_type);
	tri.req_file_idx=ntohs(freq->file_idx);
	int legal=0;
	bool r_ok=(ctsk->rt && ctsk->d.shall_render);
	bool f_ok=(ctsk->ft && ctsk->d.shall_filter);
	switch(tri.req_file_type)
	{
		case FRFT_None:  break;
		case FRFT_RenderIn:   legal=(tri.req_file_idx==0 && r_ok);  break;
		case FRFT_RenderOut:  legal=(tri.req_file_idx==0 && 
				(f_ok && !r_ok) || (r_ok && ctsk->rt->resume));  break;
		case FRFT_FilterOut:  /*legal=0*/  break;
		case FRFT_AddRender:
			legal=(r_ok && int(tri.req_file_idx)<tri.non_skipped_radd_files);  break;
		case FRFT_AddFilter: 
			legal=(f_ok && int(tri.req_file_idx)<tri.non_skipped_fadd_files);  break;
	}
	// Okay, get the file: 
	TaskFile tfile;
	if(legal)
	{
		tfile=GetTaskFileByEntryDesc(
			/*dir=*/(tri.req_file_type==FRFT_RenderOut && 
			        r_ok && ctsk->rt->resume) ? (+1) : (-1),
			tri.scheduled_to_send,tri.req_file_type,tri.req_file_idx);
		if(!tfile)  legal=0;
	}
	if(!legal)
	{
		Error("Client %s: Received illegal file request (type=%d, idx=%d) "
			"[frame %d]\n",
			_ClientName().str(),
			int(tri.req_file_type),int(tri.req_file_idx),
			ctsk->frame_no);
		return(-1);
	}
	
	tri.req_tfile=tfile;
	if(tri.req_file_type==FRFT_AddRender || tri.req_file_type==FRFT_AddFilter)
	{
		tri.req_file_size=tfile.GetFixedState(NULL);
		// Otherwise skip flag has to be set: 
		assert(tri.req_file_size>=0);
	}
	else
	{
		tri.req_file_size=tfile.FileLength();
		if(tri.req_file_size<0)
		{
			int errn=errno;
			Error("Client %s: Trying to stat requested file \"%s\": "
				"%s [frame %d]\n",
				_ClientName().str(),tfile.HDPathRV().str(),
				tri.req_file_size==-2 ? "no hd path" : strerror(errn),
				tri.scheduled_to_send->frame_no);
			return(-2);
		}
	}
	
	tri.task_request_state=TRC_SendFileDownloadH;
	// cpnotify_outpump_start() will be called. 
	
	return(0);
}


int LDRClient::_ParseTaskResponse(RespBuf *buf)
{
	assert(buf->content==Cmd_TaskResponse);
	LDRTaskResponse *fresp=(LDRTaskResponse*)(buf->data);
	
	if(!tri.scheduled_to_send || 
	   tri.scheduled_to_send->task_id!=ntohl(fresp->task_id) || 
	   tri.task_request_state!=TRC_WaitForResponse )
	{  Error("Client %s: Unexpected task response. (Kicking it.)\n",
		_ClientName().str());  return(-1);  }
	
	if(fresp->length!=sizeof(LDRTaskResponse))
	{  Error("Client %s: Illegal-sized task response (%u/%u bytes)\n",
		_ClientName().str(),fresp->length,sizeof(LDRTaskResponse));  return(-1);  }
	
	// Reset state: 
	CompleteTask *ctsk=tri.scheduled_to_send;
	tri.scheduled_to_send=NULL;
	tri.non_skipped_radd_files=0;
	tri.non_skipped_fadd_files=0;
	tri.task_request_state=TRC_None;
	tri.req_tfile=TaskFile();
	
	// Okay, parse task response: 
	int rv=ntohs(fresp->resp_code);
	Verbose(TDR,"Client %s: Received task reponse: %s [frame %d]\n",
		_ClientName().str(),LDRTaskResponseString(rv),
		ctsk->frame_no);
	switch(rv)
	{
		case TRC_Accepted:  // Well, that's cool. 
			break;
		case TRC_UnknownRender:
		case TRC_UnknownFilter:
		case TRC_UnknownROFormat:
		case TRC_TooManyTasks:
			// Note: this may not happen because we only send a task to 
			// a client if it supports the renderer / filter and if 
			// is does NOT have "too many tasks" assigned yet. 
			Error("Client %s: Reports \"%s\" for [frame %d]. Kicking it.\n",
				_ClientName().str(),LDRTaskResponseString(rv),
				ctsk->frame_no);
			return(-1);
		default:
			Error("Client %s: Illegal task response code %d.\n",
				_ClientName().str(),rv);
			return(-1);
	}
	
	// Tell task driver interface about it: 
	tdif->TaskLaunchResult(ctsk,rv);
	
	// cpnotify_outpump_start() will NOT be called. 
	return(0);
}


// Used by _ParseTaskDone(); see there. 
const char *LDRClient::_ParseTaskDone_QuickInfoString(bool shall,
	const TaskExecutionStatus *tes)
{
	if(!shall)  return("not assigned");
	if(tes->rflags==TTR_Unset)  return("not processed");
	if(tes->IsCompleteSuccess())  return("success");
	return("processed");
}

int LDRClient::_ParseTaskDone(RespBuf *buf)
{
	assert(buf->content==Cmd_TaskDone);
	
	LDRTaskDone *tdone=(LDRTaskDone*)(buf->data);
	
	if(tdi.done_ctsk || tdi.task_done_state!=TDC_None)
	{  Error("Client %s: Unexpected task done notify. (Kicking it.)\n",
		_ClientName().str());  return(-1);  }
	
	if(tdone->length!=sizeof(LDRTaskDone))
	{  Error("Client %s: Illegal-sized task done notify (%u/%u bytes)\n",
		_ClientName().str(),tdone->length,sizeof(LDRTaskDone));  return(-1);  }
	
	// Let's find the task the client is talking about: 
	u_int32_t task_id=ntohl(tdone->task_id);
	int frame_no=(int)ntohl(tdone->frame_no);
	CompleteTask *ctsk=tdif->FindTaskByTaskID(task_id);
	if(!ctsk || ctsk->d.ldrc!=this || ctsk->frame_no!=frame_no)
	{
		Error("Client %s: Received illegal task notify (task ID 0x%x).\n",
			_ClientName().str(),task_id);
		if(!ctsk)
		{  Error("  Task ID 0x%x [allegedly frame %d] unknown to server.\n",
			task_id,frame_no);  }
		else if(ctsk->d.ldrc!=this)
		{  Error("  Task ID 0x%x [frame %d] is%s assigned to %s.\n",
			task_id,ctsk->frame_no,
			ctsk->d.ldrc ? "" : "not",
			ctsk->d.ldrc ? ctsk->d.ldrc->_ClientName().str() : "any client");  }
		else if(ctsk->frame_no!=frame_no)
		{  Error("  Task ID 0x%x is frame %d and not frame %d as reported.\n",
			task_id,ctsk->frame_no,frame_no);  }
		return(-1);
	}
	
	// Okay, check & store passed information: 
	// Note that it is stored in TDI and not in CompleteTask here, so 
	// there is no risk we overwrite already-set success info. 
	int fail = 
		(LDRGetTaskExecutionStatus(&tdi.save_rtes,&tdone->rtes) ? 1 : 0) | 
		(LDRGetTaskExecutionStatus(&tdi.save_ftes,&tdone->ftes) ? 2 : 0) ;
	if(fail)
	{
		Error("Client %s: Task notify "/*"(task ID 0x%x) "*/"[frame %d] contains "
			"illegal %s job status.\n",
			_ClientName().str(), /*task_id,*/ ctsk->frame_no,
			(fail & 1) ? "render" : "filter");
		return(-1);
	}
	
	// The client may always return TTR_Unset info (for what reason ever, 
	// maybe to give back tasks because of SIGINT, etc). 
	// BUT, the client may never return !=TTR_Unset if it should not 
	// process this part (render/filter) of the task. In fact, such 
	// execution status would be completely bogus because we did only 
	// send the task part(s) to the client which it had to do. 
	{
		char *complain=NULL;
		if(!ctsk->d.shall_render && tdi.save_rtes.rflags!=TTR_Unset)
		{  complain="render";  }
		else if(!ctsk->d.shall_filter && tdi.save_ftes.rflags!=TTR_Unset)
		{  complain="filter";  }
		if(complain)
		{
			Error("Client %s: reports %s job status although it should not "
				"perform %sing [frame %d]\n",
				_ClientName().str(),complain,complain,ctsk->frame_no);
			return(-1);
		}
	}
	
	// We wait for file upload / final task state packet now. 
	tdi.done_ctsk=ctsk;
	// The client had to do something, right?! :
	assert(ctsk->d.shall_render || ctsk->d.shall_filter);
	tdi.task_done_state=TDC_WaitForResp;
	tdi.recv_file=TaskFile();  // be sure
	
	// Okay, dump some info. 
	// NOTE that we may NOT use TaskManager::Completely_Partly_Not_Processed(ctsk) 
	// becuase it relies on ctsk->state (ToBeRendered...TaskDone) and ctsk->{rf}tes 
	// which is not yet set. 
	Verbose(TDR,"Client %s reports task [frame %d] as done "
		"(render: %s, filter: %s).\n",
		_ClientName().str(),ctsk->frame_no,
		_ParseTaskDone_QuickInfoString(ctsk->d.shall_render,&tdi.save_rtes),
		_ParseTaskDone_QuickInfoString(ctsk->d.shall_filter,&tdi.save_ftes));
	
	// cpnotify_outpump_start() will NOT be called. 
	return(0);
}

// Retval: 0, -1, -2. 
int LDRClient::_ParseFileUpload(RespBuf *buf,const HTime *fdtime)
{
	assert(buf->content==Cmd_FileUpload);
	
	LDRFileUpload *fupl=(LDRFileUpload*)(buf->data);
	
	if(!tdi.done_ctsk || tdi.task_done_state!=TDC_WaitForResp)
	{  Error("Client %s: Unexpected file upload request. (Kicking it.)\n",
		_ClientName().str());  return(-1);  }
	
	// Read in header...
	if(fupl->length!=sizeof(LDRFileUpload))
	{
		Error("Client %s: Received illegal-sized file upload header "
			"(%u/%u bytes) [frame %d]\n",
			_ClientName().str(),
			fupl->length,sizeof(LDRFileUpload),
			tdi.done_ctsk->frame_no);
		return(-1);
	}
	int valid=0;
	u_int16_t filetype=ntohs(fupl->file_type);
	do {
		if(ntohl(fupl->task_id)!=tdi.done_ctsk->task_id)  break;
		if(filetype==FRFT_RenderOut && tdi.done_ctsk->d.shall_render)
		{  tdi.recv_file=tdi.done_ctsk->rt->outfile;  assert(tdi.done_ctsk->rt);  }
		else if(filetype==FRFT_FilterOut && tdi.done_ctsk->d.shall_filter)
		{  tdi.recv_file=tdi.done_ctsk->ft->outfile;  assert(tdi.done_ctsk->ft);  }
		else  break;
		if(!tdi.recv_file)  break;
		valid=1;
	} while(0);
	if(!valid)
	{
		Error("Client %s: Received illegal file upload header "
			"[frame %d]",
			_ClientName().str(),tdi.done_ctsk->frame_no);
		return(-1);
	}
	u_int64_t _fsize=ntohll(fupl->size);
	int64_t fsize=_fsize;
	if(u_int64_t(fsize)!=_fsize || fsize<0)
	{
		Error("Client %s: Received file upload header containing "
			"illegal file size.\n",_ClientName().str());
		return(-1);
	}
	// Okay, then let's start to receive the file. 
	// There is a verbose message below. 
	//Verbose(DBG,"Client %s: Starting to receive file (%s; %lld bytes)\n",
	//	_ClientName().str(),tdi.recv_file.HDPathRV().str(),fsize);
	
	const char *path=tdi.recv_file.HDPathRV().str();
	int rv=_FDCopyStartRecvFile(path,fsize);
	if(rv)
	{
		int errn=errno;
		if(rv==-1)  _AllocFailure();
		else if(rv==-2)
		{  Error("Client %s: Failed to start uploading/receiving requested "
			"file:\n"
			"   While opening \"%s\": %s\n",
			_ClientName().str(),path,strerror(errn));  }
		else assert(0);  // rv=-3 (fsize<0) checked above
		return(-2);
	}
	else
	{
		Verbose(TDR,"Client %s: Receiving %s file \"%s\" "
			"(%lld bytes) [frame %d]\n",_ClientName().str(),
			FileRequestFileTypeString(filetype),
			tdi.recv_file.HDPathRV().str(),
			fsize,
			tdi.done_ctsk->frame_no);
	}
	
	// Update time for statistics: 
	tdi.recv_start_time=*fdtime;
	
	in_active_cmd=Cmd_FileUpload;
	tdi.task_done_state=TDC_UploadBody;
	assert(!!tdi.recv_file);  // Was set above. MAY NOT fail. 
	
	return(0);
}	


int LDRClient::_ParseDoneComplete(RespBuf *buf)
{
	assert(buf->content==Cmd_DoneComplete);
	
	LDRDoneComplete *done=(LDRDoneComplete*)(buf->data);
	
	if(!tdi.done_ctsk || tdi.task_done_state!=TDC_WaitForResp)
	{  Error("Client %s: Unexpected completion notify. (Kicking it.)\n",
		_ClientName().str());  return(-1);  }
	
	// Read in header...
	if(done->length!=sizeof(LDRDoneComplete))
	{
		Error("Client %s: Received illegal-sized completion notify header "
			"(%u/%u bytes) [frame %d]\n",
			_ClientName().str(),
			done->length,sizeof(LDRDoneComplete),
			tdi.done_ctsk->frame_no);
		return(-1);
	}
	if(ntohl(done->task_id)!=tdi.done_ctsk->task_id)
	{
		Error("Client %s: Received illegal completion notify [frame %d]",
			_ClientName().str(),tdi.done_ctsk->frame_no);
		return(-1);
	}
	
	assert(!tdi.recv_file);  // Must be NULL here. 
	
	// Okay, we're complete. Store the success info: 
	// Of course, we may only store the success info for things the 
	// client has actually done. If we tell it to render but not to 
	// filter, then filter info may not be stored. Same for processed_by. 
	RefString proc_by_tmp;
	proc_by_tmp.sprintf(0,"LDR: %s",_ClientName().str());
	// (If that sprintf fails, oh well... you won't see the info...)
	// NOTE: I check that the info is valid when we receive it (i.e. 
	//       in _ParseTaskDone()). 
	#if TESTING
	assert(tdi.done_ctsk->d.shall_render || tdi.done_ctsk->d.shall_filter);
	#endif
	if(tdi.done_ctsk->d.shall_render)
	{
		tdi.done_ctsk->rtes.tes=tdi.save_rtes;
		tdi.done_ctsk->rtes.processed_by=proc_by_tmp;
	}
	if(tdi.done_ctsk->d.shall_filter)
	{
		tdi.done_ctsk->ftes.tes=tdi.save_ftes;
		tdi.done_ctsk->ftes.processed_by=proc_by_tmp;
	}
	
	--assigned_jobs;
	assert(assigned_jobs>=0);
	
	// Done. Tell interface. 
	tdif->TaskTerminationNotify(tdi.done_ctsk);
	
	tdi.task_done_state=TDC_None;
	tdi.done_ctsk=NULL;
	
	return(0);
}	


int LDRClient::_ParseControlResponse(RespBuf *buf)
{
	assert(buf->content==Cmd_ControlResponse);
	
	static const char *_illsize_msg=
		"Client %s: Received illegal-sized control "
		"response%s%s%s (%u/>=%u bytes)\n";
	
	LDRClientControlResponse *resp=(LDRClientControlResponse*)(buf->data);
	assert(resp->command==Cmd_ControlResponse);  // can be left away
	if(resp->length<sizeof(LDRClientControlResponse))
	{
		Error(_illsize_msg,
			_ClientName().str(),
			"","","",
			resp->length,sizeof(LDRClientControlResponse));
		return(-1);
	}
	
	size_t min_len=0;
	size_t data_len=0;
	LDRClientControlCommand l_cccmd=
		(LDRClientControlCommand)(ntohs(resp->ctrl_cmd));
	int easy_type=0;
	switch(l_cccmd)
	{
		case LCCC_Kill_UserInterrupt:
		case LCCC_Kill_ServerError:
		case LCCC_StopJobs:
		case LCCC_ContJobs:
		case LCCC_PingPong:
		case LCCC_ClientQuit:
			easy_type=1;
			min_len=sizeof(LDRClientControlResponse);
			data_len=0;
			break;
		case LCCC_CN_NoMoreTasks:
			easy_type=2;
			min_len=sizeof(LDRClientControlResponse);
			data_len=0;
			break;
		case LCCC_GiveBackTasks:
			easy_type=3;
			data_len=sizeof(LDRCCC_GBT_Resp_Data);
			min_len=sizeof(LDRClientControlResponse)+data_len;
			break;
		//default: min_len stays 0
	}
	if(!min_len)
	{
		Error("Client %s: Received unknown control response (%d, %u bytes)\n",
			_ClientName().str(),int(l_cccmd),resp->length);
		return(-1);
	}
	if(resp->length<min_len)
	{
		Error(_illsize_msg,
			_ClientName().str(),
			" (",LDRClientControlCommandString(l_cccmd),")",
			resp->length,min_len);
		return(-1);
	}
	
	Verbose(DBGV,"Client %s: Received control cmd (resp): %s\n",
		_ClientName().str(),LDRClientControlCommandString(l_cccmd));
	
	// First, check if the response corresponds to a command in the 
	// command queue and remove it there: 
	if(easy_type==1 || easy_type==3)
	{
		// Okay, got response, remove the entry from the queue. 
		int rv=cmd_queue.RespRemoveEntry(l_cccmd,ntohs(resp->seq));
		if(rv)
		{
			CommandQueue::CQEntry *expct=cmd_queue.queue_waitresp.first();
			Error("Client %s: Received unexpected control response: "
				"recv: (%d/%s) expect: (%d/%s) (rv=%d)\n",
				_ClientName().str(),
				ntohs(resp->seq),LDRClientControlCommandString(l_cccmd),
				expct ? int(expct->seq) : -1,
				expct ? LDRClientControlCommandString(
					(LDRClientControlCommand)expct->cmd) : "[none]",
				rv);
			return(-1);
		}
		
		// So, as the entry was actually in the queue, we received a valid 
		// response. Update the timeout: 
		_UpdateRespTimeout();
	}
	
	// Next, see what we can to for the response: 
	if(easy_type==1)
	{
		// And tell the manager if needed: 
		if(l_cccmd==LCCC_PingPong)
		{
			--queued_ping_requests;
			if(queued_ping_requests<0)  queued_ping_requests=0;  // should never happen
		}
		else if(l_cccmd==LCCC_ClientQuit)
		{
			// We have sent a quit cmd (it was in the queue), so
			// this flag has to be set: 
			assert(client_quit_queued==2);
			client_quit_queued=3;
		}
		else
		{  tdif->HandleControlCommandResponse(this,l_cccmd);  }
	}
	else if(easy_type==2)
	{
		// This is sent by the client without a request by the 
		// server thus we will not find it in the waitresp queue. 
		_UpdateRespTimeout();
		
		if(l_cccmd==LCCC_CN_NoMoreTasks)
		{
			// The client does not want more tasks. 
			// It is enough if we set the apropriate flag: 
			Verbose(TDR,"Client %s: Does not want any more tasks. Uh...\n",
				_ClientName().str());
			client_no_more_tasks=1;
		}
		else assert(0);  // If fail: just implement it!
	}
	else if(easy_type==3)
	{
		// With data of constant length. 
		assert(l_cccmd==LCCC_GiveBackTasks);
		
		LDRCCC_GBT_Resp_Data *respdata=(LDRCCC_GBT_Resp_Data*)(resp->data);
		int n_todo=ntohl(respdata->n_todo);
		int n_proc=ntohl(respdata->n_proc);
		int n_done=ntohl(respdata->n_done);
		
		Verbose(TDR,"Client %s: Will give back tasks. "
			"(todo: %d, proc: %d, done: %d)\n",
			_ClientName().str(),n_todo,n_proc,n_done);
		
		// Well, we trust the client that it will actually give the 
		// tasks back... After all, LDR has to trust the client in some 
		// way; nobody can prevent it from returning entirely black 
		// frames as "completely processed". 
	}
	else
	{
		// Other commands: (None yet)
		// Do what is apropriate, but always: remove from queue. 
		assert(0);
	}
	
	return(0);
}	


// Return value: -1 -> _KickMe() called; 0 -> not handled; 1 -> handled
int LDRClient::_AuthConnFDNotify(FDBase::FDInfo *fdi)
{
	if(fdi->revents & POLLHUP)
	{
		_ClientDisconnectMessage();
		_KickMe();  return(-1);
	}
	if(fdi->revents & POLLIN)
	{
		// Read protocol header: 
		// We want to get it atomically. 
		LDRHeader hdr;
		int rv=_AtomicRecvData(&hdr,sizeof(hdr),sizeof(hdr));
		if(rv<0)
		{  _KickMe();  return(-1);  }
		
		rv=_HandleReceivedHeader(&hdr);
		if(rv<0)
		{  _KickMe();  }
		
		// Update response time: 
		last_response_time=*fdi->current;
		
		return(rv);
	}
	
	// _AuthConnFDNotify() is (currently) only called for POLLIN. 
	// We may NOT reach here. 
	assert(0);  // If caught: SIMPLE PROGRAMMING ERROR. 
	return(0);
}

// Called via TaskDriverInterface_LDR: 
int LDRClient::fdnotify2(FDBase::FDInfo *fdi)
{
	Verbose(DBG,"--<LDRClient:fdnotify2>--<fd=%d, ev=0x%x, rev=0x%x>--\n",
		fdi->fd,int(fdi->events),int(fdi->revents));
	
	#if 0
	assert(fdi->fd==sock_fd && fdi->pollid==pollid);
	#else
	if(pollid!=fdi->pollid || sock_fd!=fdi->fd)
	{
		fprintf(stderr,
			"OOPS: pollid=%p, fdi->pollid=%p, sock_fd=%d, fdi->fd=%d\n",
			pollid,fdi->pollid,sock_fd,fdi ? fdi->fd : -9999);
		if(!pollid && sock_fd<0)
		{
			// We should not be here. HLib's fault (probably), if we do. 
			return(0);
		}
		assert(0);
	}
	#endif
	assert(connected_state);  /* Otherwise: we may not be here; we have no fd. */
	
	if(connected_state==1)  // waiting for response to connect(2)
	{
		if(_DoFinishConnect(fdi))
		{
			// Failed; we are not connected; give up. 
			connected_state=0;
			ShutdownFD(pollid);
			assert(pollid==NULL);
			tdif->FailedToConnect(this);  // This will delete us. 
			return(0);
		}
		return(0);
	}
	
	// We're connected. 
	if(auth_state==0)   // auth_state==1 uses FDCopy facility
	{
		// Still not yet authenticated. 
		if(_DoAuthHandshake(fdi))
		{
			// Failed. Give up. 
			connected_state=0;
			ShutdownFD(pollid);
			tdif->FailedToConnect(this);  // This will delete us. 
			return(0);
		}
		return(0);
	}
	
	// NOTE: next_send_cmd in no longer used if auth is done. 
	assert(next_send_cmd==Cmd_NoCommand);
	// expect_cmd may be Cmd_NowConnected in case auth_state==1. 
	
	// We have passed auth. We are the server the client listenes to. 
	// All the conversation is now made using the FD copy facility. 
	int handeled=0;
	if(fdi->revents & (POLLIN | POLLHUP))
	{
		handeled=_AuthConnFDNotify(fdi);
		if(handeled<0)  // _KickMe() called. 
		{  return(0);  }
	}
	if(fdi->revents & POLLOUT)
	{
		Error("fdnotify2(): WHY ARE WE HERE??? "
			"(Probably bug, please report)\n");
		FDChangeEvents(fdi->pollid,0,POLLOUT);
		return(0);
	}
	
	if(fdi->revents & POLLHUP)
	{
		_ClientDisconnectMessage();
		handeled=1;
		_KickMe();
		return(0);
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
			Error("Client %s: Connection error: %s (Kicking client)\n",
				_ClientName().str(),strerror(errval));
		}
		else
		{
			Error("Client %s: unexpected revents=0x%x. Kicking client.\n",
				_ClientName().str(),int(fdi->revents));
		}
		_KickMe();
	}
	
	return(0);
}


// Returns static data. 
char *_BytesPerSecond_Str(const HTime &start,const HTime *end,
	int64_t transferred,double *_sec)
{
	static char bps_str[48];
	if(start.IsInvalid())
	{
		strcpy(bps_str," [timer invalid!]");    // <-- Should never happen. 
		*_sec=-1.0;
	}
	else
	{
		double sec=( (end ? *end : HTime::MostCurr) - start ).GetD(HTime::seconds);
		*_sec=sec;
		if((sec<0.0 ? -sec : sec)<0.001)
		{  *bps_str='\0';  }
		else
		{
			double bps=double(transferred)/sec;
			bool larger=bps>10240.0;
			snprintf(bps_str,48," (%.0f%sb/s)",
				larger ? bps/1024.0 : bps,
				larger ? "k" : "");
		}
	}
	return(bps_str);
}


// Return value: 1 -> called _KickMe(); 0 -> normal
int LDRClient::cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi)
{
	// ---------<FIRST PART: HANDLE TERMINATION OF CURRENT REQUEST>---------
	
	// This will be decreased for files below. 
	++out.stat.count_headers;   // statistics
	
	Verbose(DBGV,"Client %s: Sending done: %s\n",
		_ClientName().str(),LDRCommandString(out_active_cmd));
	
	switch(out_active_cmd)
	{
		case Cmd_TaskRequest:
		{
			assert(tri.task_request_state==TRC_SendTaskRequest && 
				tri.scheduled_to_send);
			
			// NOTE: scheduled_to_send!=NULL, i.e. still the task we're talking 
			//       about. But task_request_state=TRC_WaitForResponse so that we do not 
			//       (accidentally) send it again. When client wants that we 
			//       send a file, then this has to be set accordingly. 
			tri.task_request_state=TRC_WaitForResponse;
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
			// Poll(events=0)? - not necessary (FDCopyPump does that). 
		} break;
		case Cmd_FileDownload:
		{
			#if TESTING
			if((tri.task_request_state!=TRC_SendFileDownloadH && 
		    	tri.task_request_state!=TRC_SendFileDownloadB ) || 
			   !tri.scheduled_to_send)
			{  assert(0);  }
			#endif
			
			assert(outpump_lock==IOPL_Download);
			
			if(tri.task_request_state==TRC_SendFileDownloadH)
			{
				// Okay, LDRFileDownload header was sent. Now, we send 
				// the file itself. (next cpnotify_outpump_start()). 
				tri.task_request_state=TRC_SendFileDownloadB;
			}
			else  // TRC_SendFileDownloadB
			{
				--out.stat.count_headers;   // statistics
				++out.stat.count_files;   // statistics
				
				// Okay, if we used the FD->FD pump, we must close the 
				// input file again: 
				if(cpi->pump->Src()->Type()==FDCopyIO::CPT_FD)
				{
					assert(cpi->pump==out.pump_fd && cpi->pump->Src()==out.io_fd);
					if(CloseFD(out.io_fd->pollid)<0)
					{  hack_assert(0);  }  // Actually, this may not fail, right?
				}
				
				{
					// The non-FD->FD pump is obly used for zero-sized files. 
					int64_t transferred=
						cpi->pump->Src()->Type()==FDCopyIO::CPT_FD ? 
							out.io_fd->transferred : (int64_t)0;
					double sec;
					const char *bps_str=_BytesPerSecond_Str(
						tri.send_start_time,cpi->fdtime,transferred, &sec);
					bool smaller10=sec<10.0;
					Verbose(TDR,"Client %s: Sending \"%s\" completed: "
						"%lld/%lld bytes in %ld%ssec%s [frame %d]\n",
						_ClientName().str(),
						tri.req_tfile.HDPathRV().str(),
						transferred, tri.req_file_size,
						smaller10 ? long(sec*1000+0.5) : long(sec+0.5),
						smaller10 ? "m" : "",
						bps_str,
						tri.scheduled_to_send->frame_no);
				}
				
				tri.task_request_state=TRC_WaitForResponse;
				tri.req_tfile=TaskFile();
				
				tri.send_start_time.SetInvalid();
				
				outpump_lock=IOPL_Unlocked;
			}
			
			// This is needed so that cpnotify_outpump_start() won't go mad. 
			send_buf.content=Cmd_NoCommand;
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
		} break;
		case Cmd_ControlRequest:
		{
			assert(!cmd_queue.queue_to_send.is_empty());
			assert(sent_cmd_queue_ent);
			
			if(sent_cmd_queue_ent->cmd==LCCC_ClientQuit)
			{
				assert(client_quit_queued==1);
				client_quit_queued=2;
				
				Verbose(TDR,"  Sent quit to client %s.\n",
					_ClientName().str());
			}
			
			cmd_queue.EntryProcessed(sent_cmd_queue_ent,cpi->fdtime);
			sent_cmd_queue_ent=NULL;
			
			_UpdateRespTimeout();
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		default:
		{
			Error("cpnotify_outpump_done: Hack on... "
				"(implementation incomplete)\n");
			assert(0);
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
	}
	
	return(0);
}

// Return value: 1 -> called _KickMe(); 0 -> normal
int LDRClient::cpnotify_outpump_start()
{
	if(out.ioaction!=IOA_None || out_active_cmd!=Cmd_NoCommand)
	{
		// This only happens if we're not called from cpnotify() but 
		// from somewhere else. (Protected by assert() in cpnotify().) 
		// It means that another copy notify is running and this 
		// function will be called again when it is over. 
		return(0);
	}
	
	if(client_quit_queued==2)
	{
		// We sent the quit command. Then, we will not send 
		// anything more but wait for response and disconnect. 
		return(0);
	}
	
	// ---------<SECOND PART: LAUNCH NEW REQUEST IF NEEDED>---------
		
	// See what we can send...
	if(!outpump_lock && !cmd_queue.queue_to_send.is_empty())
	{
		// Control commands have precedence (unless of course, the outpump 
		// is locked (because we just sent a header (e.g. file download) 
		// and now MUST send the file body). 
		assert(!sent_cmd_queue_ent);  // may not be active here (bug)
		CommandQueue::CQEntry *cqent=cmd_queue.queue_to_send.first();
		int fail=0;
		do {
			size_t need_len=sizeof(LDRClientControlRequest)+cqent->datalen;
			assert(send_buf.content==Cmd_NoCommand);
			if(_ResizeRespBuf(&send_buf,need_len))
			{  fail=-1;  break;  }
			
			send_buf.content=Cmd_ControlRequest;
			LDRClientControlRequest *pack=
				(LDRClientControlRequest*)(send_buf.data);
			pack->length=need_len;  // host order
			pack->command=Cmd_ControlRequest;  // host order
			pack->ctrl_cmd=htons(cqent->cmd);
			pack->seq=htons(cqent->seq);
			if(cqent->datalen)
			{  memcpy(pack->data,cqent->data,cqent->datalen);  }
			
			// Okay, make ready to send it: 
			fail=_FDCopyStartSendBuf(pack);  // Currently never fails. 
			if(fail)  break;
			
			sent_cmd_queue_ent=cqent;
		} while(0);
		if(fail)
		{
			const char *msg=NULL;
			switch(fail)
			{
				case -1:  msg=cstrings.allocfail;  break;
				default:  assert(0);  // huh?
			}
			Error("Client %s: Sending control command (%s): %s. Kicking.\n",
				_ClientName().str(),
				LDRClientControlCommandString(
					(LDRClientControlCommand)cqent->cmd),
				msg);
			_KickMe();  return(1);
		}
	}
	else if(tri.scheduled_to_send)
	{
		// Okay; we shall send this task or one of the needed files. 
		if(!outpump_lock && tri.task_request_state==TRC_SendTaskRequest)
		{
			// Immediately before sending the main task data, 
			// check the additional files. 
			if(tri.scheduled_to_send->rt && tri.scheduled_to_send->d.shall_render)
			{  _InspectAndFixAddFiles(&tri.scheduled_to_send->radd,
				tri.scheduled_to_send);  }
			if(tri.scheduled_to_send->ft && tri.scheduled_to_send->d.shall_filter)
			{  _InspectAndFixAddFiles(&tri.scheduled_to_send->fadd,
				tri.scheduled_to_send);  }
			
			// Okay, we have to send the main task data. 
			int fail=_Create_TaskRequest_Packet(&send_buf);
			if(!fail)
			{
				fail=_FDCopyStartSendBuf((LDRHeader*)(send_buf.data));
				// Currently, _FDCopyStartSendBuf() only returns 0, but I add 
				// 100 here to be able to distinguish the error codes (once 
				// needed).
				if(fail)  fail+=100;
			}  // NO <else> !
			if(fail)
			{
				// Failure. 
				const char *msg=NULL;
				switch(fail)
				{
					case -1:  msg=cstrings.allocfail;  break;
					// FrameClockVal: UNUSED  (retval -2 cannot happen)
					//case -2:  msg="failed to convert frame clock "
					//	"to 32bit representation";  break;
					//case 99: <--  _FDCopyStartSendBuf() returns -1
					default:  assert(0);  // huh?
				}
				Error("Client %s: Sending task request [frame %d]: %s. "
					"Kicking.\n",
					_ClientName().str(),tri.scheduled_to_send->frame_no,msg);
				_KickMe(fail==-2 ? tri.scheduled_to_send : NULL,EF_JK_LDRFail);
				return(1);
			}
		}
		else if(!outpump_lock && tri.task_request_state==TRC_SendFileDownloadH)
		{
			// Well, then let's send the file request response. 
			// First, we send the header, then we copy the data: 
			int fail=0;
			do {
				assert(send_buf.content==Cmd_NoCommand);
				if(_ResizeRespBuf(&send_buf,sizeof(LDRFileDownload)))
				{  fail=-1;  break;  }
				
				send_buf.content=Cmd_FileDownload;
				LDRFileDownload *pack=(LDRFileDownload*)(send_buf.data);
				pack->length=sizeof(LDRFileDownload);  // host order
				pack->command=Cmd_FileDownload;  // host order
				pack->task_id=htonl(tri.scheduled_to_send->task_id);
				pack->file_type=htons(tri.req_file_type);
				pack->file_idx=htons(tri.req_file_idx);
				pack->size=htonll(tri.req_file_size);
				
				// Okay, make ready to send it: 
				fail=_FDCopyStartSendBuf(pack);  // Currently never fails. 
				if(fail)  break;
				
				// When pump is running, lock here so that data comes 
				// directly after header. 
				outpump_lock=IOPL_Download;
			} while(0);
			if(fail)
			{
				const char *msg=NULL;
				switch(fail)
				{
					case -1:  msg=cstrings.allocfail;  break;
					default:  assert(0);  // huh?
				}
				Error("Client %s: Sending file req response [frame %d]: %s. "
					"Kicking.\n",
					_ClientName().str(),tri.scheduled_to_send->frame_no,msg);
				_KickMe();  return(1);
			}
			else
			{
				Verbose(TDR,"Client %s: Sending %s file \"%s\" "
					"(%lld bytes) [frame %d]\n",_ClientName().str(),
					FileRequestFileTypeString(tri.req_file_type),
					tri.req_tfile.HDPathRV().str(),
					tri.req_file_size,
					tri.scheduled_to_send->frame_no);
			}
		}
		else if(tri.task_request_state==TRC_SendFileDownloadB)
		{
			assert(outpump_lock==IOPL_Download);
			
			// Finally, send the file body. 
			
			// If this assert fails, then we're in trouble. 
			// The stuff should have been set earlier. 
			assert(!!tri.req_tfile);
			
			assert(out_active_cmd==Cmd_NoCommand);
			int rv=_FDCopyStartSendFile(tri.req_tfile.HDPathRV().str(),
				tri.req_file_size);
			if(rv)
			{
				if(rv==-1)
				{  _AllocFailure();  }
				else if(rv==-2)
				{
					int errn=errno;
					Error("Client %s: Failed to open requested file \"%s\": "
						"%s [frame %d]. Kicking.\n",
						_ClientName().str(),tri.req_tfile.HDPathRV().str(),
						strerror(errn),tri.scheduled_to_send->frame_no);
				}
				else assert(0);  // rv=-3 may not happen here 
				_KickMe(rv==-2 ? tri.scheduled_to_send : NULL,
					EF_JK_LDRFail_File);
				return(1);
			}
			
			// Note down time for nice statistics...
			tri.send_start_time.SetCurr();
			
			out_active_cmd=Cmd_FileDownload;
		}
		//else wait for client
	}
	
	return(0);
}

// Return value: 1 -> called _KickMe(); 0 -> normal; 2 -> Disconnect called. 
int LDRClient::cpnotify_inpump(FDCopyBase::CopyInfo *cpi)
{
	// NOTE: This looks like something strange but it is an 
	// internal error if it fails. Because we tell the pump to 
	// copy exactly LDRHeader->length - sizeof(LDRHeader) bytes 
	// and if it reports SCLimit here, then they have to be 
	// there. 
	// NOTE: The exception for file upload body is necessary 
	//       becase for files of size 0, we use FD->Buf pump with 
	//       buffer of size 1. (See netiobase.cpp for details.) 
	#if TESTING
	if(cpi->pump==in.pump_s && 
	   !(in_active_cmd==Cmd_FileUpload && tdi.task_done_state==TDC_UploadBody) )
	{
		FDCopyIO_Buf *_dst=(FDCopyIO_Buf*)(cpi->pump->Dest());
		if(_dst->bufdone+sizeof(LDRHeader)!=
			((LDRHeader*)(recv_buf.data))->length)
		{  assert(0);  }
	}
	#endif
	
	int dont_start_outpump=0;
	
	switch(in_active_cmd)
	{
		case Cmd_NowConnected:
		{
			// Do NOT! assert(auth_state==2) HERE. Check done gracefully by 
			// _ParseNowConnected(). 
			// This must still be set: 
			assert(expect_cmd==Cmd_NowConnected);
			
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseNowConnected(&recv_buf);
			
			// Now, the auth is over and we can set expect_cmd=Cmd_NoCommand. 
			expect_cmd=Cmd_NoCommand;
			
			recv_buf.content=Cmd_NoCommand;
			
			if(rv)
			{
				if(rv<0)
				{  _KickMe();  }
				if(rv==2)
				{
					int rrv=Disconnect();
					if(rrv==0 && client_quit_queued)
					{  return(2);  }
					// Otherwise _KickMe() was called...
				}
				// rv=1 -> Not actually _KickMe() called but we're 
				//         getting deleted.
				return(1);
			}
		} break;
		case Cmd_FileRequest:
		{
			assert(cpi->pump==in.pump_s);  // can be left away
			
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseFileRequest(&recv_buf);
			if(rv<0)
			{
				_KickMe(rv==-2 ? tri.scheduled_to_send : NULL,
					EF_JK_LDRFail_File);
				return(1);
			}
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_TaskResponse:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseTaskResponse(&recv_buf);
			if(rv<0)
			{  _KickMe();  return(1);  }
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
			// dont_start_outpump=1: ?!
		} break;
		case Cmd_TaskDone:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseTaskDone(&recv_buf);
			if(rv<0)
			{  _KickMe();  return(1);  }
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
			cpnotify_outpump_start();
			// dont_start_outpump=1 previously set with comment: 
			//   NO cpnotify_outpump_start() because we're waiting. 
		} break;
		case Cmd_FileUpload:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			if(tdi.task_done_state==TDC_UploadBody)
			{
				assert(tdi.done_ctsk);  // Must be set if tdi.task_done_state==TDC_UploadBody. 
				
				// Read in body (complete). 
				++in.stat.count_files;   // statistics
				
				{
					// The non-FD->FD pump is obly used for zero-sized files. 
					int64_t transferred=
						cpi->pump->Dest()->Type()==FDCopyIO::CPT_FD ? 
							in.io_fd->transferred : (int64_t)0;
					double sec;
					const char *bps_str=_BytesPerSecond_Str(
						tdi.recv_start_time,cpi->fdtime,transferred, &sec);
					bool smaller10=sec<10.0;
					Verbose(TDR,"Client %s: Receiving \"%s\" done: "
						"%lld bytes in %ld%ssec%s [frame %d]\n",
						_ClientName().str(),
						tdi.recv_file.HDPathRV().str(),
						transferred,
						smaller10 ? long(sec*1000+0.5) : long(sec+0.5),
						smaller10 ? "m" : "",
						bps_str,
						tdi.done_ctsk->frame_no);
				}
				
				// If we used the FD->FD pump, we must close the output file. 
				if(cpi->pump->Dest()->Type()==FDCopyIO::CPT_FD)
				{
					assert(cpi->pump==in.pump_fd && cpi->pump->Dest()==in.io_fd);
					if(CloseFD(in.io_fd->pollid)<0)
					{
						int errn=errno;
						Error("Client %s: While closing \"%s\": %s\n",
							_ClientName().str(),
							tdi.recv_file.HDPathRV().str(),strerror(errn));
						_KickMe(tdi.done_ctsk,EF_JK_LDRFail_File);
						return(1);
					}
				}
				
				tdi.recv_start_time.SetInvalid();
				
				tdi.recv_file=TaskFile();
				tdi.task_done_state=TDC_WaitForResp;
			}
			else
			{
				assert(cpi->pump==in.pump_s);  // can be left away
				
				int rv=_ParseFileUpload(&recv_buf,cpi->fdtime);
				if(rv<0)
				{
					_KickMe(rv==-2 ? tdi.done_ctsk : NULL,EF_JK_LDRFail_File);
					return(1);
				}
				assert(rv==0);
				
				// dont_start_outpump=1; previously set with comment 
				//   "because we're waiting for file body." WHAT A REASON?!
			}
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_DoneComplete:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseDoneComplete(&recv_buf);
			if(rv<0)
			{  _KickMe();  return(1);  }
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_ControlResponse:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseControlResponse(&recv_buf);
			if(rv<0)
			{  _KickMe();  return(1);  }
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		default:
			// This is an internal error. Only known packets may be accepted 
			// in _HandleReceivedHeader(). 
			Error("Done; hack on... (implementation incomplete)\n");
			assert(0);
	}
	
	if(dont_start_outpump)
	{  return(0);  }
	return(cpnotify_outpump_start()==1 ? 1 : 0);
}


// Handle status code reported by cpnotify. 
// NOTE: cpnotify_outpump_done() contains the "success path", 
//       while this function contains the "error path". 
// Return value:
//   0 -> OK
//   1 -> _KickMe() called
int LDRClient::cpnotify_handle_errors(FDCopyBase::CopyInfo *cpi)
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
		Error("Client %s: Filesys/network IO error: %s\n",
			_ClientName().str(),CPNotifyStatusString(cpi));
		cpi->scode=(FDCopyPump::StatusCode)(cpi->scode|FDCopyPump::SCFinal);
		int errval=GetSocketError(sock_fd);
		if(errval)
		{  Error("     network socket: %s%s (%d)\n",
			errval==-1 ? "getsockopt failed: " : strerror(errval),
			errval==-1 ? strerror(errno) : "",errval);  }
		if(cpi->pump==out.pump_fd && out_active_cmd==Cmd_FileDownload && 
			tri.task_request_state==TRC_SendFileDownloadB)
		{
			assert(tri.scheduled_to_send);  // can be left away (but we use it below)
			Error("   after %lld/%lld bytes while reading \"%s\". [frame %d]\n",
				out.io_fd->transferred,tri.req_file_size,
				tri.req_tfile.HDPathRV().str(),tri.scheduled_to_send->frame_no);
			// Closing file done by _KickMe(). 
		}
		if(cpi->pump==in.pump_fd && in_active_cmd==Cmd_FileUpload && 
			tdi.task_done_state==TDC_UploadBody)
		{
			assert(tdi.done_ctsk);  // otherwise: BUG
			Error("   after %lld/%lld bytes while writing \"%s\". [frame %d]\n",
				in.io_fd->transferred,tdi.recv_file.GetFixedState(),
				tdi.recv_file.HDPathRV().str(),tdi.done_ctsk->frame_no);
			// Closing file done by _KicKMe(). 
		}
		_KickMe();  return(1);
	}
	else
	{
		if(cpi->scode & FDCopyPump::SCEOI)
		{
			// End of input...
			if(cpi->pump==in.pump_fd || cpi->pump==in.pump_s)
			{
				// This means, the client disconnected. 
				_ClientDisconnectMessage();
				_KickMe();  return(1);
			}
			else if(cpi->pump==out.pump_fd)
			{
				// This means, we are sending a file and encountered EOF. 
				// Probably early EOF because the file changed its size. 
				if(out_active_cmd==Cmd_FileDownload && 
				   tri.task_request_state==TRC_SendFileDownloadB)
				{
					assert(tri.scheduled_to_send);  // can be left away
					assert(outpump_lock==IOPL_Download);  // can be left away
					
					assert(cpi->pump->Src()==out.io_fd);
					Error("Client %s: Early EOF (after %lld/%lld bytes) while "
						"serving \"%s\" for download. "
						"Kicking client. [frame %d]\n",
						_ClientName().str(),
						out.io_fd->transferred,tri.req_file_size,
						tri.req_tfile.HDPathRV().str(),
						tri.scheduled_to_send->frame_no);
					
					// As we used the FD->FD pump, we must close the input file. 
					// Done by _KickMe(): 
					
					_KickMe(tri.scheduled_to_send,EF_JK_LDRFail_File);
					return(1);
				}
				else ++illegal;
			}
			else ++illegal;
		}
		if(cpi->scode & FDCopyPump::SCEOO)
		{
			// Cannot send any more...
			if(cpi->pump==out.pump_fd || cpi->pump==out.pump_s)
			{
				// This means, the client disconnected. 
				_ClientDisconnectMessage();
				_KickMe();  return(1);
			}
			else if(cpi->pump==in.pump_fd)
			{
				if(in_active_cmd==Cmd_FileUpload && 
					tdi.task_done_state==TDC_UploadBody)
				{
					assert(tdi.done_ctsk);  // otherwise: BUG
					
					assert(cpi->pump->Dest()==in.io_fd);  // otherwise: BUG!
					Error("Client %s: Mysterious output EOF (after %lld/%lld "
						"bytes) while receiving \"%s\" from upload. "
						"Kicking client. [frame %d]\n",
						_ClientName().str(),
						in.io_fd->transferred,tdi.recv_file.GetFixedState(),
						tdi.recv_file.HDPathRV().str(),
						tdi.done_ctsk->frame_no);
					
					// As we used the FD->FD pump, we must close the output file. 
					// Done by _KickMe(). 
					
					_KickMe();  return(1);
				}
				else ++illegal;
			}
			else ++illegal;
		} // if(EOO)
	}
	
	if(illegal)
	{
		const char *pumpstr="???";
		if(cpi->pump==in.pump_s)  pumpstr="in.pump_s";
		else if(cpi->pump==in.pump_fd)  pumpstr="in.pump_fd";
		else if(cpi->pump==out.pump_s)  pumpstr="out.pump_s";
		else if(cpi->pump==out.pump_fd)  pumpstr="out.pump_fd";
		Error("Client %s: Internal (?) error: Received illegal cpnotify.\n"
			"  pump: %s, src=%d, dest=%d; code: %s\n",
			_ClientName().str(),
			pumpstr,
			cpi->pump->Src() ? cpi->pump->Src()->Type() : (-1),
			cpi->pump->Dest() ? cpi->pump->Dest()->Type() : (-1),
			CPNotifyStatusString(cpi));
		int errval=GetSocketError(sock_fd);
		if(errval)
		{  Error("     network socket: %s%s (%d)\n",
			errval==-1 ? "getsockopt failed: " : strerror(errval),
			errval==-1 ? strerror(errno) : "",errval);  }
		// For now, I do a hack_assert(0), because I want to catch these 
		// in case they ever happen: 
		hack_assert(0);
		_KickMe();  return(1);
	}
	
	return(0);
}


int LDRClient::cpnotify(FDCopyBase::CopyInfo *cpi)
{
	Verbose(DBG,"--<cpnotify>--<%s%s: %s>--\n",
		(cpi->pump==in.pump_s || cpi->pump==in.pump_fd) ? "IN" : "OUT",
		(cpi->pump==in.pump_s || cpi->pump==out.pump_s) ? "buf" : "fd",
		CPNotifyStatusString(cpi));
	
	// We are only interested in FINAL codes. 
	if(!(cpi->scode & FDCopyPump::SCFinal))
	{  return(0);  }
	
	// Update statistics: 
	if(cpi->pump==in.pump_s || cpi->pump==in.pump_fd)
	{  in.stat.tot_transferred+=u_int64_t(in.io_sock->transferred);  }
	else if(cpi->pump==out.pump_s || cpi->pump==out.pump_fd)
	{  out.stat.tot_transferred+=u_int64_t(out.io_sock->transferred);  }
	else assert(0);  // ?!
	
	if((cpi->scode & FDCopyPump::SCKilled))
	{
		// Killed? Dann wird's das schon gebraucht haben! ;)
		if(pollid && (cpi->pump==in.pump_s || cpi->pump==in.pump_fd))
		{  _DoPollFD(POLLIN,0);  }
		return(0);
	}
	
	// NOTE: next_send_cmd in no longer used if auth is done. 
	assert(next_send_cmd==Cmd_NoCommand);
	assert(auth_state==2 || expect_cmd==Cmd_NowConnected);
	
	// Check error conditions etc. 
	if(cpnotify_handle_errors(cpi))
	{
		// This is probably not needed but should not harm. 
		if(pollid && (cpi->pump==in.pump_s || cpi->pump==in.pump_fd))
		{  _DoPollFD(POLLIN,0);  }
		return(0);
	}
	
	do {
		if(cpi->pump==in.pump_s || cpi->pump==in.pump_fd)
		{
			int rv=cpnotify_inpump(cpi);
			// rv=1 -> _KickMe() called or we're getting deleted. 
			// rv=2 -> Disconnect() called. 
			last_response_time=*cpi->fdtime;   // Update response time. 
			
			// We're always listening to the client. 
			if(in_active_cmd==Cmd_NoCommand && pollid)
			{  _DoPollFD(POLLIN,0);  }
			
			if(rv==1)  break;
		}
		else if(cpi->pump==out.pump_s || cpi->pump==out.pump_fd)
		{
			// If we're here, we must have authenticated. 
			assert(auth_state==2);
			
			if(cpnotify_outpump_done(cpi))  break;  // _KickMe() called
			
			assert(out.ioaction==IOA_None);  // If FDCopyPump is running, we may not be here. 
			if(cpnotify_outpump_start())  break;   // _KickMe() called
		}
		else assert(0);
	} while(0);
	
	return(0);
}


int LDRClient::cpprogress(FDCopyBase::ProgressInfo *pi)
{
	Verbose(DBG,"--<cpprogress>--<%s=%d bytes>--\n",
		pi->moved_bytes<0 ? "IN" : "OUT",
		pi->moved_bytes<0 ? (-pi->moved_bytes) : pi->moved_bytes);
	
	// Update IO time if sufficient amount of data was transferred 
	// at this call. 256 bytes is sufficient, do not make that larger 
	// than the MTU to be sure...
	// NOTE: pi->moved_bytes<0 -> read; >0 -> write
	if(pi->moved_bytes<=-256 || pi->moved_bytes>=256)
	{  last_datapump_io_time=*pi->fdtime;  }
	
	return(0);
}


// Inspect all additional files and call SetFixedState() or SetSkipFlag(1). 
// (skip flag is reset before). 
void LDRClient::_InspectAndFixAddFiles(CompleteTask::AddFiles *af,
	CompleteTask *ctsk_for_msg)
{
	for(int i=0; i<af->nfiles; i++)
	{
		TaskFile *tf=&(af->tfile[i]);
		tf->ClearFixedState();
		HTime mtime;
		// Get real file length and set fixed state below. 
		int64_t size=tf->FileLength(&mtime);
		if(size<0)
		{
			int errn=errno;
			Warning("Stat'ing add. %s file \"%s\" [frame %d]: %s (skipping)\n",
				TaskFile::IOTypeString(tf->GetIOType()),tf->HDPathRV().str(),
				ctsk_for_msg->frame_no,strerror(errn));
			tf->SetSkipFlag(1);
		}
		else
		{
			tf->SetSkipFlag(0);
			int rv=tf->SetFixedState(size,&mtime,/*downloading=*/0);
			assert(rv==0);
		}
	}
}


// See if we have to set a timeout waiting for a response to a command 
// in the queue. 
void LDRClient::_UpdateRespTimeout()
{
	HTime oldest(HTime::Invalid);
	
	// The first element in the waitresp queue of the command queue 
	// is the oldest one. Thus, it is sufficient if we take the first 
	// element in the waitresp queue which has non-invalid timeout. 
	for(CommandQueue::CQEntry *cqent=cmd_queue.queue_waitresp.first(); 
		cqent; cqent=cqent->next)
	{
		if(!cqent->sendtime.IsInvalid())
		{  oldest=cqent->sendtime;  break;  }
	}
	
	// Update the timeout: 
	// This will calculate the timeout time etc. 
	tdif->UpdateRespTimeout(this,&oldest);
}


void LDRClient::_DoMarkJobFailed(CompleteTask *ctsk,
	TaskDriverType dtype,TaskExecutionStatus *tts,int why)
{
	// Make sure we set failure. 
	if(tts->rflags)
	{
		bool do_overwrite=tts->IsCompleteSuccess();
		Warning("Client %s: [frame %d], while marking %s job as failed:\n"
			"    Exec status already set: %s %s\n",
			_ClientName().str(),ctsk->frame_no,DTypeString(dtype),
			tts->TermStatusString(),  // <- This does not print details. 
			do_overwrite ? "[overwrite]" : "[keep]");
		if(!do_overwrite)  return;
	}
	
	Error("Client %s: Marking %s job as failed (%s) [frame %d]\n",
		_ClientName().str(),DTypeString(dtype),
		TaskExecutionStatus::EF_JK_String(why),ctsk->frame_no);
	
	assert(why!=0);  // 0 -> success...
	tts->rflags=(TaskTerminationReason)
		((tts->rflags & TTR_JK_Mask) | TTR_ExecFail);
	tts->signal=why;
}

void LDRClient::_MarkTaskFailed(CompleteTask *ctsk,int why)
{
	if(ctsk->d.shall_render && ctsk->rt)
	{  _DoMarkJobFailed(ctsk,DTRender,&ctsk->rtes.tes,why);  }
	if(ctsk->d.shall_filter && ctsk->ft)
	{  _DoMarkJobFailed(ctsk,DTFilter,&ctsk->ftes.tes,why);  }
}

// Do not set do_send_quit=1 unless we're actually in correct state and 
// disconnect without error (or minor error). Any major error / protocol 
// violation must use do_send_quit=0. 
void LDRClient::_KickMe(CompleteTask *this_task_failed,int why)
{
	// We should not call KickMe() twice. 
	// This assert is the bug trap for that: 
	assert(!DeletePending());
	
	if(sock_fd>=0)  // Otherwise: Not connected or already disconnected. 
	{
		// Make sure we close files: 
		if(out.io_fd && out.io_fd->pollid && CloseFD(out.io_fd->pollid)<0)
		{
			hack_assert(0);   // Actually, this may not fail, right?
			// This would be the apropriyate call, but we should probably 
			// be careful using it. If simply putting back the task and 
			// trying with a new client is supposed to work, then do 
			// not mark the task as failed here. 
			//if(tri.scheduled_to_send!=this_task_failed)
			//{  _MarkTaskFailed(tri.scheduled_to_send,EF_JK_LDRFail_File);  }
		}
		if(in.io_fd && in.io_fd->pollid && CloseFD(in.io_fd->pollid)<0)
		{
			int errn=errno;
			// This may write "(null)" as file name... 
			Error("Client %s: While closing \"%s\": %s [frame %d]\n",
				_ClientName().str(),
				tdi.recv_file.HDPathRV().str(),strerror(errn),
				tdi.done_ctsk->frame_no);
			// Do not call _KickMe()... we're already there!
			// BUT, mark task as failed. 
			if(tdi.done_ctsk!=this_task_failed)
			{  _MarkTaskFailed(tdi.done_ctsk,EF_JK_LDRFail_File);  }
		}
		
		Verbose(DBG,"Kicking client %s...\n",_ClientName().str());
		_ShutdownConnection();
	}
	
	if(this_task_failed)
	{
		// Store failure info: 
		_MarkTaskFailed(this_task_failed,why);
	}
	
	// Make sure there are no dangling task file references: 
	tri.req_tfile=TaskFile();
	tdi.recv_file=TaskFile();
	
	tdif->ClientDisconnected(this);  // This will delete us. 
}


RefString LDRClient::_ClientName()
{
	#warning "!!cache this name (so that we do not have to sprintf() it all the time!!"
	RefString s;
	if(!cp || !cp->name.str())
	{  s.set("???");  }
	else if(isdigit(cp->name.str()[0]))
	{  s=cp->addr.GetAddress();  }
	else
	{  s.sprintf(128,"%s (%s)",cp->name.str(),cp->addr.GetAddress().str());  }
	return(s);
}


LDRClient::LDRClient(TaskDriverInterface_LDR *_tdif,
	int *failflag) : 
	LinkedListBase<LDRClient>(),
	NetworkIOBase_LDR(failflag),
	last_response_time(HTime::Invalid),
	last_datapump_io_time(HTime::Invalid),
	tri(failflag),
	tdi(failflag),
	cmd_queue(/*queue_side=*/+1,failflag),
	connected_since(HTime::Invalid)
{
	int failed=0;
	
	tdif=_tdif;
	cp=NULL;
	
	_tid_connect_to=NULL;
	_tid_control_resp=NULL;
	
	for(int i=0; i<_DTLast; i++)
	{
		n_supp_descs[i]=0;
		supp_desc[i]=NULL;
		_cache_known_supported[i]=NULL;
	}
	
	connected_state=0;
	auth_state=0;
	_counted_as_client=0;
	client_no_more_tasks=0;
	crun_status=ESS_Running;
	stopcont_calls_pending=0;
	caught_keepalive_during_quit=0;
	client_quit_queued=0;
	
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	outpump_lock=IOPL_Unlocked;
	
	tri.scheduled_to_send=NULL;
	tri.non_skipped_radd_files=0;
	tri.non_skipped_radd_files=0;
	tri.task_request_state=TRC_None;
	
	tdi.done_ctsk=NULL;
	tdi.should_render=0;
	tdi.should_filter=0;
	tdi.task_done_state=TDC_None;
	
	c_jobs=0;
	assigned_jobs=0;
	c_task_thresh_high=0;
	_may_keep_value_sent=-2;  // Really -2, not -1. 
	
	sent_cmd_queue_ent=NULL;
	queued_ping_requests=0;
	_ping_skipme_counter=0;
	
	// Register at TaskDriverInterface_LDR (-> task manager): 
	assert(component_db()->taskmanager());
	if(tdif->RegisterLDRClient(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("LDRClient");  }
}

LDRClient::~LDRClient()
{
	// NOTE: tri.scheduled_to_send and tdi.done_ctsk may be 
	//       NON-NULL here but that does NOT cause any trouble. 
	//       Because they are still owned by TaskManager. 
	
	// Unrergister at TaskDriverInterface_LDR (-> task manager): 
	tdif->UnregisterLDRClient(this);
	
	// These MUST BE NULL here. Becuase TaskDriverInterface_LDR manages 
	// them. We may risk mem leaks. Or even worse: The mem gets freed 
	// in the end (by the TimeoutBase destructor) but we may accumulate 
	// thousands of timeouts for the LDR interface. 
	assert(!_tid_connect_to && !_tid_control_resp);
	
	assert(!cp);  // TaskDriverInterface_LDR must have cleaned up. 
	
	for(int i=0; i<_DTLast; i++)
	{  LFree(supp_desc[i]);  }
}
