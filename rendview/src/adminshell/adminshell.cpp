/*
 * adminshell/adminshell.cpp
 * 
 * RendView admin shell implementation. 
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

#include "adminshell.hpp"

#include <netdb.h>
#include <arpa/inet.h>

#include <ctype.h>

#include <assert.h>

using namespace RVAP;


int RVAdminShell::InterpreteLine(const RefString &line)
{
	assert(out.ioaction==IOA_ReadLine);
	
	// Kill white space at beginning and end. 
	RefString tmp;
	const char *str=line.str();
	while(isspace(*str))  ++str;
	const char *start=str;
	while(*str)  ++str;  --str;
	while(isspace(*str) && str>=start)  --str;  ++str;
	
	// Empty command: 
	if(str<=start)  return(0);
	size_t str_len=str-start;
	
	if(str_len==4 && !strncmp(start,"quit",4))
	{  return(1);  }
	
	// Okay, now we send the command to the server. 
	size_t packlen=sizeof(RVAPCommandString)+str_len;
	RVAPCommandString *pack=(RVAPCommandString*)LMalloc(packlen);
	CheckMalloc(pack);
	
	pack->length=htonl(packlen);
	pack->ptype=htons(RVAP_CommandString);
	memcpy(pack->command_str,start,str_len);  // No NUL termination. 
	
	assert(!send_pack);
	send_pack=pack;
	send_len=packlen;
	send_done=0;
	
	FDChangeEvents(pollid,POLLOUT,0);
	out.ioaction=IOA_SendLine;
	DisableInput();
	
	return(0);
}


int RVAdminShell::Run()
{
	int failed=0;
	
	// Make sure we're interruptible while doing this: 
	struct sigaction save_sa_int;
	struct sigaction save_sa_term;
	sigaction(SIGINT,NULL,&save_sa_int);
	sigaction(SIGTERM,NULL,&save_sa_term);
	signal(SIGINT,SIG_DFL);
	signal(SIGTERM,SIG_DFL);
	
	char *password=NULL;
	
	do {
		// Lookup server: 
		switch(server_addr.SetAddress(server_name.str(),server_port))
		{
			case 0:  break;
			case -1:
				fprintf(stderr,"Failed to resolve host \"%s\": %s\n",
					server_name.str(),hstrerror(h_errno));
				++failed;
				break;
			case -2:
				fprintf(stderr,"gethostbyname() did not return AF_INET "
					"for \"%s\".\n",server_name.str());
				++failed;
				break;
			default: assert(0);
		}
		if(failed) break;
		
		// Connect: 
		sock_fd=server_addr.socket();
		if(sock_fd<0)
		{  fprintf(stderr,"failed to create inet socket: %s\n",
			strerror(errno));  ++failed;  break;  }
		
		// Okay, now get the password: 
		password=getpass("Passord: ");
		
		if(server_addr.connect(sock_fd)<0)
		{   fprintf(stderr,"Failed to connect to %s: %s\n",
			server_name.str(),strerror(errno));  ++failed;  break;  }
		
		if(PollFD(sock_fd,POLLIN,/*dptr=*/NULL,&pollid)<0)
		{  pollid=NULL;  }
		if(!pollid)
		{  fprintf(stderr,"PollFD failed.\n");  ++failed;  break;  }
		
		RVAPChallengeRequest chreq;
		int rv=_AtomicRecvData(&chreq,sizeof(chreq));
		if(rv)
		{
			if(rv==1)
			{  fprintf(stderr,"%s closed connection (unexpectedly).\n",
				server_addr.GetAddress().str());  }
			++failed;  break;
		}
		
		if(chreq.length!=sizeof(chreq) || chreq.ptype!=RVAP_ChallengeRequest)
		{
			fprintf(stderr,"%s: Illegal response: length=%u,ptype=%u\n",
				server_addr.GetAddress().str(),chreq.length,chreq.ptype);
			++failed;  break;
		}
		idle_timeout=LDR::LDR_timeout_ntoh(chreq.idle_msec);
		char idletmp[32];
		if(idle_timeout<0)  strcpy(idletmp,"[none]");
		else snprintf(idletmp,32,"%ld sec",idle_timeout/1000);
		int proto_vers=ntohs(chreq.protocol_vers);
		fprintf(stdout,
			"Now connected to %s: %.*s, RVAP v%d, idle timeout: %s\n",
			server_name.str(),RVAPIDStringLength,chreq.id_string,
			proto_vers,idletmp);
		if(proto_vers!=RVAPProtocolVersion)
		{
			fprintf(stderr,"%s: Protocol version mismatch: "
				"got v%d, expect v%d\n",server_addr.GetAddress().str(),
				proto_vers,RVAPProtocolVersion);
			++failed;  break;
		}
		
		// Compute challenge response: 
		RVAPChallengeResponse chresp;
		chresp.length=sizeof(RVAPChallengeResponse);
		chresp.ptype=RVAP_ChallengeResponse;
		const char *id_str="RVAdminShell-" VERSION;
		strncpy((char*)chresp.id_string,id_str,RVAPIDStringLength);  // Padding with NUL is what we want. 
		RVAPComputeCallengeResponse((uchar*)chreq.challenge,
			(char*)chresp.response,password);
		// And send it: 
		rv=_AtomicSendData(&chresp);
		if(rv)
		{  ++failed;  break;  }
		
		// Wait for success message: 
		// If auth failed, the server simply closes the connection. 
		// Wait for something...
		auth_state=0;
	} while(0);
	
	if(password)
	{  memset(password,0,strlen(password));  password=NULL;  }
	
	if(failed)
	{
		if(sock_fd>=0)
		{  shutdown(sock_fd,2);  close(sock_fd);  sock_fd=-1;  }
	}
	
	sigaction(SIGINT,&save_sa_int,NULL);
	sigaction(SIGTERM,&save_sa_term,NULL);
	
	return(failed ? 1 : 0);
}


