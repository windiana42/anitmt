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

#include <assert.h>

using namespace LDR;


// Actually send data; only do if you may POLLOUT. 
// NOTE: This will get the data length from d->length, then 
//       translate d->length to network order. 
// Gives up if the write is not atomically. 
// Retun value: 
//  0 -> OK, written
// -1 -> error; called _ConnCloseUnexpected(). 
// -2 -> short write; called _ConnCloseUnexpected(). 
int TaskSource_LDR::_AtomicSendData(ServerConn *sc,LDRHeader *d)
{
	size_t len=d->length;
	d->length=htonl(len);
	
	ssize_t wr=write(sc->fd,(char*)d,len);
	if(wr<0)
	{
		Warning("Failed to send %u bytes to %s: %s\n",
			len,sc->addr.GetAddress().str(),strerror(errno));
		_ConnCloseUnexpected(sc);
		return(-1);
	}
	if(size_t(wr)<len)
	{
		Warning("Short write (sent %u/%u bytes) to %s: %s\n",
			size_t(wr),len,sc->addr.GetAddress().str(),strerror(errno));
		_ConnCloseUnexpected(sc);
		return(-1);
	}
	return(0);
}


// Called to close down the connection unexpectedly 
// or if the connection was closed unexpectedly by peer: 
void TaskSource_LDR::_ConnCloseUnexpected(ServerConn *sc)
{
	// First, make sure we close down. 
	PollFDDPtr(sc->pollid,NULL);
	ShutdownFD(sc->pollid);  // sets pollid=NULL
	sc->fd=-1;
	
	// Okay, if that was not our server, then everything is okay: 
	if(!sc->authenticated)
	{
		Verbose("Closed connection to %s.\n",
			sc->addr.GetAddress().str());
		// Hehe... simply delete it: 
		delete sconn.dequeue(sc);
		return;
	}
	
	Warning("Unexpected connection close with auth server %s.\n",
		sc->addr.GetAddress().str());
	assert(0);  // handle me! kill all tasks,...
}


void TaskSource_LDR::_FillInLDRHEader(ServerConn *sc,
	LDRHeader *d,LDRCommand cmd,size_t length)
{
	assert(length<0xffffffff);
	
	d->length=length;  // STILL IN HOST ORDER
	d->command=htons(cmd);
	d->seq_no=htons(sc->next_send_seq_no);
	d->ack_no=htons(sc->next_send_ack_no);
	
	++sc->next_send_seq_no;
}


void TaskSource_LDR::_SendChallengeRequest(ServerConn *sc)
{
	LDRChallengeRequest d;
	_FillInLDRHEader(sc,&d,LDR_ChallengeRequest,sizeof(LDR_ChallengeRequest));
	uchar *cr=((uchar*)&d)+sizeof(LDRHeader);
	uchar *ce=((uchar*)&d)+sizeof(LDR_ChallengeRequest);
	memset(cr,0,ce-cr);
	strcpy((char*)d.id_string,"RendView-");
	strncpy((char*)d.id_string+9,VERSION,LDRIDStringLength-9);
	d.protocol_vers=htons(LDRProtocolVersion);
	#warning missing: challenge
	//d.challenge
	
	// That was the packet. Send it. 
	// I assume the challenge request is so small (64 bytes or so) 
	// that it can be sent atomically. Otherwise, we give up. 
	if(_AtomicSendData(sc,&d))  return;
	
}


void TaskSource_LDR::_ListenFdNotify(FDInfo *fdi)
{
	if(fdi->revents & (POLLERR | POLLHUP))
	{
		Error("Unknown revents for listening socket %d: %d. Aborting.\n",
			fdi->fd,fdi->revents);
		abort();  return;
	}
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
			if(PollFD(sc->fd,POLLOUT,sc,&sc->pollid))
			{  delete sc;  sc=NULL;  }
		}
		if(sc)
		{
			sc->addr=addr;
			sc->fd=as;
			sc->next_send_cmd=LDR_ChallengeRequest;
		}
		else
		{
			Error("LDR: Accept failed (alloc failure).\n");
			close(as);
			return;
		}
		
		// Okay, accepted a connection: 
		Verbose("Accepted connection from %s.\n",sc->addr.GetAddress().str());
		sconn.append(sc);
	}
}


void TaskSource_LDR::_SConnFDNotify(FDInfo *fdi,ServerConn *sc)
{
	// See what we must do...
	if(fdi->revents & POLLIN)
	{
		
	}
	if(fdi->revents & POLLOUT)
	{
		switch(sc->next_send_cmd)
		{
			case LDR_ChallengeRequest:  _SendChallengeRequest(sc);  break;
		}
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
	Error("Iclude assert in next line!!\n"); assert(0);
	//assert(sizeof(LDR::LDRHeader)==6);  <---- UNCOMMENT!!
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
	
	next_send_cmd=LDR_NoCommand;
	last_recv_cmd=LDR_NoCommand;
	
	next_send_seq_no=1;
	next_send_ack_no=0;
	next_expect_ack_no=65535;  // any value
	next_expect_seq_no=65535;  // any value
}
