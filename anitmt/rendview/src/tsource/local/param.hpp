/*
 * param.hpp
 * 
 * Header for parameter & factory class of local task source. 
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

#ifndef _RNDV_TSOURCE_LOCALPARS_HPP_
#define _RNDV_TSOURCE_LOCALPARS_HPP_ 1

class ComponentDataBase;
class ImageFormat;
class RenderDesc;
class FilterDesc;


class TaskSourceFactory_Local : 
	public TaskSourceFactory
{
	friend class TaskSource_Local;
	private:
		// We may access ComponentDataBase::component_db. 
		
		RefString inp_frame_pattern;
		RefString outp_frame_pattern;
		
		int startframe;
		// NOTE: fjump may be negative. 
		int fjump;
		// nframes=-1 -> as many as we can get
		int nframes;
		
		int width,height;
		RefString size_string;
		
		// Output format to use: 
		const ImageFormat *oformat;
		RefString oformat_string;  // NULL after CheckParams(). 
		
		RefString rdesc_string;
		const RenderDesc *rdesc;
		
		// Additional args for the renderer: 
		RefStrList radd_args;
		
		// Passed -continue?
		bool cont_flag;
		long response_delay;
		
		int _CheckFramePattern(const char *name,RefString *s);
		
		int _RegisterParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// Overriding virtuial from TaskSourceFactory: 
		int FinalInit();  // called after CheckParams(), before Create()
	public: _CPP_OPERATORS_FF
		TaskSourceFactory_Local(ComponentDataBase *cdb,int *failflag=NULL);
		~TaskSourceFactory_Local();
		
		// Called on program start to set up the TaskSourceFactory_Local. 
		// TaskSourceFactory_Local registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Create a TaskSource_Local: (Call after FinalInit())
		TaskSource *Create();
};

#endif  /* _RNDV_TSOURCE_LOCALPARS_HPP_ */