// RETURNS HEADER IN HOST ORDER. 
// Return value: 0 -> OK; <0 -> error, 1 -> read 0 bytes. 
int RVAdminShell::_AtomicRecvData(RVAPHeader *d,size_t len)
{
	ssize_t rd;
	do
	{  rd=read(sock_fd,(char*)d,len);  }
	while(rd<0 && errno==EINTR);
	if(rd<0)
	{
		fprintf(stderr,"%s: While reading: %s\n",
			server_addr.GetAddress().str(),strerror(errno));
		return(-2);
	}
	in.tot_transferred+=u_int64_t(rd);
	if(!rd)
	{
		assert(len);  // We may not attempt zero-sized reads. 
		return(1);
	}
	if(size_t(rd)>=sizeof(RVAPHeader))
	{
		// Translate to host order: 
		d->length=ntohl(d->length);
		d->ptype=ntohs(d->ptype);
		if(d->length<sizeof(RVAPHeader))
		{
			fprintf(stderr,"Illegally-sized packet from %s (%u<%u)\n",
				server_addr.GetAddress().str(),d->length,sizeof(RVAPHeader));
			return(-5);
		}
	}
	if(size_t(rd)!=len)
	{
		fprintf(stderr,"Short read from %s (%d/%u bytes)\n",
			server_addr.GetAddress().str(),rd,len);
		return(-4);
	}
	return(0);
}


int RVAdminShell::_AtomicSendData(RVAPHeader *d)
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
		fprintf(stderr,"Failed to send %u bytes to %s: %s\n",
			len,server_addr.GetAddress().str(),strerror(errno));
		return(-1);
	}
	out.tot_transferred+=u_int64_t(wr);
	if(size_t(wr)<len)
	{
		fprintf(stderr,"Short write (sent %u/%u bytes) to %s.\n",
			size_t(wr),len,server_addr.GetAddress().str());
		return(-2);
	}
	return(0);
}


void RVAdminShell::_QuitNow(int exit_val)
{
	if(sock_fd>=0)
	{
		ShutdownFD(pollid);  // sets pollid=NULL
		sock_fd=-1;
	}
	
	FDManager::manager->Quit(exit_val);
}


