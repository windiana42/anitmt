/*
 * ldrsconn.cpp
 * 
 * LDR task source server connection. 
 * 
 * Copyright (c) 2002 -- 2003 by Wolfgang Wieser (wwieser@gmx.de) 
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


static inline long _timeout_ntoh(u_int32_t x)
{
	return((x==0xffffffffU) ? (-1) : long(ntohl(x)));
}


inline TaskSourceFactory_LDR *TaskSource_LDR_ServerConn::P()
{  return(back->P());  }
inline ComponentDataBase *TaskSource_LDR_ServerConn::component_db()
{  return(P()->component_db());  }


void TaskSource_LDR_ServerConn::schedule_outpump_start()
{
	if(!outpump_scheduled)
	{
		UpdateTimer(schedule_tid,0,0);
		outpump_scheduled=1;
	}
}


// Actually send data; only do if you may POLLOUT. 
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
	if(size_t(wr)<len)
	{
		Error("LDR: Short write (sent %u/%u bytes) to %s.\n",
			size_t(wr),len,addr.GetAddress().str());
		return(-2);
	}
	return(0);
}

// RETURNS HEADER IN HOST ORDER. 
// Return value: 0 -> OK; -1 -> error; 1 -> read quit packet
int TaskSource_LDR_ServerConn::_AtomicRecvData(LDRHeader *d,
	size_t len)
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
	if(!rd)
	{
		assert(len);  // We may not attempt zero-sized reads. 
		Error("LDR: %s disconnected unexpectedly.\n",
			addr.GetAddress().str());
		return(-1);
	}
	if(size_t(rd)>=sizeof(LDRHeader))
	{
		// Translate to host order: 
		d->length=ntohl(d->length);
		d->command=ntohs(d->command);
	}
	if(size_t(rd)==sizeof(LDRQuitNow))
	{
		LDRQuitNow *qn=(LDRQuitNow *)d;
		if(qn->command==Cmd_QuitNow && qn->length==sizeof(LDRQuitNow))
		{
			// This actually is a quit command. 
			return(1);  // Yes, +1.
		}	
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
		if(last_recv_cmd!=expect_cmd)
		{  Error("LDR: conversation error with %s (expected: %s; received: %s)\n",
			addr.GetAddress().str(),
			LDRCommandString(expect_cmd),
			LDRCommandString(last_recv_cmd));  break;  }
		size_t len=d->length;
		if(len<min_len || len>max_len)
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
	#warning missing: challenge (currently NULL)
	//d.challenge
	
	// That was the packet. Send it. 
	// I assume the challenge request is so small (64 bytes or so) 
	// that it can be sent atomically. Otherwise, we give up. 
	if(_AtomicSendData(&d))  return(-1);
	
	// Fill in what we expect: 
	LDRComputeCallengeResponse(&d,expect_chresp,back->P()->password.str());
	
	expect_cmd=Cmd_ChallengeResponse;
	next_send_cmd=Cmd_NoCommand;
	_DoPollFD(POLLIN,POLLOUT);
	
	return(0);
}


// Retval: -1 -> error (_ConnClose() MUST BE called); 0 -> OK; 1 -> got quit
int TaskSource_LDR_ServerConn::_RecvChallengeResponse()
{
	assert(!authenticated);
	
	LDRChallengeResponse d;
	int rv=_AtomicRecvData(&d,sizeof(d));
	if(rv)  {  return(rv);  }
	
	// Check what we got...
	size_t len=_CheckRespHeader(&d,sizeof(d),sizeof(d),sizeof(d));
	if(!len)  return(-1);
	
	// See if the response is okay: 
	int authcode=100;  // illegal value
	if(memcmp(expect_chresp,d.response,LDRChallengeLength))
	{
		Error("LDR: Illegal challenge response from %s.\n",
			addr.GetAddress().str());
		authcode=CAC_AuthFailed;
	}
	else if(back->GetAuthenticatedServer() || back->Recovering())
	{
		Warning("LDR: Denying conn to %s (%s%s).\n",
			addr.GetAddress().str(),
			back->Recovering() ? "currently recovering" : "already connected to ",
			back->GetAuthenticatedServer() ? 
				back->GetAuthenticatedServer()->addr.GetAddress().str() : "");
		authcode=CAC_AlreadyConnected;
	}
	else
	{
		// So, we must set the authenticated flag here. 
		authenticated=1;
		
		// Okay, correct challenge and no other authenticated server. 
		VerboseSpecial("LDR: Now connected to %s (%.*s).",
			addr.GetAddress().str(),
			LDRIDStringLength,d.id_string);
		authcode=CAC_Success;
		
		// And we must make sure that sc becomes the 
		// first elem in the list: 
		back->ServerHasNowAuthenticated(this);
	}
	memset(expect_chresp,0,LDRChallengeLength);
	now_conn_auth_code=authcode;
	
	expect_cmd=Cmd_NoCommand;
	next_send_cmd=Cmd_NowConnected;
	_DoPollFD(POLLOUT,POLLIN);
	
	return(0);
}


// Return value: 0 -> OK; -1 -> _ConnClose() called. 
int TaskSource_LDR_ServerConn::_SendNowConnected()
{
	assert(!authenticated || now_conn_auth_code==CAC_Success);
	
	LDRNowConnected *dummy=NULL;
	size_t len = (now_conn_auth_code==CAC_Success) ? 
		sizeof(LDRNowConnected) : 
		(((char*)&dummy->auth_code)-((char*)dummy)+sizeof(dummy->auth_code));
	char tmp[len];
	LDRNowConnected *d=(LDRNowConnected *)tmp;
	d->length=len;  // STILL IN HOST ORDER
	d->command=Cmd_NowConnected;  // STILL IN HOST ORDER
	d->auth_code=htons(now_conn_auth_code);
	if(now_conn_auth_code==CAC_Success)
	{
		TaskManager *taskman=component_db()->taskmanager();
		d->njobs=htons(taskman->Get_njobs());
		d->task_thresh_high=
			htons(taskman->Get_todo_thresh_high()+taskman->Get_njobs());
		HTime2LDRTime(taskman->Get_starttime(),&d->starttime);
		int lv=::GetLoadValue();
		d->loadval = (lv>=0 && lv<0xffff) ? htons(lv) : 0xffffU;
	}
	
	// That was the packet. Send it. 
	// I assume the challenge request is so small (64 bytes or so) 
	// that it can be sent atomically. Otherwise, we give up. 
	if(_AtomicSendData(d))
	{  _ConnClose(0);  return(-1);  }
	
	if(now_conn_auth_code!=CAC_Success)
	{
		now_conn_auth_code=0;
		_ConnClose(2);
		return(-1);
	}
	
	// now_conn_auth_code==CAC_Success here. 
	expect_cmd=Cmd_NoCommand;
	next_send_cmd=Cmd_NoCommand;
	_DoPollFD(POLLIN,POLLOUT);
	now_conn_auth_code=0;
	// out.ioaction, in.ioaction currently IOA_Locked: 
	out.ioaction=IOA_None;
	in.ioaction=IOA_None;
	
	return(0);
}


// Header in HOST order. 
// Return value: 
//  1 -> handeled; -1 -> must call _ConnClose(). 
int TaskSource_LDR_ServerConn::_HandleReceivedHeader(LDRHeader *hdr)
{
	// BE CAREFUL!! hdr ALLOCATED ON THE STACK. 
	
	// See what we receive: 
	LDRCommand recv_cmd=LDRCommand(hdr->command);
	
	fprintf(stderr,"Received header: >%s< (length=%u)\n",
		LDRCommandString(recv_cmd),hdr->length);
	
	switch(recv_cmd)
	{
		case Cmd_TaskRequest:
		case Cmd_FileDownload:
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
	
	Error("LDR: Received unexpected command header %s (cmd=%u, length=%u).\n",
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
	back->TellTaskManagerToGetTask(tri.ctsk);
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
	
	#warning hack this check (too many tasks)
	if(0  /*we_have_enough_tasks_already() FIXME ##*/)
	{
		DELETE(tri.ctsk);
		tri.resp_code=TRC_TooManyTasks;
		return(2);
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
	
	int f_desc_slen=ntohs(pack->f_desc_slen);
	size_t f_add_args_size=ntohl(pack->f_add_args_size);
	
	int r_n_files=ntohs(pack->r_n_files);
	int f_n_files=ntohs(pack->f_n_files);
	
	char *ptr=(char*)pack->data;
	char *end_ptr=ptr+pack->length;
	int64_t lenaccu=0;   // I'm paranoid here with overflows...
	
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
	#warning check task ID
	
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
	#warning missing checks with r_n_files, f_n_files (packet size overflow)
	if(!rdesc_name && 
	   (r_oformat_slen || r_add_args_size || r_n_files))
	{
		Error("%s%d] containing unecessary render task info.\n",_rtr,frame_no);
		return(1);
	}
	if(!fdesc_name && 
	   (f_add_args_size || f_n_files))
	{
		Error("%s%d] containing unecessary filter task info.\n",_rtr,frame_no);
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
		
		rt->rdesc=rdesc;
		rt->width=ntohs(pack->r_width);
		rt->height=ntohs(pack->r_height);
		rt->oformat=r_oformat;
		rt->timeout=_timeout_ntoh(pack->r_timeout);
		rt->resume=0;
		
		u_int16_t r_flags=ntohs(pack->r_flags);
		if((r_flags & TRRF_Unfinished) && rdesc->can_resume_render)
		{  rt->resume=1;  }
		// Currently, the only flag we know is the TRRF_Unfinished: 
		if((r_flags & ~TRRF_Unfinished)!=0)
		{
			Error("%s%d] with illegal flags 0x%x.\n",_rtr,frame_no,r_flags);
			delete ctsk;  return(1);
		}
		
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
	}
	if(fdesc)
	{
		FilterTask *ft=NEW<FilterTask>();
		if(!ft)
		{  delete ctsk;  return(-1);  }  // NO need to delete ctsk->rt (done by ~CompleteTask())
		ctsk->ft=ft;
		
		ft->fdesc=fdesc;
		ft->timeout=_timeout_ntoh(pack->f_timeout);
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
			_AllocFailure(ctsk,rv==-1);
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
			_AllocFailure(ctsk,rv==-1);
			return(-1);
		}
		else assert(rv==0);  // Only rv=0, -1 and -2 known. 
	}
	
	// Assign input and output files. 
	int sv=_TaskRequest_SetUpTaskFiles(ctsk);
	if(sv==-1)
	{
		_AllocFailure(ctsk,/*fail=*/-1);
		delete ctsk;
		return(-1);
	}
	else assert(sv==0);
	
	#if 1
	// Okay, dump to the user: 
