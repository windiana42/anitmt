/*
 * tasksource.hpp
 * 
 * Task source virtualisation. 
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

#ifndef _RNDV_TASKSOURCE_HPP_
#define _RNDV_TASKSOURCE_HPP_ 1

#include "../database.hpp"
#include "taskfile.hpp"

//#include <taskmanager.hpp>
//#include "tsfactory.hpp"


// CompleteTask is the class holding all information about a task. 
// CompleteTask is allocated by GetTask() and destroyed by DoneTask(). 
// Between GetTask() and DoneTask(), the list is held by TaskManager. 
// All the other time, a TaskSource MAY have it in a list. 
struct CompleteTask : LinkedListBase<CompleteTask>
{
	// This is set up to TaskDone by the constructor and is returned 
	// with this state by GetTask. TasmManager sets the apropriate state 
	// when getting the task. 
	// This is what to be done next with the tast. (A task which has to 
	// be rendered and filtered is initially ToBeRendered, then 
	// ToBeFiltered.) 
	enum State
	{
		TaskDone=0,
		ToBeRendered,
		ToBeFiltered
	} state;
	
	// Returns string representation of State: 
	static const char *StateString(State s);
	
	// If this CompleteTask is currently processed, d contains a pointer to 
	//    the TaskDriver/LDRClient which is currently processing the task. 
	// If the CompleteTask currently is NOT processed, all of them 
	//    are NULL and any() returns false. 
	struct
	{
		TaskDriver *td;
		LDRClient *ldrc;
		// What the client / task driver shall do with the task: 
		int shall_render : 1;
		int shall_filter : 1;
		int _padding : (sizeof(int)*8-2);
		
		inline bool any()  const
			{  return(td || ldrc);  }
	} d;
	
	// Frame number (only used for user output). 
	int frame_no;
	
	// Unique task ID given to the task by the (local) task source. 
	u_int32_t task_id;  // initially 0 
	
	// Allocated for CompleteTask and freed by destructor ~CompleteTask(). 
	RenderTask *rt;   // or NULL 
	FilterTask *ft;   // or NULL
	
	// Allocated by TaskDriverInterface and freed by destructor ~CompleteTask():
	// Set up to be NULL by task source. 
	RenderTaskParams *rtp;   // or NULL
	FilterTaskParams *ftp;   // or NULL
	
	// Additional files needed to render/filter the frame. 
	// Arrays allocated via NEWarray<>() and DELarray()'d by ~CompleteTask(). 
	struct AddFiles
	{
		int nfiles;
		TaskFile *tfile;  // array [nfiles]
	} radd,fadd;
	
	// Render and filter task execution status: 
	struct TES
	{
		TaskExecutionStatus tes;
		RefString processed_by;
		
		_CPP_OPERATORS_FF
		TES(int *failflag=NULL);
		~TES() {}
	};
	TES rtes;
	TES ftes;
	
	_CPP_OPERATORS_FF
	CompleteTask(int *failflag=NULL);
	~CompleteTask();
	
	// Returns true if this task was rendered and this 
	// was completely successfully. 
	bool WasSuccessfullyRendered() const;
	// NOTE: Partly rendered means that rendering was 
	//       interrupted AND WE CAN GO ON (continue). 
	// THUS, partly filtered is always false. 
	bool IsPartlyRenderedTask() const;  // only use if not currently procesed
	bool IsPartlyFilteredTask() const;  // only use if not currently procesed
	// Returns 1 if the passed task is being processed or was processed 
	// by us: 
	inline bool ProcessedTask() const
	{
		return(d.any() || 
		   (state==CompleteTask::TaskDone) ||
		   (state==CompleteTask::ToBeFiltered && rt) );
	}
	// Some more... 
	//  -1 -> no render/filter task / ttr is unset
	//   0 -> success
	//   1 -> partly done
	//   2 -> failure
	int DidRenderTaskFail() const;
	int DidFilterTaskFail() const;
	
	// Returns string "completely", "partly" or "not" depending on 
	// how the task was processed. If non-NULL, level returns the same 
	// info (2 -> completely, 1 -> partly, 0 -> not processed). 
	char *Completely_Partly_Not_Processed(int *level=NULL) const;
	
	// This can be used to dump the complete info of a task using the 
	// specified verbose level. 
	// when: 0 -> when task was obtained from task source 
	//              (timeout, nice value etc. not set)
	//       1 -> when reporting task back to task source ("task done")
	void DumpTask(int vlevel,int when) const;
	// These functions are internally used by DumpTask(): 
	void PrintTaskExecuted(int vlevel,const TaskParams *tp,
		const TaskStructBase *tsb,const char *binpath,
		bool was_processed) const;
	void PrintTaskToBeExecuted(int vl,const TaskStructBase *tsb,
		const char *binpath) const;
	void PrintTaskExecStatus(int vlevel,const TES *ct_tes,
		TaskDriverType dtype) const;
	
	// This is just a debugging helper: 
	// Number of currently existing CompleteTask objects: 
	static int n_complete_tasks;
};


class TaskSourceConsumer;

struct TaskSource_NAMESPACE
{
	enum TSAction
	{
		ANone=0,        // should never happen
		AConnect,
		AGetTask,
		ADoneTask,
		ADisconnect,
		AActive   // <-- TST_Active task source - specific, see TSTActiveStat
	};
	enum ConnectStat
	{
		CSNone=0,       // ...not talking about connect...
		CSWorking,      // Issued connect(), waiting for it to happen
		CSConnected,    // Okay, we're connected. 
		CSTimeout,      // timeout while waiting for connection to establish
		CSFailed        // failed to connect (see cmd & cmd_errno)
	};
	enum GetTaskStat
	{
		GTSNone=0,
		GTSWorking,
		GTSGotTask,
		GTSAllocFailed,
		GTSNoMoreTasks,
		GTSEnoughTasks,   // task source thinks that we have enough tasks
		GTSFileCollision   // special error (e.g. if "scene.pov" is an additional 
	                       // file and a scene input file)
	};
	enum DoneTaskStat
	{
		DTSNone=0,
		DTSWorking,
		DTSOkay
	};
	enum DisconnectStat
	{
		DSNone=0,
		DSWorking,
		DSQuitting,
		DSOkay,
		DSQuitOkay
	};
	enum TSTActiveStat
	{
		TASNone=0,
		TASTakeTask,   // active task source got task (passed in ctsk)
		TASRecoveringBad,   // lost connection to server; must recover
		TASRecoveringQuit,  // closed connection to server due to quit
		TASSimpleCommand,   // execute simple command NOW
		TASGiveBackTasks    // see int_payload for may_keep value
	};
	
	enum ErrCommand
	{
		EC_none,
		EC_connect,
		EC_read,
		EC_write
		//EC_shutdown,
		//EC_
	};
	
	// NOTE: DO NOT MIX UP WITH LDR::LDRClientControlCommand. 
	enum ClientControlCommand
	{
		CCC_None=0,
		CCC_Kill_UserInterrupt,
		CCC_Kill_ServerError,
		CCC_StopJobs,
		CCC_ContJobs
	};
	static const char *ClientControlCommandString(ClientControlCommand cccmd);
	
	enum TaskSourceType
	{
		TST_Passive,  // like local source
		TST_Active    // like LDR source (tells manager when to start processing) 
	};
	
	struct TSNotifyInfo
	{
		TSAction action;          // why this was called 
		
		// Depending on action, check one of these: 
		ConnectStat connstat;
		GetTaskStat getstat;
		DoneTaskStat donestat;
		DisconnectStat disconnstat;
		TSTActiveStat activestat;
		
		// Further data fields: 
		CompleteTask *ctsk;    // only for TSGetTask and TASTakeTask
		
		// What failed: 
		ErrCommand cmd;
		int cmd_errno;
		
		// Simple command to execute for TASSimpleCommand:
		ClientControlCommand cccmd;  // <-- !NOT! LDRClientControlCommand
		
		// Integer payload (to pass may_keep for TASGiveBackTasks): 
		int int_payload;    // set to 0 by constructor
		
		_CPP_OPERATORS_FF
		// Currently this cannot fail and NEW<> is not used: 
		TSNotifyInfo(/*int *failflag=NULL*/);
		~TSNotifyInfo()  {  ctsk=NULL;  }
	};
};

