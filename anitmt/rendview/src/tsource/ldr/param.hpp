/*
 * param.hpp
 * 
 * Header for parameter & factory class of LDR task source. 
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

#ifndef _RNDV_TSOURCE_LDRPARS_HPP_
#define _RNDV_TSOURCE_LDRPARS_HPP_ 1

#include <lib/myaddrinfo.hpp>

class ComponentDataBase;
class ImageFormat;
class RenderDesc;
class FilterDesc;


class TaskSourceFactory_LDR : 
	public TaskSourceFactory
{
	friend class TaskSource_LDR;
	friend class TaskSource_LDR_ServerConn;
	private:
		// We may access ComponentDataBase::component_db. 
		
		// Port we will bind the listening socket: 
		int listen_port;
		
		// Password required by server to connect. 
		RefString password;
		
		// Allowed server address/netmask, comma or space-separated list. 
		RefString server_net_str;
		struct ServerNet : LinkedListBase<ServerNet>
		{
			RefString name;  // As specified (symbolic or numerical) 
			MyAddrInfo adr;
			u_int32_t mask;
			
			_CPP_OPERATORS_FF
			ServerNet(int *failflag=NULL) : LinkedListBase<ServerNet>(), 
				name(failflag), adr(failflag)
				{  mask=0xffffffffU;  }
			~ServerNet() {}
		};
		LinkedList<ServerNet> server_net_list;
		
		// Return string representation of server net: 
		RefString ServerNetString(TaskSourceFactory_LDR::ServerNet *sn);
		
		// Param spec string for the flags below. 
		RefString transfer_spec_str;
		struct
		{
			int render_src : 1;
			int render_dest : 1;
			int filter_dest : 1;
			int additional : 1;
		} transfer;
		
		// Listening socket (managed by TaskSource_LDR, of course)
		int listen_fd;
		
		int _RegisterParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// Overriding virtuial from TaskSourceFactory: 
		int FinalInit();  // called after CheckParams(), before Create()
	public: _CPP_OPERATORS_FF
		TaskSourceFactory_LDR(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskSourceFactory_LDR();
		
		// Called on program start to set up the TaskSourceFactory_LDR. 
		// TaskSourceFactory_LDR registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Create a TaskSource_LDR: (Call after FinalInit())
		TaskSource *Create();
		
		// Get description string: 
		const char *TaskSourceDesc() const;
};

#endif  /* _RNDV_TSOURCE_LDRPARS_HPP_ */
