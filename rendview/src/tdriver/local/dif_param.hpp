/*
 * dif_param.hpp
 * 
 * Header for parameter & factory class of local task driver interface. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_TDRIVER_LOCAL_PARS_HPP_
#define _RNDV_TDRIVER_LOCAL_PARS_HPP_ 1

class ComponentDataBase;


class TaskDriverInterfaceFactory_Local : 
	public TaskDriverInterfaceFactory
{
	friend class TaskDriverInterface_Local;
	private:
		// We may access component_db(). 
		
		// Number of simultanious jobs (limit and optimum): 
		int njobs;
		
		// Term-kill delay in msec: 
		long term_kill_delay;
		
		// As set by user: (ONLY USE FOR PARAM CODE!)
		int thresh_param_low, thresh_param_high;
		// todo_thresh_low, todo_thresh_high in TaskDriverInterface. 
		int thresh_param_donehigh;  // -> done_thresh_high
		
		struct DTPrm
		{
			// Limit for simultanious render and filter jobs: 
			int maxjobs;
			// Nice value or NoNiceValSpec
			int niceval;
			// Call setsid() (recommended): 
			bool call_setsid;
			// Change nice value by +-1 
			bool nice_jitter;
			// Execution timeout or -1: 
			long timeout;
			// mute: stdout -> /dev/null; quiet: stdout & stderr -> /dev/null
			bool mute,quiet;
		} prm[_DTLast];
		
		
		int _SetUpParams(TaskDriverType dtype,Section *top);
		int _RegisterParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// Overriding virtuial from TaskSourceFactory: 
		int FinalInit();  // called after CheckParams(), before Create()
	public: _CPP_OPERATORS_FF
		TaskDriverInterfaceFactory_Local(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskDriverInterfaceFactory_Local();
		
		// Called on program start to set up the TaskDriverInterfaceFactory_Local. 
		// TaskDriverInterfaceFactory_Local registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Create a TaskDriverInterface_Local: (Call after FinalInit())
		TaskDriverInterface *Create();
		
		// Get description string: 
		const char *DriverInterfaceDesc() const;
};

#endif  /* _RNDV_TDRIVER_LOCAL_PARS_HPP_ */
