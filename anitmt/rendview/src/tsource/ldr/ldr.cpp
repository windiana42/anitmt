/*
 * ldr.cpp
 * 
 * Task source implementing Local Distributed Rendering. 
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

#include "ldr.hpp"
#include "param.hpp"

#include "../taskfile.hpp"
#include "../../taskmanager.hpp"

#include <assert.h>

using namespace LDR;



static void _AllocFailure(int fail=-1)
{
	if(fail)
	{  Error("LDR: Alloc failure.\n");  }
}


static inline long _timeout_ntoh(u_int32_t x)
{
	return((x==0xffffffffU) ? (-1) : long(ntohl(x)));
}


void TaskSource_LDR::ConnClose(TaskSource_LDR_ServerConn *sc,int reason)
{
	// Okay, if that was not our server, then everything is okay: 
	if(!sc->authenticated)
	{
		Verbose(TSLLR,"LDR: Closed connection to %s.\n",
			sc->addr.GetAddress().str());
		// Hehe... simply delete it: 
		delete sconn.dequeue(sc);
		return;
	}
	assert(reason!=2);  // Auth failure may only happen if !sc->authenticated. 
	
	if(reason==1)
	{
		Verbose(TSLLR,"LDR: Got quit request from server %s.\n",
			sc->addr.GetAddress().str());
		#warning This is okay as long as we do not have tasks...?
		delete sconn.dequeue(sc);
		return;
	}
	
	#warning Is that <unexpected> if reason==1
	Error("LDR: Unexpected connection close with auth server %s.\n",
		sc->addr.GetAddress().str());
	
	assert(0);  // handle me! kill all tasks,...
	//--> tell task manager. (missing)
}


int TaskSource_LDR::fdnotify(FDInfo *fdi)
{
	assert(fdi->dptr==NULL);  // Can be removed. 
	
	if(fdi->revents & POLLIN)
	{
		MyAddrInfo addr;
		int as=addr.accept(p->listen_fd);
		if(as<0)
		{
			// Oh dear, something failed. 
			Warning("LDR: Failed to accept connection: %s\n",
				strerror(errno));
			return(0);
		}
		
		// Okay, we may receive a connection. 
		TaskSource_LDR_ServerConn *sc=NEW1<TaskSource_LDR_ServerConn>(this);
		if(sc && sc->Setup(as,&addr))
		{  delete sc;  sc=NULL;  }
		if(!sc)
		{
			Error("LDR: Accept failed (alloc failure).\n");
			close(as);
			return(0);
		}
		
		// Okay, accepted a connection: 
		Verbose(TSLLR,"LDR: Accepted connection from %s.\n",sc->addr.GetAddress().str());
		sconn.append(sc);
		return(0);
	}
	if(fdi->revents & (POLLERR | POLLHUP))
	{
		Error("LDR: Unknown revents for listening socket %d: %d. Aborting.\n",
			fdi->fd,fdi->revents);
		abort();  return(0);
	}
	
	return(0);
}


int TaskSource_LDR::timernotify(TimerInfo *ti)
{
	assert(ti->tid==rtid);
	
	_StopSchedTimer();
	
	TSNotifyInfo ni;
	ni.action=pending;
	
	switch(pending)
	{
		case ANone:
			// should never happen
			fprintf(stderr,"OOPS: ldr:%d:pending=ANone\n",__LINE__);
			abort();
			break;
		case AConnect:
			connected=1;
			// Okay, connected, right?
			ni.connstat=CSConnected;
			break;
		case AGetTask:
			break;
		case ADoneTask:
			break;
		case ADisconnect:
			connected=0;
			// Of course, we disconnected...
			ni.disconnstat=DSOkay;
			break;
	}
	
	TaskSourceConsumer *client=cclient;
	// Reset cclient and action before calling the virtual 
	// function so that this function can call 
	// GetTask / DoneTask. 
	cclient=NULL;
	pending=ANone;
	
	call_tsnotify(client,&ni);
	
	return(0);
}


// overriding virtuals from TaskSource: 
int TaskSource_LDR::srcConnect(TaskSourceConsumer *cons)
{
	if(connected)  return(2);
	
	// Okay, then let's connect...
	pending=AConnect;
	cclient=cons;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcGetTask(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	pending=AGetTask;
	cclient=cons;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	if(!connected)  return(2);
	
	pending=ADoneTask;
	cclient=cons;
#warning done task...
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcDisconnect(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	// Okay, then let's disconnect...
	pending=ADisconnect;
	cclient=cons;
	_StartSchedTimer();
	return(0);
}


long TaskSource_LDR::ConnectRetryMakesSense()
{
	// No, it makes absolutely no sense to re-try to connect to 
	// this task source. If it failed, it failed definitely. 
	return(0);
}


TaskSource_LDR::TaskSource_LDR(TaskSourceFactory_LDR *tsf,int *failflag) : 
	TaskSource(tsf->component_db(),failflag),
	FDBase(failflag),
	sconn(failflag)
{
	// Important: set TaskSourceType: 
	tstype=TST_Active;
	
	p=tsf;
	pending=ANone;
	connected=0;
	
	int failed=0;
	
	rtid=InstallTimer(-1,0);
	if(!rtid)
	{  ++failed;  }
	
	// Create a poll node for the listening fd and poll for input: 
	assert(p->listen_fd>=0);  // otherwise we may not be here. 
	l_pid=NULL;
	if(PollFD(p->listen_fd,POLLIN,NULL,&l_pid))
	{  ++failed;  }
	else assert(l_pid);  // l_pid may only be/stay NULL if PollFD returns failure
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSource_LDR");  }
	
	// Make sure the LDR header has correct length. 
	// Otherwise the struct LDRHeader should probably have been packed: 
	assert(sizeof(LDR::LDRHeader)==6);
}


TaskSource_LDR::~TaskSource_LDR()
{
	assert(pending==ANone);
	assert(!connected);
	
	// Make sure we disconnect from all servers...
	while(!sconn.is_empty())
	{
		TaskSource_LDR_ServerConn *sc=sconn.popfirst();
		delete sc;
	}
	
	// The listen FD is closed & shut down by the factory. 
	// We just have to explicitly unpoll it: 
	UnpollFD(l_pid);
}


/******************************************************************************/
/*********** TaskSource_LDR_ServerConn                             ***********/

