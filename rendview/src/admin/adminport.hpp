/*
 * adminport.hpp
 * 
 * Header for admin port class. 
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

#ifndef _RNDV_ADMIN_ADMINPORT_HPP_
#define _RNDV_ADMIN_ADMINPORT_HPP_ 1

#include "adminconn.hpp"
#include "admincexec.hpp"


class RendViewAdminPort : 
	public FDBase,
	//public TimeoutBase,
	public par::ParameterConsumer_Overloaded
{
	friend class RendViewAdminConnection;
	private:
		// Well... what could that be? ;)
		ComponentDataBase *cdb;
		
		// Enable admin port? (Default: no)
		bool enable_admin_port;
		
		// Port we will bind the listening socket: 
		int listen_port;
		// PollID for socket we're listening to: 
		PollID l_pid;
		
		// Password required to connect. 
		RefString password;
		
		// Auth timeout in msec; -1 to disable: 
		long auth_timeout;
		// Idle timeout; disconnect admin shell when idle for longer than 
		// this amount of time; -1 to disable. 
		long idle_timeout;
		
		// Listening socket (bound to listen_port). 
		int listen_fd;
		
		// List of all accepted connections: 
		LinkedList<RendViewAdminConnection> conn_list;
		
		// List of all command executors: 
		LinkedList<RendViewAdmin_CommandExecuter> exec_list;
		
		int _RegisterParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// [overriding virtual from FDBase:]
		int fdnotify(FDInfo *fdi);
		
	public: _CPP_OPERATORS_FF
		RendViewAdminPort(ComponentDataBase *cdb,int *failflag=NULL);
		~RendViewAdminPort();
		
		// Called by the TaskManager to finally set up the admin port: 
		int FinalInit();
		
		// Called by connection when closed down: 
		void ConnClose(RendViewAdminConnection *conn,int reason);
		
		// Called by RendViewAdminConnection to actually execute the 
		// passed command and store the result in the passed 
		// buffer. The complete execution has to be done on the stack. 
		void ExecuteCommand(const RefString &cmd,RVAPGrowBuffer *dest);
		
		// Used by RendViewAdmin_CommandExecuter to (un)register: 
		void Register(RendViewAdmin_CommandExecuter *exc);
		void Unregister(RendViewAdmin_CommandExecuter *exc);
};

#endif  /* _RNDV_ADMIN_ADMINPORT_HPP_ */