Error("The following should not be dumped here, right?\n");
	fprintf(stderr,"Received task request:\n");
	fprintf(stderr,"  frame_no=%u; id=%u\n",
		ctsk->frame_no,ctsk->task_id);
	if(ctsk->rt)
	{
		fprintf(stderr,"  Render: %s (%ux%u; timeout=%ld; oformat=%s)%s\n",
			ctsk->rt->rdesc->name.str(),
			ctsk->rt->width,ctsk->rt->height,ctsk->rt->timeout,
			  ctsk->rt->oformat->name,
			ctsk->rt->resume ? " [resume]" : "");
		fprintf(stderr,"    Args:");
		for(const RefStrList::Node *i=ctsk->rt->add_args.first(); i; i=i->next)
		{  fprintf(stderr," %s",i->str());  }
		fprintf(stderr,"\n");
		fprintf(stderr,"    Add files: %d\n",ctsk->radd.nfiles);
	}
	else
	{  fprintf(stderr,"  Render: [none]\n");  }
	if(ctsk->ft)
	{
		fprintf(stderr,"  Filter: %s (timeout=%ld)\n",
			ctsk->ft->fdesc->name.str(),
			ctsk->ft->timeout);
		fprintf(stderr,"    Args:");
		for(const RefStrList::Node *i=ctsk->ft->add_args.first(); i; i=i->next)
		{  fprintf(stderr," %s",i->str());  }
		fprintf(stderr,"\n");
		fprintf(stderr,"    Add files: %d\n",ctsk->fadd.nfiles);
	}
	else
	{  fprintf(stderr,"  Filter: [none]\n");  }
	#endif
	
	// Okay, so let's fill in the success code and go on. 
	tri->resp_code=TRC_Accepted;
	tri->ctsk=ctsk;
	tri->next_action=TRINA_None;
	
	return(0);
}