inline TaskSourceFactory_LDR *TaskSource_LDR_ServerConn::P()
{  return(back->P());  }
inline ComponentDataBase *TaskSource_LDR_ServerConn::component_db()
{  return(P()->component_db());  }


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
	
	ssize_t wr=write(sock_fd,(char*)d,len);
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
	ssize_t rd=read(sock_fd,(char*)d,len);
	if(rd<0)
	{
		Error("LDR: %s: while reading: %s\n",
			addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	if(!rd)
	{
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
	FDChangeEvents(pollid,POLLIN,POLLOUT);
	
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
	else if(back->GetAuthenticatedServer())
	{
		Warning("LDR: Denying conn to %s (already connected to %s).\n",
			addr.GetAddress().str(),
			back->GetAuthenticatedServer()->addr.GetAddress().str());
		authcode=CAC_AlreadyConnected;
	}
	else
	{
		// So, we must set the authenticated flag here. 
		authenticated=1;
		// And we must make sure that sc becomes the 
		// first elem in the list: 
		back->ServerHasNowAuthenticated(this);
		
		// Okay, correct challenge and no other authenticated server. 
		VerboseSpecial("LDR: Now connected to %s (%.*s).",
			addr.GetAddress().str(),
			LDRIDStringLength,d.id_string);
		authcode=CAC_Success;
	}
	memset(expect_chresp,0,LDRChallengeLength);
	now_conn_auth_code=authcode;
	
	expect_cmd=Cmd_NoCommand;
	next_send_cmd=Cmd_NowConnected;
	FDChangeEvents(pollid,POLLOUT,POLLIN);
	
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
		HTime2LDRTime(taskman->Get_starttime(),&d->starttime);
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
	FDChangeEvents(pollid,POLLIN,POLLOUT);
	now_conn_auth_code=0;
	// out.ioaction, in.ioaction currently IOA_Locked: 
	out.ioaction=IOA_None;
	in.ioaction=IOA_None;
	
	return(0);
}


// Header in HOST order. 
// Return value: see _AuthSConnFDNotify(). 
int TaskSource_LDR_ServerConn::_HandleReceivedHeader(LDRHeader *hdr)
{
	// BE CAREFUL!! hdr ALLOCATED ON THE STACK. 
	
	// See what we receive: 
	LDRCommand recv_cmd=LDRCommand(hdr->command);
	
	fprintf(stderr,"Received header: >%s< (length=%u)\n",
		LDRCommandString(recv_cmd),hdr->length);
	
	if(recv_cmd==Cmd_TaskRequest)
	{
		// Okay, then let's get the body. 
		assert(recv_buf.content==Cmd_NoCommand);  // Can that happen?
		int rv=_StartReadingCommandBody(&recv_buf,hdr);
		if(rv)
		{
			if(rv==-2)
			{  Error("LDR: Too long %s packet (header reports %u bytes)\n",
				LDRCommandString(recv_cmd),hdr->length);  }
			else _AllocFailure();
			return(-1);
		}
		
		return(1);
	}
	
	Error("LDR: Received unexpected command header %s "
		"(cmd=%u, length=%u).\n",
		LDRCommandString(recv_cmd),recv_cmd,hdr->length);
	
	return(-1);  // -->  _ConnClose()
}


// Return value: 
//   0 -> OK
//   1 -> error; disconnect  (message already reported to user)
//   2 -> error; refuse task (message already reported to user)
//  -1 -> alloc failure  (!NOT REPORTED!)
//  -2 -> illegal size entries in header  (message written)
int TaskSource_LDR_ServerConn::_ParseTaskRequest(RespBuf *buf)
{
	assert(buf->content==Cmd_TaskRequest);  // otherwise strange internal error
	LDRDoTask *pack=(LDRDoTask*)(buf->data);
	assert(pack->command==Cmd_TaskRequest);  // can be left away
	
	size_t len=pack->length;
	assert(len>=sizeof(LDRHeader));  // can be left away
	
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
	int64_t lenaccu=0;   // I'm paranoid here with overflows...
	
	char *rdesc_str=ptr;      ptr+=r_desc_slen;      lenaccu+=r_desc_slen;
	char *fdesc_str=ptr;      ptr+=f_desc_slen;      lenaccu+=f_desc_slen;
	char *r_oformat_str=ptr;  ptr+=r_oformat_slen;   lenaccu+=r_oformat_slen;
	char *r_add_args=ptr;     ptr+=r_add_args_size;  lenaccu+=r_add_args_size;
	char *f_add_args=ptr;     ptr+=f_add_args_size;  lenaccu+=f_add_args_size;
	
	if( int64_t(len) != lenaccu+int64_t(sizeof(LDRDoTask)) )
	{
		fprintf(stderr,"LDR: Illegal size entries in received task request "
			"(header: %u; data: %u)\n",len,size_t(lenaccu)+sizeof(LDRDoTask));
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
	#warning missing checks with r_n_files, f_n_files
	if(!rdesc_name && 
	   (r_oformat_slen || r_add_args_size))
	{
		Error("%s%d] containing unecessary render task info.\n",_rtr,frame_no);
		return(1);
	}
	if(!fdesc_name && 
	   (f_add_args_size))
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
			// FIXME: Must store apropriate refusal code.  
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
			// FIXME: Must store apropriate refusal code.  
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
			// FIXME: Must store apropriate refusal code.  
			return(2);
		}
	}
	
	// Okay, set up a CompleteTask structure: 
	CompleteTask *ctsk=NEW<CompleteTask>();
	if(!ctsk)
	{  return(-1);  }
	
	ctsk->frame_no=frame_no;
	#warning task id...
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
	
	// Okay, and now... let's copy the additional args. 
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
	}
	
	// MISSING: 
	// infile,outfile,wdir (rt and ft)
	
	#if 1
	// Okay, dump to the user: 
	fprintf(stderr,"Received task request:\n");
	fprintf(stderr,"  frame_no=%u; id=%u\n",
		ctsk->frame_no,ntohl(pack->task_id));
	if(ctsk->rt)
	{
		fprintf(stderr,"  Render: %s (%ux%u; timeout=%d; oformat=%s)\n",
			ctsk->rt->rdesc->name.str(),
			ctsk->rt->width,ctsk->rt->height,ctsk->rt->timeout,
			  ctsk->rt->oformat->name);
		fprintf(stderr,"    Args:");
		for(const RefStrList::Node *i=ctsk->rt->add_args.first(); i; i=i->next)
		{  fprintf(stderr," %s",i->str());  }
		fprintf(stderr,"\n");
	}
	else
	{  fprintf(stderr,"  Render: [none]\n");  }
	if(ctsk->ft)
	{
		fprintf(stderr,"  Filter: %s (timeout=%d)\n",
			ctsk->ft->fdesc->name.str(),
			ctsk->ft->timeout);
		fprintf(stderr,"    Args:");
		for(const RefStrList::Node *i=ctsk->ft->add_args.first(); i; i=i->next)
		{  fprintf(stderr," %s",i->str());  }
		fprintf(stderr,"\n");
	}
	else
	{  fprintf(stderr,"  Filter: [none]\n");  }
	#endif
	
	#if 0
	assert(pack->length<=buf->alloc_len);
	fprintf(stderr,"DUMP(length=%u)>>",pack->length);
	for(char *c=(char*)pack,*cend=c+pack->length; c<cend; c++)
	{
		if(*(unsigned char*)c>=32 && *(unsigned char*)c!=127)
		{  write(2,c,1);  }
		else
		{  write(2,".",1);  }
	}
	fprintf(stderr,"<<\n");
	#endif
	
	// AND DO THE RANGE CHECK ABOVE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Error("Hack on...\n");
	assert(0);
	
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
		{  _ConnClose(rv<0 ? 0 : 1);  return(1);  }
		
		return(_HandleReceivedHeader(&hdr));
	}
	
	Error("*** hack on\n");
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
			Error("LDR: %s: unexpected revents=%d (state %d,%d). Quitting.\n",
				addr.GetAddress().str(),fdi->revents,
				expect_cmd,next_send_cmd);
		}
		_ConnClose(0);
	}
	
	return(0);
}


