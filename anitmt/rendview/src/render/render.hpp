/*
 * render.hpp
 * 
 * Renderer desc and renderer driver interface. 
 * Virtualisation of renderer interface. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_RENDER_RENDER_HPP_
#define _RNDV_RENDER_RENDER_HPP_ 1

// Linked list hold by RenderDataBase. 
struct RenderDesc : RF_DescBase
{
	RefString binpath;  // path to binary 
	RefStrList required_args;   // required args to pass to the renderer 
	RefStrList include_path;    // [implicit] include path for the renderer 
	
	_CPP_OPERATORS_FF
	RenderDesc(int *failflag=NULL);
	~RenderDesc();
};


struct RenderTask : public TaskStructBase
{
	const RenderDesc *rdesc;  // renderer description (for binpath, required args...)
	int width;         // width of image to render
	int height;        // height of image to render
	const ImageFormat *oformat;  // image output format
	
	_CPP_OPERATORS_FF
	RenderTask(int *failflag=NULL);
	~RenderTask();
	
	private:  // don't copy: 
		RenderTask(const RenderTask &) : TaskStructBase() {}
		void operator=(const RenderTask &) {}
};

struct RenderTaskParams : public TaskParams
{
	int stdout_fd;      // fd to direct renderer's stdout to (or -1)
	int stderr_fd;      // fd to direct renderer's stderr to (or -1)
	int stdin_fd;       // redirect this fd to renderer's stdin (-1)
	
	_CPP_OPERATORS_FF
	RenderTaskParams(int *failflag=NULL);
	~RenderTaskParams();
	
	private:  // don't copy: 
		RenderTaskParams(const RenderTaskParams &) : TaskParams() {}
		void operator=(const RenderTaskParams &) {}
};


// Linked list hold by RenderDataBase. 
class RenderDriver : public TaskDriver
{
	private:
		// Actually execute a render: Called by TaskDriver. 
		// Calls virtual Execute() for the drivers. 
		// [Overriding virtual from TaskDriver.]
		int Run(
			const TaskStructBase *tsb,
			const TaskParams *tp);
	protected:
		// Called by derived class: 
		int StartProcess(
			const RenderTask *rt,
			const RenderTaskParams *rtp,
			ProcessBase::ProcPath *sp_p,
			RefStrList            *sp_a,  // ProcessBase::ProcArgs 
			ProcessBase::ProcMisc *sp_m,
			ProcessBase::ProcFDs  *sp_f,
			ProcessBase::ProcEnv  *sp_e)
		{  return(TaskDriver::StartProcess(rt,rtp,sp_p,sp_a,sp_m,sp_f,sp_e));  }
	public:  _CPP_OPERATORS_FF
		// Driver name copied into RefString. 
		RenderDriver(TaskDriverFactory *f,int *failflag=NULL);
		virtual ~RenderDriver();
		
		// Called on statup to initialize the render drivers factories; 
		// Return value: 0 -> OK; >0 -> error. 
		static int init_factories(ComponentDataBase *cdb);
		
		// Actually execute a renderer. 
		// Return value: See TaskDriver::Run(). 
		// (This function gets called by RenderDriver::Run() if 
		// TaskDriver::Run() is called; NEVER override Run().) 
		virtual int Execute(
			const RenderTask *ft,
			const RenderTaskParams *ftp
			) HL_PureVirt(1)
		
		
};

#endif  /* _RNDV_RENDER_RENDER_HPP_ */