// Used by _ParseTaskRequest_Intrnl(): Set up TaskFile stuff. 
int TaskSource_LDR_ServerConn::_TaskRequest_SetUpTaskFiles(CompleteTask *ctsk)
{
	// Set up: 
	// infile,outfile,wdir (rt and ft)
	
	#warning "thes are quick hacks; must be able to set rdir/fdir"
	if(ctsk->rt)
	{
		RenderTask *rt=(RenderTask*)ctsk->rt;
		if(rt->wdir.set("."))  return(-1);
		RefString infile,outfile;
		if(infile.sprintf(0,"f-%08x-%07d.a",ctsk->task_id,ctsk->frame_no))  return(-1);
		if(outfile.sprintf(0,"f-%08x-%07d.b",ctsk->task_id,ctsk->frame_no))  return(-1);
		rt->infile=TaskFile::GetTaskFile(infile,
			TaskFile::FTFrame,TaskFile::IOTRenderInput,TaskFile::FCLDR);
		rt->outfile=TaskFile::GetTaskFile(outfile,
			TaskFile::FTImage,TaskFile::IOTRenderOutput,TaskFile::FCLDR);
		if(!rt->infile || !rt->outfile)  return(-1);
		if(rt->resume)
		{  rt->outfile.SetIncomplete(1);  }
		rt->infile.SetDelSpec(P()->rin_delspec);
		rt->outfile.SetDelSpec(P()->rout_delspec);
	}
	if(ctsk->ft)
	{
		FilterTask *ft=(FilterTask*)ctsk->ft;
		if(ft->wdir.set("."))  return(-1);
		RefString infile,outfile;
		if(infile.sprintf(0,"f-%08x-%07d.b",ctsk->task_id,ctsk->frame_no))  return(-1);
		if(outfile.sprintf(0,"f-%08x-%07d.c",ctsk->task_id,ctsk->frame_no))  return(-1);
		ft->infile=TaskFile::GetTaskFile(infile,
			TaskFile::FTImage,TaskFile::IOTFilterInput,TaskFile::FCLDR);
		ft->outfile=TaskFile::GetTaskFile(outfile,
			TaskFile::FTImage,TaskFile::IOTFilterOutput,TaskFile::FCLDR);
		if(!ft->infile || !ft->outfile)  return(-1);
		ft->infile.SetDelSpec(P()->fin_delspec);
		ft->outfile.SetDelSpec(P()->fout_delspec);
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
		case FRFT_RenderIn:
			//if( (tri.ctsk->rt && P()->transfer.render_dest && !tri.ctsk->ft) || 
			//    (tri.ctsk->ft && P()->transfer.render_dest && !tri.ctsk->rt) )
			//{  tri.req_file_type=FRFT_RenderOut;  tri.req_file_idx=0;  break;  }
			if((tri.ctsk->ft && P()->transfer.render_dest && !tri.ctsk->rt) || 
			   (tri.ctsk->rt && tri.ctsk->rt->resume) )
			{  tri.req_file_type=FRFT_RenderOut;  tri.req_file_idx=0;  break;  }
		case FRFT_RenderOut:
			//if(tri.ctsk->ft && P()->transfer.filter_dest)
			//{  tri.req_file_type=FRFT_FilterOut;  tri.req_file_idx=0;  break;  }
		case FRFT_FilterOut:
			if(tri.ctsk->rt && P()->transfer.additional && tri.ctsk->radd.nfiles)
			{  tri.req_file_type=FRFT_AddRender;  tri.req_file_idx=0;  break;  }
		case FRFT_AddRender:
			++tri.req_file_idx;
			if(tri.ctsk->rt && P()->transfer.additional && 
			   int(tri.req_file_idx)<tri.ctsk->radd.nfiles)  break;
			if(tri.ctsk->ft && P()->transfer.additional && tri.ctsk->fadd.nfiles)
			{  tri.req_file_type=FRFT_AddFilter;  tri.req_file_idx=0;  break;  }
		case FRFT_AddFilter:
			++tri.req_file_idx;
			if(tri.ctsk->ft && P()->transfer.additional && 
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
	
	if(tri.req_file_type==FRFT_AddRender || tri.req_file_type==FRFT_AddFilter)
	{
		// Only request files which we need (i.e.: if they already 
		// exist, have proper time stamp and same size, then do not 
		// transfer them). 
		// So, see if the file exists: 
		do {
			HTime local_mtime;
			errno=0;
			int64_t local_size=tfile.FileLength(&local_mtime,/*aware_of_fixed_state=*/1);
			if(local_size<0 && errno==ENOENT) break;  // we need the file
			HTime remote_mtime;
			int64_t remote_size=tfile.GetFixedState(&remote_mtime);
			assert(remote_size>=0);  // Must have called SetFixedState(). 
			if(local_size!=remote_size) break;  // need the file
			local_mtime.Add(P()->timestamp_thresh,HTime::msec);
			if(remote_mtime>=local_mtime) break;
			// Seems we do not need the file. 
			Verbose(TSLLR,"LDR: NOT requesting %s file %s [frame %d]\n",
				FileRequestFileTypeString(tri.req_file_type),
				tri.req_tfile.BaseNamePtr(),
				tri.ctsk->frame_no);
			int rv=tfile.ClearFixedState();
			assert(!rv);  // must have been in fixed state
			goto retry;
		} while(0);
	}
	
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
				if(tdi.done_ctsk->rt && P()->transfer.render_dest)
				{
					tdi.upload_file_type=FRFT_RenderOut;
					tdi.upload_file=tdi.done_ctsk->rt->outfile;
					tts=&tdi.done_ctsk->rtes;
					break;
				}
			case FRFT_RenderOut:
				if(tdi.done_ctsk->ft && P()->transfer.filter_dest)
				{
					tdi.upload_file_type=FRFT_FilterOut;
					tdi.upload_file=tdi.done_ctsk->ft->outfile;
					tts=&tdi.done_ctsk->ftes;
					break;
				}
			// final fallthrough:
				tdi.upload_file_type=FRFT_None;
				tdi.upload_file=TaskFile();
		}
		
		// See if we actually want/can upload tdi.upload_file_type: 
		if(tdi.upload_file_type==FRFT_None)  break;
		
		// If the job was not launched skip to next file. 
		if(tts->tes.status==TTR_Unset || tts->tes.status==TTR_ExecFailed)
			continue;
		
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
			if(tts->tes.status==TTR_Success || tts->tes.status==TTR_JobTerm)
			{  Warning(_err_fmt,
				tdi.upload_file.HDPath().str(),err_str,tdi.done_ctsk->frame_no);  }
			else
			{  Verbose(TSLLR,_err_fmt,
				tdi.upload_file.HDPath().str(),err_str,tdi.done_ctsk->frame_no);  }
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
	LDRFileDownload *pack=(LDRFileDownload*)(recv_buf.data);
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
	fprintf(stderr,"Starting to download file (%s; %lld bytes)\n",
		tri.req_tfile.HDPath().str(),fsize);
	
	const char *path=tri.req_tfile.HDPath().str();
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
		{  _ConnClose(rv<0 ? 0 : 1);  return(-1);  }
		
		rv=_HandleReceivedHeader(&hdr);
		if(rv<0)
		{  _ConnClose(0);  }
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
	assert(pollid==fdi->pollid);
	assert(sock_fd==fdi->fd);
	
	int handeled=0;
	
	// See what we must do...
	if(fdi->revents & POLLIN)
	{
		if(expect_cmd==Cmd_ChallengeResponse)
		{
			int rv=_RecvChallengeResponse();
			if(rv)
			{  _ConnClose(rv<0 ? 0 : 1);  return(0);  }
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
			if(_SendNowConnected())  return(0);
			handeled=1;
		}
		/*else if(authenticated)
		{
			handeled=_AuthSConnFDNotify(fdi);
			if(handeled<0)  return(0);
		}*/
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
	
	switch(out_active_cmd)
	{
		case Cmd_TaskResponse:
		{
			// Okay, finally sent task response. 
			fprintf(stderr,"TaskResponse sent.\n");
			
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
			fprintf(stderr,"FileRequest sent.\n");
			
			assert(tri.resp_code==TRC_Accepted && tri.next_action==TRINA_FileReq);
			// Now, receive files. 
			tri.next_action=TRINA_FileRecvH;
			
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
		} break;
		case Cmd_TaskDone:
		{
			// Okay, sent TaskDone. We can now go on uploading the files. 
			fprintf(stderr,"TaskDone sent.\n");
			
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
				// Okay, if we used the FD->FD pump, we must close the 
				// input file again: 
				if(cpi->pump->Src()->Type()==FDCopyIO::CPT_FD)
				{
					assert(cpi->pump==out.pump_fd && cpi->pump->Src()==out.io_fd);
					if(CloseFD(out.io_fd->pollid)<0)
					{  hack_assert(0);  }  // Actually, this may not fail, right?
				}
				
				fprintf(stderr,"File upload completed.\n");
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
	if(!outpump_lock && (tri.resp_code>0 ||  // Not -1 "unset" and not TRC_Accepted. 
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
			// Currently. only alloc failure can happen here. 
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
			int rv=_FDCopyStartSendFile(tdi.upload_file.HDPath().str(),
				tdi.upload_file_size);
			static int warned=0;
			if(!warned)
			{  fprintf(stderr,"hack code to handle failure. (5)\n");  ++warned;  }
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
						tdi.upload_file.HDPath().str(),
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
			
			int o_start=0;
			
			if(tri.next_action==TRINA_FileRecvB)
			{
				// Read in body (complete). 
				
				// If we used the FD->FD pump, we must close the output file. 
				if(cpi->pump->Dest()->Type()==FDCopyIO::CPT_FD)
				{
					assert(cpi->pump==in.pump_fd && cpi->pump->Dest()==in.io_fd);
					if(CloseFD(in.io_fd->pollid)<0)
					{
						int errn=errno;
						Error("LDR: While closing \"%s\": %s\n",
							tri.req_tfile.HDPath().str(),strerror(errn));
						_ConnClose(0);  return(1);
					}
				}
				
				// File downloading done. 
				if(tri.req_tfile.GetFType()==TaskFile::FTAdd)
				{
					int rv=tri.req_tfile.ClearFixedState();
					assert(!rv);  // must have been in fixed state
				}
				tri.req_tfile=TaskFile();
				tri.next_action=TRINA_FileReq;
				o_start=1;
			}
			else
			{
				assert(cpi->pump==in.pump_s);  // can be left away
				
				int rv=_ParseFileDownload(&recv_buf);
				if(rv<0)
				{  _ConnClose(0);  return(1);  }
				assert(rv==0);
				
				// o_start=0 because we're waiting for file body. 
			}
			
			recv_buf.content=Cmd_NoCommand;
			if(o_start)
			{
				if(cpnotify_outpump_start()==1)
				{  return(1);  }
			}
		} break;
		default:
			// This is an internal error. Only known packets may be accepted 
			// in _HandleReceivedHeader(). 
			Error("cpnotify_inpump: Done; hack on... (Implementation incomplete)\n");
			assert(0);
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
	
	if((cpi->scode & FDCopyPump::SCKilled))
	{
		// Killed? Dann wird's das schon gebraucht haben! ;)
		return(0);
	}
	
	if( ! (cpi->scode & FDCopyPump::SCLimit) )
	{
		// SCError and SCEOF; handling probably in cpnotify_inpump() 
		// and cpnotify_outpump_done(). 
		Error("%s pump: %s\n",
			(cpi->pump==in.pump_s || cpi->pump==in.pump_fd) ? "Input" : "Output",
			CPNotifyStatusString(cpi));
		hack_assert(0);  // ##
	}
	
	// This is only used for auth: 
	assert(next_send_cmd==Cmd_NoCommand);
	
	do {
		if(cpi->pump==in.pump_s || cpi->pump==in.pump_fd)
		{
			if(cpnotify_inpump(cpi))  break;  // _ConnClose() was called. 
			
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


int TaskSource_LDR_ServerConn::timernotify(FDBase::TimerInfo *ti)
{
	assert(ti->tid==schedule_tid);
	assert(!DeletePending());
	
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
	
	return(0);
}


// Called to close down the connection or if it was closed down. 
// reason: 0 -> general
//         1 -> received Cmd_QuitNow
//         2 -> auth failure
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
		// Make sure we close down. 
		_ShutdownConnection();
	}
	
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
	
	return(0);
}


TaskSource_LDR_ServerConn::TaskSource_LDR_ServerConn(TaskSource_LDR *_back,
	int *failflag) : 
	LinkedListBase<TaskSource_LDR_ServerConn>(),
	NetworkIOBase_LDR(failflag),
	tri(failflag),
	tdi(failflag),
	addr(failflag)
{
	int failed=0;
	
	back=_back;
	
	authenticated=0;
	outpump_scheduled=0;
	
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	outpump_lock=IOPL_Unlocked;
	
	tri.ctsk=NULL;
	tri.task_id=0;
	tri.resp_code=-1;
	
	tdi.done_ctsk=NULL;
	tdi.next_action=TDINA_None;
	tdi.upload_file_type=FRFT_None;
	
	memset(expect_chresp,0,LDRChallengeLength);
	
	schedule_tid=InstallTimer(-1,0);
	if(!schedule_tid)  ++failed;
	
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSource_LDR_ServerConn");  }
}

TaskSource_LDR_ServerConn::~TaskSource_LDR_ServerConn()
{
	memset(expect_chresp,0,LDRChallengeLength);
	
	if(sock_fd>=0)
	{
		Warning("LDR: Still connected to %s; disconnecting. [OOPS!]\n",
			addr.GetAddress().str());
		ShutdownFD(sock_fd);  pollid=NULL;
	}
	
	if(tri.ctsk)
	{
		Warning("LDR: OOPS... Dangling task [frame %d] left.\n",
			tri.ctsk->frame_no);
		DELETE(tri.ctsk);
	}
	
	back=NULL;
}
