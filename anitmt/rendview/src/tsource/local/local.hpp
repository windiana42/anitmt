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
#include "param.hpp"


class TaskSource_Local : 
	public TaskSource,
	public FDBase
{
	public:
		typedef TaskSourceFactory_Local::PerFrameTaskInfo PerFrameTaskInfo;
		struct FrameToProcessInfo
		{
			const PerFrameTaskInfo *fi;
			
			int frame_no;
			int tobe_rendered : 1;
			int tobe_filtered : 1;
			int resume_flag : 1;  // shall resume be passed to renderer?
			int : 29;
			
			// Render and filte input/output: 
			RefString r_infile;
			RefString r_outfile;
			RefString f_infile;
			RefString f_outfile;
			
			_CPP_OPERATORS_FF
			FrameToProcessInfo(int *failflag=NULL);
			~FrameToProcessInfo()  {  fi=NULL;  }
		};
	private:
		// Our parameters: 
		TaskSourceFactory_Local *p;
		
		// Response (normally 0 msec) timer: 
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
		// This is needed for detection of nonexistant frames: 
		int nonexist_in_seq;
		
		// Do some actaul work: 
		void _ProcessGetTask(TSNotifyInfo *ni);
		void _ProcessDoneTask(TSNotifyInfo *ni);
		
		int _FillInRenderJobFiles(FrameToProcessInfo *ftpi);
		int _FillInFilterJobFiles(FrameToProcessInfo *ftpi);
		int _GetNextFTPI_FillInFiles(FrameToProcessInfo *ftpi);
		int _GetNextFrameToProcess(FrameToProcessInfo *ftpi);
		
		// Update/start rtid timer: 
		inline void _StartRTimer();
		inline void _StopRTimer()
			{  UpdateTimer(rtid,-1,0);  }
		
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
