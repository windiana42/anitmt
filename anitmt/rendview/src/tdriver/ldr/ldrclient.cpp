/*
 * ldrclient.cpp
 * 
 * LDR task driver stuff: LDR client representation on server side. 
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


#include "../../taskmanager.hpp"
#include "dif_ldr.hpp"
#include "dif_param.hpp"
#include "ldrclient.hpp"

#include <ctype.h>
#include <assert.h>


using namespace LDR;


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif

#warning IMPORTANT: Remove potentially dangerous asserts in destructors (like assert(fd<0))

#warning make it configurable: 
static int max_jobs_per_client=24;


static void _AllocFailure(int fail=-1)
{
	if(fail)
	{  Error("LDR: Alloc failure.\n");  }
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
	
	if(!send_quit_cmd)
	{
		// We need not fiddle around with next_send_cmd and expect_cmd 
		// as send_quit_cmd has precedence and the other vars might be 
		// changed again before fdnotify() by some other notify. 
		send_quit_cmd=1;
		_DoPollFD(POLLOUT,POLLIN);
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
	
	ssize_t wr=write(sock_fd,(char*)d,len);
	if(wr<0)
	{
		int errn=errno;
		Error("Client %s: Failed to send %u bytes: %s\n",
			_ClientName().str(),len,strerror(errn));
		return(-1);
	}
	if(size_t(wr)<len)
	{
		Error("Client %s: Short write (sent %u/%u bytes).\n",
			_ClientName().str(),size_t(wr),len);
		return(-1);
	}
	return(0);
}

// Return value: len or -1 -> error
ssize_t LDRClient::_AtomicRecvData(LDRHeader *dest,size_t len,size_t min_len)
{
	ssize_t rd=read(sock_fd,(char*)dest,len);
	if(rd<0)
	{
		int errn=errno;
		Error("Client %s: While reading: %s\n",
			_ClientName().str(),strerror(errn));
		return(-1);
	}
	else if(!rd)
	{
		Error("Client %s: Disconnected unexpectedly.\n",
			_ClientName().str());
		return(-1);
	}
	else if(size_t(rd)<min_len)
	{
		Error("Client %s: Short (packet \"%s\": %d/%u bytes)\n",
			_ClientName().str(),LDRCommandString(expect_cmd),rd,min_len);
		return(-1);
	}
	// Translate to host order: 
	dest->length=ntohl(dest->length);
	dest->command=ntohs(dest->command);
	return(len);
}


// Finish up connect(2). 
// Return value: 
//  0 -> OK, now connected. 
//  1 -> failure; remove client
int LDRClient::_DoFinishConnect(FDBase::FDInfo *fdi)
{
	assert(connected_state==1);
	
	// Okay, see if we have POLLIN. (DO THAT FIRST; YES!!)
	if(fdi->revents & POLLIN)
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
	else
	{
		Error("Client %s: No POLLIN after connect. Removing client.\n",
			_ClientName().str());
		return(1);
	}
	
	// Seems that we may connect without problems. 
	// See if other flags are set: 
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
	LDRComputeCallengeResponse(d,(char*)r->response,cp->password.str());
	
	return(0);
}


// NOTE: LDRHeader in HOST order. 
// Returns packet length or 0 -> error. 
size_t LDRClient::_CheckRespHeader(LDRHeader *d,size_t read_len,
	size_t min_len,size_t max_len)
{
	if(read_len<sizeof(LDRHeader))
	{
		Error("Client %s: Packet too short (header incomplete: %u bytes)\n",
			_ClientName().str(),read_len);
		return(0);  // YES!!
	}
	LDRCommand last_recv_cmd=LDRCommand(d->command);
	if(last_recv_cmd!=expect_cmd)
	{
		Error("Client %s: Conversation error (expected: %s; received: %s)\n",
			_ClientName().str(),
			LDRCommandString(expect_cmd),
			LDRCommandString(last_recv_cmd));
		return(0);  // YES!!
	}
	size_t len=d->length;
	if(len<min_len || len>max_len)
	{
		Error("Client %s: Packet too %s (header: %u bytes; %s: %u bytes)\n",
			_ClientName().str(),
			len<min_len ? "short" : "long",
			len,
			len<min_len ? "min" : "max",
			len<min_len ? min_len : max_len);
		return(0);  // YES!!
	}
	return(len);
}


// Return value: 0 -> OK
// 1 -> failure; close doen conn. 
int LDRClient::_DoAuthHandshake(FDBase::FDInfo *fdi)
{
	assert(connected_state==2 && !auth_passed);
	
	int handeled=0;
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
			if(!_CheckRespHeader(&d,size_t(rd),sizeof(d),sizeof(d)))
			{  return(1);  }
			u_int16_t clientpv=ntohs(d.protocol_vers);
			if(clientpv!=LDRProtocolVersion)
			{
				Error("Client %s: LDR proto version mismatch: client: %d; server %d.\n",
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
		else if(expect_cmd==Cmd_NowConnected)
		{
			assert(next_send_cmd==Cmd_NoCommand);
			
			// Last auth packet. Either challenge response was accepted 
			// or not. Want to get that atomically, too. 
			LDRNowConnected d;
			size_t min_len=(((char*)&d.auth_code)-((char*)&d)+sizeof(d.auth_code));
			ssize_t rd=_AtomicRecvData(&d,sizeof(d),min_len);
			if(rd<0)
			{  return(1);  }
			
			// Check fields: 
			size_t len=_CheckRespHeader(&d,size_t(rd),min_len,sizeof(d));
			if(!len)
			{  return(1);  }
			
			// Get code: 
			int okay=0;
			switch(ntohs(d.auth_code))
			{
				case CAC_Success:  okay=1;  break;
				case CAC_AuthFailed:
					Warning("Client %s: Auth failed (illegal challenge).\n",
						_ClientName().str());
					break;
				case CAC_AlreadyConnected:
					Warning("Client %s: Already connected to some other server.\n",
						_ClientName().str());
					break;
				default:
					Error("Client %s: Illegal auth code %d.\n",
						_ClientName().str(),int(ntohs(d.auth_code)));
					break;
			}
			if(!okay)
			{  return(-1);  }
			
			c_jobs=ntohs(d.njobs);
			HTime up_since;
			LDRTime2HTime(&d.starttime,&up_since);
			
			if(c_jobs>max_jobs_per_client)
			{
				Warning("Client %s: reports njobs=%d; using %d.\n",
					_ClientName().str(),c_jobs,max_jobs_per_client);
				c_jobs=max_jobs_per_client;
			}
			
			// Okay, we are now connected. 
			{
				char ltmp[12];
				if(d.loadval==0xffffU)
				{  strcpy(ltmp,"[unknown]");  }
				else
				{
					int lv=ntohs(d.loadval);
					snprintf(ltmp,12,"%d.%02d",lv/100,lv%100);
				}
				Verbose(TDR,"Client %s: Now connected: njobs=%d (parallel jobs)\n"
					"  Up since: %s  (local)\n"
					"  Current load avg: %s\n",
					_ClientName().str(),c_jobs,
					up_since.PrintTime(1),
					ltmp);
			}
			
			auth_passed=1;
			// out.ioaction, in.ioaction currently IOA_Locked: 
			out.ioaction=IOA_None;
			in.ioaction=IOA_None;
			next_send_cmd=Cmd_NoCommand;
			expect_cmd=Cmd_NoCommand;
			
			_DoPollFD(POLLIN,POLLOUT);  // EOF and things
			handeled=1;
			
			// Tell interface to that we are now connected. 
			tdif->SuccessfullyConnected(this);
		}
	}
	if(fdi->revents & POLLOUT)
	{
		if(next_send_cmd==Cmd_ChallengeResponse)
		{
			assert(send_buf.data && send_buf.alloc_len>=sizeof(LDRChallengeResponse));
			assert(expect_cmd==Cmd_NoCommand);
			
			assert(send_buf.content==Cmd_ChallengeResponse);
			if(_AtomicSendData((LDRHeader *)(send_buf.data)))
			{  return(1);  }
			
			next_send_cmd=Cmd_NoCommand;
			expect_cmd=Cmd_NowConnected;
			send_buf.content=Cmd_NoCommand;
			
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
			Error("Client %s: %s\n",_ClientName().str(),strerror(errval));
			return(1);
		}
		else
		{
			Error("Client %s: Unexpected revents=%d (state %d,%d). "
				"Disconnecting.\n",
				_ClientName().str(),fdi->revents,
				expect_cmd,next_send_cmd);
			return(1);
		}
	}
	return(0);
}


void LDRClient::_DoSendQuit(FDBase::FDInfo *fdi)
{
	if(send_quit_cmd==1)
	{
		if(fdi->revents & POLLHUP)
		{
			// This is unexpected but okay, because we want to 
			// disconnect now. 
			// For poll emulation, we can use the POLLOUT below. 
			Verbose(TDR,"Client %s disconnected just in time.\n",
				_ClientName().str());
			send_quit_cmd=3;
		}
		else if(fdi->revents & POLLOUT)
		{
			// Must send quit cmd. 
			LDRQuitNow d;
			d.length=sizeof(LDRQuitNow);  // STILL IN HOST ORDER
			d.command=Cmd_QuitNow;    // STILL IN HOST ORDER
			
			if(_AtomicSendData(&d))
			{
				Error("  (Error above happened quring quit. No real problem.)\n");
				send_quit_cmd=3;
			}
			else
			{
				Verbose(TDR,"  Sent quit to client %s.\n",_ClientName().str());
				send_quit_cmd=2;
				_DoPollFD(POLLIN,POLLOUT);
			}
		}
		else
		{
			Verbose(TDR,"Client %s: strange revents %d during quitting.\n",
				_ClientName().str(),fdi->revents);
			send_quit_cmd=3;  // simply shut down NOW. 
		}
	}
	else if(send_quit_cmd==2)
	{
		// We expect that the client disconnects. 
		// Otherwise we do that. 
		int client_disconnected=0;
		if(fdi->revents & POLLHUP)
		{  client_disconnected=1;  }
		else if(fdi->revents & POLLIN)
		{
			char buf[64];
			ssize_t rd=read(sock_fd,buf,64);
			if(rd<0)
			{
				int errn=errno;
				Error("Client %s: During quit (read): %s\n",
					_ClientName().str(),strerror(errn));
			}
			else if(!rd)
			{  client_disconnected=1;  }
		}
		if(client_disconnected)
		{  Verbose(TDR,"Client %s disconnected due to our quit request.\n",
			_ClientName().str());  }
		else
		{
			// Hmmm... this may have a problem: There were just still 
			// some bytes on the network wire. We should wait. ##FIXME
			Warning("Client %s: did not disconnect after our request. "
				"Shutting down conn.\n",_ClientName().str());
		}
		
		// Disconnect done. 
		send_quit_cmd=3;
	}
	
	if(send_quit_cmd==3)
	{
		_KickMe(0);
		return;
	}
}


// This formats the whole LDRTaskRequest data into the RespBuf *dest. 
// (Note: Header in HOST oder)
// Return value: 
//   0 -> OK complete packet in resp_buf; ready to send it 
//  -1 -> alloc failure
int LDRClient::_Create_TaskRequest_Packet(CompleteTask *ctsk,RespBuf *dest)
{
	assert(dest->content==Cmd_NoCommand);
	
	RenderTask *rt=ctsk->rt;
	FilterTask *ft=ctsk->ft;
	
	// First, compute the size of the "packet": 
	size_t totsize=sizeof(LDRTaskRequest);
	
	// All the additional args have to be passed: 
	size_t r_add_args_size;
	size_t r_desc_slen,r_oformat_slen;
	if(rt)
	{
		r_add_args_size=SumUpSize(&rt->add_args);
		r_desc_slen=rt->rdesc->name.len();
		r_oformat_slen=strlen(rt->oformat->name);
	} else {
		r_add_args_size=0;
		r_desc_slen=0;
		r_oformat_slen=0;
	}
	totsize+=(r_add_args_size+r_desc_slen+r_oformat_slen);
	
	size_t f_add_args_size;
	size_t f_desc_slen;
	if(ft)
	{
		f_add_args_size=SumUpSize(&ft->add_args);
		f_desc_slen=ft->fdesc->name.len();
	} else {
		f_add_args_size=0;
		f_desc_slen=0;
	}
	totsize+=(f_add_args_size+f_desc_slen);
	
	// Count required files: 
	#warning need code to pass the required files...
	int r_n_files=0,f_n_files=0;
	if(rt)
	{
		r_n_files=ctsk->radd.nfiles;
		totsize+=LDRSumFileInfoSize(&ctsk->radd);
	}
	if(ft)
	{
		f_n_files=ctsk->fadd.nfiles;
		totsize+=LDRSumFileInfoSize(&ctsk->fadd);
	}
	
	// Okay, these limits are meant to be never exceeded. 
	// But we must be sure: 
	assert(r_desc_slen<=0xffff && r_oformat_slen<=0xffff);
	assert(f_desc_slen<=0xffff);
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
		pack->r_timeout = rt->timeout<0 ? 0xffffffffU : htonl(rt->timeout);
	}
	else
	{  pack->r_width=0;  pack->r_height=0;  pack->r_timeout=0;  }
	pack->r_desc_slen=htons(r_desc_slen);
	pack->r_add_args_size=htonl(r_add_args_size);
	pack->r_oformat_slen=htons(r_oformat_slen);
	
	if(ft)
	{  pack->f_timeout = ft->timeout<0 ? 0xffffffffU : htonl(ft->timeout);  }
	else
	{  pack->f_timeout=0;  }
	pack->f_desc_slen=htons(f_desc_slen);
	pack->f_add_args_size=htonl(f_add_args_size);
	
	pack->r_n_files=htons(r_n_files);
	pack->f_n_files=htons(f_n_files);
	
	char *dptr=(char*)(pack->data);
	if(rt)
	{  strcpy(dptr,rt->rdesc->name.str());  dptr+=r_desc_slen;  }
	if(ft)
	{  strcpy(dptr,ft->fdesc->name.str());  dptr+=f_desc_slen;  }
	
	if(rt)
	{
		strcpy(dptr,rt->oformat->name);      dptr+=r_oformat_slen;
		dptr=CopyStrList2Data(dptr,&rt->add_args);
	}
	if(ft)
	{
		dptr=CopyStrList2Data(dptr,&ft->add_args);
	}
	
	{
		char *end=dest->data+totsize;
		int err_file;
		LDRStoreFileInfoEntries(dptr,end,&ctsk->radd,&err_file);
		LDRStoreFileInfoEntries(dptr,end,&ctsk->fadd,&err_file);
		if(r_n_files || f_n_files)
		{
			fprintf(stderr,"Hack code to handle LDRStoreFileInfoEntries - errors\n");
			assert(0);
		}
	}
	
	// Now, we must be exactly at the end of the buffer: 
	assert(dptr==dest->data+totsize);
	
	#if 0
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
	
	dest->content=Cmd_TaskRequest;
	return(0);
}


int LDRClient::SendTaskToClient(CompleteTask *ctsk)
{
	if(!ctsk || (!ctsk->rt && !ctsk->ft))  return(-2);
	if(!auth_passed)  return(-1);
	if(assigned_jobs>=c_jobs)  return(2);
	
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


// Return value: 1 -> handeled; -1 -> must call _KickMe(). 
int LDRClient::_HandleReceivedHeader(LDRHeader *hdr)
{
	// BE CAREFUL!! hdr ALLOCATED ON THE STACK. 
	
	// See what we receive: 
	LDRCommand recv_cmd=LDRCommand(hdr->command);
	
	fprintf(stderr,"Client %s: Received header: >%s< (length=%u)\n",
		_ClientName().str(),LDRCommandString(recv_cmd),hdr->length);
	
	switch(recv_cmd)
	{
		case Cmd_FileRequest:
		case Cmd_TaskResponse:
		{
			// Okay, then let's get the body. 
			assert(recv_buf.content==Cmd_NoCommand);  // Can that happen?
			int rv=_StartReadingCommandBody(&recv_buf,hdr);
			if(rv)
			{
				if(rv==-2)
				{  Error("Client %s: Too long %s packet (header reports %u bytes)\n",
					_ClientName().str(),LDRCommandString(recv_cmd),hdr->length);  }
				else _AllocFailure();
				return(-1);
			}
		
			return(1);
		}
		default:;
	}
	
	Error("Client %s: Received unexpected command header %s (cmd=%u, length=%u).\n",
		_ClientName().str(),LDRCommandString(recv_cmd),recv_cmd,hdr->length);
	
	return(-1);  // -->  _KickMe()
}


// Return value: -1 -> call _KickMe(0)
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
	{  Error("Client %s: Illegal-sized file request (%u/%u bytes)\n",
		_ClientName().str(),freq->length,sizeof(LDRFileRequest));  return(-1);  }
	
	// Okay, parse file request: 
	// Of course, only INPUT files may be requested. 
	CompleteTask *ctsk=tri.scheduled_to_send;
	tri.req_file_type=ntohs(freq->file_type);
	tri.req_file_idx=ntohs(freq->file_idx);
	int legal=0;
	switch(tri.req_file_type)
	{
		case FRFT_None:  break;
		case FRFT_RenderIn:   legal=(tri.req_file_idx==0 && ctsk->rt);  break;
		case FRFT_RenderOut:
			legal=(tri.req_file_idx==0 && ctsk->ft && !ctsk->rt);  break;
		case FRFT_FilterOut:  /*legal=0*/  break;
		case FRFT_AddRender:
			legal=(ctsk->rt && int(tri.req_file_idx)<ctsk->radd.nfiles);  break;
		case FRFT_AddFilter: 
			legal=(ctsk->ft && int(tri.req_file_idx)<ctsk->fadd.nfiles);  break;
	}
	// Okay, get the file: 
	TaskFile *tfile=NULL;
	if(legal)
	{
		tfile=GetTaskFileByEntryDesc(/*dir=*/-1,tri.scheduled_to_send,
			tri.req_file_type,tri.req_file_idx);
		if(!tfile)  legal=0;
	}
	if(!legal)
	{
		#warning "could use more fancy error message than %d,%d"
		Error("Client %s: Received illegal file request (%d/%d) [frame %d]\n",
			_ClientName().str(),
			int(tri.req_file_type),int(tri.req_file_idx),
			ctsk->frame_no);
		return(-1);
	}
	
	tri.req_tfile=tfile;
	tri.req_file_size=tfile->FileLength();
	if(tri.req_file_size<0)
	{
		int errn=errno;
		Error("Client %s: Trying to stat requested file \"%s\": %s [frame %d]\n",
			_ClientName().str(),tfile->HDPath().str(),
			tri.req_file_size==-2 ? "no hd path" : strerror(errn),
			tri.scheduled_to_send->frame_no);
		return(-1);
	}
	
	
	tri.task_request_state=TRC_SendFileDownloadH;
	// cpnotify_outpump_start() will be called. 
	
	return(0);
}