// This is the class TaskSource. The TaskSource is used to retrieve tasks 
// (over the network or from the local hd). In a rendview process, only 
// one TaskSource exists. 
// The class(es) which use the TaskSource must be derived from 
// class TaskSourceConsumer (further down). 
class TaskSource : public TaskSource_NAMESPACE
{
	protected:
		// Important: this defaults to TST_Passive, so active task sources 
		// must set that to a proper value: 
		TaskSourceType tstype;
		
		// Pointer to component data base: 
		ComponentDataBase *component_db;
		
		// Current client or NULL if no request is pending. 
		TaskSourceConsumer *cclient;
		
		// Used by derived classes as they are not friends of 
		// TaskSourceConsumer: 
		// Return value: that of tsnotify(). 
		int call_tsnotify(TaskSourceConsumer *cons,TSNotifyInfo *ni);
		
		// These are the functions which must be overridden by the 
		// TaskSource derived classes: 
		virtual int srcConnect(TaskSourceConsumer *) HL_PureVirt(-1)
		virtual int srcGetTask(TaskSourceConsumer *) HL_PureVirt(-1)
		virtual int srcDoneTask(TaskSourceConsumer *,CompleteTask *) HL_PureVirt(-1)
		virtual int srcDisconnect(TaskSourceConsumer *,int) HL_PureVirt(-1)
		
