/*
 * tdriver.hpp
 * 
 * Generic task driver. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _RNDV_TDRIVER_LOCALDRIVER_HPP_
#define _RNDV_TDRIVER_LOCALDRIVER_HPP_ 1

enum TaskTerminationReason
{
	TTR_Unset=-1,  // not yet filled in
	TTR_Success=0,
	TTR_ExecFailed,  // error in StartProcess or until execve()
	TTR_RunFail,  // job exits with non-zero code
	TTR_ATerm,    // abnormal termination (killed / dumped)
	TTR_JobTerm   // rendview killed the job (see signal for more info)
};

enum
{
	// no symbol may be 0!
	//-- for TTR_JobTerm --
	JK_UserInterrupt=1,    // e.g. user pressed ^C. 
	JK_Timeout,            // well... what was that again??
	JK_ServerError,        // rendview does not want to go on for what 
	                       // reason ever
	//-- for TTR_ExecFailed--
	JK_FailedToOpenIn,   // failed to open requied input file
	JK_FailedToOpenOut,  // failed to open requied output file
	JK_FailedToExec,     // failed to exec job because access(X_OK) / execve failed
	JK_InternalError,    // any other reason
	JK_NotSupported      // failed because requested action is not suported 
	                     // (e.g. wrong image format) 
};

struct TaskExecutionStatus
{
	TaskTerminationReason status;
	// signal has lots of meanings: 
	// normally: 0
	// TTR_ATerm -> signal which caused termination
	// TTR_RunFail -> exit status (non-zero)
	// TTR_JobTerm, TTR_ExecFailed -> one of the JK_* values from above
	int signal;
	
	// This is for more statistics if supported 
	// (filled in _before_ IAmDone())
	HTime starttime;   // when process was started
	HTime endtime;     // when SIGCHLD was caught
	HTime utime;       // time spent in user mode
	HTime stime;       // time spend in system mode
	
	// Return string representation of status (reason, signal): 
	const char *StatusString();
	// Return string representation of JK_* - value: 
	static const char *JK_String(int jk_value);
	
	_CPP_OPERATORS_FF
	TaskExecutionStatus(int *failflag=NULL);
	~TaskExecutionStatus() {}
};


// Base type for RenderTaskParams, FilterTaskParams
struct TaskParams
{
	enum { NoNice=-32000 };
	
	struct DriverHook
	{
		_CPP_OPERATORS_FF
		DriverHook(int * /*failflag*/) { }
		virtual ~DriverHook() { }
	};
	
	// THIS MUST BE FILLED IN [By Render/FilterTaskParams]: 
	TaskDriverType dtype;
	
	//RefStrList add_args;   // additional cmd line args  [not needed: use task source or render/filter information]
	int niceval;        // nice value (process priority) (or NoNice)
	int call_setsid;    // call setsid() (recommended)
	long timeout;       // render timeout (local); -1 for none
	
	// Hook for lowest driver level to hang any data on. 
	// (FDs to be closed, temporary files to be removed...)
	// (Must be derived from TaskParams::DriverHook.) 
	DriverHook *hook;
	
	_CPP_OPERATORS_FF
	TaskParams(int *failflag=NULL);
	virtual ~TaskParams();
	
	private:  // don't copy: 
		TaskParams(const TaskParams &) {}
		void operator=(const TaskParams &) {}
};


// Base type for RenderTask, FilterTask
struct TaskStructBase
{
	// THIS MUST BE FILLED IN [By Render/FilterTask]: 
	TaskDriverType dtype;
	
	// NOTE!! infile and outfile are DELETED by ~TaskStructBase(). 
	TaskFile *infile;      // primary input file 
	TaskFile *outfile;     // primary output file
	RefStrList add_args;   // additional cmd line args
	RefString wdir;        // directory to chdir into before 
	                       // starting the renderer/filter
	long timeout;          // additional timeout (from task source)
	
	_CPP_OPERATORS_FF
	TaskStructBase(int *failflag=NULL);
	virtual ~TaskStructBase();
};


class ComponentDataBase;
class RF_DescBase;
class TaskDriverInterface_Local;

// NOTE: 
// All TaskDriver-derived classes (RenderDriver, POVRayDriver, ...)
// may not contain global settings (as set by cmd line, etc.). 
// These settings are put into a class derived from TaskDriverFactory 
// (e.g. POVRayDriverFactory). 
// One instance of TaskDriver exists for every task which is currently 
// running. 
// But only one instance of the factories exist and the factories have 
// to register at the ComponentDataBase. 