int LDRClient::_ParseTaskResponse(RespBuf *buf)
{
	assert(buf->content==Cmd_TaskResponse);
	LDRTaskResponse *fresp=(LDRTaskResponse *)(buf->data);
	
	if(!tri.scheduled_to_send || 
	   tri.scheduled_to_send->task_id!=ntohl(fresp->task_id) || 
	   tri.task_request_state!=TRC_WaitForResponse )
	{  Error("Client %s: Unexpected task response. (Kicking it)\n",
		_ClientName().str());  return(-1);  }
	
	if(fresp->length!=sizeof(LDRTaskResponse))
	{  Error("Client %s: Illegal-sized task response (%u/%u bytes)\n",
		_ClientName().str(),fresp->length,sizeof(LDRTaskResponse));  return(-1);  }
	
	// Okay, parse task response: 
	int rv=ntohs(fresp->resp_code);
	Verbose(TDR,"Client %s: Received task reponse: %s [frame %d]\n",
		_ClientName().str(),LDRTaskResponseString(rv),
		tri.scheduled_to_send->frame_no);
	switch(rv)
	{
		case TRC_Accepted:  // Well, that's cool. 
			tri.scheduled_to_send=NULL;
			tri.task_request_state=TRC_None;
			tri.req_tfile=NULL;
			break;
		case TRC_UnknownRender:
		case TRC_UnknownFilter:
		case TRC_UnknownROFormat:
			// What shall we do with this one? 
			// "Solution" for now: Kick it. 
			// #### should be handled by tdif->TaskLaunchResult() called 
			//      below!! ####FIXME###
			fprintf(stderr,"****** KICKING CLIENT (FIXME!!! 1) *******\n");
			return(-1);
			break;
		case TRC_TooManyTasks:
			// What shall we do with this one? 
			// "Solution" for now: Kick it. 
			fprintf(stderr,"****** KICKING CLIENT (FIXME!!! 2) *******\n");
			return(-1);
			break;
		default:
			Error("Client %s: Illegal task response code %d.\n",
				_ClientName().str(),rv);
			return(-1);
	}
	
	// Tell task driver interface about it: 
	tdif->TaskLaunchResult(tri.scheduled_to_send,rv);
	
	// cpnotify_outpump_start() will NOT be called. 
	return(0);
}


