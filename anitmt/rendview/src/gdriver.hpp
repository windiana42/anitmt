/*
 * gdriver.hpp
 * 
 * Generic (render/filter) task driver. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _RNDV_GDRIVER_HPP_
#define _RNDV_GDRIVER_HPP_ 1

enum TaskDriverType
{
	DTNone=-1,
	DTRender,
	DTFilter,
	_DTLast
};

enum TaskTerminationReason
{
	TTR_Unset=-1,  // not yet filled in
	TTR_Success=0,
	TTR_Timeout,
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
	JK_ServerError,        // rendview does not want to go on for what 
	                       // reason ever
	//-- for TTR_ExecFailed--
	JK_FailedToExec,    // failed to exec job because access(X_OK) / execve failed
	JK_InternalError    // any other reason
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


// Returns string representation of TaskDriverType: 
extern char *DTypeString(TaskDriverType dt);


// Base type for RenderTaskParams, FilterTaskParams
struct TaskParams
{
	enum { NoNice=-32000 };
	
	// THIS MUST BE FILLED IN [By Render/FilterTaskParams]: 
	TaskDriverType dtype;
	
	RefStrList add_args;   // additional cmd line args
	int niceval;        // nice value (process priority) (or NoNice)
	int call_setsid;    // call setsid() (recommended)
	long timeout;       // render timeout; -1 for none
	RefString crdir;    // chroot() to this dir before chdir to wdir. 
	RefString wdir;     // directory to chdir into before starting the renderer
	
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
	TaskFile *infile;       // primary input file 
	TaskFile *outfile;      // primary output file
	RefStrList add_args;   // additional cmd line args
	//long timeut;           // additional timeout (from server)
	
	_CPP_OPERATORS_FF
	TaskStructBase(int *failflag=NULL);
	virtual ~TaskStructBase();
};


class ComponentDataBase;

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
		
		// Create a TaskDriver: 
		virtual TaskDriver *Create() HL_PureVirt(NULL)
		
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


// THE LINKED TASK DRIVER LIST IS HELD BY TaskManager 
// (NOT BY ComponentDataBase). 
class TaskDriver : 
	public LinkedListBase<TaskDriver>,
	public FDBase,
	public ProcessBase
{
	friend class TaskManager;
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
		
		// May also be used by TaskManager: 
		// reason_detail: one of the JK_* - values
		int KillProcess(TaskTerminationReason reason,int reason_detail);
		
		// Used by PSFailedErrorString(): 
		const char *_ExecFailedError_SyscallName(PSDetail x);
		
		// Notify driver level using virtual ProcessError(): 
		int _SendProcessError(ProcessErrorType pet,
			const ProcStatus *ps=NULL,int errno_val=0);
	protected:
		// Pointer to associated factory. 
		TaskDriverFactory *f;
		
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
		// Returns error string (static data); if errno-string (or special 
		// code string) should also be written, the corresponding strerror() 
		// value is in error_str (otherwise NULL). 
		const char *StartProcessErrorString(ProcessErrorInfo *pei,
			const char **error_str);
		// Print error when ProcessBase::PSFailed occurs. 
		const char *PSFailedErrorString(ProcessErrorInfo *pei);
		
		// Downcall:
		// gets called on error or to notify driver of things (for 
		// verbose messages). 
		// Return value: currently ignored; use 0. 
		virtual int ProcessError(ProcessErrorInfo *) HL_PureVirt(0)
		
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		TaskDriver(TaskDriverFactory *f,int *failflag=NULL);
		virtual ~TaskDriver();
		
		// Get the associated factory (for DType(), DriverName(), ...)
		TaskDriverFactory *GetFactory()  {  return(f);  }
		
		// TASK SOURCE / TASK MANAGER INTERFACE: 
		
		// ** Start (execute) the filter proram: **
		// All the needed info is passed in 
		//   * TaskStructBase: cast it to RenderTask, FilterTask..
		//   * TaskParams: cast it to RenderTaskParams, FilterTaskParams
		// BEFORE casting, check the dtype member to be correct 
		//   (DTRender, DTFilter) (abort() in this case). 
		//   [DONE BY RenderDriver/FilterDriver.]
		// Return value: 
		//   1 -> invalid TaskStructBase / TaskParams
		//   0 -> success
		//  <0 -> see ProcessBase::StartProcess()
		//  -1 -> allocation failure 
		// (This calls Render/FilterDriver::Execute(). 
		virtual int Run(
			const TaskStructBase * /*tsb*/,
			const TaskParams * /*tp*/
			) HL_PureVirt(1)
		
};

#endif  /* _RNDV_GDRIVER_HPP_ */