// THE LINKED TASK DRIVER LIST IS HELD BY ComponentDataBase 
// (NOT BY TaskManager). 
class TaskDriverFactory : 
	public LinkedListBase<TaskDriverFactory>, 
	public par::ParameterConsumer_Overloaded
{
	friend class TaskDriver;
	private:
		TaskDriverType dtype;
	protected:
		RefString driver_name;
		ComponentDataBase *component_db;
		
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		TaskDriverFactory(
			ComponentDataBase *cdb,
			const char *driver_name,
			TaskDriverType tdt,
			int *failflag=NULL);
		virtual ~TaskDriverFactory();
		
		// Check the Render/Filter Desc and set up those fields not 
		// set up by the desc file (i.e. render resume capability which 
		// depends on the driver). 
		// Return value: 0 -> OK
		//  any other value -> error & error was written
		virtual int CheckDesc(RF_DescBase *) HL_PureVirt(1)
		
		// Create a TaskDriver: 
		virtual TaskDriver *Create(TaskDriverInterface_Local *tdif) HL_PureVirt(NULL)
		
		// Called on statup to initialize the (render and filter) 
		// driver factories; Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		TaskDriverType DType() const  {  return(dtype);  }
		
		// Returns the driver name (NOT the renderer name; but they 
		// may be similar/equal.) 
		const char *DriverName() const  {  return(driver_name);  }
		
		// Returns driver description string. 
		virtual const char *DriverDesc() const HL_PureVirt(NULL)
};