	public:  _CPP_OPERATORS_FF
		TaskSource(ComponentDataBase *cdb,int *failflag=NULL);
		virtual ~TaskSource();
		
		// Task source interface: 
		// See TaskSourceConsumer for more info. 
		int Connect(TaskSourceConsumer *);
		int GetTask(TaskSourceConsumer *);
		int DoneTask(TaskSourceConsumer *,CompleteTask *ct);
		int Disconnect(TaskSourceConsumer *,int special);
		
		// Does it make sense to re-try if the Connect() call fails? 
		// (Returns delay in msec or 0 if you should not re-try.)
		virtual long ConnectRetryMakesSense() HL_PureVirt(-1);
		
		// Get TaskSourceType: 
		TaskSourceType GetTaskSourceType()
			{  return(tstype);  }
		
		// For active task sources, the passed TaskSourceConsumer will 
		// be persistent and no other TaskSourceConsumer can use it. 
		// Does nothing for pssive task sources. 
		virtual void SetPersistentConsumer(TaskSourceConsumer * /*persistent*/) {}
};


// Derive the class which uses the TaskSource from this class. 
class TaskSourceConsumer : public TaskSource_NAMESPACE
{
	friend class TaskSource;
	public:
		struct TSDebugInfo
		{
			// Number of tasks in todo and done queue: 
			int todo_queue;
			int proc_queue;
			int done_queue;
		};
	private:
		// Pointer to the (associated) TaskSource: 
		TaskSource *tsource;
		
		// Used by the write functions below: 
		char *_TSTaskQueueStatStr(int special=0);  // Returns static data. 
		// Used by TSWriteError(): 
		void _TSWriteError_Connect(const TSNotifyInfo *ni);
		void _TSWriteError_GetTask(const TSNotifyInfo *ni);
		void _TSWriteError_DoneTask(const TSNotifyInfo *ni);
		void _TSWriteError_Disconnect(const TSNotifyInfo *ni);
		void _TSWriteError_Active(const TSNotifyInfo *ni);
		void _WriteErrCmd(const TSNotifyInfo *ni);
	protected: 
		// This is the function called by the TaskSource notifying you 
		// of the status of some TS* call. 
		// Return value: currently unused, use 0. 
		virtual int tsnotify(TSNotifyInfo *) HL_PureVirt(0)
		