// Return value: -1 -> _KickMe() called; 0 -> not handled; 1 -> handled
int LDRClient::_AuthConnFDNotify(FDBase::FDInfo *fdi)
{
	if(fdi->revents & POLLIN)
	{
		// Read protocol header: 
		// We want to get it atomically. 
		LDRHeader hdr;
		int rv=_AtomicRecvData(&hdr,sizeof(hdr),sizeof(hdr));
		if(rv<0)
		{  _KickMe(0);  return(-1);  }
		
		rv=_HandleReceivedHeader(&hdr);
		if(rv<0)
		{  _KickMe(0);  }
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
	assert(fdi->fd==sock_fd && fdi->pollid==pollid);
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
	
	if(send_quit_cmd)
	{
		_DoSendQuit(fdi);
		return(0);
	}
	
	// We're connected. 
	if(!auth_passed)
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
	
	// We have passed auth. We are the server the client listenes to. 
	// All the conversation is now made using the FD copy facility. 
	int handeled=0;
	if(fdi->revents & POLLIN)
	{
		handeled=_AuthConnFDNotify(fdi);
		if(handeled<0)  // _KickMe() called. 
		{  return(0);  }
	}
	if(fdi->revents & POLLOUT)
	{
		Error("fdnotify2(): WHY ARE WE HERE???\n");
		FDChangeEvents(fdi->pollid,0,POLLOUT);
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
			Error("Client %s: unexpected revents=%d. Kicking client.\n",
				_ClientName().str(),fdi->revents);
		}
		_KickMe(0);
	}
	
	return(0);
}