int TaskSource_LDR_ServerConn::cpnotify(FDCopyBase::CopyInfo *cpi)
{
	fprintf(stderr,"cpnotify(scode=0x%x (final=%s; limit=%s), err_no=%d (%s))\n",
		cpi->scode,
		(cpi->scode & FDCopyPump::SCFinal) ? "yes" : "no",
		(cpi->scode & FDCopyPump::SCLimit) ? "yes" : "no",
		cpi->err_no,strerror(cpi->err_no));
	
	// We are only interested in FINAL codes. 
	if(!(cpi->scode & FDCopyPump::SCFinal))
	{  return(0);  }
	
	if(cpi->pump==in.pump_s && in_active_cmd==Cmd_TaskRequest)
	{
		in_active_cmd==Cmd_NoCommand;
		in.ioaction=IOA_None;
		
		if((cpi->scode & FDCopyPump::SCLimit))
		{
			// NOTE: This looks like something strange but it is an 
			// internal error if it fails. Because we tell the pump to 
			// copy exactly LDRHeader->length - sizeof(LDRHeader) bytes 
			// and if it reports SCLimit here, then they have to be 
			// there. 
			#if TESTING
			if(cpi->pump->Dest()->bufdone+sizeof(LDRHeader)!=
				((LDRHeader*)recv_buf)->length)
			{  assert(0);  }
			#endif
			
			int rv=_ParseTaskRequest(&recv_buf);
			if(rv)
			{
				if(rv==-1)  _AllocFailure();
				else assert(rv>0 || rv==-2);  // For those <0, WE MUST WRITE ERROR HERE. 
				
				_ConnClose(0);
				return(0);
			}
		}
		else
		{
			// #error #warning ### must hack message for different errors (timeout, pollerr, etc)
			// MUST ALSO CHECK FOR EOF!!
			Error("LDR: Receiving failed during command %s: %d,%s [<- HACK ME!]\n",
				LDRCommandString(recv_buf.content),
				cpi->err_no,strerror(cpi->err_no));
			// Act correctly (disconnect). 
			assert(0);
		}
		return(0);
	}
	
	Error("DONE -> hack on\n");
	assert(0);
	
	return(0);
}


// Called to close down the connection or if it was closed down. 
// reason: 0 -> general
//         1 -> received Cmd_QuitNow
//         2 -> auth failure
void TaskSource_LDR_ServerConn::_ConnClose(int reason)
{
	if(sock_fd>=0)  // Otherwise: Not connected or already disconnected. 
	{
		// First, make sure we close down. 
		PollFDDPtr(pollid,NULL);
		ShutdownFD(pollid);  // sets pollid=NULL
		sock_fd=-1;
		
		back->ConnClose(this,reason);
	}
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
	addr(failflag)
{
	back=_back;
	
	authenticated=0;
	
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	memset(expect_chresp,0,LDRChallengeLength);
}

TaskSource_LDR_ServerConn::~TaskSource_LDR_ServerConn()
{
	if(sock_fd>=0)
	{
		fprintf(stderr,"OOPS: still connected to %s\n",
			addr.GetAddress().str());
		ShutdownFD(sock_fd);  pollid=NULL;
	}
	
	memset(expect_chresp,0,LDRChallengeLength);
	back=NULL;
}
