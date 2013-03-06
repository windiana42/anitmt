/*
 * local.hpp
 * 
 * Local task source. 
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

#ifndef _RNDV_TSOURCE_LOCAL_HPP_
#define _RNDV_TSOURCE_LOCAL_HPP_ 1

#include "../tasksource.hpp"
#include "param.hpp"
#include <lib/statcache.hpp>


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
			int r_resume_flag : 1;  // shall resume be passed to renderer?
			int : (sizeof(int)*8 - 3);   // <-- Use modulo if more than 16 bits. 
			
			// Render and filte input/output: 
			// (As passed to the renderer/filter, relative to rdir/wdir.) 
			RefString r_infile_job;
			RefString r_outfile_job;
			RefString f_infile_job;
			RefString f_outfile_job;
			// These files as paths for RendView (relative to cwd). 
			RefString r_infile_rv;
			RefString r_outfile_rv;
			RefString f_infile_rv;
			RefString f_outfile_rv;
			
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
		int we_are_quitting;
		
		// Are we connected? This is just a dummy to ensure 
		// correct behavior of the other classes...
		int connected;
		// Used to save CompleteTask passed to srcDoneTask(): 
		CompleteTask *done_task;
		
		// What we're currently doing: 
		int next_frame_no;
		// This is needed for detection of nonexistant frames: 
		int nonexist_in_seq;
		
		// Used to speed up modification time tests: 
		FileStateCache statcache_add;
		
		// Do some actaul work: 
		void _ProcessGetTask(TSNotifyInfo *ni);
		void _ProcessDoneTask(TSNotifyInfo *ni);
		
		int _FillInJobFiles(TaskDriverType dtype,FrameToProcessInfo *ftpi);
		int _GetNextFTPI_FillInFiles(FrameToProcessInfo *ftpi);
		int _GetNextFrameToProcess(FrameToProcessInfo *ftpi);
		
		// Return value: 0 -> OK; -1 -> alloc failure; -2 -> GetTaskFile() failed. 
		int _SetUpAddTaskFiles(CompleteTask::AddFiles *af,
			const RefStrList *flist,TaskFile::IOType iotype,int frame_no);
		
		// Check additional files and see if the frame has to be re-processed. 
		int _CheckAddFilesMTime(HTime *output_mtime,TaskDriverType dtype,
			const PerFrameTaskInfo *fi,int frame_no);
		
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
		int srcDisconnect(TaskSourceConsumer *,int do_quit);
		
		long ConnectRetryMakesSense();
	public: _CPP_OPERATORS_FF
		TaskSource_Local(TaskSourceFactory_Local *,int *failflag=NULL);
		~TaskSource_Local();
};

#endif  /* _RNDV_TSOURCE_LOCAL_HPP_ */
