/*
 * filter.hpp
 * 
 * Filter desc and filter driver interface. 
 * Virtualisation of filter interface. 
 * 
 * Copyright (c) 2001--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_FILTER_FILTER_HPP_
#define _RNDV_FILTER_FILTER_HPP_ 1

struct FilterDesc : RF_DescBase
{
	RefString binpath;  // path to binary 
	RefStrList required_args;   // required args to pass to the filter
	
	_CPP_OPERATORS_FF
	FilterDesc(int *failflag=NULL);
	~FilterDesc();
};

struct FilterTask : public TaskStructBase
{
	const FilterDesc *fdesc;  // filter description (for binpath, required args...)
	const ImageFormat *ofotmat;  // image output format
	
	_CPP_OPERATORS_FF
	FilterTask(int *failflag=NULL);
	~FilterTask();
	
	private:  // don't copy: 
		FilterTask(const FilterTask &) : TaskStructBase() {}
		void operator=(const FilterTask &) {}
};


struct FilterTaskParams : public TaskParams
{
	int stderr_fd;      // fd to direct renderer's stderr to (or -1)
	
	_CPP_OPERATORS_FF
	FilterTaskParams(int *failflag=NULL);
	~FilterTaskParams();
	
	private:  // don't copy: 
		FilterTaskParams(const FilterTaskParams &) : TaskParams() {}
		void operator=(const FilterTaskParams &) {}
};


// Linked list hold by DataBase. 
class FilterDriver : public TaskDriver
{
	private:
		// Actually execute a filter: Called by TaskDriver. 
		// Calls virtual Execute() for the drivers. 
		// [Overriding virtual from TaskDriver.]
		int Run(
			const TaskStructBase *tsb,
			const TaskParams *tp);
	protected:
		// Called by derived class: 
		int StartProcess(
			const FilterTask *ft,
			const FilterTaskParams *ftp,
			ProcessBase::ProcPath *sp_p,
			RefStrList            *sp_a,  // ProcessBase::ProcArgs
			ProcessBase::ProcMisc *sp_m,
			ProcessBase::ProcFDs  *sp_f,
			ProcessBase::ProcEnv  *sp_e)
		{  return(TaskDriver::StartProcess(ft,ftp,sp_p,sp_a,sp_m,sp_f,sp_e));  }
		
		// Useful for ProcessError() in derived classes. 
		// See driver.cpp for more info on these: 
		// Return value: print_cmd. 
		int ProcessError_PrimaryReasonMessage(const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		void ProcessError_PrintCommand(int print_cmd,const char *prefix,
			const char *prg_name,ProcessErrorInfo *pei);
		void ProcessErrorStdMessage(const char *prefix,const char *prg_name,
			ProcessErrorInfo *pei);
		
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		FilterDriver(TaskDriverFactory *f,TaskDriverInterface_Local *tdif,int *failflag=NULL);
		virtual ~FilterDriver();
		
		// Called on statup to initialize the filter drivers factories; 
		// Return value: 0 -> OK; >0 -> error. 
		static int init_factories(ComponentDataBase *cdb);
		
		// Actually execute a filter. 
		// Return value: See TaskDriver::Run(). 
		// (This function gets called by FilterDriver::Run() if 
		// TaskDriver::Run() is called; NEVER override Run().) 
		virtual int Execute(
			const FilterTask * /*ft*/,
			const FilterTaskParams * /*ftp*/
			) HL_PureVirt(1)
};

#endif  /* _RNDV_FILTER_FILTER_HPP_ */