// THE LINKED TASK DRIVER LIST IS HELD BY TaskDriverInterface_Local
// (which is the right hand of TaskManager).  
// (NOT BY ComponentDataBase). 
class TaskDriver : 
	public LinkedListBase<TaskDriver>,
	public FDBase,
	public ProcessBase
{
	friend class TaskDriverInterface_Local;
	public:
		// Everything the task driver and the manager (have to) know 
		// about a process: 
		struct PInfo
		{
			pid_t pid;
			RefStrList args;
			const TaskStructBase *tsb;
			const TaskParams *tp;
			
			// Where the TaskStructBase comes from: 
			CompleteTask *ctsk;
			
			// Okay, now success / failure info: 
			TaskExecutionStatus tes;
			
			_CPP_OPERATORS_FF
			PInfo(int *failflag=NULL);
			~PInfo();
		};
		
		// NOTE: Positive values are errors,
		//       negative ones verbose/warning messages. 
		enum ProcessErrorType
		{
			// verbose messages: 
			PEI_Starting=-32000,    // starting program
			PEI_StartSuccess,       // StartProcess() succeeded 
			PEI_ExecSuccess,        // Got PSExecuted|PSSuccess; process IS running!
			PEI_RunSuccess,         // Program terminated normally
			// warning/error messages (as you like to define it): 
			PEI_Timeout=1,          // process execution time limit exceeded
			// error messages: 
			PEI_StartFailed,        // StartProcess() returned error 
			PEI_ExecFailed,         // failed to exec program (see details)
			PEI_RunFailed           // program terminated abnormally (killed/
			                        // nonzero exit status/dumped)
		};
		struct ProcessErrorInfo
		{
			ProcessErrorType reason;  // what failed/should be printed
			// This contains about everything which is known about 
			// the process: 
			const TaskDriver::PInfo *pinfo;
			// As passed to procnotify() or NULL: 
			const ProcStatus *ps;
			// Only used for PEI_StartFailed [because then ps=NULL]: 
			int errno_val;  // value of errno; else 0
			// Is this the last call to ProcessError() (info for 
			// lowest driver level)? 
			unsigned int is_last_call : 1;
			unsigned int  : (sizeof(unsigned int)*8-1);  /* padding */
		};
		enum
		{
			// BE SURE THAT THESE DO NOT COLLIDE WITH THE SPS_* FROM 
			// ProcessManager:  
			SPSi_Success=0,
			SPSi_IllegalParams=-32010,   // = internal error (BUG)
			SPSi_OpenInFailed,
			SPSi_OpenOutFailed,
			SPSi_NotSupported     // e.g. requested image format not supported 
			                      //      by the used renderer 
		};
	private:
		enum ExecStatus
		{
			ESNone=0,
			ESRunning,
			ESDone
		};
		enum ExecStatusDetail
		{
			// ESNone: 
			EDNone=0,
			// ESRunning: 
			EDLaunched,  // launched but not known if actually running
			EDRunning,   // process running (got PSExecuted, PSSuccess)
			// ESDone: 
			EDSuccess,  // successful 
			EDTskErr,   // task returned non-zero exit code
			EDFailed,   // execution failed
			EDATerm,    // abnormally terminated (killed, dumped)
			EDTimeout   // time limit exceeded
		};
		
		// Inform mamager about state change: 
		inline void _StateChanged();
		
		void _FillInStatistics(const ProcStatus *ps);
		
		// This is part of StartProcess() dealing with error at 
		// ProcessBase::StartProcess() (called from TaskDriver::StartProcess()) 
		// and errors in XYZDriver::Execute() (called from 
		// TaskDriverInterface::LaunchTask()) 
		void _StartProcess_ErrorPart(int save_errno);
		
		// May also be used by TaskManager - actually TaskDriverInterface_Local: 
		// reason_detail: one of the JK_* - values
		int KillProcess(int reason_detail);
		
		// Notify driver level using virtual ProcessError(): 
		int _SendProcessError(ProcessErrorType pet,
			const ProcStatus *ps=NULL,int errno_val=0);
	protected:
		// Pointer to associated factory. 
		TaskDriverFactory *f;
		
		// We never talk to TaskManager directly but via TaskDriverInterface_Local. 
		TaskDriverInterface_Local *tdif;
		
		// Process info structure: 
		PInfo pinfo;
		
		// Execution status: 
		ExecStatus estat;
		ExecStatusDetail esdetail;
		
		// Timeout timer: 
		TimerID tid_timeout;
		
		inline ComponentDataBase *component_db()
			{  return(f->component_db);  }
		
		// Called by derived classes: 
		// Return value: 0 -> OK; failure -> that of ProcessBase::StartProcess(). 
		int StartProcess(
			const TaskStructBase *tsb,
			const TaskParams *tp,
			ProcessBase::ProcPath *sp_p,
			RefStrList            *sp_a,  // for ProcessBase::ProcArgs
			ProcessBase::ProcMisc *sp_m,
			ProcessBase::ProcFDs  *sp_f,
			ProcessBase::ProcEnv  *sp_e);
		
		// Overriding virtuals: 
		int timernotify(TimerInfo *ti);
		void procnotify(const ProcStatus *ps);
		
		// Can be used to get error code string returned by StartProcess. 
		// Returns error string (static data). 
		const char *StartProcessErrorString(ProcessErrorInfo *pei);
		// Print error when ProcessBase::PSFailed occurs. 
		const char *PSFailedErrorString(ProcessErrorInfo *pei);
		
		// Can be used by implementations overriding ProcessError() virtual: 
		int ProcessError_PrimaryReasonMessage(const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		void ProcessError_PrintCommand(int print_cmd,const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		// More special functions: (return print_cmd)
		int ProcessErrorStdMessage_Timeout(const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		int ProcessErrorStdMessage_ExecFailed(const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		int ProcessErrorStdMessage_RunFailed(const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		
		
		// Downcall:
		// gets called on error or to notify driver of things (for 
		// verbose messages). 
		// Return value: currently ignored; use 0. 
		virtual int ProcessError(ProcessErrorInfo *) HL_PureVirt(0)
		
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		TaskDriver(TaskDriverFactory *f,TaskDriverInterface_Local *tdif,
			int *failflag=NULL);
		virtual ~TaskDriver();
		
		// Get the associated factory (for DType(), DriverName(), ...)
		TaskDriverFactory *GetFactory()  {  return(f);  }
		
		// TASK MANAGER INTERFACE via TaskDriverInterface/TaskDriverInterfac_Local: 
		
		// ** Start (execute) the program: **
		// All the needed info is passed in 
		//   * TaskStructBase: cast it to RenderTask, FilterTask..
		//   * TaskParams: cast it to RenderTaskParams, FilterTaskParams
		// BEFORE casting, check the dtype member to be correct 
		//   (DTRender, DTFilter) (abort() in this case). 
		//   [DONE BY RenderDriver/FilterDriver.]
		// Return value: 
		//   SPSi_Open{In,Out}Failed  -> failed to open required file
		//   SPSi_IllegalParams -> invalid TaskStructBase / TaskParams
		//   SPSi_NotSupported  -> some action is not supported (e.g. wrong 
		//                         image format)
		//   SPSi_Success (0)   -> success
		//   <0 -> see ProcessBase::StartProcess() (SPS_*)
		//   SPS_LMallocFailed  -> allocation failure 
		// (This calls Render/FilterDriver::Execute(). 
		virtual int Run(
			const TaskStructBase * /*tsb*/,
			const TaskParams * /*tp*/
			) HL_PureVirt(1)
		
};

#endif  /* _RNDV_TDRIVER_LOCALDRIVER_HPP_ */
