/*
 * adminshell/adminshell.hpp
 * 
 * RendView admin shell header file. 
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

#ifndef _RNDV_ADMINSHELL_ADMINSHELL_HPP_
#define _RNDV_ADMINSHELL_ADMINSHELL_HPP_ 1

#include "hreadline.h"

#include <lib/prototypes.hpp>
#include <lib/myaddrinfo.hpp>

#include <hlib/valuehandler.h>
#include <hlib/parconsumerovl.h>
#include <hlib/secthdl.h>

#include <admin/rvap_proto.hpp>


class RVAdminShell : 
	public HReadLine,
	public par::ParameterConsumer_Overloaded,
	public par::SectionParameterHandler
{
	private:
		// Server to connect to: 
		RefString server_name;
		bool more_than_one_error;
		// Server TCP port: 
		int server_port;
		// Server address: 
		MyAddrInfo server_addr;
		
		// Server FD and PollID: 
		int sock_fd;
		PollID pollid;
		
		// 0 -> not authenticated; 1 -> yes
		int auth_state;
		
		enum IOAction
		{
			IOA_None=0,
			// OUT: 
			IOA_ReadLine,
			IOA_SendLine,
			IOA_WaitResp,
			// IN: 
			IOA_ReadHeader,
			IOA_ReadBody,
		};
		
		struct
		{
			u_int64_t tot_transferred;
			IOAction ioaction;
		} in,out;
		
		// Idle timeout of RendView server in msec: 
		// (Transferred upon connect.) 
		long idle_timeout;
		// Keepalive divider; send keepalive every 
		// idle_timeout/keepalive_div msec; -1 to disable. 
		int keepalive_div;
		// For the timer: 
		TimerID idle_tid;
		bool send_keepalive;
		
		// Enable command history?
		bool enable_history;
		
		// Previous line entered by user: 
		RefString last_line;
		// Current packet to be transferred: 
		RVAP::RVAPHeader *send_pack;
		// Length of send_cmd_pack and how much of 
		// send_cmd_pack was already sent. 
		size_t send_len,send_done;
		
		// Current amount of data to receive: 
		size_t recv_len,recv_done;
		// Buffer for input reading. 
		char *in_read_buf;
		size_t in_read_size;
		
		// Interprete command line: 
		int InterpreteLine(const RefString &line);
		
		// Used during auth and for RVAP headers: 
		int _AtomicRecvData(RVAP::RVAPHeader *d,size_t len);
		int _AtomicSendData(RVAP::RVAPHeader *d);
		
		// The name says it all...
		void _QuitNow(int exit_val);
		
		// Initialize parameters: 
		// Return value: 0 -> OK; 1 -> error
		int _SetUpParams();
		
		// Overriding virtual from HReadLine: 
		int rlnotify(RLNInfo *rli);
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// [overriding virtual from FDBase:]
		int fdnotify(FDBase::FDInfo *fdi);
		int signotify(const SigInfo *si);
		int timernotify(FDBase::TimerInfo *ti);
		// [overriding virtual from SectionParameterHandler:]
		int parse(const Section *,SPHInfo *);
		
	public:  _CPP_OPERATORS_FF
		RVAdminShell(par::ParameterManager *parman,int *failflag=NULL);
		~RVAdminShell();
		
		// Called to tell the RVAdminShell that we will be running. 
		// Connects to server. 
		// Return value: 
		//  0 -> OK
		//  1 -> error
		int Run();
};

#endif  /* _RNDV_ADMINSHELL_ADMINSHELL_HPP_ */
