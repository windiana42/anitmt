/*
 * ldr.hpp
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

#ifndef _RNDV_TSOURCE_LDR_HPP_
#define _RNDV_TSOURCE_LDR_HPP_ 1

#include "../tasksource.hpp"

#include <lib/myaddrinfo.hpp>
#include <lib/ldrproto.hpp>

#include <hlib/cpmanager.h>
#include <hlib/cpbase.h>


class TaskSourceFactory_LDR;

class TaskSource_LDR : 
	public TaskSource,
	public FDBase,
	public FDCopyBase
{
	private:
		// Our parameters: 
		TaskSourceFactory_LDR *p;
		
		// 0 msec response/schedule timer: 
		TimerID rtid;
		
		// PollID for socket we're listening to: 
		PollID l_pid;
		
		// What we're currently doing: 
		TSAction pending;
		
		// Are we connected? This is just a dummy to ensure 
		// correct behavior of the other classes...
		int connected;  // <- to TaskManager, NOT to server. 
		
		// Describing a server connection. Currently, only 
		// one server may be connected at a time which is 
		// the first in the list. 
		struct ServerConn : LinkedListBase<ServerConn>
		{
			MyAddrInfo addr;  // server address
			int fd;           // socket fd 
			PollID pollid;    // PollID of socket fd
			
			// This is 1 if that is the authenticated server we are 
			// taking orders from. (Only sconn.first().) 
			int authenticated;
			
			union
			{
				char expect_chresp[LDRChallengeLength];
				int now_conn_auth_code;
			};
			
			// LDE Commands (host order)
			LDR::LDRCommand next_send_cmd;
			LDR::LDRCommand last_recv_cmd;
			LDR::LDRCommand expect_cmd;
			
			_CPP_OPERATORS_FF
			ServerConn(int *failflag=NULL);
			~ServerConn();
		};
		LinkedList<ServerConn> sconn;
		
		// Update/start rtid timer: 
		inline void _StartSchedTimer()
			{  UpdateTimer(rtid,0,0);  }
		inline void _StopSchedTimer()
			{  UpdateTimer(rtid,-1,0);  }
		
		int _AtomicSendData(ServerConn *sc,LDR::LDRHeader *d);
		int _AtomicRecvData(ServerConn *sc,LDR::LDRHeader *d,size_t len);
		
		// Returns packet length or 0 -> error. 
		size_t _CheckRespHeader(ServerConn *sc,
			LDR::LDRHeader *d,size_t read_len,
			size_t min_len,size_t max_len);
		
		// Packet handling: 
		void _SendChallengeRequest(ServerConn *sc);
		int _RecvChallengeResponse(ServerConn *sc);
		void _SendNowConnected(ServerConn *sc);
		
		// FD handling: 
		void _ListenFdNotify(FDInfo *fdi);
		void _SConnFDNotify(FDInfo *fdi,ServerConn *sc);
		int _AuthSConnFDNotify(FDInfo *fdi,ServerConn *sc);
		
		void _ConnClose(ServerConn *sc,int reason);
		
		// overriding virtuals from FDbase: 
		int timernotify(TimerInfo *);
		int fdnotify(FDInfo *);
		// overriding virtuals from TaskSource: 
		int srcConnect(TaskSourceConsumer *);
		int srcGetTask(TaskSourceConsumer *);
		int srcDoneTask(TaskSourceConsumer *,CompleteTask *ct);
		int srcDisconnect(TaskSourceConsumer *);
		
		long ConnectRetryMakesSense();
	public: _CPP_OPERATORS_FF
		TaskSource_LDR(TaskSourceFactory_LDR *,int *failflag=NULL);
		~TaskSource_LDR();
};

#endif  /* _RNDV_TSOURCE_LDR_HPP_ */
