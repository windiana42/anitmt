/*
 * local.hpp
 * 
 * Local task source. 
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

#ifndef _RNDV_TSOURCE_LOCAL_HPP_
#define _RNDV_TSOURCE_LOCAL_HPP_ 1

#include "../tasksource.hpp"


class TaskSourceFactory_Local;

class TaskSource_Local : 
	public TaskSource,
	public FDBase
{
	private:
		// Our parameters: 
		TaskSourceFactory_Local *p;
		
		// Response 0msec timer: 
		TimerID rtid;
		
		// What we're currently doing: 
		TSAction pending;
		
		// Are we connected? This is just a dummy to ensure 
		// correct behavior of the other classes...
		int connected;
		// Used to save CompleteTask passed to srcDoneTask(): 
		CompleteTask *done_task;
		
		// What we're currently doing: 
		int next_frame_no;
		
		// Do some actaul work: 
		void _ProcessGetTask(TSNotifyInfo *ni);
		void _ProcessDoneTask(TSNotifyInfo *ni);
		
		int _GetNextFiles(RefString *inf,RefString *outf,int *resume_flag);
		
		// Update/start rtid timer: 
		inline void _Start0msecTimer();
		inline void _Stop0msecTimer();
		
		// overriding virtuals from FDbase: 
		int timernotify(TimerInfo *);
		// overriding virtuals from TaskSource: 
		int srcConnect(TaskSourceConsumer *);
		int srcGetTask(TaskSourceConsumer *);
		int srcDoneTask(TaskSourceConsumer *,CompleteTask *ct);
		int srcDisconnect(TaskSourceConsumer *);
		
		long ConnectRetryMakesSense();
	public: _CPP_OPERATORS_FF
		TaskSource_Local(TaskSourceFactory_Local *,int *failflag=NULL);
		~TaskSource_Local();
};

#endif  /* _RNDV_TSOURCE_LOCAL_HPP_ */
