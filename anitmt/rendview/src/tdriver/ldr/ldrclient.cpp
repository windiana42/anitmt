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


// Return value: 
//   0 -> connecting...
//   1 -> connected successfully without delay
//  -1 -> error
int LDRClient::ConnectTo(ClientParam *cp)
{
	Verbose(TDR,"  Client %s (%s): ",cp->name.str(),cp->addr.GetAddress().str());
	
	const char *failure="FAILURE\n";
	
	int sock=cp->addr.socket();
	if(sock<0)
	{
		Verbose(TDR,failure);
		Error("Failed to create inet socket: %s\n",strerror(errno));
		return(-1);
	}
	
	int fail=1;
	int already_connected=0;
	do {
		if(SetNonblocking(sock)<0)
		{
			Verbose(TDR,failure);
			Error("Failed to set socket non-blocking: %s\n",strerror(errno));
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
				Verbose(TDR,failure);
				Error("Failed to connect to %s: %s\n",
					cp->addr.GetAddress().str(),strerror(errno));
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
		pollid=tdif->PollFD_Init(this,sock);  // does POLLIN. 
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
	
	connected_state=already_connected+1;
	// Okay, now we're doing POLLIN and waiting for answer. 
	
	this->cp=cp;
	
	return(already_connected);
}


// Return value: 
//  1 -> already connected
//  0 -> wait for disconnect to happen. 
int LDRClient::Disconnect()
{
	if(connected_state<2)
	{
		Verbose(TDR,"  Nothing to do for disconnect from %s.\n",
			_ClientName().str());
		ShutdownFD();
		return(1);
	}
	
	if(!send_quit_cmd)
	{
		// We need not fiddle around with next_send_cmd and expect_cmd 
		// as send_quit_cmd has precedence and the other vars might be 
		// changed again before fdnotify() by some other notify. 
		send_quit_cmd=1;
		PollFD(POLLOUT);
	}
	
	return(0);
}


// Return value: 0 -> OK; -1 -> error
int LDRClient::_AtomicSendData(LDRHeader *d)
{
	size_t len=d->length;
	d->length=htonl(len);
	
	ssize_t wr=write(sock_fd,(char*)d,len);
	if(wr<0)
	{
		Error("Client %s: Failed to send %u bytes: %s\n",
			_ClientName().str(),len,strerror(errno));
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

// Return value: 0 -> OK; -1 -> error
ssize_t LDRClient::_AtomicRecvData(LDRHeader *dest,size_t len,size_t min_len)
{
	ssize_t rd=read(sock_fd,(char*)dest,len);
	if(rd<0)
	{
		Error("Client %s: While reading: %s\n",
			_ClientName().str(),strerror(errno));
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
	r->command=htons(next_send_cmd);
	
	LDRSetIDString((char*)r->id_string,LDRIDStringLength);
	LDRComputeCallengeResponse(d,(char*)r->response,cp->password.str());
	
	return(0);
}


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
	last_recv_cmd=(LDRCommand)ntohs(d->command);
	if(last_recv_cmd!=expect_cmd)
	{
		Error("Client %s: Conversation error (expected: %s; received: %s)\n",
			_ClientName().str(),
			LDRCommandString(expect_cmd),
			LDRCommandString(last_recv_cmd));
		return(0);  // YES!!
	}
	size_t len=ntohl(d->length);
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
			
			PollFD(POLLOUT);
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
			Verbose(TDR,"Client %s: Now connected: njobs=%d (parallel jobs)\n"
				"  Up since: %s  (local)\n",
				_ClientName().str(),c_jobs,
				up_since.PrintTime(1));
			
			auth_passed=1;
			next_send_cmd=Cmd_NoCommand;
			expect_cmd=Cmd_NoCommand;
			
			PollFD(POLLIN);  // EOF and things
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
			
			PollFD(POLLIN);
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
			d.command=htons(Cmd_QuitNow);
			
			if(_AtomicSendData(&d))
			{
				Error("  (Error above happened quring quit. No real problem.)\n");
				send_quit_cmd=3;
			}
			else
			{
				Verbose(TDR,"  Sent quit to client %s.\n",_ClientName().str());
				send_quit_cmd=2;
				PollFD(POLLIN);
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
			{  Error("Client %s: During quit (read): %s\n",
				_ClientName().str(),strerror(errno));  }
			else if(!rd)
			{  client_disconnected=1;  }
		}
		if(client_disconnected)
		{  Verbose(TDR,"Client %s disconnected due to our quit request.\n",
			_ClientName().str());  }
		else
		{  Warning("Client %s: did not disconnect after our request. "
			"Shutting down conn.\n",_ClientName().str());  }
		
		// Disconnect done. 
		send_quit_cmd=3;
	}
	
	if(send_quit_cmd==3)
	{
		// We must shutdown anyway...
		ShutdownFD();
		
		tdif->ClientDisconnected(this);  // This will delete us. 
		return;
	}
}


static inline size_t _SumUpSize(RefStrList *l)
{
	size_t len=0;
	for(const RefStrList::Node *i=l->first(); i; i=i->next)
	{  len+=(i->len()+1);  }
	return(len);
}

// Returns updated dest pointer; copies '\0' at the end. 
static char *_CopyStrList2Data(char *dest,RefStrList *l)
{
	for(const RefStrList::Node *i=l->first(); i; i=i->next)
	{
		const char *s=i->str();
		while( (*(dest++)=*(s++)) );  // The routine that made C famous - ;)
	}
	return(dest);
}


// This formats the whole LDRDoTask data into the RespBuf *dest. 
// Return value: 
//   0 -> OK complete packet in resp_buf; ready to send it 
//  -1 -> alloc failure
int LDRClient::_Create_DoTask_Packet(CompleteTask *ctsk,RespBuf *dest)
{
	assert(dest->content==Cmd_NoCommand);
	
	RenderTask *rt=ctsk->rt;
	FilterTask *ft=ctsk->ft;
	
	// First, compute the size of the "packet": 
	size_t totsize=sizeof(LDRDoTask);
	
	// All the additional args have to be passed: 
	size_t r_add_args_size;
	size_t r_desc_slen,r_oformat_slen;
	if(rt)
	{
		r_add_args_size=_SumUpSize(&rt->add_args);
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
		f_add_args_size=_SumUpSize(&ft->add_args);
		f_desc_slen=ft->fdesc->name.len();
	} else {
		f_add_args_size=0;
		f_desc_slen=0;
	}
	totsize+=(f_add_args_size+f_desc_slen);
	
	// Count required files: 
	#warning need code to pass the required files...
	int r_n_files=0;
	int f_n_files=0;
	// MISSING: totsize += <length of all LDRFileInfoEntries>
	
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
	LDRDoTask *pack=(LDRDoTask *)dest->data;
	pack->length=htonl(totsize);
	pack->command=htons(Cmd_TaskRequest);
	
	pack->frame_no=htonl(ctsk->frame_no);
	#warning MISSING: task_id!!
	pack->task_id=htonl(0);
	
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
	{
		strcpy(dptr,rt->rdesc->name.str());  dptr+=r_desc_slen;
		strcpy(dptr,rt->oformat->name);      dptr+=r_oformat_slen;
		dptr=_CopyStrList2Data(dptr,&rt->add_args);
	}
	if(ft)
	{
		strcpy(dptr,ft->fdesc->name.str());  dptr+=f_desc_slen;
		dptr=_CopyStrList2Data(dptr,&ft->add_args);
	}
	
	#warning MISSING: copy file info!
	// Take care of alignment issues...
	
	// Now, we must be exactly at the end of the buffer: 
	assert(dptr==dest->data+totsize);
	
	dest->content=Cmd_TaskRequest;
	return(0);
}


int LDRClient::SendTaskToClient(CompleteTask *ctsk)
{
	if(!ctsk || (!ctsk->rt && !ctsk->ft))  return(-2);
	if(!auth_passed)  return(-1);
	if(assigned_jobs>=c_jobs)  return(2);
	
	//if(currently_sending_task_to_client)  <--- MISSING!!
	//{  return(1);  }
	
	#warning ! must make sure that we put back those tasks assigned to a client when it disconnects unexpectedly. 
	#warning check CanDoTask (&& MISSING -> when we are currently busy sending a task)
	
	assert(0);
	
	return(0);
}


// Called via TaskDriverInterface_LDR: 
void LDRClient::fdnotify(FDBase::FDInfo *fdi)
{
	assert(fdi->fd==sock_fd && fdi->pollid==pollid);
	assert(connected_state);  /* otherwise: we may not be here; we have no fd */
	
	if(connected_state==1)  // waiting for response to connect(2)
	{
		if(_DoFinishConnect(fdi))
		{
			// Failed; we are not connected; give up. 
			connected_state=0;
			ShutdownFD();
			assert(pollid==NULL);
			tdif->FailedToConnect(this);  // This will delete us. 
			return;
		}
		return;
	}
	
	if(send_quit_cmd)
	{
		_DoSendQuit(fdi);
		return;
	}
	
	// We're connected. 
	if(!auth_passed)
	{
		// Still not yet authenticated. 
		if(_DoAuthHandshake(fdi))
		{
			// Failed. Give up. 
			connected_state=0;
			ShutdownFD();
			tdif->FailedToConnect(this);  // This will delete us. 
			return;
		}
		return;
	}
	
	// We have passed auth. We are the server the client listenes to. 
	Error("*** hack on...\n");
	assert(0);
}


void LDRClient::cpnotify(FDCopyBase::CopyInfo *cpi)
{
	assert(0);
}


int LDRClient::_ResizeRespBuf(RespBuf *buf,size_t newlen)
{
	// This assertion is for implementation security reason. 
	// If there is a case in which is fails although the code is okay, then 
	// remove it. 
	assert(buf->content==Cmd_NoCommand);
	
	if(buf->alloc_len<newlen || newlen*2<buf->alloc_len)
	{
		char *oldval=buf->data;
		buf->data=(char*)LRealloc(buf->data,newlen);
		if(!buf->data)
		{
			LFree(oldval);
			buf->alloc_len=0;
			return(1);
		}
		buf->alloc_len=newlen;
	}
	return(0);
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
	LinkedListBase<LDRClient>()
{
	int failed=0;
	
	tdif=_tdif;
	cp=NULL;
	
	sock_fd=-1;
	pollid=NULL;
	
	connected_state=0;
	auth_passed=0;
	_counted_as_client=0;
	send_quit_cmd=0;
	
	next_send_cmd=Cmd_NoCommand;
	last_recv_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	send_buf.alloc_len=0;
	send_buf.data=NULL;
	send_buf.content=Cmd_NoCommand;
	recv_buf.alloc_len=0;
	recv_buf.data=NULL;
	recv_buf.content=Cmd_NoCommand;
	
	#if 0
	#error HACK ME...
	send_pump.cpid=NULL;
	send_pump.cmd=Cmd_NoCommand;
	recv_pump.cpid=NULL;
	recv_pump.cmd=Cmd_NoCommand;
	#endif
	
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
	ShutdownFD();  // be sure...
	
	send_buf.data=(char*)LFree(send_buf.data);
	recv_buf.data=(char*)LFree(recv_buf.data);
	
	// Unrergister at TaskDriverInterface_LDR (-> task manager): 
	tdif->UnregisterLDRClient(this);
	
	assert(!cp);  // TaskDriverInterface_LDR must have cleaned up. 
}