		// Use this to store some info; used by verbose output. 
		// Must return 0. 
		virtual int tsGetDebugInfo(TSDebugInfo * /*dest*/) HL_PureVirt(-1)
		
		// Write error according to TSNotifyInfo. 
		// If TSNotifyInfo contains success code, only verbose output 
		// is done. 
		void TSWriteError(const TSNotifyInfo *);
	public:  _CPP_OPERATORS_FF
		TaskSourceConsumer(int * /*failflag*/=NULL)
			{  tsource=NULL;  }
		virtual ~TaskSourceConsumer()
			{  tsource=NULL;  }
		
		// Specify the TaskSource to use: 
		void UseTaskSource(TaskSource *ts);
		
		// Get TaskSource pointer... (you should notmally not need 
		// that but you may i.e. to see if the TaskSource is set). 
		TaskSource *tasksource()
			{  return(tsource);  }
		
		//#warning need function to be able to interrupt task source action. 
		// NOTE: THIS IS NEEDED IF NRP EVER GETS IMPLEMENTED. CURRENTLY NO NEED. 
		// int Cancel() / Stop() / ...?
		
		// All these functions use the virtual function tsnotify() to 
		// reoport information. (See class TaskConsumer.)
		// The integer return value is just used to return immediate 
		// failures (allocation, ...)
		// It is thought that if you need one or more tasks, you 
		// connect, tell the source about done task(s), 
		// get the next task(s) and disconnect again while working. 
		// NOTE: All these functions are guaranteed to not call tsnotify() 
		//       while they are executed. For example the local source 
		//       uses a 0msec timer to call the tsnotify() for TSConnect(). 
		
		// Common return values: 
		//  0 -> okay, wait for tsnotify(). 
		//  1 -> working (another request is pending, wait for this 
		//       tsnotify() first). 
		//  2 -> not connected. 
		//  3 -> quitting (becuase of TSDisconnect(/*special=*/1)) 
		
		// Conect to the task source. Do this before calling TSGetTask() 
		// or TSDoneDask(). 
		// Different return value: 
		//  2 -> already connected (instead of `not connected´) 
		int TSConnect()
			{  return(tsource->Connect(this));  }
		
		// Get the next task. You may only call this if TSConnect() was 
		// successful (i.e. tsnotify(CSConnected)). 
		int TSGetTask()
			{  return(tsource->GetTask(this));  }
		
		// A task was done. You may only call this if TSConnect() was 
		// successful (i.e. tsnotify(CSConnected)). 
		// Additional return values: 
		// 10 -> invalid task
		int TSDoneTask(CompleteTask *ct)
			{  return(tsource->DoneTask(this,ct));  }
		
		// Disconnect. 
		// special: 1 -> task source should quit (active & passive)
		//          2 -> task source shall not report more tasks 
		//    (may one more: the one just downloading; active only)
		// If do_quit is set, then the task source should quit, too. 
		// Additional return values: 
		// 10 -> invalid "special" value
		// 11 -> special=2 and already quitting (special=1)
		int TSDisconnect(int special=0)
			{  return(tsource->Disconnect(this,special));  }
		
		// Does it make sense to re-try if the Connect() call fails? 
		// (Returns delay in msec or 0 if you should not re-try.) 
		long TSConnectRetryMakesSense()
			{  return(tsource->ConnectRetryMakesSense());  }
		
		// Query TaskSourceType: 
		TaskSourceType GetTaskSourceType()
			{  return(tsource->GetTaskSourceType());  }
};

#endif  /* _RNDV_TASKSOURCE_HPP_ */