int RVAdminShell::rlnotify(RLNInfo *rli)
{
	if(rli->status==-1)
	{  CheckMalloc(NULL);  }
	
	if(!rli->line)
	{
		// EOF. 
		DisableInput();
		printf("quit\n");
		_QuitNow(0);
		return(0);
	}
	
	if(!*rli->line.str())
	{
		// Empty line. 
		return(0);  // Do not add to history. 
	}
	
	// Interprete line: 
	if(InterpreteLine(rli->line))
	{
		DisableInput();
		_QuitNow(1);
		return(0);
	}
	
	int add_to_hist=enable_history;
	if(!!last_line && rli->line==last_line)
	{  add_to_hist=0;  }
	else
	{  last_line=rli->line;  }
	if(add_to_hist && rli->line.str()[0]==' ')
	{  add_to_hist=0;  }
	
	return(add_to_hist);
}


int RVAdminShell::fdnotify(FDInfo *fdi)
{
	// Must call this one to keep readline informed: 
	HReadLine::fdnotify(fdi);
	
	if(fdi->pollid==pollid)
	{
		if(fdi->revents & (POLLPRI | POLLERR | POLLNVAL))
		{
			// See if there is an error: 
			int errval=GetSocketError(sock_fd);
			if(errval)
			{  fprintf(stderr,"%s: connection error: %s\n",
				server_addr.GetAddress().str(),strerror(errval));  }
			else
			{  fprintf(stderr,"%s: unexpected revents=%d. Quitting.\n",
				server_addr.GetAddress().str(),fdi->revents);  }
			_QuitNow(1);  return(0);
		}
		
		if(auth_state==0)
		{
			if(fdi->revents & POLLHUP)
			{
				auth_failed:;
				fprintf(stderr,"%s: Auth failed. Probably wrong password.\n",
					server_addr.GetAddress().str());
				_QuitNow(1);  return(0);
			}
			assert(!(fdi->revents & POLLOUT));
			if(fdi->revents & POLLIN)
			{
				RVAPNowConnected ncon;
				int rv=_AtomicRecvData(&ncon,sizeof(ncon));
				if(rv==1)  goto auth_failed;
				if(rv)
				{  _QuitNow(1);  return(0);  }
				
				if(ncon.length!=sizeof(ncon) || ncon.ptype!=RVAP_NowConnected)
				{
					fprintf(stderr,"Illegal response from %s: length=%u,ptype=%u\n",
						server_addr.GetAddress().str(),ncon.length,ncon.ptype);
					_QuitNow(1);  return(0);
				}
				
				if(idle_timeout>=0 && keepalive_div>0)
				{
					long keepalive_msec=idle_timeout/keepalive_div;
					fprintf(stdout,"Sending keepalives every %ld msec.\n",
						keepalive_msec);
					UpdateTimer(idle_tid,keepalive_msec,FDAT_FirstEarlier | 10);
				}
				
				// Okay, we're now connected. 
				auth_state=1;
				RefString tmp;
				if(tmp.sprintf(0,"RV@%s> ",server_name.str()))
				{  CheckMalloc(NULL);  }
				SetPrompt(tmp);
				
				out.ioaction=IOA_ReadLine;
				EnableInput();
			}
		}
		else
		{
			if(fdi->revents & POLLHUP)
			{
				disconnected:;
				fprintf(stderr,"\n%s: Server closed connection.\n",
					server_addr.GetAddress().str());
				_QuitNow(1);  return(0);
			}
			
			if(fdi->revents & POLLIN)
			{
				if(in.ioaction==IOA_ReadHeader)
				{
					RVAP::RVAPHeader hdr;
					int rv=_AtomicRecvData(&hdr,sizeof(hdr));
					if(rv)
					{
						if(rv==1)  goto disconnected;
						_QuitNow(1);  return(0);
					}
					
					if(out.ioaction!=IOA_WaitResp && 
						hdr.ptype!=RVAP_Message)
					{  fprintf(stderr,"\n[Just receiving unwanted data...]\n");  }
					
					if(hdr.ptype==RVAP_CommandResp)
					{
						// This only works because RVAPCommandResp is effectively 
						// a plain RVAPHeader with no additional fields. 
						if(sizeof(RVAPCommandResp)!=sizeof(RVAPHeader))
						{  assert(0);  }
						
						RVAPCommandResp *pack=(RVAPCommandResp*)&hdr;
						if(pack->length<sizeof(RVAPCommandResp))
						{  fprintf(stderr,"Illegally-sized packet from %s "
							"(%u<%u)\n",server_addr.GetAddress().str(),
							pack->length,sizeof(RVAPCommandResp));
							_QuitNow(1);  return(0);  }
						
						recv_len=((uchar*)pack)+pack->length-pack->response;
						if(!recv_len)
						{
							fprintf(stdout,"[empty response]\n");
							
							//in.ioaction=OA_ReadHeader;  // ...still
							out.ioaction=IOA_ReadLine;
							EnableInput();
						}
						else
						{
							in.ioaction=IOA_ReadBody;
							recv_done=0;
						}
					}
					else
					{
						fprintf(stderr,"Received unknown header ptype=%u, "
							"len=%u from %s\n",hdr.ptype,hdr.length,
							server_addr.GetAddress().str());
						_QuitNow(1);  return(0);
					}
				}
				else if(in.ioaction==IOA_ReadBody)
				{
					// This is fairly easy. We simply read it and dump it to 
					// the terminal. 
					ssize_t rd;
					
					size_t want=recv_len-recv_done;
					if(want>in_read_size)  want=in_read_size;
					assert(want);
					
					do
					{  rd=::read(sock_fd,in_read_buf,want);  }
					while(rd<0 && errno==EINTR);
					
					if(rd<0)
					{
						fprintf(stderr,"%s: While receiving: %s\n",
							server_addr.GetAddress().str(),
							strerror(errno));
						_QuitNow(1);  return(0);
					}
					if(!rd)  goto disconnected;
					
					in.tot_transferred+=rd;
					
					for(ssize_t done=0; done<rd;)
					{
						ssize_t wr=0;
						do
						{ wr=::write(fileno(stdout),in_read_buf+done,rd-done); }
						while(wr<0 && errno==EINTR);
						
						if(wr<=0)
						{
							fprintf(stderr,"OOPS: Terminal write error:\n",
								wr==0 ? "hangup" : strerror(errno));
							_QuitNow(1);  return(0);
						}
						done+=wr;
					}
					
					recv_done+=rd;
					if(recv_done>=recv_len)
					{
						in.ioaction=IOA_ReadHeader;
						recv_done=0;
						recv_len=0;
						
						out.ioaction=IOA_ReadLine;
						EnableInput();
					}
				}
			}
			if(fdi->revents & POLLOUT)
			{
				if(out.ioaction==IOA_SendLine)
				{
					assert(send_pack);
					size_t want_send=send_len-send_done;
					assert(want_send);
					
					char *buf=((char*)send_pack)+send_done;
					
					ssize_t wr;
					do
					{  wr=::write(sock_fd,buf,want_send);  }
					while(wr<0 && errno==EINTR);
					
					if(wr<=0)
					{
						fprintf(stderr,"%s: While sending: %s\n",
							server_addr.GetAddress().str(),
							wr==0 ? "hangup" : strerror(errno));
						_QuitNow(1);  return(0);
					}
					
					send_done+=wr;
					out.tot_transferred+=wr;
					if(send_done>=send_len)
					{
						// Done sending. 
						send_pack=(RVAPHeader*)LFree(send_pack);
						send_len=0;
						send_done=0;
						
						FDChangeEvents(pollid,0,POLLOUT);
						out.ioaction=IOA_WaitResp;
						in.ioaction=IOA_ReadHeader;
						
						// We just sent something...
						send_keepalive=0;
						ResetTimer(idle_tid,FDAT_FirstEarlier | 10);
					}
				}
				else if(send_keepalive)
				{
					RVAPHeader pack;
					pack.length=sizeof(pack);
					pack.ptype=RVAP_NoOp;
					
					fprintf(stderr,"[keepalive]");
					
					if(_AtomicSendData(&pack))
					{  _QuitNow(1);  return(0);  }
					
					send_keepalive=0;
					if(out.ioaction!=IOA_SendLine)
					{  FDChangeEvents(pollid,0,POLLOUT);  }
				}
				else assert(0);
			}
		}
	}
	
	return(0);
}


