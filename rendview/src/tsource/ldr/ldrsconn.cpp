/*
 * ldrsconn.cpp
 * 
 * LDR task source server connection. 
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

#include "ldr.hpp"
#include "param.hpp"

#include "../taskfile.hpp"
#include "../../taskmanager.hpp"

#include <string.h>
#include <assert.h>

using namespace LDR;

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


static void _AllocFailure(CompleteTask *ctsk_for_frame,int fail=-1)
{
	if(fail)
	{  Error("LDR: %s [frame %d].\n",cstrings.allocfail,
		ctsk_for_frame ? ctsk_for_frame->frame_no : (-1));  }
}


inline TaskSourceFactory_LDR *TaskSource_LDR_ServerConn::P()
{  return(back->P());  }
inline ComponentDataBase *TaskSource_LDR_ServerConn::component_db()
{  return(P()->component_db());  }


inline void TaskSource_LDR_ServerConn::schedule_outpump_start()
{
	if(!outpump_scheduled)
	{
		UpdateTimer(schedule_tid,0,0);
		outpump_scheduled=1;
	}
}


inline void TaskSource_LDR_ServerConn::_ResetKeepaliveTimeout(const HTime * /*curr*/)
{
	if(keepalive_timeout>=0)
	{  ResetTimer(keepalive_tid,
		FDAT_FirstLater | FDAT_AlignToShorter | 10);  }
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
int TaskSource_LDR_ServerConn::_AtomicSendData(LDRHeader *d)
{
	// Convert to network order: 
	size_t len=d->length;
	d->length=htonl(len);
	d->command=htons(d->command);
	
	ssize_t wr;
	do
	{  wr=write(sock_fd,(char*)d,len);  }
	while(wr<0 && errno==EINTR);
	if(wr<0)
	{
		Error("LDR: Failed to send %u bytes to %s: %s\n",
			len,addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	out.stat.tot_transferred+=u_int64_t(wr);
	if(size_t(wr)<len)
	{
		Error("LDR: Short write (sent %u/%u bytes) to %s.\n",
			size_t(wr),len,addr.GetAddress().str());
		return(-2);
	}
	return(0);
}


void TaskSource_LDR_ServerConn::_ServerDisconnectMessage()
{
	Error("LDR: Server %s disconnected unexpectedly.\n",
		addr.GetAddress().str());
}

// RETURNS HEADER IN HOST ORDER. 
// Return value: 0 -> OK; -1 -> error 
int TaskSource_LDR_ServerConn::_AtomicRecvData(LDRHeader *d,size_t len)
{
	ssize_t rd;
	do
	{  rd=read(sock_fd,(char*)d,len);  }
	while(rd<0 && errno==EINTR);
	if(rd<0)
	{
		Error("LDR: %s: While reading: %s\n",
			addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	in.stat.tot_transferred+=u_int64_t(rd);
	if(!rd)
	{
		assert(len);  // We may not attempt zero-sized reads. 
		_ServerDisconnectMessage();
		return(-1);
	}
	if(size_t(rd)>=sizeof(LDRHeader))
	{
		// Translate to host order: 
		d->length=ntohl(d->length);
		d->command=ntohs(d->command);
	}
	if(size_t(rd)!=len)
	{
		Error("LDR: Short read from %s at challenge (%d/%u bytes)\n",
			addr.GetAddress().str(),rd,len);
		return(-1);
	}
	return(0);
}


// Expects header in HOST order. 
// Returns packet length or 0 -> error. 
size_t TaskSource_LDR_ServerConn::_CheckRespHeader(
	LDRHeader *d,size_t read_len,
	size_t min_len,size_t max_len)
{
	do {
		if(read_len<sizeof(LDRHeader))
		{  Error("LDR: %s: packet too short (header incomplete: %u bytes)\n",
			addr.GetAddress().str(),read_len);  break;  }
		LDRCommand last_recv_cmd=LDRCommand(d->command);
		if(expect_cmd!=Cmd_NoCommand && last_recv_cmd!=expect_cmd)
		{  Error("LDR: conversation error with %s (expected: %s; received: %s)\n",
			addr.GetAddress().str(),
			LDRCommandString(expect_cmd),
			LDRCommandString(last_recv_cmd));  break;  }
		size_t len=d->length;
		if(len<min_len || (max_len && len>max_len))
		{  Error("LDR: packet from %s too %s (header: %u bytes; %s: %u bytes)\n",
			addr.GetAddress().str(),
			len<min_len ? "short" : "long",
			len,
			len<min_len ? "min" : "max",
			len<min_len ? min_len : max_len);  break;  }
		return(len);
	} while(0);
	
	return(0);  // Corretct. 
}


// Return value: 0 -> OK; -1 -> must call _ConnClose(). 
int TaskSource_LDR_ServerConn::_SendChallengeRequest()
{
	assert(!authenticated);
	
	LDRChallengeRequest d;
	d.length=sizeof(LDRChallengeRequest);  // STILL IN HOST ORDER
	d.command=Cmd_ChallengeRequest;   // STILL IN HOST ORDER
	uchar *cr=((uchar*)&d)+sizeof(LDRHeader);
	uchar *ce=((uchar*)&d)+sizeof(LDRChallengeRequest);
	memset(cr,0,ce-cr);
	LDRSetIDString((char*)d.id_string,LDRIDStringLength);
	d.protocol_vers=htons(LDRProtocolVersion);
	// Fill in random challenge: 
	LDRGetChallengeData(d.challenge);
	
	// Finally, save the time when the challenge request is sent 
	// (needed for server time correction). 
	// Saved immediately before sending. 
	chreq_send_time.SetCurr();
	
	// That was the packet. Send it. 
	// I assume the challenge request is so small (64 bytes or so) 
	// that it can be sent atomically. Otherwise, we give up. 
	if(_AtomicSendData(&d))  return(-1);
	
	++out.stat.count_headers;   // statistics
	
	// Fill in what we expect: 
	LDRComputeCallengeResponse(d.challenge,expect_chresp,
		back->P()->password.str());
	
	expect_cmd=Cmd_ChallengeResponse;
	next_send_cmd=Cmd_NoCommand;
	_DoPollFD(POLLIN,POLLOUT);
	
	return(0);
}


// Retval: 1 -> error (_ConnClose() was called); 0 -> OK 
int TaskSource_LDR_ServerConn::_RecvChallengeResponse()
{
	assert(!authenticated);
	
	LDRChallengeResponse d;
	int rv=_AtomicRecvData(&d,sizeof(d));
	if(rv)
	{  _ConnClose(0);  return(1);  }
	
	// Check what we got...
	size_t len=_CheckRespHeader(&d,sizeof(d),sizeof(d),sizeof(d));
	if(!len)
	{  _ConnClose(0);  return(1);  }
	
	++in.stat.count_headers;   // statistics
	
	// See if the response is okay: 
	int authcode=100;  // illegal value
	ExecStopStatus tm_ess=component_db()->taskmanager()->Get_ExecStopStatus();
	//fprintf(stderr,"Expected challenge response:\n");
	//WriteHexDump(stderr,expect_chresp,LDRChallengeRespLength);
	//fprintf(stderr,"Actual challenge response:\n");
	//WriteHexDump(stderr,(char*)d.response,LDRChallengeRespLength);
	if(memcmp(expect_chresp,d.response,LDRChallengeRespLength))
	{
		Error("LDR: Illegal challenge response from %s.\n",
			addr.GetAddress().str());
		authcode=CAC_AuthFailed;
	}
	else if(back->GetAuthenticatedServer() || back->RecoveringOrQuitting() || 
		tm_ess==ESS_Stopping || tm_ess==ESS_Stopped)
	{
		const char *msgA="",*msgB="";
		if(tm_ess==ESS_Stopping || tm_ess==ESS_Stopped)
		{  msgA="exec currently ";  msgB=ExecStopStatus_String(tm_ess);  }
		else if(back->GetAuthenticatedServer())
		{
			msgA="already connected to ";
			msgB=back->GetAuthenticatedServer()->addr.GetAddress().str();
		}
		else
		{  msgA="currently recovering/quitting";  }
		Warning("LDR: Denying conn to %s (%s%s).\n",
			addr.GetAddress().str(),msgA,msgB);
		authcode=CAC_AlreadyConnected;
	}
	else
	{
		// So, we must set the authenticated flag here. 
		authenticated=1;
		
		// Get current time locally and on server: 
		// (Do it earl, because time is running!)
		HTime curr(HTime::Curr);
		HTime curr_server;
		LDRTime2HTime(&d.current_time,&curr_server);
		
		// Okay, correct challenge and no other authenticated server. 
		VerboseSpecial("LDR: Now connected to %s (%.*s).",
			addr.GetAddress().str(),
			LDRIDStringLength,d.id_string);
		authcode=CAC_Success;
		
		keepalive_timeout=-1;
		u_int32_t kaval=ntohl(d.keepalive_msec);
		long srv_keepalive_msec=(kaval==0xffffffffU ? (-1L) : long(kaval));
		if(srv_keepalive_msec<=0)
		{  srv_keepalive_msec=-1;  }  // be sure...
		if(srv_keepalive_msec>0)
		{
			keepalive_timeout=srv_keepalive_msec/10L*P()->keepalive_mult;
			if(keepalive_timeout<=0)
			{  keepalive_timeout=-1;  }
		}
		
		char tmps[24];
		char tmpto[24];
		// keepalive_timeout<0 here if feature disabled by client (or server). 
		if(keepalive_timeout<0)
		{  strcpy(tmps,"[disabled]");  tmpto[0]='\0';  }
		else
		{
			snprintf(tmps,24,"%ld msec",srv_keepalive_msec);
			snprintf(tmpto,24,"%ld %ssec",
				keepalive_timeout>=10000 ? 
					(keepalive_timeout/1000) : keepalive_timeout,
				keepalive_timeout>=10000 ? "" : "m");
		}
		Verbose(TSLLR,"LDR:   Server keepalive time: %s%s%s\n",
			tmps,
			keepalive_timeout<0 ? "" : "; using idle timeout: ",tmpto);
		
		
		// Calculate the server time correction: 
		//  chreq_send_time: client time when client sent the 
		//      LDRChallengeRequest. 
		//  curr_server: current time on server side as reported 
		//      using the LDRChallengeResponse. 
		//  curr: current time on client side, queried above. 
		// _Current_ server time then can be estimated in the interval: 
		//    curr(server) = curr(client) + [dmin ... dmax]
		// with dmin as follows: 
		HTime dmin = curr_server - curr;
		HTime dmax = curr_server - chreq_send_time;
		assert(!dmin.IsInvalid() && !dmax.IsInvalid());
		HTime delta=dmax-dmin;
		
		// Worst case current server time (current means: at the time curr). 
		//HTime curr_server_min=curr_server;
		HTime curr_server_max=curr_server+(curr-chreq_send_time);
		Verbose(TSLLR,"LDR:   Reported server time: %s (min; local)\n",
			curr_server.PrintTime(1,1));
		Verbose(TSLLR,"LDR:   Server curr time max: %s (delta: %ld msec)\n",
			curr_server_max.PrintTime(1,1),delta.GetR(HTime::msec));
		Verbose(TSLLR,"LDR:   Current client time:  %s (local)\n",
			curr.PrintTime(1,1));
		Verbose(TSLLR,"LDR:   Server-client time diff: %s%s",
			dmin<HTime::Null ? "" : "+",dmin.PrintElapsed(1));
		HTime tmp=delta;
		tmp.Sub(1,HTime::msec);
		if(tmp>=HTime::Null)
		{  Verbose(TSLLR," ... %s%s\n",
			dmax<HTime::Null ? "" : "+",dmax.PrintElapsed(1));  }
		else
		{  Verbose(TSLLR,"\n");  }
		
		// MODIFICATION TIME correction: 
		mtime_corr=dmin;
		tmp.Set(999,HTime::msec);
		Verbose(TSLLR,"LDR:   MTime correction: %s%s%s  %s\n",
			mtime_corr<HTime::Null ? "" : "+",mtime_corr.PrintElapsed(1),
			(mtime_corr+tmp)<=HTime::Null ? " (pessimized)" : 
				((mtime_corr-tmp)>=HTime::Null ? " (optimized)" : ""),
			P()->server_time_correction_enabled ? "[enabled]" : "[disabled]");
		
		// Save server-client time difference: 
		server_client_dtmin=dmin;
		server_client_dtmax=dmax;
		
		// Enable the timer (keepalive expected): 
		if(keepalive_timeout>0)
		{  UpdateTimer(keepalive_tid,keepalive_timeout,
			FDAT_FirstLater | FDAT_AlignToShorter | 10);  }
		else  // Disable it because it may have been used as auth timeout: 
		{  UpdateTimer(keepalive_tid,-1,0);  }
		
		// And we must make sure that sc becomes the 
		// first elem in the list: 
		back->ServerHasNowAuthenticated(this);
	}
	memset(expect_chresp,0,LDRChallengeRespLength);
	now_conn_auth_code=authcode;
	
	// Schedule sending: 
	expect_cmd=Cmd_NoCommand;
	next_send_cmd=Cmd_NowConnected;
	if(authcode!=CAC_Success)
	{
		// Deny packet is done with atomic sends, 
		// accepting is done via copy facility. 
		_DoPollFD(POLLOUT,POLLIN);
		return(0);
	}
	
	// We now switch to FDCopy facility. 
	// No need to poll for writing (in case we do): 
	_DoPollFD(/*set=*/POLLIN,/*clear=*/POLLOUT);
	// out.ioaction, in.ioaction currently IOA_Locked: 
	out.ioaction=IOA_None;
	in.ioaction=IOA_None;
	return(cpnotify_outpump_start());
}


// Send a NowConnected packet denying the connection. 
// Return value: -1 -> _ConnClose() called (always)
int TaskSource_LDR_ServerConn::_SendNowConnectedDenied()
{
	assert(!authenticated && now_conn_auth_code!=CAC_Success);
	
	LDRNowConnected *dummy=NULL;
	size_t len = (((char*)&dummy->auth_code)-
		((char*)dummy)+sizeof(dummy->auth_code));
	char tmp[len];
	LDRNowConnected *d=(LDRNowConnected*)tmp;
	d->length=len;  // STILL IN HOST ORDER
	d->command=Cmd_NowConnected;  // STILL IN HOST ORDER
	d->auth_code=htons(now_conn_auth_code);
	
	// That was the packet. Send it. 
	// I assume the LDRNowConnected packet is so small 
	// that it can be sent atomically. Otherwise, we give up. 
	// (Hey, it's the deny-packet, it IS small!)
	if(_AtomicSendData(d))
	{  _ConnClose(0);  return(-1);  }
	
	++out.stat.count_headers;   // statistics
	
	now_conn_auth_code=-999;  // do not use 0 = CAC_Success
	_ConnClose(2);
	return(-1);
}


// Return value: 
//   0 -> OK
//  -1 -> alloc failure
int TaskSource_LDR_ServerConn::_CreateLDRNowConnectedPacket(RespBuf *destbuf)
{
	TaskManager *taskman=component_db()->taskmanager();
	
	// We do not send render/filter descs if there is a definite 
	// limit of 0 on the number render/filter tasks: 
	int rt_limit=taskman->Get_JobLimit(DTRender);
	int ft_limit=taskman->Get_JobLimit(DTFilter);
	assert(rt_limit!=-2 && ft_limit!=-2);  // -2 -> HL_HureVirt() retval!!
	// Note: We run into a problem if the max is not known (i.e. no 
	// local driver) and that is one problem which would have to be 
	// dealt with when using an LDR forwarder. 
	
	// Count desc size (which has to be put into the packet): 
	size_t rdesclen=0;
	if(rt_limit!=0)
	{
		for(const RF_DescBase *i=component_db()->GetDescList(DTRender)->first(); 
			i; i=i->next)
		{  rdesclen+=i->name.len()+1;  }
	}
	size_t fdesclen=0;
	if(ft_limit!=0)
	{
		for(const RF_DescBase *i=component_db()->GetDescList(DTFilter)->first(); 
			i; i=i->next)
		{  fdesclen+=i->name.len()+1;  }
	}
	
	// These are important: 
	assert(rdesclen<=0xffffU);
	assert(fdesclen<=0xffffU);
	
	size_t need_len=sizeof(LDRNowConnected)+rdesclen+fdesclen;
	if(destbuf->alloc_len<need_len && _ResizeRespBuf(destbuf,need_len))
	{  return(-1);  }
	
	// Compose the packet: 
	destbuf->content=Cmd_NowConnected;
	LDRNowConnected *pack=(LDRNowConnected *)(destbuf->data);
	pack->length=need_len;  // STILL IN HOST ORDER
	pack->command=Cmd_NowConnected;  // STILL IN HOST ORDER
	assert(now_conn_auth_code==CAC_Success);  // We may only be here in this case. 
	pack->auth_code=htons(now_conn_auth_code);
	
	// Info from the task manager: 
	pack->njobs=htons(taskman->Get_njobs());
	pack->task_thresh_high=
		htons(taskman->Get_todo_thresh_high()+taskman->Get_njobs());
	HTime2LDRTime(taskman->Get_starttime(),&pack->starttime);
	{
		int lv=::GetLoadValue();
		pack->loadval = (lv>=0 && lv<0xffff) ? htons(lv) : 0xffffU;
		HTime idle_since;
		int wcc=taskman->Get_work_cycle_count(&idle_since);
		HTime idle_elapsed=HTime(HTime::Curr)-idle_since;
		if(wcc>0xffff)  wcc=0xffff;
		pack->work_cycle_count=htons(u_int16_t(wcc));
		HTime2LDRTime(&idle_elapsed,&pack->idle_elapsed);
	}
	// current_time stored below. 
	
	// Info about our render/filter descs: 
	pack->r_descs_size=htons(rdesclen);
	pack->f_descs_size=htons(fdesclen);
	char *dest=(char*)(pack->data);
	if(rt_limit!=0)
	{
		for(const RF_DescBase *i=component_db()->GetDescList(DTRender)->first(); 
			i; i=i->next)
		{
			// Made sure above that everything fits into dest. 
			const char *src=i->name.str();
			while((*dest++=*src++));  // <-- The routine which made C famous.
		}
	}
	if(ft_limit!=0)
	{
		for(const RF_DescBase *i=component_db()->GetDescList(DTFilter)->first(); 
			i; i=i->next)
		{
			const char *src=i->name.str();
			while((*dest++=*src++));  // <-- The routine which made C famous.
		}
	}
	assert(dest==((char*)pack)+need_len);   // can be left away
	
	// Finally, store the current time on client side: 
	HTime curr(HTime::Curr);
	HTime2LDRTime(&curr,&pack->current_time);
	
	// And store the server-client time difference: 
	HTime2LDRTime(&server_client_dtmin,&pack->time_diff_min);
	HTime2LDRTime(&server_client_dtmax,&pack->time_diff_max);
	
	return(0);
}


// Header in HOST order. 
// Return value: 
//  1 -> handeled; -1 -> must call _ConnClose(). 
int TaskSource_LDR_ServerConn::_HandleReceivedHeader(LDRHeader *hdr)
{
	// BE CAREFUL!! hdr ALLOCATED ON THE STACK. 
	++in.stat.count_headers;   // statistics
	
	// See what we receive: 
	LDRCommand recv_cmd=LDRCommand(hdr->command);
	
	Verbose(DBGV,"LDR: Received header: %s (length=%u)\n",
		LDRCommandString(recv_cmd),hdr->length);
	
	if(next_send_cmd!=Cmd_NoCommand || expect_cmd!=Cmd_NoCommand)
	{
		assert(next_send_cmd==Cmd_NowConnected);
		// This means, authentication was not yet completely done 
		// (we did not yet send the CAC_Success to the server). 
		// Can't he wait or what?
		Error("LDR: Received %s packet before sending successful auth "
			"info to server. Disconnecting.\n",LDRCommandString(recv_cmd));
		return(-1);
	}
	
	switch(recv_cmd)
	{
		case Cmd_TaskRequest:
		case Cmd_FileDownload:
		case Cmd_ControlRequest:
		{
			// Okay, then let's get the body. 
			assert(recv_buf.content==Cmd_NoCommand);  // Can that happen?
			int rv=_StartReadingCommandBody(&recv_buf,hdr);
			if(rv)
			{
				if(rv==-2)
				{  Error("LDR: Too long %s packet (header reports %u bytes)\n",
					LDRCommandString(recv_cmd),hdr->length);  }
				else _AllocFailure(NULL);
				return(-1);
			}
			
			return(1);
		}
		default:;
	}
	
	Error("LDR: Received unexpected command header (%s) (cmd=%u, length=%u).\n",
		LDRCommandString(recv_cmd),recv_cmd,hdr->length);
	
	return(-1);  // -->  _ConnClose()
}


// How it works: 
//  _ParseTaskRequest -> cpnotify_outpump_start() [file request]
//  _TaskRequestComplete() -> TellTaskManagerToGetTask()
//  TellTaskManagerToGetTask(): schedule get task; later: TaskManagerGotTask()
//  TaskManagerGotTask() -> cpnotify_outpump_start() [TRC_Accepted]
//  cpnotify_outpump_done(TaskResponse) -> prepare for next file


// Tell TaskManager to get the task: 
void TaskSource_LDR_ServerConn::_TaskRequestComplete()
{
	assert(tri.resp_code==TRC_Accepted && tri.ctsk);
	
	tri.next_action=TRINA_Complete;
	// ...but only if we're not being kicked. 
	if(!DeletePending())
	{  back->TellTaskManagerToGetTask(tri.ctsk);  }
}

// Called when the TaskManager finally got the task
void TaskSource_LDR_ServerConn::TaskManagerGotTask()
{
	// ##FIXME##
	// Is it possible that _ConnClose() was called until here?
	hack_assert(!DeletePending());   // Probably, if that fails...
	
	assert(tri.resp_code==TRC_Accepted && tri.ctsk && 
	       tri.next_action==TRINA_Complete);
	
	// First of all, the TaskManager now has the complete task, so we have 
	// to set the NULL pointer without deleting/freeing it: 
	tri.ctsk=NULL;
	tri.next_action=TRINA_Response;  // Send TaskResponse. 
	
	schedule_outpump_start();
}


// Return value: (all errors reported to user)
//   0 -> OK
//   1 -> error; disconnect
//   2 -> error; send LDRTaskResponse and refuse task
//   3 -> _ConnClose() was called for some other reason 
int TaskSource_LDR_ServerConn::_ParseTaskRequest(RespBuf *buf)
{
	if(tri.resp_code!=-1 || tri.ctsk!=NULL)
	{
		// OOPS: Another task is pending here and we're getting info 
		// for the next one. Server error. Quit now. 
		Error("LDR: Server sends task request while previous request "
			"is still incomplete.\n");
		return(1);
	}
	
	int rv=_ParseTaskRequest_Intrnl(buf,&tri);
	switch(rv)
	{
		case -2:  return(1);
		case -1:  _AllocFailure(tri.ctsk);  return(1);
		case  0:  break;
		case  1:  return(1);
		case  2:  assert(tri.resp_code>0);  return(2);
		default:  assert(0);  break;
	}
	
	{
		TaskManager *taskman=component_db()->taskmanager();
		// This is what we reported to the server as task-thresh: 
		int task_thresh=taskman->Get_todo_thresh_high()+taskman->Get_njobs();
		if(taskman->GetTaskList()->todo_nelem>=task_thresh)
		{
			// We have enough tasks already. This should never happen. 
			Error("LDR: [frame %d] Server gives us too many tasks "
				"(todo queue: %d, task thresh: %d + %d)\n",
				tri.ctsk->frame_no,
				taskman->GetTaskList()->todo_nelem,taskman->Get_njobs(),
				taskman->Get_todo_thresh_high());
			DELETE(tri.ctsk);
			tri.resp_code=TRC_TooManyTasks;
			return(2);
		}
	}
	
	assert(tri.resp_code==TRC_Accepted);
	
	// Next action: request files from server. 
	tri.next_action=TRINA_FileReq;
	tri.req_file_type=FRFT_None;
	tri.req_file_idx=0;
	tri.req_tfile=TaskFile();
	
	if(cpnotify_outpump_start()==1)  // -> send file request
	{  return(3);  }
	
	return(0);
}

// Read in the next nent LDRFileInfoEntries. 
// Return value: 
//   0 -> OL
//  -1 -> alloc failure
//  -2 -> format error (illegal data in packet) [error written]
// assert on wrong size entry (input buffer too small) becuase 
// this is checked earlier. 
// Updated buf pointer returned in *buf. 
int TaskSource_LDR_ServerConn::_GetFileInfoEntries(TaskFile::IOType iotype,
	CompleteTask::AddFiles *dest,char **buf,char *bufend,int nent,
	CompleteTask *ctsk_for_msg,RefString *prepend_path)
{
	assert(!dest->nfiles && !dest->tfile);  // appending currently not supported
	                                        // (will currently not happen)
	dest->tfile=NEWarray<TaskFile>(nent);
	if(nent && !dest->tfile)  return(-1);
	dest->nfiles=nent;
	
	char *src=*buf;
	int retval=0;
	for(int i=0; i<nent; i++)
	{
		ssize_t rv;
		dest->tfile[i]=LDRGetFileInfoEntry(&rv,
			(LDRFileInfoEntry*)src,bufend-src,
			prepend_path,iotype);
		assert(rv!=-5);  // -5 -> buf too small
		if(rv>=0)
		{
			src+=rv;
			assert(src<=bufend);  // Must have been checked before. 
			continue;
		}
		
		// Error condition here. 
		if(rv==-1)
		{  retval=-1;  goto retfail;  }
		Error("LDR: Server sends illegal file info entry "
			"[frame %d] (iotype=%d, n=%d, rv=%d)\n",
			ctsk_for_msg->frame_no,iotype,i,rv);
		retval=-2;
		goto retfail;
	}
	
	// Store updated buffer pointer and return success: 
	*buf=src;
	return(0);
	
retfail:;
	dest->tfile=DELarray(dest->tfile);
	dest->nfiles=0;
	return(retval);
}

// Internally used by _ParseTaskRequest(). 
// Return value: 
//   0 -> OK
//   1 -> error; disconnect  (message already reported to user)
//   2 -> error; refuse task (message already reported to user)
//  -1 -> alloc failure  (!NOT REPORTED!)
//  -2 -> illegal size entries in header  (message written)
int TaskSource_LDR_ServerConn::_ParseTaskRequest_Intrnl(
	RespBuf *buf,TaskRequestInfo *tri)
{
	assert(buf->content==Cmd_TaskRequest);  // otherwise strange internal error
	LDRTaskRequest *pack=(LDRTaskRequest*)(buf->data);
	assert(pack->command==Cmd_TaskRequest);  // can be left away
	
	size_t len=pack->length;
	assert(len<=buf->alloc_len);  // must be the case due to reading with data pump
	assert(len>=sizeof(LDRTaskRequest));  // can be left away
	if(len<sizeof(LDRTaskRequest))
	{
		Error("LDR: Received too short task request (%u bytes, min=%u).\n",
			len,sizeof(LDRTaskRequest));
		return(-2);
	}
	
	// We don't need sanity checks for the various lengths here 
	// becuase there was a packet length check at the beginning 
	// (max_ldr_pack_len in netiobase_ldr.cc). So, buffer range 
	// checking is sufficient. 
	int r_desc_slen=ntohs(pack->r_desc_slen);
	size_t r_add_args_size=ntohl(pack->r_add_args_size);
	int r_oformat_slen=ntohs(pack->r_oformat_slen);
	int r_iofile_slen=ntohs(pack->r_iofile_slen);
	
	int f_desc_slen=ntohs(pack->f_desc_slen);
	size_t f_add_args_size=ntohl(pack->f_add_args_size);
	int f_iofile_slen=ntohs(pack->f_iofile_slen);
	
	int r_n_files=ntohs(pack->r_n_files);
	int f_n_files=ntohs(pack->f_n_files);
	
	char *ptr=(char*)pack->data;
	char *end_ptr=ptr+pack->length;
	int64_t lenaccu=0;   // I'm paranoid here with overflows...
	
	char *r_in_out_file=ptr;  ptr+=r_iofile_slen;    lenaccu+=r_iofile_slen;
	char *f_in_out_file=ptr;  ptr+=f_iofile_slen;    lenaccu+=f_iofile_slen;
	char *rdesc_str=ptr;      ptr+=r_desc_slen;      lenaccu+=r_desc_slen;
	char *fdesc_str=ptr;      ptr+=f_desc_slen;      lenaccu+=f_desc_slen;
	char *r_oformat_str=ptr;  ptr+=r_oformat_slen;   lenaccu+=r_oformat_slen;
	char *r_add_args=ptr;     ptr+=r_add_args_size;  lenaccu+=r_add_args_size;
	char *f_add_args=ptr;     ptr+=f_add_args_size;  lenaccu+=f_add_args_size;
	char *add_files_ptr=ptr;
	// Walk the additional file list: 
	{
		u_int16_t tmp16;
		for(int i=r_n_files+f_n_files; i; i--)
		{
			if(ptr>end_ptr-sizeof(LDRFileInfoEntry) || 
			   ptr<(char*)pack->data)  goto illegal_size;
			_memcpy16(&tmp16,&((LDRFileInfoEntry*)ptr)->name_slen);  // alignment issue...
			size_t delta=sizeof(LDRFileInfoEntry)+ntohs(tmp16);
			ptr+=delta;  lenaccu+=delta;
		}
	}
	
	if( int64_t(len) != lenaccu+int64_t(sizeof(LDRTaskRequest)) )
	{
		illegal_size:;
		Error("LDR: Illegal size entries in received task request "
			"(header: %u; data: >=%u)\n",
			len,size_t(lenaccu)+sizeof(LDRTaskRequest));
		return(-2);
	}
	assert(int64_t(ptr-(char*)pack->data)==lenaccu);   // "must" be the case (see test above).
	
	static const char *_rtr="LDR: Received task request [frame ";
	
	int frame_no=ntohl(pack->frame_no);
	if(frame_no<0)
	{
		Error("%s%d] with illegal frame number.\n",_rtr,frame_no);
		return(1);
	}
	
	tri->task_id=ntohl(pack->task_id);
	//#warning check task ID
	
	// First, extract render desc, filter desc strings: 
	RefString rdesc_name,fdesc_name;
	if( (r_desc_slen ? rdesc_name.set0(rdesc_str,r_desc_slen) : 0) || 
	    (f_desc_slen ? fdesc_name.set0(fdesc_str,f_desc_slen) : 0) )
	{  return(-1);  }
	
	if(!rdesc_name && !fdesc_name)
	{
		Error("%s%d] containing neither render nor filter job\n",_rtr,frame_no);
		return(1);
	}
	
	// Some more sanity checks: 
	if(!rdesc_name && 
	   (r_oformat_slen || r_add_args_size || r_n_files || r_iofile_slen))
	{
		Error("%s%d] containing unnecessary render task info.\n",_rtr,frame_no);
		return(1);
	}
	if(!fdesc_name && 
	   (f_add_args_size || f_n_files || f_iofile_slen))
	{
		Error("%s%d] containing unnecessary filter task info.\n",_rtr,frame_no);
		return(1);
	}
	
	// See if we know the requuested rdesc/fdesc and oformat: 
	const RenderDesc *rdesc=NULL;
	if(!(!rdesc_name))
	{
		rdesc=component_db()->FindRenderDescByName(rdesc_name.str());
		if(!rdesc)
		{
			Error("%s%d] for unknown render desc \"%s\".\n",
				_rtr,frame_no,rdesc_name.str());
			tri->resp_code=TRC_UnknownRender;
			return(2);
		}
	}
	
	const FilterDesc *fdesc=NULL;
	if(!(!fdesc_name))
	{
		fdesc=component_db()->FindFilterDescByName(fdesc_name.str());
		if(!fdesc)
		{
			Error("%s%d] for unknown filter desc \"%s\".\n",
				_rtr,frame_no,fdesc_name.str());
			tri->resp_code=TRC_UnknownFilter;
			return(2);
		}
	}
	
	const ImageFormat *r_oformat=NULL;
	if(rdesc)
	{
		if(!r_oformat_slen)
		{
			Error("%s%d] without output format spec.\n",_rtr,frame_no);
			return(1);
		}
		RefString oformat_name;
		if(oformat_name.set0(r_oformat_str,r_oformat_slen))
		{  return(-1);  }
		r_oformat=component_db()->FindImageFormatByName(oformat_name.str());
		if(!r_oformat)
		{
			Error("%s%d] containing unknown image format \"%s\".\n",
				_rtr,frame_no,oformat_name.str());
			tri->resp_code=TRC_UnknownROFormat;
			return(2);
		}
	}
	
	// Okay, set up a CompleteTask structure: 
	CompleteTask *ctsk=NEW<CompleteTask>();
	if(!ctsk)
	{  return(-1);  }
	
	ctsk->frame_no=frame_no;
	ctsk->task_id=tri->task_id;
	if(rdesc)
	{
		RenderTask *rt=NEW<RenderTask>();
		if(!rt)
		{  delete ctsk;  return(-1);  }
		ctsk->rt=rt;
		
		rt->frame_no=frame_no;
		rt->rdesc=rdesc;
		rt->width=ntohs(pack->r_width);
		rt->height=ntohs(pack->r_height);
		rt->oformat=r_oformat;
		rt->timeout=LDR_timeout_ntoh(pack->r_timeout);
		rt->resume=0;
		
		u_int16_t r_flags=ntohs(pack->r_flags);
		if((r_flags & TRRF_Unfinished) && rdesc->can_resume_render)
		{  rt->resume=1;  }
		if((r_flags & TRRF_ResumeFlagActive))
		{  rt->resume_flag_set=1;  }
		if((r_flags & TRRF_UseFrameClock))
		{  rt->use_frame_clock=1;  }
		// Currently, the only flags we know are 
		// TRRF_Unfinished, TRRF_ResumeFlagActive and TRRF_UseFrameClock. 
		if((r_flags & ~(TRRF_Unfinished | TRRF_ResumeFlagActive | 
			TRRF_UseFrameClock))!=0)
		{
			Error("%s%d] with illegal flags 0x%x.\n",_rtr,frame_no,r_flags);
			delete ctsk;  return(1);
		}
		
		#if 0  	// FrameClockVal: UNUSED
		int rv=Int32ToDouble(pack->r_frame_clock,&rt->frame_clock);
		if(rv==0)
		{
			// Valid frame clock was found. Check if that is okay. 
			if(!rdesc->can_pass_frame_clock)
			{
				Error("%s%d] containing unsupported frame clock (%g).\n",
					_rtr,frame_no,rt->frame_clock);
				delete ctsk;  return(1);
			}
		}
		else if(rv!=3)  // 3 -> NaN -> okay (no frame clock val)
		{
			Error("%s%d] with out of range frame clock.\n",_rtr,frame_no);
			delete ctsk;  return(1);
		}
		#else
		// Frame clock was found. Check if that is okay. 
		if(!rdesc->can_pass_frame_clock)
		{
			Error("%s%d] containing unsupported frame clock.\n",
				_rtr,frame_no);
			delete ctsk;  return(1);
		}
		#endif
	}
	if(fdesc)
	{
		FilterTask *ft=NEW<FilterTask>();
		if(!ft)
		{  delete ctsk;  return(-1);  }  // NO need to delete ctsk->rt (done by ~CompleteTask())
		ctsk->ft=ft;
		
		ft->frame_no=frame_no;
		ft->fdesc=fdesc;
		ft->timeout=LDR_timeout_ntoh(pack->f_timeout);
	}
	
	// Okay, and now... let's copy the additional args & required files. 
	if(rdesc)
	{
		int rv=CopyData2StrList(&ctsk->rt->add_args,r_add_args,r_add_args_size);
		if(rv<0)
		{
			delete ctsk;  // NO need to delete ctsk->rt,ft (done by ~CompleteTask())
			ctsk=NULL;
			if(rv==-1)
			{  return(-1);  }
			Error("%s%d] with format error (add. render args).\n",
				_rtr,frame_no);
			return(1);
		}
		rv=_GetFileInfoEntries(TaskFile::IOTRenderInput,&ctsk->radd,
			&add_files_ptr,end_ptr,r_n_files,ctsk,&P()->radd_path);
		if(rv==-1 || rv==-2)
		{
			delete ctsk;
			return(-1);
		}
		else assert(rv==0);  // Only rv=0, -1 and -2 known. 
	}
	if(fdesc)
	{
		int rv=CopyData2StrList(&ctsk->ft->add_args,f_add_args,f_add_args_size);
		if(rv<0)
		{
			delete ctsk;  // NO need to delete ctsk->rt,ft (done by ~CompleteTask())
			ctsk=NULL;
			if(rv==-1)
			{  return(-1);  }
			Error("%s%d] with format error (add. filter args).\n",
				_rtr,frame_no);
			return(1);
		}
		rv=_GetFileInfoEntries(TaskFile::IOTFilterInput,&ctsk->fadd,
			&add_files_ptr,end_ptr,f_n_files,ctsk,&P()->fadd_path);
		if(rv==-1 || rv==-2)
		{
			delete ctsk;
			return(-1);
		}
		else assert(rv==0);  // Only rv=0, -1 and -2 known. 
	}
	
	// Assign input and output files. 
	int64_t r_in_file_size=ntohll(pack->r_in_file_size);
	int64_t f_in_file_size=ntohll(pack->f_in_file_size);
	HTime r_in_file_mtime,f_in_file_mtime;
	LDRTime2HTime(&pack->r_in_file_mtime,&r_in_file_mtime);
	LDRTime2HTime(&pack->f_in_file_mtime,&f_in_file_mtime);
	
	int sv=_TaskRequest_SetUpTaskFiles(ctsk,
		r_in_out_file,r_iofile_slen,r_in_file_size,&r_in_file_mtime,
		f_in_out_file,f_iofile_slen,f_in_file_size,&f_in_file_mtime);
	if(sv==1 || sv==-1)
	{
		// -1 -> alloc failure (err reported by caller)
		// +1 -> format error (error already written)
		delete ctsk;
		return(sv);
	}
	else assert(sv==0);
	
	// Dump all the information to the user (if requested):
	if(component_db()->taskmanager()->TestDumpTaskInfo(TaskManager::DTSK_Arrival))
	{
		char tmp[196];
		snprintf(tmp,196,"LDR: Received new task [frame %d] from LDR server %s.",
			ctsk->frame_no,
			back->GetAuthenticatedServer() ? 
				back->GetAuthenticatedServer()->addr.GetAddress().str() : "???");
		component_db()->taskmanager()->DumpTaskInfo(ctsk,
			tmp,TaskManager::DTSK_Arrival,VERBOSE_TSLLR);
	}
	
	// Okay, so let's fill in the success code and go on. 
	tri->resp_code=TRC_Accepted;
	tri->ctsk=ctsk;
	tri->next_action=TRINA_None;
	
	return(0);
}


// Returns !=0 on error. 
static inline int _TaskRequest_SUTF_CheckSizeMTime(
	int64_t size,const HTime *mtime)
{
	if(size<0)  return(1);
	if(mtime->IsInvalid())
	{  return(size!=0);  }
	return(0);
}

// r_io,f_io: strings of length r_io_len,f_io_len containing the 
// following: input\0output of render/filter job. 
// Used by _ParseTaskRequest_Intrnl(): Set up TaskFile stuff. 
// Return value: 
//   0 -> OK
//   1 -> format error
//  -1 -> alloc failure
int TaskSource_LDR_ServerConn::_TaskRequest_SetUpTaskFiles(CompleteTask *ctsk,
	const char *r_io,int r_io_len,int64_t r_in_size,const HTime *r_in_mtime,
	const char *f_io,int f_io_len,int64_t f_in_size,const HTime *f_in_mtime)
{
	// Set up: 
	// infile,outfile,wdir (rt and ft)
	
	const char *_rtri="LDR: Received task request [frame %d] with "
		"illegal %s IO file spec (%s)\n";
	
	// First, check the format: 
	TaskDriverType fmt_err=DTNone;
	if(str_find_chr(r_io,r_io_len,'/'))  fmt_err=DTRender;
	if(str_find_chr(f_io,f_io_len,'/'))  fmt_err=DTFilter;
	if(fmt_err!=DTNone)
	{
		Error(_rtri,ctsk->frame_no,DTypeString(fmt_err),"'/' in file name");
		return(1);
	}
	// We already checked that r_io_len is non-NULL only if there is 
	// a render task; same for filter. 
	// But not the other way round (= missing file for render/filter job)...
	const char *r_in_file=NULL,*r_out_file=NULL;
	int r_out_len=0;
	const char *f_in_file=NULL,*f_out_file=NULL;
	int f_out_len=0;
	fmt_err=DTNone;
	do {
		if(r_io_len)
		{
			r_in_file=r_io;
			r_out_file=str_find_chr(r_io,r_io_len,'\0')+1;
			r_out_len=(r_io+r_io_len)-r_out_file;
			if(!r_out_file || r_out_len<1 || *r_in_file=='\0')
			{  fmt_err=DTRender;  break;  }
		}
		if(f_io_len)
		{
			f_in_file=f_io;
			f_out_file=str_find_chr(f_io,f_io_len,'\0')+1;
			f_out_len=(f_io+f_io_len)-f_out_file;
			if(!f_out_file || f_out_len<1 || *f_in_file=='\0')
			{  fmt_err=DTFilter;  break;  }
		}
	} while(0);
	if(fmt_err!=DTNone)
	{
		Error(_rtri,ctsk->frame_no,DTypeString(fmt_err),"missing file spec");
		return(1);
	}
	
	// The case that mtime is invalid may be a bit special. 
	// We simply store it; an invalid mtime cannot be compared 
	// when deciding whether to download the file and hence we 
	// WILL download it IF WE NEED IT (i.e. we will NOT download 
	// the filter source if we also do rendering). 
	// --> BUT, the size must be 0 if mtime is invalid. 
	fmt_err=DTNone;
	do {
		if(r_in_size<0 || (r_in_mtime->IsInvalid() && r_in_size!=0))
		{  fmt_err=DTRender;  break;  }
		if(f_in_size<0 || (f_in_mtime->IsInvalid() && f_in_size!=0))
		{  fmt_err=DTFilter;  break;  }
	} while(0);
	if(fmt_err!=DTNone)
	{
		Error(_rtri,ctsk->frame_no,DTypeString(fmt_err),"invalid size/mtime");
		return(1);
	}
	
	if(ctsk->rt)
	{
		RenderTask *rt=(RenderTask*)ctsk->rt;
		if(rt->wdir.set("."))  return(-1);
		
		RefString infile,outfile;
		if(infile.sprintf(0,"%s",r_in_file))  return(-1);
		if(outfile.sprintf(0,"%.*s",r_out_len,r_out_file))  return(-1);
		
		// FLAT DIR STRUCTURE: hdpath_rv = hdpath_job. 
		rt->infile=TaskFile::GetTaskFile(infile,
			TaskFile::FTFrame,TaskFile::IOTRenderInput,TaskFile::FCLDR,
			/*hdpath_job=*/&infile);
		rt->outfile=TaskFile::GetTaskFile(outfile,
			TaskFile::FTImage,TaskFile::IOTRenderOutput,TaskFile::FCLDR,
			/*hdpath_job=*/&outfile);
		if(!rt->infile || !rt->outfile)  return(-1);
		
		if(rt->resume)
		{  rt->outfile.SetIncomplete(1);  }
		
		rt->infile.SetDelSpec(P()->rin_delspec);
		rt->outfile.SetDelSpec(P()->rout_delspec);
		rt->infile.SetFixedState(r_in_size,r_in_mtime,/*downloading=*/0);
	}
	if(ctsk->ft)
	{
		FilterTask *ft=(FilterTask*)ctsk->ft;
		if(ft->wdir.set("."))  return(-1);
		
		RefString infile,outfile;
		if(infile.sprintf(0,"%s",f_in_file))  return(-1);
		if(outfile.sprintf(0,"%.*s",f_out_len,f_out_file))  return(-1);
		
		// FLAT DIR STRUCTURE: hdpath_rv = hdpath_job. 
		ft->infile=TaskFile::GetTaskFile(infile,
			TaskFile::FTImage,TaskFile::IOTFilterInput,TaskFile::FCLDR,
			/*hdpath_job=*/&infile);
		ft->outfile=TaskFile::GetTaskFile(outfile,
			TaskFile::FTImage,TaskFile::IOTFilterOutput,TaskFile::FCLDR,
			/*hdpath_job=*/&outfile);
		if(!ft->infile || !ft->outfile)  return(-1);
		
		ft->infile.SetDelSpec(P()->fin_delspec);
		ft->outfile.SetDelSpec(P()->fout_delspec);
		ft->infile.SetFixedState(f_in_size,f_in_mtime,/*downloading=*/0);
	}
	
	// Got to set delete spec for additional files: 
	int end=ctsk->radd.nfiles;
	for(int i=0; i<end; i++)
	{  ctsk->radd.tfile[i].SetDelSpec(P()->radd_delspec);  }
	end=ctsk->fadd.nfiles;
	for(int i=0; i<end; i++)
	{  ctsk->fadd.tfile[i].SetDelSpec(P()->fadd_delspec);  }
	
	return(0);
}


// Return value: 
//   0 -> okay, next request started. 
//   1 -> all requests done
//  -1 -> error; call _ConnClose(). 
int TaskSource_LDR_ServerConn::_StartSendNextFileRequest()
{
	retry:;
	assert(tri.ctsk && tri.next_action==TRINA_FileReq);
	
	// See which file we request next. 
	assert(tri.req_file_idx!=0xffff);
	
	// Of course, we will NOT request output files. 
	switch(tri.req_file_type)
	{  // NOTE: Lots of fall-through logic; be careful!!
		case FRFT_None:
			if(tri.ctsk->rt && P()->transfer.render_src)
			{  tri.req_file_type=FRFT_RenderIn;  tri.req_file_idx=0;  break;  }
			// Not requesting render input file, so release fixed state: 
			if(tri.ctsk->rt)   // render infile must be in fixed state
			{  int rv=tri.ctsk->rt->infile.ClearFixedState();  assert(!rv);  }
		case FRFT_RenderIn:
			//if( (tri.ctsk->rt && P()->transfer.render_dest && !tri.ctsk->ft) || 
			//    (tri.ctsk->ft && P()->transfer.render_dest && !tri.ctsk->rt) )
			//{  tri.req_file_type=FRFT_RenderOut;  tri.req_file_idx=0;  break;  }
			if( ( (tri.ctsk->ft && !tri.ctsk->rt) || 
			      (tri.ctsk->rt && tri.ctsk->rt->resume) ) && 
			    P()->transfer.render_dest )
			{  tri.req_file_type=FRFT_RenderOut;  tri.req_file_idx=0;  break;  }
			// Not requesting render output = filter input file, so release fixed state: 
			// It is only in fixed state if there is a filter task. 
			if(tri.ctsk->ft)   // file must be in fixed state
			{  int rv=tri.ctsk->ft->infile.ClearFixedState();  assert(!rv);  }
		case FRFT_RenderOut:
			//if(tri.ctsk->ft && P()->transfer.filter_dest)
			//{  tri.req_file_type=FRFT_FilterOut;  tri.req_file_idx=0;  break;  }
		case FRFT_FilterOut:
			if(tri.ctsk->rt && P()->transfer.r_additional && tri.ctsk->radd.nfiles)
			{  tri.req_file_type=FRFT_AddRender;  tri.req_file_idx=0;  break;  }
		case FRFT_AddRender:
			++tri.req_file_idx;
			if(tri.ctsk->rt && P()->transfer.r_additional && 
			   int(tri.req_file_idx)<tri.ctsk->radd.nfiles)  break;
			if(tri.ctsk->ft && P()->transfer.f_additional && tri.ctsk->fadd.nfiles)
			{  tri.req_file_type=FRFT_AddFilter;  tri.req_file_idx=0;  break;  }
		case FRFT_AddFilter:
			++tri.req_file_idx;
			if(tri.ctsk->ft && P()->transfer.f_additional && 
			   int(tri.req_file_idx)<tri.ctsk->fadd.nfiles)  break;
		//complete fallthrough:
			tri.req_file_type=FRFT_None;
			tri.req_file_idx=0xffff;
	}
	
	if(tri.req_file_type==FRFT_None)
	{
		// All file requests done. 
		return(1);
	}
	
	// Get the file we're requesting: 
	TaskFile tfile=GetTaskFileByEntryDesc(
		/*dir=*/(tri.req_file_type==FRFT_RenderOut && 
		        tri.ctsk->rt && tri.ctsk->rt->resume) ? (+1) : (-1),
		tri.ctsk,tri.req_file_type,tri.req_file_idx);
	// NOTE: We may not request a file which is unknown to our CompleteTask. 
	// If this assert fails, then the chosen req_file_type,req_file_idx is 
	// illegal because unknown. 
	assert(!!tfile);
	tri.req_tfile=tfile;
	
	// Only request files which we need (i.e.: if they already 
	// exist, have proper time stamp and same size, then do not 
	// transfer them). 
	// So, see if the file exists: 
	do {
		// We need the render output for resume operation in any case. 
		if(tri.req_file_type==FRFT_RenderOut && tri.ctsk->rt->resume)  break;
		
		// Some logic for the always_download_rf_in - flag: 
		if(tri.req_file_type!=FRFT_AddRender && 
		   tri.req_file_type!=FRFT_AddFilter && 
		   P()->always_download_rf_in )
		{  break;  }   // must download the file
		
		HTime local_mtime;
		errno=0;
		int64_t local_size=tfile.FileLength(&local_mtime,
			/*aware_of_fixed_state=*/1);
		int errn=errno;
		if(local_size<0)
		{
			if(errno==ENOENT) break;  // we need the file
			Warning("LDR: Failed to stat \"%s\": %s (expect trouble)\n",
				tfile.HDPathRV().str(),strerror(errn));
			break;
		}
		
		HTime remote_mtime;
		int64_t remote_size=tfile.GetFixedState(&remote_mtime);
		assert(remote_size>=0);  // Must have called SetFixedState(). 
		if(local_size!=remote_size)  break;  // need the file
		// The next case should not happen. 
		if(remote_mtime.IsInvalid())
		{
			Error("LDR: OOPS: Remote mtime for \"%s\" invalid.\n",
				tfile.HDPathRV().str());
			break;
		}
		
		// Add time threshold: 
		local_mtime.Add(P()->timestamp_thresh,HTime::msec);
		// Add server time correction: 
		if(P()->server_time_correction_enabled && !mtime_corr.IsInvalid())
		{  local_mtime+=mtime_corr;  }
		if(remote_mtime>=local_mtime)
		{
			//-- Timestamp test says: should download again. --
			// Do not download again if the mtime on the server side 
			// did not change. 
			if(P()->re_download_enabled || 
			   tfile.GetDlSrvMTime()->IsInvalid() ||
			   *tfile.GetDlSrvMTime()!=remote_mtime )
			{  break;  }  // We need -erm-... we _want_ the file. 
			// Otherwise, do not re-download even if time stamp 
			// tells us so. 
		}
		
		// Seems we do not need the file. 
		int rv=tfile.ClearFixedState();
		assert(!rv);  // must have been in fixed state
		
		Verbose(TSLLR,"LDR: NOT requesting %s file %s [frame %d]\n",
			FileRequestFileTypeString(tri.req_file_type),
			tri.req_tfile.BaseNamePtr(),
			tri.ctsk->frame_no);
		goto retry;
	} while(0);
	
	// Will download file. 
	tfile.SetDownload(/*s=*/ 1 /* "downloading" */);
	
	int fail=0;
	do {
		assert(send_buf.content==Cmd_NoCommand);
		if(_ResizeRespBuf(&send_buf,sizeof(LDRFileRequest)))
		{  fail=-1;  break;  }
		
		// Compose the packet: 
		send_buf.content=Cmd_FileRequest;
		LDRFileRequest *pack=(LDRFileRequest*)(send_buf.data);
		pack->length=sizeof(LDRFileRequest);  // host order
		pack->command=Cmd_FileRequest;  // host order
		pack->task_id=htonl(tri.task_id);
		pack->file_type=htons(tri.req_file_type);
		pack->file_idx=htons(tri.req_file_idx);
		
		// Okay, make ready to send it: 
		fail=_FDCopyStartSendBuf(pack);
		if(fail)  break;
		
		Verbose(TSLLR,"LDR: Requesting %s file (%s) [frame %d]\n",
			FileRequestFileTypeString(tri.req_file_type),
			tri.req_tfile.BaseNamePtr(),
			tri.ctsk->frame_no);
	} while(0);
	if(fail)
	{
		if(fail==-1)
		{  _AllocFailure(tri.ctsk);  }
		else
		{  Error("LDR: Internal error (%d) while initiating file request [frame %d]\n",
			fail,tri.ctsk ? tri.ctsk->frame_no : (-1));  }
		return(-1);
	}
	
	return(0);
}


// Either send next file upload header and set apropriate file params in 
// tdi or send final task state. 
// Retval: -1 -> error; 0 -> next file; 1 -> final state
int TaskSource_LDR_ServerConn::_SendNextFileUploadHdr()
{
	assert(tdi.done_ctsk && tdi.next_action==TDINA_FileSendH);
	
	// See which is the next file we send: 
	for(;;)
	{
		CompleteTask::TES *tts=NULL;
		
		switch(tdi.upload_file_type)
		{  // NOTE: Lots of fall-through logic; be careful!!
			case FRFT_None:
				// In case it is OFS_Resume, we do not upload it if the 
				// render resume flag is not set because that would 
				// just waste bandwidth. 
				if(tdi.done_ctsk->rt && P()->transfer.render_dest)
				{
					tts=&tdi.done_ctsk->rtes;
					if(tts->tes.outfile_status==OFS_Resume && 
						!tdi.done_ctsk->rt->resume_flag_set)
					{  Verbose(TSLLR,"LDR: Not uploading unfinished output "
						"\"%s\" because render resume (-rcont) inactive "
						"[frame %d].\n",
						tdi.done_ctsk->rt->outfile.HDPathRV().str(),
						tdi.done_ctsk->frame_no);  }
					else
					{
						tdi.upload_file_type=FRFT_RenderOut;
						tdi.upload_file=tdi.done_ctsk->rt->outfile;
						break;
					}
				}
			case FRFT_RenderOut:
				if(tdi.done_ctsk->ft && P()->transfer.filter_dest)
				{
					tts=&tdi.done_ctsk->ftes;
					tdi.upload_file_type=FRFT_FilterOut;
					tdi.upload_file=tdi.done_ctsk->ft->outfile;
					break;
				}
			default:  // final fallthrough:
				tdi.upload_file_type=FRFT_None;
				tdi.upload_file=TaskFile();
		}
		
		// See if we actually want/can upload tdi.upload_file_type: 
		if(tdi.upload_file_type==FRFT_None)  break;
		
		// If the job was not launched skip to next file. 
		assert(tts);
		if(tts->tes.rflags==TTR_Unset)
			continue;
		
		// We only transfer complete files or files to be resumed. 
		// Especially no "OFS_Bad" files. 
		if(tts->tes.outfile_status!=OFS_Complete && 
		   tts->tes.outfile_status!=OFS_Resume )  continue;
		
		// If this assert fails, there is a tdi.done_ctsk->rt/fd which 
		// has an output file of NULL. This is now allowed. 
		assert(!!tdi.upload_file);
		
		// Get the file size. If we cannot, we output a warning and 
		// go on to the next file. 
		tdi.upload_file_size=tdi.upload_file.FileLength();
		if(tdi.upload_file_size<0)
		{
			const char *err_str=(tdi.upload_file_size==-2 ? 
				"no hd path" : strerror(errno));
			static const char *_err_fmt=
				"LDR: Trying to stat output file \"%s\": %s [frame %d]\n";
			if(tts->tes.outfile_status==OFS_Complete || 
			   tts->tes.outfile_status==OFS_Resume )
			{  Warning(_err_fmt,
				tdi.upload_file.HDPathRV().str(),err_str,tdi.done_ctsk->frame_no);  }
			else
			{  Verbose(TSLLR,_err_fmt,
				tdi.upload_file.HDPathRV().str(),err_str,tdi.done_ctsk->frame_no);  }
			continue;  // next file
		}
		
		// Send the header. 
		break;
	}
	
	// Send upload header for file in tdi OR LDRDoneComplete. 
	int fail=1;
	do {
		assert(send_buf.content==Cmd_NoCommand);
		
		LDRHeader *hdr;
		if(tdi.upload_file_type==FRFT_None)
		{
			tdi.next_action=TDINA_Complete;
			
			if(_ResizeRespBuf(&send_buf,sizeof(LDRDoneComplete)))  break;
			
			send_buf.content=Cmd_DoneComplete;
			LDRDoneComplete *pack=(LDRDoneComplete*)(send_buf.data);
			pack->length=sizeof(LDRDoneComplete);  // host order
			pack->command=Cmd_DoneComplete;  // host order
			pack->task_id=htonl(tdi.done_ctsk->task_id);
			hdr=pack;
		}
		else
		{
			if(_ResizeRespBuf(&send_buf,sizeof(LDRFileUpload)))  break;
			
			send_buf.content=Cmd_FileUpload;
			LDRFileUpload *pack=(LDRFileUpload*)(send_buf.data);
			pack->length=sizeof(LDRFileUpload);  // host order
			pack->command=Cmd_FileUpload;  // host order
			pack->task_id=htonl(tdi.done_ctsk->task_id);
			pack->file_type=htons(tdi.upload_file_type);
			pack->size=htonll(tdi.upload_file_size);
			
			hdr=pack;
		}
		
		// Okay, make ready to send it: 
		if(_FDCopyStartSendBuf(hdr))  break;
		
		fail=0;
	} while(0);
	if(fail)
	{
		// Currently, only alloc failures can happen. 
		_AllocFailure(tdi.done_ctsk,/*fail=*/1);
		return(-1);
	}
	
	return((tdi.upload_file_type==FRFT_None) ? 1 : 0);
}


// Retval: 0 -> OK; -1 -> Call _ConnClose();
int TaskSource_LDR_ServerConn::_ParseFileDownload(RespBuf *buf)
{
	if(!tri.ctsk || tri.next_action!=TRINA_FileRecvH)
	{
		Error("LDR: Server sends file download header although we did "
			"not request that.\n");
		return(-1);
	}
	
	// Read in header...
	LDRFileDownload *pack=(LDRFileDownload*)(buf->data);
	assert(pack->command==Cmd_FileDownload);  // can be left away
	if(pack->length!=sizeof(LDRFileDownload))
	{
		Error("LDR: Received illegal-sized file download header "
			"(%u/%u bytes) [frame %d]\n",
			pack->length,sizeof(LDRFileDownload),
			tri.ctsk->frame_no);
		return(-1);
	}
	if(ntohl(pack->task_id)!=tri.ctsk->task_id || 
	   ntohs(pack->file_type)!=tri.req_file_type || 
	   ntohs(pack->file_idx)!=tri.req_file_idx )
	{
		Error("LDR: Received non-matching download header "
			"(%u,%d,%d / %u,%d,%d) [frame %d]",
			ntohl(pack->task_id),
				int(ntohs(pack->file_type)),
				int(ntohs(pack->file_idx)),
			tri.ctsk->task_id,
				int(tri.req_file_type),int(tri.req_file_idx),
			tri.ctsk->frame_no);
		return(-1);
	}
	u_int64_t _fsize=ntohll(pack->size);
	int64_t fsize=_fsize;
	if(u_int64_t(fsize)!=_fsize || fsize<0)
	{
		Error("LDR: Received file download header containing "
			"illegal file size.\n");
		return(-1);
	}
	// Okay, then let's start to receive the file. 
	Verbose(DBG,"LDR: Starting to download file (%s; %lld bytes)\n",
		tri.req_tfile.HDPathRV().str(),fsize);
	
	const char *path=tri.req_tfile.HDPathRV().str();
	int rv=_FDCopyStartRecvFile(path,fsize);
	if(rv)
	{
		int errn=errno;
		if(rv==-1)  _AllocFailure(tri.ctsk);
		else if(rv==-2)
		{  Error("LDR: Failed to start downloading requested file [frame %d]:\n"
			"LDR:    While opening \"%s\": %s\n",
			tri.ctsk->frame_no,path,strerror(errn));  }
		else assert(0);  // rv=-3 (fsize<0) checked above
		return(-1);
	}
	
	in_active_cmd=Cmd_FileDownload;
	tri.next_action=TRINA_FileRecvB;
	
	return(0);
}


// Retval: 0 -> OK; -1 -> Call _ConnClose();
int TaskSource_LDR_ServerConn::_ParseCommandRequest(RespBuf *buf)
{
	static const char *_illsize_msg=
		"LDR: Received illegal-sized control request%s%s%s (%u/>=%u bytes)\n";
	
	LDRClientControlRequest *pack=(LDRClientControlRequest*)(buf->data);
	assert(pack->command==Cmd_ControlRequest);  // can be left away
	if(pack->length<sizeof(LDRClientControlRequest))
	{
		Error(_illsize_msg,
			"","","",
			pack->length,sizeof(LDRClientControlRequest));
		return(-1);
	}
	
	size_t min_len=0;
	size_t data_len=0;
	LDRClientControlCommand l_cccmd=
		(LDRClientControlCommand)(ntohs(pack->ctrl_cmd));
	int easy_type=0;
	TaskSource::ClientControlCommand cccmd=TaskSource::CCC_None;
	switch(l_cccmd)
	{
		case LCCC_Kill_UserInterrupt:
			cccmd=TaskSource::CCC_Kill_UserInterrupt; goto goeasy;
		case LCCC_Kill_ServerError:
			cccmd=TaskSource::CCC_Kill_ServerError;   goto goeasy;
		case LCCC_StopJobs:
			cccmd=TaskSource::CCC_StopJobs;           goto goeasy;
		case LCCC_ContJobs:
			cccmd=TaskSource::CCC_ContJobs;           goto goeasy;
		case LCCC_PingPong:                           goto goeasy;
		case LCCC_ClientQuit:                         goto goeasy;
		goeasy:
			easy_type=1;
			min_len=sizeof(LDRClientControlRequest);
			data_len=0;
			break;
		case LCCC_GiveBackTasks:
			easy_type=3;
			data_len=sizeof(LDRCCC_GBT_Req_Data);
			min_len=sizeof(LDRClientControlRequest)+data_len;
			break;
		//default: min_len stays 0
	}
	if(!min_len)
	{
		Error("LDR: Received unknown control request (%d, %u bytes)\n",
			int(l_cccmd),pack->length);
		return(-1);
	}
	if(pack->length<min_len)
	{
		Error(_illsize_msg,
			" (",LDRClientControlCommandString(l_cccmd),")",
			pack->length,min_len);
		return(-1);
	}
	
	// Store the command in the waitresp queue: 
	// Then, tell the task manager about the command. 
	// When it is done, move the command from waitresp to send and send it. 
	// When sent, remove from the send queue. 
	if(easy_type==1)
	{
		u_int16_t thisseq=ntohs(pack->seq);
		int rv=cmd_queue.RespAddEntry(l_cccmd,thisseq,data_len,pack->data);
		CommandQueue::CQEntry *cqent=cmd_queue.queue_waitresp.last();
		if(rv)
		{
			Error("LDR: Failed to store received command (%s, dlen=%u): %s\n",
				LDRClientControlCommandString(l_cccmd),data_len,
				rv==-1 ? cstrings.allocfail : 
					(rv==-2 ? "queue limit exceeded" : "???"));
			return(-1);
		}
		assert(cqent->seq==thisseq);  // otherwise bug in queue
		
		Verbose(TSLLR,"LDR: Received control command: %s\n",
			LDRClientControlCommandString(l_cccmd));
		
		if(cccmd!=TaskSource::CCC_None)
		{
			int rv=back->TellTaskManagerToExecSimpleCCmd(cccmd);
			assert(rv==0);  // There are currently no error codes. 
			
			// Also, stop/cont idle timer in case we get stop/cont: 
			if(keepalive_timeout>=0)
			{
				if(cccmd==TaskSource::CCC_StopJobs)
				{  UpdateTimer(keepalive_tid,-1,0);  }
				else if(cccmd==TaskSource::CCC_ContJobs)
				{  UpdateTimer(keepalive_tid,keepalive_timeout,
					FDAT_FirstLater | FDAT_AlignToShorter | 10);  }
			}
		}
		else if(l_cccmd==LCCC_PingPong || l_cccmd==LCCC_ClientQuit)
		{
			// Send back the ping / client quit...
			// There is nothing to do here (quitting/recovery done when 
			// sent back). 
		}
		else assert(0);  // <-- If that fails you forgot to implement new cccmd. 
		
		// The command was executed immediately, so we can enqueue 
		// it into the send queue (to send the response): 
		// This IS the correct function call (we have queue_side<0): 
		// cqent->datalen=0, so there is nothing to worry about. 
		assert(cqent->datalen==0);  // otherwise: set it to 0 or it will get sent back
		cmd_queue.EntryProcessed(cqent);
	}
	else if(easy_type==3)
	{
		assert(l_cccmd==LCCC_GiveBackTasks);
		
		LDRCCC_GBT_Req_Data *reqdata=(LDRCCC_GBT_Req_Data*)(pack->data);
		int may_keep=ntohl(reqdata->may_keep);
		// This will actually never happen... (int overflow)
		// NOTE though: special value: may_keep=0x7fffffff -> give back 
		//              all not yet processed tasks. 
		if(may_keep<0)
		{  may_keep=0x7fffffff;  }
		
		char may_keep_str[24];
		if(may_keep==0x7fffffff)
		{  strcpy(may_keep_str,"all not processed");  }
		else
		{  snprintf(may_keep_str,24,"%d",may_keep);  }
		Verbose(TSLLR,"LDR: Received control command: %s (may keep: %s)\n",
			LDRClientControlCommandString(l_cccmd),may_keep_str);
		
		int rrv=back->TellTaskManagerToGiveBackTasks(may_keep);
		assert(rrv==0);  // There are currently no error codes. 
		
		// NOTE: (See also: LDRClient::TellClientToGiveBackTasks(): 
		//       We send the response immediately. The tasks to be 
		//       given back are still in done queue. 
		
		const TaskManager_TaskList *tsklist=
			component_db()->taskmanager()->GetTaskList();
		LDRCCC_GBT_Resp_Data respdata;
		respdata.n_todo=htonl(u_int32_t(tsklist->todo_nelem));
		respdata.n_proc=htonl(u_int32_t(tsklist->proc_nelem));
		respdata.n_done=htonl(u_int32_t(tsklist->done_nelem));
		
		u_int16_t thisseq=ntohs(pack->seq);
		int rv=cmd_queue.RespAddEntry(l_cccmd,thisseq,
			/*datalen=*/sizeof(LDRCCC_GBT_Resp_Data),(uchar*)(&respdata));
		CommandQueue::CQEntry *cqent=cmd_queue.queue_waitresp.last();
		if(rv)
		{
			Error("LDR: Failed to store command response (%s, dlen=%u): %s\n",
				LDRClientControlCommandString(l_cccmd),
				sizeof(LDRCCC_GBT_Resp_Data),
				rv==-1 ? cstrings.allocfail : 
					(rv==-2 ? "queue limit exceeded" : "???"));
			return(-1);
		}
		assert(cqent->seq==thisseq);  // otherwise bug in queue
		
		// Command was executed. Move it to send queue now: 
		cmd_queue.EntryProcessed(cqent);
	}
	else
	{
		// Other commands: (None yet)
		// Execute them (or schedule them), then 
		// - call cmd_queue.EntryProcessed(cqent);
		// - store apropriate return code in cqent->data
		// - **  if there is none, set cqent->datalen=0 **
		// - make sure outpump is scheduled to send the entry 
		//   (INCLUDING THE DATA in cqent->data)
		assert(0);
	}
	
	return(0);
}


// Return value: 
//   0 -> not handeled
//   1 -> handeled
//  -1 -> called _ConnClose(). 
int TaskSource_LDR_ServerConn::_AuthSConnFDNotify(FDInfo *fdi)
{
	assert(authenticated);
	
	if(fdi->revents & POLLIN)
	{
		// Read protocol header: 
		// We want to get it atomically. 
		LDRHeader hdr;
		int rv=_AtomicRecvData(&hdr,sizeof(hdr));
		if(rv)
		{  _ConnClose(0);  return(-1);  }
		
		// Check what we got...
		size_t len=_CheckRespHeader(&hdr,sizeof(hdr),sizeof(hdr),0);
		if(!len)
		{  _ConnClose(0);  return(1);  }
		
		rv=_HandleReceivedHeader(&hdr);
		if(rv<0)
		{  _ConnClose(0);  }
		
		// Update response time: 
		_ResetKeepaliveTimeout(fdi->current);
		
		return(rv);
	}
	
	// _AuthSConnFDNotify() is (currently) only called for POLLIN. 
	// We may NOT reach here. 
	assert(0);  // If caught: SIMPLE PROGRAMMING ERROR. 
	_ConnClose(0);
	return(-1);
}


int TaskSource_LDR_ServerConn::fdnotify2(FDInfo *fdi)
{
	Verbose(DBG,"--<LDRSconn:fdnotify2>--<fd=%d, ev=0x%x, rev=0x%x>--\n",
		fdi->fd,int(fdi->events),int(fdi->revents));
	
	#if 0
	assert(pollid==fdi->pollid && fdi->fd==sock_fd);
	#else
	if(pollid!=fdi->pollid || sock_fd!=fdi->fd)
	{
		fprintf(stderr,
			"OOPS: pollid=%p, fdi->pollid=%p, sock_fd=%d, fdi->fd=%d\n",
			pollid,fdi->pollid,sock_fd,fdi ? fdi->fd : -9999);
		if(!pollid && sock_fd<0)
		{
			// We should not be here. HLib's fault, probably. 
			// FIXME: This should be investigated. We come here, in 
			// LDR client when the server disconnects unexpectedly: 
			// DEBUG DUMP:----------------------------------------------------
			// --<cpprogress>--<OUT=4096 bytes>--
			// --<cpprogress>--<IN=12288 bytes>--
			// --<cpnotify>--<OUTfd: status: Final OutputEOF (Hup)>--
			// E: LDR: Server xxx.xx.xxx.xx:51725 disconnected unexpectedly.
			// Deleting file "f0000270.png": Success
			// Deleting file "f0000270.pov": Success
			// ~CompleteTask[270] (left: 4)
			// E: LDR: Unexpected connection close with auth server xxx.xx.xxx.xx:51725.
			// Connection to server xxx.xx.xxx.xx:51725 (duration: 04:21:26.333)
			//   Network stat:  transferred   bytes/sec    hdrs  files
			//     Downstream:        14582       0.930     537      7
			//     Upstream:         981447      62.567     546      2
			// --<LDRSconn:fdnotify2>--<fd=7, ev=0x1, rev=0x19>--
			// OOPS: pollid=(nil), fdi->pollid=0x80b6dd8, sock_fd=-1, fdi->fd=7
			return(0);
		}
		assert(0);
	}
	#endif
	
	int handeled=0;
	
	// See what we must do...
	if(fdi->revents & POLLHUP)
	{
		_ServerDisconnectMessage();
		_ConnClose(0);  return(0);
	}
	if(fdi->revents & POLLIN)
	{
		if(expect_cmd==Cmd_ChallengeResponse)
		{
			int rv=_RecvChallengeResponse();
			if(rv)  // _ConnClose() was called. 
			{  return(0);  }
			handeled=1;
		}
		else if(authenticated)
		{
			handeled=_AuthSConnFDNotify(fdi);
			if(handeled<0)  return(0);
		}
	}
	if(fdi->revents & POLLOUT)
	{
		if(next_send_cmd==Cmd_ChallengeRequest)
		{
			if(_SendChallengeRequest())
			{  _ConnClose(0);  return(0);  }
			handeled=1;
		}
		else if(next_send_cmd==Cmd_NowConnected)
		{
			// NOTE: Do this here only if we deny connection...
			if(now_conn_auth_code!=CAC_Success)
			{
				_SendNowConnectedDenied();
				return(0);
			}
			// ... otherwise we should not be here. 
			// (Get error below -> !handeled)
			assert(authenticated);  // Otherwise: critical bug. 
		}
		// Cmd_NowConnected is already sent via FDCopy facility. 
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
			Error("LDR: %s: connection error: %s\n",
				addr.GetAddress().str(),strerror(errval));
		}
		else
		{
			Error("LDR: %s: unexpected revents=%d. Quitting.\n",
				addr.GetAddress().str(),fdi->revents);
		}
		_ConnClose(0);
	}
	
	return(0);
}


// Return value: 1 -> called _ConnClose(); 0 -> normal
int TaskSource_LDR_ServerConn::cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi)
{
	// ---------<FIRST PART: HANDLE TERMINATION OF CURRENT REQUEST>---------
	
	// next_send_cmd is only used for auth. 
	assert(next_send_cmd==Cmd_NoCommand || out_active_cmd==Cmd_NowConnected);
	
	// This will be decreased for files below. 
	++out.stat.count_headers;   // statistics
	
	Verbose(DBGV,"LDR: Sending done: %s\n",LDRCommandString(out_active_cmd));
	
	switch(out_active_cmd)
	{
		case Cmd_NowConnected:
		{
			// This must still be set. Because other parts relay on it. 
			assert(next_send_cmd==Cmd_NowConnected);
			
			// We may only use the copy facility in this case: 
			assert(now_conn_auth_code==CAC_Success);
			
			//fprintf(stderr,"Successfuly NowConnected sent.\n");
			
			expect_cmd=Cmd_NoCommand;
			next_send_cmd=Cmd_NoCommand;
			now_conn_auth_code=-999;   // do not use 0 = CAC_Success
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_TaskResponse:
		{
			// Okay, finally sent task response. 
			//fprintf(stderr,"TaskResponse sent.\n");
			
			// Clean up the stuff...
			tri.resp_code=-1;  // unset
			// Note that tri.ctsk should be 0 here if we passed it to the 
			// process manager. 
			DELETE(tri.ctsk);
			//tri.task_id and tri.next_action do not matter. 
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_FileRequest:
		{
			// Okay, sent file request. 
			//fprintf(stderr,"FileRequest sent.\n");
			
			assert(tri.resp_code==TRC_Accepted && tri.next_action==TRINA_FileReq);
			// Now, receive files. 
			tri.next_action=TRINA_FileRecvH;
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_TaskDone:
		{
			assert(!outpump_lock); // Not 100% sure if that is correct (?) At least outpump_lock!=IOPL_Upload should be correct. 
			
			// Okay, sent TaskDone. We can now go on uploading the files. 
			//fprintf(stderr,"TaskDone sent.\n");
			
			assert(tdi.next_action==TDINA_SendDone);
			// Now, send files (or maybe final task state). 
			tdi.next_action=TDINA_FileSendH;  // The rest is decided when sending. 
			tdi.upload_file_type=FRFT_None;
			tdi.upload_file=TaskFile();
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_FileUpload:
		{
			#if TESTING
			if(!tdi.done_ctsk || 
			   (tdi.next_action!=TDINA_FileSendH && 
			    tdi.next_action!=TDINA_FileSendB) )
			{  assert(0);  }
			#endif
			
			assert(outpump_lock==IOPL_Upload);
			
			if(tdi.next_action==TDINA_FileSendH)
			{
				// Okay, LDRFileUpload header was sent. Now, we send 
				// the file itself. (next cpnotify_outpump_start()). 
				tdi.next_action=TDINA_FileSendB;
			}
			else
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
				
				Verbose(DBG,"LDR: File upload completed.\n");
				// Go on senting file header (or final state info). 
				tdi.next_action=TDINA_FileSendH;
				tdi.upload_file=TaskFile();
				
				outpump_lock=IOPL_Unlocked;
			}
			
			// This is needed so that cpnotify_outpump_start() won't go mad. 
			send_buf.content=Cmd_NoCommand;
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
		} break;
		case Cmd_DoneComplete:
		{
			// Okay, we're finally through with reporting the task as done. 
			assert(tdi.done_ctsk && tdi.next_action==TDINA_Complete);
			
			CompleteTask *tmp_ctsk=tdi.done_ctsk;
			
			// Reset state: 
			tdi.done_ctsk=NULL;   // We do NOT free it. 
			tdi.next_action=TDINA_None;
			tdi.upload_file=TaskFile();  // be sure
			
			// Tell TaskManager the good news: 
			// (Note: TaskReportedDone() will start 1 schedule timer 
			//  so action cannot happen on the stack.)
			back->TaskReportedDone(tmp_ctsk);
			tmp_ctsk=NULL;  // ...was deleted by TaskReportedDone(). 
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_ControlResponse:
		{
			assert(!cmd_queue.queue_to_send.is_empty());
			assert(sent_cmd_queue_ent);
			
			Verbose(DBGV,"LDR: Removed from resp queue (done): %s\n",
				LDRClientControlCommandString(
					(LDR::LDRClientControlCommand)sent_cmd_queue_ent->cmd));
			
			bool was_quit=(sent_cmd_queue_ent->cmd==LCCC_ClientQuit);
			
			cmd_queue.DoneRemoveEntry(sent_cmd_queue_ent);
			sent_cmd_queue_ent=NULL;
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
			
			if(was_quit)
			{
				// Well, now as we sent the quit control response, 
				// actually disconnect from the server. There is no 
				// point in sendind any data to the server any more 
				// beause it would get pissed at us and immediately 
				// kick us. 
				_ConnClose(/*reason=*/1);
				return(1);
			}
		} break;
		default:
		{
			// out_active_cmd==Cmd_FileDownload -> TRINA_FileReq
			
			// Note: This assertion may never fail. 
			// If it does, then the implementation is incomplete. 
			// (I used that during development.) 
			Error("cpnotify_outpump_done: Hack on... (implementation incomplete)\n");
			assert(0);   // OKAY. 
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
	}
	
	return(0);
}

// Return value: 1 -> called _ConnClose(); 0 -> normal
int TaskSource_LDR_ServerConn::cpnotify_outpump_start()
{
	if(out.ioaction!=IOA_None || out_active_cmd!=Cmd_NoCommand)
	{
		// This only happens if we're not called from cpnotify() but 
		// from somewhere else. (Protected by assert() in cpnotify().) 
		// It means that another copy notify is running and this 
		// function will be called again when it is over. 
		return(0);
	}
	
	// ---------<SECOND PART: LAUNCH NEW REQUEST IF NEEDED>---------
	
	// See what we can send...
	if(must_quit_from_server)
	{
Verbose(DBG,"DO SEND THE QUIT COMMAND...\n");
		_ConnClose(/*reason=*/3);
		return(1);
	}
	else if(next_send_cmd==Cmd_NowConnected)
	{
		// This path is only used if we do NOT deny. 
		assert(!outpump_lock);
		assert(authenticated && now_conn_auth_code==CAC_Success);
		assert(expect_cmd==Cmd_NoCommand);
		
		int fail=0;
		do {
			assert(send_buf.content==Cmd_NoCommand);
			
			fail=_CreateLDRNowConnectedPacket(&send_buf);
			if(fail)  break;
			
			// Okay, make ready to send it: 
			fail=_FDCopyStartSendBuf((LDRNowConnected*)(send_buf.data));
			if(fail)  break;
			
			fail=0;
		} while(0);
		if(fail)
		{
			// Currently, only alloc failure can happen here. 
			assert(fail==-1);
			_AllocFailure(tri.ctsk);
			_ConnClose(0);
			return(1);
		}
	}
	else if(!outpump_lock && !cmd_queue.queue_to_send.is_empty())
	{
		// Control commands have precedence (unless of course, the outpump 
		// is locked) (because we just sent a header (e.g. file upload) 
		// and now MUST send the file body). 
		assert(!sent_cmd_queue_ent);  // may not be active here (bug)
		CommandQueue::CQEntry *cqent=cmd_queue.queue_to_send.first();
		int fail=0;
		do {
			size_t need_len=sizeof(LDRClientControlResponse)+cqent->datalen;
			assert(send_buf.content==Cmd_NoCommand);
			if(_ResizeRespBuf(&send_buf,need_len))
			{  fail=-1;  break;  }
			
			send_buf.content=Cmd_ControlResponse;
			LDRClientControlResponse *pack=
				(LDRClientControlResponse*)(send_buf.data);
			pack->length=need_len;  // host order
			pack->command=Cmd_ControlResponse;  // host order
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
			Error("LDR: Sending control response (%s): %s.\n",
				LDRClientControlCommandString((LDRClientControlCommand)cqent->cmd),
				msg);
			_ConnClose(0);  return(1);
		}
	}
	else if(!outpump_lock && (tri.resp_code>0 ||  // Not -1 "unset" and not TRC_Accepted. 
	   (tri.resp_code==TRC_Accepted && tri.next_action==TRINA_Response)) )
	{
		// tri.resp_code>0 
		//      -> okay, we have to refuse the task. 
		// tri.resp_code==TRC_Accepted && tri.next_action==TRINA_Response 
		//      -> accept the task (TRC_Accepted) 
		int fail=1;
		do {
			assert(send_buf.content==Cmd_NoCommand);
			if(send_buf.alloc_len<sizeof(LDRTaskResponse) && 
			   _ResizeRespBuf(&send_buf,sizeof(LDRTaskResponse)) )
			{  break;  }
			
			// Compose the packet: 
			send_buf.content=Cmd_TaskResponse;
			LDRTaskResponse *pack=(LDRTaskResponse*)(send_buf.data);
			pack->length=sizeof(LDRTaskResponse);  // host order
			pack->command=Cmd_TaskResponse;  // host order
			pack->task_id=htonl(tri.task_id);
			pack->resp_code=htons(tri.resp_code);
			
			// Okay, make ready to send it: 
			if(_FDCopyStartSendBuf(pack))  break;
			
			fail=0;
		} while(0);
		if(fail)
		{
			// Currently, only alloc failure can happen here. 
			_AllocFailure(tri.ctsk);
			_ConnClose(0);
			return(1);
		}
	}
	else if(!outpump_lock && 
		tri.resp_code==TRC_Accepted && tri.next_action==TRINA_FileReq)
	{
		// NO PIPELINING will be supported. You have a fast net and 
		// thus response time is <1msec. Yes, the box is rendering some 
		// other frame; we need not get the last 3% speed improvement. 
		int rv=_StartSendNextFileRequest();
		switch(rv)
		{
			case 1:  _TaskRequestComplete();  break;  // no more file requests
			case 0:  break;
			case -1:  _ConnClose(0);  return(1);
			default: assert(0);
		}
	}
	else if(tdi.done_ctsk)
	{
		if(!outpump_lock && tdi.next_action==TDINA_SendDone)
		{
			// We send the LDRTaskDone packet. 
			int fail=1;
			do {
				assert(send_buf.content==Cmd_NoCommand);
				if(_ResizeRespBuf(&send_buf,sizeof(LDRTaskDone)))
				{  break;  }
				
				// Compose the packet: 
				send_buf.content=Cmd_TaskDone;
				LDRTaskDone *pack=(LDRTaskDone*)(send_buf.data);
				pack->length=sizeof(LDRTaskDone);  // host order
				pack->command=Cmd_TaskDone;  // host order
				pack->frame_no=htonl(tdi.done_ctsk->frame_no);
				pack->task_id=htonl(tdi.done_ctsk->task_id);
				LDRStoreTaskExecutionStatus(&pack->rtes,&tdi.done_ctsk->rtes.tes);
				LDRStoreTaskExecutionStatus(&pack->ftes,&tdi.done_ctsk->ftes.tes);
				
				// Okay, make ready to send it: 
				if(_FDCopyStartSendBuf(pack))  break;
				
				fail=0;
			} while(0);
			if(fail)
			{
				// Currently. only alloc failure can happen here. 
				_AllocFailure(tri.ctsk);
				_ConnClose(0);
				return(1);
			}
		}
		else if(!outpump_lock && tdi.next_action==TDINA_FileSendH)
		{
			// This either sends the next file upload header or final 
			// completion (done completely) and sets the params 
			// of the file in question in tdi. 
			int rv=_SendNextFileUploadHdr();
			// rv: 0 -> Sending next file upload header; 
			//     1 -> sending "done completely"
			//    -1 -> error
			if(rv==0)
			{  outpump_lock=IOPL_Upload;  }
			else if(rv<0)
			{
				// Message already written. 
				_ConnClose(0);
				return(1);
			}
		}
		else if(tdi.next_action==TDINA_FileSendB)
		{
			assert(outpump_lock==IOPL_Upload);
			
			// Finally, send the file body. 
			
			// If this assert fails, then we're in trouble. 
			// The stuff should have been set earlier. 
			assert(!!tdi.upload_file && tdi.upload_file_size>=0);
			
			assert(out_active_cmd==Cmd_NoCommand);
			int rv=_FDCopyStartSendFile(tdi.upload_file.HDPathRV().str(),
				tdi.upload_file_size);
			if(rv)
			{
				if(rv==-1)
				{
					_AllocFailure(tdi.done_ctsk);
					_ConnClose(0);
					return(1);
				}
				else if(rv==-2)
				{
					int errn=errno;
					Error("LDR: Failed to open output file \"%s\" for upload: "
						"%s [frame %d] Giving up.\n",
						tdi.upload_file.HDPathRV().str(),
						strerror(errn),tdi.done_ctsk->frame_no);
					// Okay, so this means that we already sent the request 
					// header but are not able to supply the actual file. 
					// This is bad and so we quit here. 
					// Maybe quitting seems a bit hard but there is not much 
					// point in doing work if we cannot deliver the result. 
					// OTOH, what about the other tasks... grmbl.. ###FIXME###
					// Fix: Keep it reasonable. This error is unlikely to 
					// happen and if it does, oh well, the other tasks get 
					// lost, too. 
					_ConnClose(0);
					return(1);
				}
				else assert(0);  // rv=-3 may not happen here 
			}
			
			out_active_cmd=Cmd_FileUpload;
		}
		else assert(0);  // illegal internal state (forgot to clear tdi.done_ctsk?)
	}
	// else: Nothing to do (we're waiting). 
	
	return(0);
}

// Return value: 1 -> called _ConnClose(); 0 -> normal
int TaskSource_LDR_ServerConn::cpnotify_inpump(FDCopyBase::CopyInfo *cpi)
{
	int dont_start_outpump=0;
	
	switch(in_active_cmd)
	{
		case Cmd_TaskRequest:
		{
			assert(cpi->pump==in.pump_s);  // can be left away
			
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			// NOTE: This looks like something strange but it is an 
			// internal error if it fails. Because we tell the pump to 
			// copy exactly LDRHeader->length - sizeof(LDRHeader) bytes 
			// and if it reports SCLimit here, then they have to be 
			// there. 
			#if TESTING
			FDCopyIO_Buf *_dst=(FDCopyIO_Buf*)(cpi->pump->Dest());
			if(_dst->bufdone+sizeof(LDRHeader)!=
			   ((LDRHeader*)(recv_buf.data))->length)
			{  assert(0);  }
			#endif
			
			int rv=_ParseTaskRequest(&recv_buf);
			if(rv==1)
			{  _ConnClose(0);  return(1);  }
			else if(rv==2)
			{
				assert(tri.resp_code>0);
				if(cpnotify_outpump_start()==1)
				{  return(1);  }
			}
			else if(rv==3)
			{  return(1);  }
			else assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_FileDownload:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			if(tri.next_action==TRINA_FileRecvB)
			{
				// Read in body (complete). 
				++in.stat.count_files;   // statistics
				
				// If we used the FD->FD pump, we must close the output file. 
				if(cpi->pump->Dest()->Type()==FDCopyIO::CPT_FD)
				{
					assert(cpi->pump==in.pump_fd && cpi->pump->Dest()==in.io_fd);
					if(CloseFD(in.io_fd->pollid)<0)
					{
						int errn=errno;
						Error("LDR: While closing \"%s\": %s\n",
							tri.req_tfile.HDPathRV().str(),strerror(errn));
						_ConnClose(0);  return(1);
					}
				}
				
				// File downloading done. 
				// Store the server modification time (reported to 
				// us before downloading) in the TaskFile: 
				int64_t remote_size=tri.req_tfile.GetFixedState(
					/*save it here:*/tri.req_tfile.GetDlSrvMTime());
				// Check: Must have called SetFixedState(). 
				// Only one exception: The render output when resuming. 
				bool fixed_state_exception=
					(tri.req_file_type==FRFT_RenderOut && 
					 tri.ctsk->rt && tri.ctsk->rt->resume);
				assert(fixed_state_exception || remote_size>=0);
				
				int rv=tri.req_tfile.ClearFixedState();
				if(fixed_state_exception)
				{  tri.req_tfile.SetDownload(/*s=*/ 2 /* "downloaded" */);  }
				else
				{  assert(!rv);  }   // Must have been in fixed state.
				
				assert(tri.req_tfile.WasDownloaded());
				tri.req_tfile=TaskFile();
				tri.next_action=TRINA_FileReq;
			}
			else
			{
				assert(cpi->pump==in.pump_s);  // can be left away
				
				int rv=_ParseFileDownload(&recv_buf);
				if(rv<0)
				{  _ConnClose(0);  return(1);  }
				assert(rv==0);    // else: unknown error code
				
				// dont_start_outpump=1; previously set with comment: 
				//   because we're waiting for file body.  WHAT A REASON?
			}
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_ControlRequest:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseCommandRequest(&recv_buf);
			if(rv<0)
			{  _ConnClose(0);  return(1);  }
			assert(rv==0);    // else: unknown error code
			
			recv_buf.content=Cmd_NoCommand;
		} break;
		default:
			// This is an internal error. Only known packets may be accepted 
			// in _HandleReceivedHeader(). 
			Error("cpnotify_inpump: Done; hack on... (Implementation incomplete)\n");
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
//   1 -> _ConnClose() called
int TaskSource_LDR_ServerConn::cpnotify_handle_errors(FDCopyBase::CopyInfo *cpi)
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
		Error("LDR: Filesys/network IO error: %s\n",CPNotifyStatusString(cpi));
		cpi->scode=(FDCopyPump::StatusCode)(cpi->scode|FDCopyPump::SCFinal);
		int errval=GetSocketError(sock_fd);
		if(errval)
		{  Error("     network socket: %s%s (%d)\n",
			errval==-1 ? "getsockopt failed: " : strerror(errval),
			errval==-1 ? strerror(errno) : "",errval);  }
		if(cpi->pump==out.pump_fd && out_active_cmd==Cmd_FileUpload && 
		   tdi.next_action==TDINA_FileSendB)
		{
			assert(tdi.done_ctsk);  // can be left away
			Error("   after %lld/%lld bytes while reading \"%s\".\n",
				out.io_fd->transferred,tdi.upload_file_size,
				tdi.upload_file.HDPathRV().str());
			// Closing file done by _ConnClose(). 
		}
		if(cpi->pump==in.pump_fd && in_active_cmd==Cmd_FileDownload && 
			tri.next_action==TRINA_FileRecvB)
		{
			assert(tri.ctsk);  // otherwise: BUG
			Error("   after %lld/%lld bytes while writing \"%s\".\n",
				in.io_fd->transferred,tri.req_tfile.GetFixedState(),
				tri.req_tfile.HDPathRV().str());
			// Closing file done by _ConnClose(). 
		}
		_ConnClose(0);  return(1);
	}
	else
	{
		if(cpi->scode & FDCopyPump::SCEOI)
		{
			// End of input...
			if(cpi->pump==in.pump_fd || cpi->pump==in.pump_s)
			{
				// This means, the server disconnected. 
				_ServerDisconnectMessage();
				_ConnClose(0);  return(1);
			}
			else if(cpi->pump==out.pump_fd)
			{
				// This means, we are sending a file and encountered EOF. 
				// Probably early EOF because the file changed its size. 
				if(out_active_cmd==Cmd_FileUpload && 
				   tdi.next_action==TDINA_FileSendB)
				{
					assert(tdi.done_ctsk);  // can be left away
					assert(outpump_lock==IOPL_Upload);  // can be left away
					
					assert(cpi->pump->Src()==out.io_fd);
					Error("LDR: Early EOF (after %lld/%lld bytes) while "
						"sending \"%s\" for upload. Kicking.\n",
						out.io_fd->transferred,tdi.upload_file_size,
						tdi.upload_file.HDPathRV().str());
					
					// As we used the FD->FD pump, we must close the input file. 
					// Done by _ConnClose(): 
					_ConnClose(0);  return(1);
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
				_ServerDisconnectMessage();
				_ConnClose(0);  return(1);
			}
			else if(cpi->pump==in.pump_fd)
			{
				if(in_active_cmd==Cmd_FileDownload && 
					tri.next_action==TRINA_FileRecvB)
				{
					assert(tri.ctsk);  // otherwise: BUG
					
					assert(cpi->pump->Dest()==in.io_fd);  // otherwise: BUG!
					Error("LDR: Mysterious output EOF (after %lld/%lld bytes) "
						"while receiving \"%s\" from download. Kicking.\n",
						in.io_fd->transferred,tri.req_tfile.GetFixedState(),
						tri.req_tfile.HDPathRV().str());
					
					// As we used the FD->FD pump, we must close the output file. 
					// Done by _ConnClose(). 
					
					_ConnClose(0);  return(1);
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
		Error("LDR: Internal (?) error: Received illegal cpnotify.\n"
			"  pump: %s, src=%d, dest=%d; code: %s\n",
			pumpstr,
			cpi->pump->Src() ? cpi->pump->Src()->Type() : (-1),
			cpi->pump->Dest() ? cpi->pump->Dest()->Type() : (-1),
			CPNotifyStatusString(cpi));
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


int TaskSource_LDR_ServerConn::cpnotify(FDCopyBase::CopyInfo *cpi)
{
	Verbose(DBG,"--<cpnotify>--<%s%s: %s>--\n",
		(cpi->pump==in.pump_s || cpi->pump==in.pump_fd) ? "IN" : "OUT",
		(cpi->pump==in.pump_s || cpi->pump==out.pump_s) ? "buf" : "fd",
		CPNotifyStatusString(cpi));
	/*fprintf(stderr,"transf=%ld/%ld, %ld/%ld;  len=%u/%u, %u/%u;  sock=%ld/%ld, %ld/%ld\n",
		long(out.io_fd->transferred),long(out.pump_fd->limit),
		long(in.io_fd->transferred),long(in.pump_fd->limit),
		out.io_buf->bufdone,out.io_buf->buflen,
		in.io_buf->bufdone,in.io_buf->buflen,
		long(out.io_sock->transferred),long(out.pump_fd->limit),
		long(in.io_sock->transferred),long(in.pump_fd->limit) );*/
	
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
	
	// next_send_cmd is only used for auth:  
	// Cmd_NowConnected is sent via copy facility. 
	assert(next_send_cmd==Cmd_NoCommand || next_send_cmd==Cmd_NowConnected);
	assert(expect_cmd==Cmd_NoCommand);
	
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
			// next_send_cmd is only used for auth:  
			assert(next_send_cmd==Cmd_NoCommand);
			
			if(cpnotify_inpump(cpi))  break;  // _ConnClose() was called. 
			_ResetKeepaliveTimeout(cpi->fdtime);
			
			// We're always listening to the server. 
			if(in_active_cmd==Cmd_NoCommand)
			{  _DoPollFD(POLLIN,0);  }
		}
		else if(cpi->pump==out.pump_s || cpi->pump==out.pump_fd)
		{
			if(cpnotify_outpump_done(cpi))  break;  // _ConnClose() was called. 
			
			assert(out.ioaction==IOA_None);  // If FDCopyPump is running, we may not be here. 
			if(cpnotify_outpump_start())  break;  // _ConnClose() was called. 
		}
		else assert(0);
	} while(0);
	
	return(0);
}


int TaskSource_LDR_ServerConn::cpprogress(FDCopyBase::ProgressInfo *pi)
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


int TaskSource_LDR_ServerConn::timernotify(FDBase::TimerInfo *ti)
{
	Verbose(DBGV,"--<LDRSconn::timernotify>--\n");
	
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
			return(0);
		}
		else assert(0);
	}
	else if(ti->tid==keepalive_tid)
	{
		if(authenticated)
		{
			// Okay, we may not disconnect just because we got no 
			// special packets from the server when we're just 
			// up/downloading some really large file. So, there is 
			// last_datapump_io_time... Instead of using this 
			// time var, I could also have used _ResetKeepaliveTimeout() 
			// each time the copying shows progress but that was 
			// too much overhead for me. 
			bool is_ok=0;
			do {
				if(last_datapump_io_time.IsInvalid())  break;
				
				HTime _delta=(*ti->current)-last_datapump_io_time;
				int64_t delta=_delta.GetL(HTime::msec);
				if(delta>=keepalive_timeout)  break;
				
				// It's okay, the server is still living. 
				// Update timer to reflect time(out) difference. 
				long to_val=keepalive_timeout;
				if(delta>0)
				{  to_val-=(long)delta;  }
				UpdateTimer(keepalive_tid,to_val,
					FDAT_FirstLater | FDAT_AlignToShorter | 10);
				is_ok=1;
			} while(0);
			
			if(!is_ok)
			{
				Error("LDR: Server %s idle timeout (%ld msec) passed. Disconnecting.\n",
					addr.GetAddress().str(),keepalive_timeout);
				
				_ConnClose(0);
			}
		}
		else
		{
			Error("LDR: Server %s timed out during auth.\n",
				addr.GetAddress().str());
			
			_ConnClose(2);  // "auth failure"
		}
	}
	else assert(0);  // unknown timer
	
	return(0);
}


// Called to close down the connection or if it was closed down. 
// reason: 0 -> general
//         1 -> received LCCC_ClientQuit
//         2 -> auth failure
//         3 -> as told by QuitFromServerNow()
void TaskSource_LDR_ServerConn::_ConnClose(int reason)
{
	// We should not call ConnClose() twice. 
	// This assert is the bug trap for that: 
	assert(!DeletePending());
	
	if(outpump_scheduled)
	{
		outpump_scheduled=0;
		UpdateTimer(schedule_tid,-1,0);
	}
	
	if(sock_fd>=0)  // Otherwise: Not connected or already disconnected. 
	{
		// Make sure we close files: 
		if(out.io_fd && out.io_fd->pollid && CloseFD(out.io_fd->pollid)<0)
		{  hack_assert(0);  }  // Actually, this may not fail, right?
		if(in.io_fd && in.io_fd->pollid && CloseFD(in.io_fd->pollid)<0)
		{
			int errn=errno;
			// This may write "(null)" as file name... 
			Error("LDR: While closing \"%s\": %s\n",
				tri.req_tfile.HDPathRV().str(),strerror(errn));
			// Do not call _ConnClose(0)... we're already there!
		}
		
		// Make sure we close down. 
		_ShutdownConnection();
	}
	
	// Make sure there are no dangling task file references: 
	tri.req_tfile=TaskFile();
	tdi.upload_file=TaskFile();
	
	// Make sure we get rid of a task which may be on the flight. 
	// Also read comment in destructor (~TaskSource_LDR_ServerConn): 
	DELETE(tri.ctsk);
	DELETE(tdi.done_ctsk);
	
	back->ConnClose(this,reason);
}


void TaskSource_LDR_ServerConn::TellServerDoneTask(CompleteTask *ctsk)
{
	// So, we have to tell the LDR server that the passed task was done. 
	assert(ctsk);   // may not be NULL here. 
	// There may not be a further task which waits for getting reported as done. 
	// This is due to the fact that a task source can always only operate 
	// on one request. 
	assert(!tdi.done_ctsk);
	// Hmmm.... Can DeletePending() be set here...? [_ConnClose()...]
	assert(authenticated && !DeletePending());
	
	// Okay, begin with it...
	tdi.done_ctsk=ctsk;
	tdi.next_action=TDINA_SendDone;
	
	schedule_outpump_start();
}


int TaskSource_LDR_ServerConn::QuitFromServerNow()
{
	if(!authenticated || sock_fd<0)
	{  return(1);  }
	
	//Verbose(TSLLR,"LDR: Sending quit to server %s, then disconnecting...\n",
	//	addr.GetAddress().str());
	
	// Send a last command and quit: 
	must_quit_from_server=1;
	schedule_outpump_start();
	
	return(0);
}


int TaskSource_LDR_ServerConn::DontWantMoreTasks()
{
	if(!authenticated || sock_fd<0)
	{  return(1);  }
	
	// This will merely tell the server that we do not want more tasks. 
	// There is no guarantee that the server will NOT send more tasks. 
	// In this case, we feed them just normally into TaskManager which 
	// will give them right back. 
	// This is not a problem because the server should behave correctly. 
	// The real issue is that there might be a task on the way 
	// (half-downloaded). 
	
	// Schedule sending of a client notification control command: 
	LDRClientControlCommand cccmd=LCCC_CN_NoMoreTasks;
	Verbose(TSLLR,"LDR: Telling server we do not want more tasks.\n");
	
	// Due to pos=-1, the command is inserted at the BEGINNING. 
	int rv=cmd_queue.AddSendEntry(/*pos=*/-1,cccmd);
	if(rv<0)
	{
		// Alloc failure or queue full. Should never happen. 
		// Quit. 
		Error("LDR: Failure to enqueue LDR command (%s): %s\n",
			LDRClientControlCommandString(cccmd),
			rv==-1 ? cstrings.allocfail : 
				(rv==-2 ? "queue limit exceeded" : "???"));
		_ConnClose(0);
		return(-1);
	}
	
	cpnotify_outpump_start();
	return(0);
}


int TaskSource_LDR_ServerConn::Setup(int sock,MyAddrInfo *_addr)
{
	if(PollFD(sock,POLLOUT,NULL,&pollid)<0)
	{  return(-1);  }
	
	assert(pollid);  // Otherwise PollFD() should have returned error. 
	
	addr=*_addr;  // implicit copy
	sock_fd=sock;
	next_send_cmd=Cmd_ChallengeRequest;
	
	in.io_sock->pollid=pollid;
	in.io_sock->max_iolen=4096;
	out.io_sock->pollid=pollid;
	out.io_sock->max_iolen=4096;
	
	// Set auth timeout: 
	if(P()->auth_timeout>=0)
	{
		UpdateTimer(keepalive_tid,P()->auth_timeout,
			FDAT_FirstLater | FDAT_AlignToShorter | 10);
	}
	
	connected_since=HTime::Curr;
	
	return(0);
}


TaskSource_LDR_ServerConn::TaskSource_LDR_ServerConn(TaskSource_LDR *_back,
	int *failflag) : 
	LinkedListBase<TaskSource_LDR_ServerConn>(),
	NetworkIOBase_LDR(failflag),
	chreq_send_time(HTime::Invalid),
	mtime_corr(HTime::Invalid),
	server_client_dtmin(HTime::Invalid),
	server_client_dtmax(HTime::Invalid),
	tri(failflag),
	tdi(failflag),
	cmd_queue(/*queue_side=*/-1,failflag),
	last_datapump_io_time(HTime::Invalid),
	connected_since(HTime::Invalid),
	addr(failflag)
{
	int failed=0;
	
	back=_back;
	
	authenticated=0;
	outpump_scheduled=0;
	
	now_conn_auth_code=-999;  // Do not use 0 (which is CAC_Success). 
	
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	outpump_lock=IOPL_Unlocked;
	
	tri.ctsk=NULL;
	tri.task_id=0;
	tri.resp_code=-1;
	
	tdi.done_ctsk=NULL;
	tdi.next_action=TDINA_None;
	tdi.upload_file_type=FRFT_None;
	
	sent_cmd_queue_ent=NULL;
	
	must_quit_from_server=0;
	keepalive_timeout=-1;
	
	memset(expect_chresp,0,LDRChallengeRespLength);
	
	schedule_tid=InstallTimer(-1,0);
	if(!schedule_tid)  ++failed;
	
	keepalive_tid=InstallTimer(-1,0);
	if(!keepalive_tid)  ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSource_LDR_ServerConn");  }
}

TaskSource_LDR_ServerConn::~TaskSource_LDR_ServerConn()
{
	memset(expect_chresp,0,LDRChallengeRespLength);
	
	if(sock_fd>=0)
	{
		Warning("LDR: Still connected to %s; disconnecting. [OOPS!]\n",
			addr.GetAddress().str());
		ShutdownFD(sock_fd);  pollid=NULL;
	}
	
	const char *_dangl_msg="LDR: OOPS... Dangling task [frame %d] left. (%s)\n";
	// Okay, tdi.done_ctsk was given to us by TaskManager. 
	// He no longer owns it, so it is TaskSource_LDR_ServerConn's 
	// responsibility to delete it here as last chance. 
	if(tdi.done_ctsk)
	{
		Warning(_dangl_msg,tdi.done_ctsk->frame_no,"done");
		DELETE(tdi.done_ctsk);
	}
	// There is NO time gap / schedule between the call to tsnotify() 
	// (in which TaskManager enqueues tri.ctsk in his queue) and 
	// TaskManagerGotTask() (where we set tri.ctsk=NULL), so if it is 
	// non-NULL here, we have to delete it. 
	if(tri.ctsk)
	{
		Warning(_dangl_msg,tri.ctsk->frame_no,"req");
		DELETE(tri.ctsk);
	}
	
	back=NULL;
}
