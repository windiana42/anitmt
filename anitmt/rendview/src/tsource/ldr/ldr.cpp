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


// Actually send data; only do if you may POLLOUT. 
// NOTE: This will get the data length from d->length, then 
//       translate d->length to network order. 
// Gives up if the write is not atomically. 
// Retun value: 
//  0 -> OK, written
// -1 -> error; called _ConnClose(). 
// -2 -> short write; called _ConnClose(). 
int TaskSource_LDR::_AtomicSendData(ServerConn *sc,LDRHeader *d)
{
	size_t len=d->length;
	d->length=htonl(len);
	
	ssize_t wr=write(sc->fd,(char*)d,len);
	if(wr<0)
	{
		Error("LDR: Failed to send %u bytes to %s: %s\n",
			len,sc->addr.GetAddress().str(),strerror(errno));
		_ConnClose(sc,0);
		return(-1);
	}
	if(size_t(wr)<len)
	{
		Error("LDR: Short write (sent %u/%u bytes) to %s.\n",
			size_t(wr),len,sc->addr.GetAddress().str());
		_ConnClose(sc,0);
		return(-2);
	}
	return(0);
}

// Return value: 0 -> OK; -1 -> error; 1 -> read quit packet
int TaskSource_LDR::_AtomicRecvData(ServerConn *sc,LDRHeader *d,size_t len)
{
	ssize_t rd=read(sc->fd,(char*)d,len);
	if(rd<0)
	{
		Error("LDR: %s: while reading: %s\n",
			sc->addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	if(!rd)
	{
		Error("LDR: %s disconnected unexpectedly.\n",
			sc->addr.GetAddress().str());
		return(-1);
	}
	if(size_t(rd)==sizeof(LDRQuitNow))
	{
		LDRQuitNow *qn=(LDRQuitNow *)d;
		if(ntohs(qn->command)==Cmd_QuitNow)
		{
			// This actually is a quit command. 
			return(1);  // Yes, +1.
		}	
	}
	if(size_t(rd)!=len)
	{
		Error("LDR: Short read from %s at challenge (%d/%u bytes)\n",
			sc->addr.GetAddress().str(),rd,len);
		return(-1);
	}
	return(0);
}


// Called to close down the connection or if it was closed down. 
// reason: 0 -> general
//         1 -> received Cmd_QuitNow
//         2 -> auth failure
void TaskSource_LDR::_ConnClose(ServerConn *sc,int reason)
{
	if(sc->fd<0)   // Not connected or already disconnected. 
	{  return;  }  // Nothing to do. 
	
	// First, make sure we close down. 
	PollFDDPtr(sc->pollid,NULL);
	ShutdownFD(sc->pollid);  // sets pollid=NULL
	sc->fd=-1;
	
	// Okay, if that was not our server, then everything is okay: 
	if(!sc->authenticated)
	{
		Verbose("LDR: Closed connection to %s.\n",
			sc->addr.GetAddress().str());
		// Hehe... simply delete it: 
		delete sconn.dequeue(sc);
		return;
	}
	assert(reason!=2);  // Auth failure may only happen if !sc->authenticated. 
	
	if(reason==1)
	{
		Verbose("LDR: Got quit request from server %s.\n",
			sc->addr.GetAddress().str());
		#warning This is okay as long as we do not have tasks...?
		delete sconn.dequeue(sc);  return;
	}
	
	#warning Is that <unexpected> if reason==1
	Error("LDR: Unexpected connection close with auth server %s.\n",
		sc->addr.GetAddress().str());
	
	assert(0);  // handle me! kill all tasks,...
	//--> tell task manager. (missing)
}


// Returns packet length or 0 -> error. 
size_t TaskSource_LDR::_CheckRespHeader(ServerConn *sc,
	LDRHeader *d,size_t read_len,
	size_t min_len,size_t max_len)
{
	do {
		if(read_len<sizeof(LDRHeader))
		{  Error("LDR: %s: packet too short (header incomplete: %u bytes)\n",
			sc->addr.GetAddress().str(),read_len);  break;  }
		sc->last_recv_cmd=(LDRCommand)ntohs(d->command);
		if(sc->last_recv_cmd!=sc->expect_cmd)
		{  Error("LDR: conversation error with %s (expected: %s; received: %s)\n",
			sc->addr.GetAddress().str(),
			LDRCommandString(sc->expect_cmd),
			LDRCommandString(sc->last_recv_cmd));  break;  }
		size_t len=ntohl(d->length);
		if(len<min_len || len>max_len)
		{  Error("LDR: packet from %s too %s (header: %u bytes; %s: %u bytes)\n",
			sc->addr.GetAddress().str(),
			len<min_len ? "short" : "long",
			len,
			len<min_len ? "min" : "max",
			len<min_len ? min_len : max_len);  break;  }
		return(len);
	} while(0);
	
	_ConnClose(sc,0);
	return(0);  // Corretct. 
}


void TaskSource_LDR::_SendChallengeRequest(ServerConn *sc)
{
	assert(!sc->authenticated);
	
	LDRChallengeRequest d;
	d.length=sizeof(LDRChallengeRequest);  // STILL IN HOST ORDER
	d.command=htons(Cmd_ChallengeRequest);
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
	if(_AtomicSendData(sc,&d))  return;
	
	// Fill in what we expect: 
	LDRComputeCallengeResponse(&d,sc->expect_chresp,p->password.str());
	
	sc->expect_cmd=Cmd_ChallengeResponse;
	sc->next_send_cmd=Cmd_NoCommand;
	PollFD(sc->fd,POLLIN);
}


// Retval: -1 -> error; 0 -> OK; 1 -> got quit
int TaskSource_LDR::_RecvChallengeResponse(ServerConn *sc)
{
	assert(!sc->authenticated);
	
	LDRChallengeResponse d;
	int rv=_AtomicRecvData(sc,&d,sizeof(d));
	if(rv)  return(rv);
	
	// Check what we got...
	size_t len=_CheckRespHeader(sc,&d,sizeof(d),sizeof(d),sizeof(d));
	if(!len)  return(-1);
	
	// See if the response is okay: 
	int authcode=100;  // illegal value
	if(memcmp(sc->expect_chresp,d.response,LDRChallengeLength))
	{
		Error("LDR: Illegal challenge response from %s.\n",
			sc->addr.GetAddress().str());
		authcode=CAC_AuthFailed;
	}
	else if(sconn.first()->authenticated)
	{
		Warning("LDR: Denying conn to %s (already connected to %s).\n",
			sc->addr.GetAddress().str(),
			sconn.first()->addr.GetAddress().str());
		authcode=CAC_AlreadyConnected;
	}
	else
	{
		// So, we must set the authenticated flag here. 
		sc->authenticated=1;
		// And we must make sure that sc becomes the 
		// first elem in the list: 
		sconn.dequeue(sc);
		sconn.insert(sc);
		
		// Okay, correct challenge and no other authenticated server. 
		VerboseSpecial("LDR: Now connected to %s (%.*s).",
			sc->addr.GetAddress().str(),
			LDRIDStringLength,d.id_string);
		authcode=CAC_Success;
	}
	memset(sc->expect_chresp,0,LDRChallengeLength);
	sc->now_conn_auth_code=authcode;
	
	sc->expect_cmd=Cmd_NoCommand;
	sc->next_send_cmd=Cmd_NowConnected;
	PollFD(sc->fd,POLLOUT);
	
	return(0);
}


void TaskSource_LDR::_SendNowConnected(ServerConn *sc)
{
	assert(!sc->authenticated || sc->now_conn_auth_code==CAC_Success);
	
	LDRNowConnected *dummy=NULL;
	size_t len = (sc->now_conn_auth_code==CAC_Success) ? 
		sizeof(LDRNowConnected) : 
		(((char*)&dummy->auth_code)-((char*)dummy)+sizeof(dummy->auth_code));
	char tmp[len];
	LDRNowConnected *d=(LDRNowConnected *)tmp;
	d->length=len;  // STILL IN HOST ORDER
	d->command=htons(Cmd_NowConnected);
	d->auth_code=htons(sc->now_conn_auth_code);
	if(sc->now_conn_auth_code==CAC_Success)
	{
		TaskManager *taskman=component_db->taskmanager();
		d->njobs=htons(taskman->Get_njobs());
		HTime2LDRTime(taskman->Get_starttime(),&d->starttime);
	}
	
	// That was the packet. Send it. 
	// I assume the challenge request is so small (64 bytes or so) 
	// that it can be sent atomically. Otherwise, we give up. 
	if(_AtomicSendData(sc,d))  return;
	
	if(sc->now_conn_auth_code==CAC_Success)
	{
		sc->expect_cmd=Cmd_NoCommand;
		sc->next_send_cmd=Cmd_NoCommand;
		PollFD(sc->fd,POLLIN);
	}
	else
	{  _ConnClose(sc,2);  }
	sc->now_conn_auth_code=0;
}


void TaskSource_LDR::_ListenFdNotify(FDInfo *fdi)
{
	if(fdi->revents & POLLIN)
	{
		MyAddrInfo addr;
		int as=addr.accept(p->listen_fd);
		if(as<0)
		{
			// Oh dear, something failed. 
			Warning("LDR: Failed to accept connection: %s\n",
				strerror(errno));
			return;
		}
		
		// Okay, we may receive a connection. 
		ServerConn *sc=NEW<ServerConn>();
		if(sc)
		{
			if(PollFD(as,POLLOUT,sc,&sc->pollid))
			{  delete sc;  sc=NULL;  }
		}
		if(sc)
		{
			sc->addr=addr;
			sc->fd=as;
			sc->next_send_cmd=Cmd_ChallengeRequest;
		}
		else
		{
			Error("LDR: Accept failed (alloc failure).\n");
			close(as);
			return;
		}
		
		// Okay, accepted a connection: 
		Verbose("LDR: Accepted connection from %s.\n",sc->addr.GetAddress().str());
		sconn.append(sc);
		return;
	}
	if(fdi->revents & (POLLERR | POLLHUP))
	{
		Error("LDR: Unknown revents for listening socket %d: %d. Aborting.\n",
			fdi->fd,fdi->revents);
		abort();  return;
	}
}


// Return value: 
//   0 -> not handeled
//   1 -> handeled
//  -1 -> must call _ConnClose(). 
int TaskSource_LDR::_AuthSConnFDNotify(FDInfo *fdi,ServerConn *sc)
{
	assert(sc->authenticated);
	
	if(fdi->revents & POLLIN)
	{
		// Read protocol header: 
		// We want to get it atomically. 
		LDRHeader hdr;
		int rv=_AtomicRecvData(sc,&hdr,sizeof(hdr));
		if(rv)
		{  _ConnClose(sc,rv<0 ? 0 : 1);  return(1);  }
	}
	
	Error("*** hack on\n");
	_ConnClose(sc,0);
	return(-1);
}


void TaskSource_LDR::_SConnFDNotify(FDInfo *fdi,ServerConn *sc)
{
	int handeled=0;
	
	// See what we must do...
	if(fdi->revents & POLLIN)
	{
		if(sc->expect_cmd==Cmd_ChallengeResponse)
		{
			int rv=_RecvChallengeResponse(sc);
			if(rv)  _ConnClose(sc,rv<0 ? 0 : 1);
			handeled=1;
		}
		else if(sc->authenticated)
		{
			handeled=_AuthSConnFDNotify(fdi,sc);
			if(handeled<0)
			{  _ConnClose(sc,0);  handeled=1;  }
		}
	}
	if(fdi->revents & POLLOUT)
	{
		if(sc->next_send_cmd==Cmd_ChallengeRequest)
		{  _SendChallengeRequest(sc);  handeled=1;  }
		else if(sc->next_send_cmd==Cmd_NowConnected)
		{  _SendNowConnected(sc);  handeled=1;  }
	}
	
	if(sc->fd<0)  // _ConnClose() was called. 
	{  return;  }
	if(!handeled || (fdi->revents & (POLLPRI | POLLERR | POLLNVAL)) )
	{
		// See if there is an error: 
		int errval=GetSocketError(sc->fd);
		if(errval)
		{
			// We do not deal with the case of errval<0 here. 
			// GetSocketError() did not fail for initial connect; 
			// why sould it fail here. 
			// If you get "Unknown error" then proably getspockopt() 
			// failed...
			Error("LDR: %s: connection error: %s\n",
				sc->addr.GetAddress().str(),strerror(errval));
		}
		else
		{
			Error("LDR: %s: unexpected revents=%d (state %d,%d). Quitting.\n",
				sc->addr.GetAddress().str(),fdi->revents,
				sc->expect_cmd,sc->next_send_cmd);
		}
		_ConnClose(sc,0);
	}
}


int TaskSource_LDR::fdnotify(FDInfo *fdi)
{
	if(fdi->dptr)
	{
		// This must be one of the ServerConns: 
		ServerConn *sc=(ServerConn*)fdi->dptr;
		assert(sc->pollid==fdi->pollid);
		assert(sc->fd==fdi->fd);
		
		// Okay, process that: 
		_SConnFDNotify(fdi,sc);
		
		return(0);
	}
	
	if(fdi->fd==p->listen_fd)
	{
		_ListenFdNotify(fdi);
		return(0);
	}
	
	assert(0);
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
	FDCopyBase(failflag),
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
		ServerConn *sc=sconn.popfirst();
		if(sc->fd>=0)
		{
			fprintf(stderr,"OOPS: still connected to %s\n",
				sc->addr.GetAddress().str());
			ShutdownFD(sc->fd);  sc->pollid=NULL;
		}
		delete sc;
	}
	
	// The listen FD is closed & shut down by the factory. 
	// We just have to explicitly unpoll it: 
	UnpollFD(l_pid);  l_pid=NULL;
}


/******************************************************************************/

TaskSource_LDR::ServerConn::ServerConn(int *failflag) : 
	LinkedListBase<ServerConn>(),
	addr(failflag)
{
	fd=-1;
	pollid=NULL;
	
	authenticated=0;
	
	next_send_cmd=Cmd_NoCommand;
	last_recv_cmd=Cmd_NoCommand;
	expect_cmd=Cmd_NoCommand;
	
	memset(expect_chresp,0,LDRChallengeLength);
}

TaskSource_LDR::ServerConn::~ServerConn()
{
	memset(expect_chresp,0,LDRChallengeLength);
}