int RVAdminShell::signotify(const SigInfo *si)
{
	if(si->info.si_signo==SIGINT || si->info.si_signo==SIGTERM)
	{  _QuitNow(1);  }
	return(0);
}


int RVAdminShell::timernotify(FDBase::TimerInfo *ti)
{
	if(ti->tid==idle_tid)
	{
		// We need to send a keepalive. 
		if(out.ioaction!=IOA_SendLine)
		{
			send_keepalive=1;
			FDChangeEvents(pollid,POLLOUT,0);
		}
	}
	else assert(0);
	
	return(0);
}


int RVAdminShell::CheckParams()
{
	int failed=0;
	
	if(more_than_one_error)
	{  return(1);  }
	
	if(!server_name)
	{
		fprintf(stderr,"%s: No server specified.\n",prg_name);
		++failed;
	}
	
	if(server_port<=0 || server_port>=65536)
	{
		fprintf(stderr,"%s: Illegal server port %d.\n",prg_name,server_port);
		++failed;
	}
	
	return(failed ? 1 : 0);
}


int RVAdminShell::parse(const Section *,SPHInfo *si)
{
	do {
		if(!si->arg)  break;
		if(si->sect!=CurrentSection())  break;
		if(si->sect!=si->bot_sect)  break;
		if(si->arg->name!=si->nend)  break;
		
		const char *str=si->arg->name;
		if(!*str || *str=='-')  break;
		
		if(!server_name)
		{
			if(server_name.set(str))
			{  CheckMalloc(NULL);  }
		}
		else if(!more_than_one_error)
		{
			fprintf(stderr,"%s: More than one server specified.\n",
				prg_name);
			more_than_one_error=1;
		}
		
		return(0);
	} while(0);
	
	return(2);
}


