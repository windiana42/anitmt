/*
 * dif_param.hpp
 * 
 * Header for parameter & factory class of LDR task driver interface. 
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

#ifndef _RNDV_TDRIVER_LDR_PARS_HPP_
#define _RNDV_TDRIVER_LDR_PARS_HPP_ 1

#include <lib/myaddrinfo.hpp>


class ComponentDataBase;


class TaskDriverInterfaceFactory_LDR : 
	public TaskDriverInterfaceFactory
{
	friend class TaskDriverInterface_LDR;
	public:
		struct ClientParam : LinkedListBase<ClientParam>
		{
			RefString name;   // as specified by the user (without port)
			MyAddrInfo addr;
			RefString password;  // needed for auth at client
			
			// Client or NULL. 
			LDRClient *client;
			// 0 -> no; 1 -> this reconnect_trigger; n -> n-th reconnect trigger
			int shall_reconnect;
			
			_CPP_OPERATORS_FF
			ClientParam(int *failflag=NULL);
			~ClientParam();
		};
	private:
		// We may access component_db(). 
		
		// User-tunable param: 
		int todo_thresh_low;  // Always have this many tasks in todo (NOT proc) queue. 
		int todo_thresh_high;
		int done_thresh_high;  // Report done when this many tasks in done list. 
		
		// Default LDR port and password (if not specified after client): 
		int default_port;
		RefString default_password;
		
		long connect_timeout;   // connect(2) timeout in msec; -1 -> none
		long reconnect_interval;  // reconnect to lost clients (msec)
		
		struct DTPrm
		{
			// Execution timeout or -1: 
			long timeout;
		} prm[_DTLast];
		
		// All clients as specified by the user (IPs or (FQ)DNs): 
		RefStrList str_clients;  // cleared after CheckParams()
		
		LinkedList<ClientParam> cparam;
		
		int _SetUpParams(TaskDriverType dtype,Section *top);
		int _RegisterParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// Overriding virtuial from TaskSourceFactory: 
		int FinalInit();  // called after CheckParams(), before Create()
	public: _CPP_OPERATORS_FF
		TaskDriverInterfaceFactory_LDR(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskDriverInterfaceFactory_LDR();
		
		// Called on program start to set up the TaskDriverInterfaceFactory_LDR. 
		// TaskDriverInterfaceFactory_LDR registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Create a TaskDriverInterface_LDR: (Call after FinalInit())
		TaskDriverInterface *Create();
		
		// Get description string: 
		const char *DriverInterfaceDesc() const;
};

#endif  /* _RNDV_TDRIVER_LDR_PARS_HPP_ */