// Return value: 1 -> called _KickMe(); 0 -> normal
int LDRClient::cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi)
{
	// ---------<FIRST PART: HANDLE TERMINATION OF CURRENT REQUEST>---------
	
	switch(out_active_cmd)
	{
		case Cmd_TaskRequest:
		{
			assert(tri.task_request_state==TRC_SendTaskRequest && tri.scheduled_to_send);
			
			// NOTE: scheduled_to_send!=NULL, i.e. still the task we're talking 
			//       about. But task_request_state=TRC_WaitForResponse so that we do not 
			//       (accidentally) send it again. When client wants that we 
			//       send a file, then this has to be set accordingly. 
			tri.task_request_state=TRC_WaitForResponse;
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
			send_buf.content=Cmd_NoCommand;
			// Poll(events=0)? - not necessary (FDCopyPump does that). 
			
			fprintf(stderr,"Sending task struct done.\n");
		} break;
		case Cmd_FileDownload:
		{
			#if TESTING
			if((tri.task_request_state!=TRC_SendFileDownloadH && 
		    	tri.task_request_state!=TRC_SendFileDownloadB ) || 
			   !tri.scheduled_to_send)
			{  assert(0);  }
			#endif
			
			if(tri.task_request_state==TRC_SendFileDownloadH)
			{
				// Okay, LDRFileDownload header was sent. Now, we send 
				// the file itself. (next cpnotify_outpump_start()). 
				tri.task_request_state=TRC_SendFileDownloadB;
			}
			else  // TRC_SendFileDownloadB
			{
				// Okay, if we used the FD->FD pump, we must close the 
				// input file again: 
				if(cpi->pump->Src()->Type()==FDCopyIO::CPT_FD)
				{
					assert(cpi->pump==out.pump_fd && cpi->pump->Src()==out.io_fd);
					if(CloseFD(out.io_fd->pollid)<0)
					{  assert(0);  }  // Actually, this may not fail, right?
				}
				
				fprintf(stderr,"File download completed.\n");
				tri.task_request_state=TRC_WaitForResponse;
				tri.req_tfile=NULL;
			}
			// This is needed so that cpnotify_outpump_start() won't go mad. 
			send_buf.content=Cmd_NoCommand;
			out.ioaction=IOA_None;
			out_active_cmd=Cmd_NoCommand;
		} break;
		default:
		{
			Error("cpnotify_outpump_done: hack on...\n");
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
	
	// ---------<SECOND PART: LAUNCH NEW REQUEST IF NEEDED>---------
		
	// See what we can send...
	if(tri.scheduled_to_send)
	{
		// Okay; we shall send this task or one of the needed files. 
		if(tri.task_request_state==TRC_SendTaskRequest)
		{
			// Okay, we have to send the main task data. 
			int fail=1;
			do {
				if(_Create_TaskRequest_Packet(tri.scheduled_to_send,&send_buf))  break;
				// Okay, make ready to send it: 
				if(_FDCopyStartSendBuf((LDRHeader*)(send_buf.data)))  break;
				fail=0;
			} while(0);
			static int warned=0;
			if(!warned)
			{  fprintf(stderr,"hack code to handle failure. (1)\n");  ++warned;  }
			if(fail)
			{
				// Failure. 
				#warning HANDLE FAILURE. 
				Error("Cannot handle failure (HACK ME!)\n");
				assert(0);
			}
		}
		else if(tri.task_request_state==TRC_SendFileDownloadH)
		{
			// Well, then let's send the file request. First, we send the 
			// header, then we copy the data: 
			int fail=1;
			do {
				assert(send_buf.content==Cmd_NoCommand);
				if(_ResizeRespBuf(&send_buf,sizeof(LDRFileDownload)))  break;
				
				send_buf.content=Cmd_FileDownload;
				LDRFileDownload *pack=(LDRFileDownload*)(send_buf.data);
				pack->length=sizeof(LDRFileDownload);  // host order
				pack->command=Cmd_FileDownload;  // host order
				pack->task_id=htonl(tri.scheduled_to_send->task_id);
				pack->file_type=htons(tri.req_file_type);
				pack->file_idx=htons(tri.req_file_idx);
				pack->size=htonll(tri.req_file_size);
				
				// Okay, make ready to send it: 
				if(_FDCopyStartSendBuf(pack))  break;
				
				fail=0;
			} while(0);
			static int warned=0;
			if(!warned)
			{  fprintf(stderr,"hack code to handle failure. (2)\n");  ++warned;  }
			if(fail)
			{
				// Failure. 
				#warning HANDLE FAILURE. 
				Error("Cannot handle failure (HACK ME!)\n");
				assert(0);
			}
		}
		else if(tri.task_request_state==TRC_SendFileDownloadB)
		{
			// Finally, send the file body. 
			// Be careful with files of size 0...
			fprintf(stderr,"TEST IF FILE TRANSFER WORKS for files with size=0\n");
			
			// If this assert fails, then we're in trouble. 
			// The stuff should have been set earlier. 
			assert(tri.req_tfile);
			
			assert(out_active_cmd==Cmd_NoCommand);
			int rv=_FDCopyStartSendFile(tri.req_tfile->HDPath().str(),
				tri.req_file_size);
			static int warned=0;
			if(!warned)
			{  fprintf(stderr,"hack code to handle failure. (3)\n");  ++warned;  }
			if(rv)
			{
				if(rv==-1)
				{  _AllocFailure();  }
				else if(rv==-2)
				{
					int errn=errno;
					Error("Client %s: Failed to open requested file \"%s\": "
						"%s [frame %d]\n",
						_ClientName().str(),tri.req_tfile->HDPath().str(),
						strerror(errn),tri.scheduled_to_send->frame_no);
				}
				else assert(0);  // rv=-3 may not happen here 
				Error("Cannot handle failure (HACK ME!)\n");
				assert(0);
			}
			
			out_active_cmd=Cmd_FileDownload;
		}
		//else wait for client
	}
	
	return(0);
}

// Return value: 1 -> called _KickMe(); 0 -> normal
int LDRClient::cpnotify_inpump(FDCopyBase::CopyInfo *cpi)
{
	// NOTE: This looks like something strange but it is an 
	// internal error if it fails. Because we tell the pump to 
	// copy exactly LDRHeader->length - sizeof(LDRHeader) bytes 
	// and if it reports SCLimit here, then they have to be 
	// there. 
	#if TESTING
	if(cpi->pump==in.pump_s)
	{
		FDCopyIO_Buf *_dst=(FDCopyIO_Buf*)(cpi->pump->Dest());
		if(_dst->bufdone+sizeof(LDRHeader)!=
			((LDRHeader*)(recv_buf.data))->length)
		{  assert(0);  }
	}
	#endif
	
	switch(in_active_cmd)
	{
		case Cmd_FileRequest:
		{
			assert(cpi->pump==in.pump_s);  // can be left away
			
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseFileRequest(&recv_buf);
			if(rv<0)
			{  _KickMe(0);  return(1);  }
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
			cpnotify_outpump_start();
		} break;
		case Cmd_TaskResponse:
		{
			in_active_cmd=Cmd_NoCommand;
			in.ioaction=IOA_None;
			
			int rv=_ParseTaskResponse(&recv_buf);
			if(rv<0)
			{  _KickMe(0);  return(1);  }
			assert(rv==0);
			
			recv_buf.content=Cmd_NoCommand;
			//cpnotify_outpump_start();
		} break;
		default:
			// This is an internal error. Only known packets may be accepted 
			// in _HandleReceivedHeader(). 
			Error("DONE -> hack on\n");
			assert(0);
	}
	
	return(0);
}


int LDRClient::cpnotify(FDCopyBase::CopyInfo *cpi)
{
	fprintf(stderr,"cpnotify(scode=0x%x (final=%s; limit=%s), err_no=%d (%s), %s)\n",
		cpi->scode,
		(cpi->scode & FDCopyPump::SCFinal) ? "yes" : "no",
		(cpi->scode & FDCopyPump::SCLimit) ? "yes" : "no",
		cpi->err_no,strerror(cpi->err_no),
		(cpi->pump==in.pump_s || cpi->pump==in.pump_fd) ? "IN" : "OUT");
	
	// We are only interested in FINAL codes. 
	if(!(cpi->scode & FDCopyPump::SCFinal))
	{  return(0);  }
	
	if( ! (cpi->scode & FDCopyPump::SCLimit) )
	{
		// SCError and SCEOF; handling probably in cpnotify_inpump() 
		// and cpnotify_outpump_done(). 
		Error("cpi->scode=%x. HANDLE ME.\n",cpi->scode);
		// NOTE: Also handle case where EOF is reported because 
		//       we were sending a file which is shorter than expected. 
		assert(0);  // ##
	}
	
	// NOTE: next_send_cmd in no longer used if auth is done. 
	assert(next_send_cmd==Cmd_NoCommand);
	
	if(cpi->pump==in.pump_s || cpi->pump==in.pump_fd)
	{
		cpnotify_inpump(cpi);
		// We're always listening to the client. 
		if(in_active_cmd==Cmd_NoCommand)
		{  _DoPollFD(POLLIN,0);  }
	}
	else if(cpi->pump==out.pump_s || cpi->pump==out.pump_fd)
	{
		if(!cpnotify_outpump_done(cpi))
		{
			assert(out.ioaction==IOA_None);  // If FDCopyPump is running, we may not be here. 
			cpnotify_outpump_start();
		}
	}
	else assert(0);
	
	return(0);
}


// Do not set do_send_quit=1 unless we're actually in correct state and 
// disconnect without error (or minor error). Any major error / protocol 
// violation must use do_send_quit=0. 
void LDRClient::_KickMe(int do_send_quit)
{
	// We should not call KickMe() twice. 
	// This assert is the bug trap for that: 
	assert(!DeletePending());
	
	if(sock_fd>=0)  // Otherwise: Not connected or already disconnected. 
	{
		fprintf(stderr,"kicking %s...\n",_ClientName().str());
		if(do_send_quit)
		{
			send_quit_cmd=1;
			// We don't want to read something NOW. 
			_DoPollFD(POLLOUT,POLLIN);
			return;
		}
		else
		{  _ShutdownConnection();  }
	}
	
	tdif->ClientDisconnected(this);  // This will delete us. 
}


RefString LDRClient::_ClientName()
{
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
	NetworkIOBase_LDR(failflag)
{
	int failed=0;
	
	tdif=_tdif;
	cp=NULL;
	
	connected_state=0;
	auth_passed=0;
	_counted_as_client=0;
	send_quit_cmd=0;
	
	next_send_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	tri.scheduled_to_send=NULL;
	tri.task_request_state=TRC_None;
	tri.req_tfile=NULL;
	
	c_jobs=0;
	assigned_jobs=0;
	
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
	// Unrergister at TaskDriverInterface_LDR (-> task manager): 
	tdif->UnregisterLDRClient(this);
	
	assert(!cp);  // TaskDriverInterface_LDR must have cleaned up. 
}
