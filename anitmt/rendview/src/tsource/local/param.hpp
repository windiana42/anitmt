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
	public:
		// This is needed for the param system: 
		struct PerFrameTaskInfo_Internal
		{
			Section *section;
			
			RefString size_string;
			RefString rdesc_string;
			RefString oformat_string;
			RefString fdesc_string;
			
			ParamInfo *render_resume_pi;  // internal use only
			
			_CPP_OPERATORS_FF
			PerFrameTaskInfo_Internal(int *failflag=NULL);
			~PerFrameTaskInfo_Internal() { }
		};
		struct PerFrameTaskInfo : LinkedListBase<PerFrameTaskInfo>
		{
			// For which frames this record is valid: 
			int first_frame_no,nframes;
			int tobe_rendered : 1;
			int tobe_filtered : 1;
			int : 30;
			
			// Render stuff: 
			int width,height;            // image size
			const ImageFormat *oformat;  // image output format to use
			const RenderDesc *rdesc;     // renderer to use
			RefStrList radd_args;        // additional args for the renderer
			RefString rdir;              // cd into rdir before calling renderer
			RefString rinfpattern;       // render input file name pattern
			RefString routfpattern;      // render output file name pattern
			bool render_resume_flag;
			
			// Filter stuff: 
			const FilterDesc *fdesc;     // filter to use
			RefStrList fadd_args;        // additional args for the filter
			RefString fdir;              // cd into rdir before calling filter
			RefString foutfpattern;      // filter output file name pattern
			
			// NOTE: as long as ii!=NULL, the information in *this 
			//       was not properly set up from the data in ii. 
			PerFrameTaskInfo_Internal *ii;  // internal info
			
			_CPP_OPERATORS_FF
			PerFrameTaskInfo(int *failflag=NULL);
			~PerFrameTaskInfo();
		};
	private:
		// We may access ComponentDataBase::component_db. 
		
		int startframe;
		// NOTE: fjump may be negative. 
		int fjump;
		// nframes=-1 -> as many as we can get
		int nframes;
		
		// Here, all the information we have to know to be able to 
		// render and filter the frames is stored: 
		// The master record is for all frames which are not matched 
		// by a special record. 
		// Also, the master_fi serves as default for the frame info 
		// in the fi_list. NOTE THAT the fi_list is sorted by frame 
		// numbers. 
		PerFrameTaskInfo master_fi;
		LinkedList<PerFrameTaskInfo> fi_list;
		PerFrameTaskInfo *last_looked_up;  // for GetPerFrameTaskInfo(). 
		
		// Passed -cont/-rcont?
		bool cont_flag;
		
		// Mainly for testing: use this delay before answering any 
		// requests: 
		long response_delay;
		
		PerFrameTaskInfo *_DoGetPerFrameTaskInfo(int frame_no,int *next_pfbs);
		
		const char *_FrameInfoLocationString(const PerFrameTaskInfo *fi);
		int _CheckFramePattern(RefString *s,const char *name,
			const PerFrameTaskInfo *fi);
		int _SetUpAndCheckOutputFramePatterns(PerFrameTaskInfo *fi);
		int _Param_ParseInSizeString(PerFrameTaskInfo *fi);
		int _Param_ParseOutputFormat(PerFrameTaskInfo *fi);
		int _Param_ParseRenderDesc(PerFrameTaskInfo *fi,int warn_unspec);
		int _Param_ParseFilterDesc(PerFrameTaskInfo *fi);
		
		void _VPrintFrameInfo(PerFrameTaskInfo *fi,
			const PerFrameTaskInfo *compare_to);
		
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
		
		// Returns NULL only if frame_no is out of range. 
		const PerFrameTaskInfo *GetPerFrameTaskInfo(int frame_no);
		
		// Get description string: 
		const char *TaskSourceDesc() const;
};

#endif  /* _RNDV_TSOURCE_LOCALPARS_HPP_ */