int RVAdminShell::_SetUpParams()
{
	if(SetSection(NULL))
	{  return(1);  }
	
	add_failed=0;
	AddParam("p",
		"RendView server TCP port number",
		&server_port);
	AddParam("kad",
		"Keepalive divider; if server has idle timeout X msec, then send a "
		"keepalive every X/kad msec; -1 to disable",
		&keepalive_div);
	AddParam("hist",
		"Enable command history (default)",
		&enable_history);
	
	if(SectionParameterHandler::Attach(CurrentSection()))
	{  ++add_failed;  }
	
	return(add_failed ? 1 : 0);
}


RVAdminShell::RVAdminShell(par::ParameterManager *parman,int *failflag) : 
	HReadLine(failflag),
	par::ParameterConsumer_Overloaded(parman,failflag),
	par::SectionParameterHandler(parman,failflag),
	server_name(failflag),
	server_addr(failflag),
	last_line(failflag)
{
	int failed=0;
	
	more_than_one_error=0;
	server_port=3105;
	
	sock_fd=-1;
	pollid=NULL;
	
	auth_state=0;
	
	keepalive_div=3;
	idle_timeout=-1;
	send_keepalive=0;
	
	enable_history=1;
	
	in.tot_transferred=0;
	in.ioaction=IOA_None;
	out.tot_transferred=0;
	out.ioaction=IOA_None;
	
	send_pack=NULL;
	send_len=0;
	send_done=0;
	
	recv_len=0;
	recv_done=0;
	in_read_size=4096;
	in_read_buf=(char*)LMalloc(in_read_size);
	if(!in_read_buf)  ++failed;
	
	idle_tid=InstallTimer(-1,0);
	if(!idle_tid)  ++failed;
	
	if(_SetUpParams())  ++failed;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("RVAdminShell");  }
}

RVAdminShell::~RVAdminShell()
{
	// Be sure...
	ShutdownFD(pollid);  // sets pollid=NULL
	sock_fd=-1;
	
	send_pack=(RVAPHeader*)LFree(send_pack);
	in_read_buf=(char*)LFree(in_read_buf);
}
