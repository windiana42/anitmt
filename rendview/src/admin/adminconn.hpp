/*
 * adminconn.hpp
 * 
 * Header for admin connection class (RendView side). 
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

#ifndef _RNDV_ADMIN_ADMINCONN_HPP_
#define _RNDV_ADMIN_ADMINCONN_HPP_ 1

#include <database.hpp>
#include <lib/myaddrinfo.hpp>
#include <hlib/fdcopybase.h>
#include "rvap_proto.hpp"
#include "rvapgrowbuffer.hpp"

class RendViewAdminPort;


class RendViewAdminConnection : 
	public FDCopyBase,
	//public TimeoutBase,
	public LinkedListBase<RendViewAdminConnection>
{
	friend class RendViewAdminPort;
	private:
		// Well... what could that be? ;)
		ComponentDataBase *cdb;
		
		// PollID and socket we're listening to: 
		PollID pollid;
		int sock_fd;
		
		// Address of other side (admin shell): 
		MyAddrInfo addr;
		
		// Time when the connection was made: 
		HTime connected_since;
		
		// Idle/timeout timer. Also auth timeout timer. 
		FDBase::TimerID idle_tid;
		
		// Schedule sending data: 
		FDBase::TimerID schedule_tid;
		bool outpump_scheduled;
		bool exec_cmd_scheduled;
		
		// Auth state: 
		enum AuthState
		{
			AS_None=0,
			AS_SendChReq,  // send challenge request
			AS_WaitChResp, // wait for challenge response
			AS_SendOK,     // send RVAPNowConnected
			AS_Passed,     // auth passed (successfully)
		} auth_state;
		// Expected challenge response: 
		char expect_chresp[RVAPChallengeRespLength];
		
		enum IOAction
		{
			IOA_None=0,   // doing nothing, waiting
			IOA_Locked,   // locked (not yet authenticated)
			IOA_Buf,      // buffer IO active
		};
		
		struct FDCopy
		{
			FDCopyIO_FD *io_sock;
			FDCopyIO_Buf *io_buf;
			FDCopyPump_Simple *pump_s;  // io_sock <-> io_buf
			IOAction ioaction;
			
			// Some statistics: 
			u_int64_t tot_transferred;
			
			int Setup(FDCopyBase *fcb);
			void Cleanup();
			FDCopy();   // NOTE: ONLY NULL-INITIALISATION
			~FDCopy() { /*empty*/ }
		};
		// FDCopy in is for READING FROM sock_fd: 
		// Read from io_sock and write to io_buf or io_fd. 
		FDCopy in;    // reading from sock_fd
		// FDCopy out is for WRITING TO sock_fd: 
		// Write data from io_buf or io_fd to io_sock. 
		FDCopy out;   // writing to sock_fd
		
		struct IOBuf
		{
			char *data;
			size_t len;
			size_t alloc_len;
			int content;
			
			int Resize(size_t new_size);
			
			IOBuf()
				{  data=NULL;  len=0;  alloc_len=0;  content=0;  }
			~IOBuf()
				{  data=(char*)LFree(data);  alloc_len=0;  len=0;  }
		} recv_buf;
		
		// IO pump state: 
		int in_active_cmd;  // 0 for none
		int out_active_cmd;  // 0 for none
		
		// Command scheduled for execution: 
		// May always only be one. 
		RefString scheduled_cmd;
		
		// Response buffer: 
		// Has response header at beginning. 
		RVAPGrowBuffer cmd_resp_buf;
		bool send_cmd_resp;
		
		// Return pointer to the admin port class: 
		RendViewAdminPort *AP()
			{  return(cdb->adminport());  }
		
		inline void schedule_outpump_start();
		
		inline void schedule_exec_cmd();
		
		inline void _ResetIdleTimeout(const HTime *curr=NULL);
		
        // Calls FDChangeEvents() and checks for errors: 
        void _DoChangeEvents_Error(int rv);
		inline void _DoPollFD(short set_events,short clear_events)
        {
            int rv=FDChangeEvents(pollid,set_events,clear_events);
            if(rv<0)
            {  _DoChangeEvents_Error(rv);  }
        }
		
		// Actually execute a command. Called from schedule timer. 
		void ExecuteCommand(RefString &command);
		
		// Auth handling: 
		int _SendChallengeRequest();
		int _RecvChallengeResponse();
		int _SendNowConnected();
		
		int _ParseCommandString(IOBuf *buf);
		
		int _AuthSConnFDNotify(FDInfo *fdi);
		int _HandleReceivedHeader(RVAP::RVAPHeader *hdr);
		
		int _StartReadingCommandBody(IOBuf *dest,RVAP::RVAPHeader *hdr);
		int _FDCopyStartRecvBuf(char *buf,size_t len);
		int _FDCopyStartSendBuf(RVAP::RVAPHeader *hdr);
		
		int _AtomicSendData(RVAP::RVAPHeader *d);
		int _AtomicRecvData(RVAP::RVAPHeader *d,size_t len);
		
		// Manage the data punps: 
		int cpnotify_outpump_start();
		int cpnotify_outpump_done(FDCopyBase::CopyInfo *cpi);
		int cpnotify_inpump(FDCopyBase::CopyInfo *cpi);
		int cpnotify_handle_errors(FDCopyBase::CopyInfo *cpi);
		
		// Terminate connection (at nearly any time): 
		// Reason: 1 -> hangup; 0 -> error; 2 -> auth failure; 3 -> timeout
		void _ConnClose(int reason);
		
		// Overriding virtuals from FDCopyBase: 
		int fdnotify2(FDBase::FDInfo *fdi);
		int cpnotify(FDCopyBase::CopyInfo *cpi);
		int cpprogress(FDCopyBase::ProgressInfo *pi);
		int timernotify(FDBase::TimerInfo *ti);
		
	public: _CPP_OPERATORS_FF
		RendViewAdminConnection(ComponentDataBase *cdb,int *failflag=NULL);
		~RendViewAdminConnection();
		
		// Called immediatly after construction to set up: 
		int Setup(int accept_socket,MyAddrInfo *addr);
};

#endif  /* _RNDV_ADMIN_ADMINCONN_HPP_ */
