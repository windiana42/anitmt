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
	public TaskSourceFactory,
	par::SectionParameterHandler
{
	friend class TaskSource_Local;
	public:
		// enum for PerFrameTaskInfo::set_flags: 
		enum
		{
			SF_size=         0x0000001,
			SF_oformat=      0x0000002,
			SF_rdesc=        0x0000004,
			SF_radd_args=    0x0000008,
			SF_rdir=         0x0000010,
			SF_rinfpattern=  0x0000020,
			SF_routfpattern= 0x0000040,
			SF_r_resume_flag=0x0000080,
			SF_rtimeout=     0x0000100,
			SF_radd_files=   0x0000200,
			SF_fdesc=        0x0000400,
			SF_fadd_args=    0x0000800,
			SF_fdir=         0x0001000,
			SF_foutfpattern= 0x0002000,
			SF_ftimeout=     0x0004000,
			SF_fadd_files=   0x0008000,
			SF_use_clock=    0x0010000,
			SF_clock_start=  0x0020000,
			SF_clock_step=   0x0040000,
			SF_ALL_FLAGS=    0x007ffff   // <-- don't forget
		};
		// Return string representation of flag which must have 
		// set EXACTLY ONE SF_* flag bit. 
		static const char *SF_FlagString(int flag);
		
		// This is needed for the param system: 
		struct PerFrameTaskInfo_Internal
		{
			Section *section;
			
			RefString size_string;
			RefString rdesc_string;
			RefString oformat_string;
			RefString fdesc_string;
			
			ParamInfo *render_resume_pi;  // internal use only
			ParamInfo *use_frameclock_pi;  // internal use only
			
			_CPP_OPERATORS_FF
			PerFrameTaskInfo_Internal(int *failflag=NULL);
			~PerFrameTaskInfo_Internal() { }
		};
		struct PerFrameTaskInfo : LinkedListBase<PerFrameTaskInfo>
		{
			// For which frames this record is valid: 
			// master_fi: nframes=-1 -> as many as we can get
			int first_frame_no,nframes;
			// Flags: Which params were explicitly set by the user?
			// NOTE: set_flags can have SF_rdesc set although 
			//       rdesc is NULL if rendering is switched off for 
			//       the corresponding frame range. 
			// Be careful with radd_args,radd_files,... because 
			// these can be appended or overridden depending if 
			// the NULL ref is still there. 
			int set_flags;
			
			// Render stuff: 
			int width,height;            // image size
			const ImageFormat *oformat;  // image output format to use
			const RenderDesc *rdesc;     // renderer to use
			RefStrList radd_args;        // additional args for the renderer
			RefString rdir;              // cd into rdir before calling renderer
			RefString rinfpattern;       // render input file name pattern
			RefString routfpattern;      // render output file name pattern
			bool render_resume_flag;
			long rtimeout;               // render timeout (msec; initially seconds)
			RefStrList radd_files;       // Additional files
			// For passing of clock values: 
			bool use_clock;          // MUST BE bool !!
			double clock_start;
			double clock_step;
			
			// Filter stuff: 
			const FilterDesc *fdesc;     // filter to use
			RefStrList fadd_args;        // additional args for the filter
			RefString fdir;              // cd into fdir before calling filter
			RefString foutfpattern;      // filter output file name pattern
			long ftimeout;               // filter timeout (msec; initially seconds)
			RefStrList fadd_files;       // Additional files
			
			// NOTE: as long as ii!=NULL, the information in *this 
			//       was not properly set up from the data in ii. 
			PerFrameTaskInfo_Internal *ii;  // internal info
			
			_CPP_OPERATORS_FF
			PerFrameTaskInfo(int *failflag=NULL);
			// NOTE: ii is not copied. 
			PerFrameTaskInfo(PerFrameTaskInfo *copy_from,int *failflag);
			~PerFrameTaskInfo();
		};
	private:
		// We may access ComponentDataBase::component_db. 
		
		// NOTE: fjump may be negative. 
		int fjump;
		
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
		
		// Check if the per-frame block f0,n will ever be used (1) or not (0): 
		inline int _CheckWillUseFrameBlock(int f0,int n);
		
		PerFrameTaskInfo *_DoGetPerFrameTaskInfo(int frame_no,int *next_pfbs);
		
		const char *_FrameInfoLocationString(const PerFrameTaskInfo *fi);
		int _CheckFramePattern(RefString *s,const char *name,
			const PerFrameTaskInfo *fi,int may_miss_spec);
		int _SetUpAndCheckOutputFramePatterns(PerFrameTaskInfo *fi);
		int _Param_ParseInSizeString(PerFrameTaskInfo *fi);
		int _Param_ParseOutputFormat(PerFrameTaskInfo *fi);
		int _Param_ParseRenderDesc(PerFrameTaskInfo *fi,int warn_unspec);
		int _Param_ParseFilterDesc(PerFrameTaskInfo *fi);
		
		void _FillIn_SetFlags(PerFrameTaskInfo *fi);
		inline void _InsertPFIAtCorrectPos(PerFrameTaskInfo *fi);
		int _FixupAdditionalFileSpec(RefStrList *add_files,
			PerFrameTaskInfo *fi,TaskDriverType dtype);
		int _FixupPerFrameBlock(PerFrameTaskInfo *fi);
		int _DeleteNeverUsedPerFrameBlocks();
		// first_frame_no and nframes are NOT compared. 
		// Return value: operator==(), i.e. true if equal.
		bool _ComparePerFrameInfo(const PerFrameTaskInfo *a,
			const PerFrameTaskInfo *b);
		
		int _PFBMergePtr(const void **dest,
			const void *sa,bool set_a,const void *sb,bool set_b);
		int _PFBMergeString(RefString *dest,
			const RefString *sa,bool set_a,const RefString *sb,bool set_b);
		int _PFBMergeStrList(RefStrList *dest,
			const RefStrList *sa,bool set_a,const RefStrList *sb,bool set_b);
		int _PFBMergeInt(long *dest,
			long sa,bool set_a,long sb,bool set_b);
		int _PFBMergeBool(bool *dest,
			bool sa,bool set_a,bool sb,bool set_b);
		int _PFBMergeDouble(double *dest,
			double sa,bool set_a,double sb,bool set_b);
		int _PFBMergeSize(int *dest_w,int *dest_h,
			int sa_w,int sa_h,bool set_a,int sb_w,int sb_h,bool set_b);
		int _MergePerFrameBlock(PerFrameTaskInfo *dest,
			const PerFrameTaskInfo *sa,const PerFrameTaskInfo *sb);
		
		void _VPrintFrameInfo_DumpListIfNeeded(const char *title,
			const RefStrList *compare_to,const RefStrList *list);
		void _VPrintFrameInfo(PerFrameTaskInfo *fi,
			const PerFrameTaskInfo *compare_to);
		
		Section *fi_topsect;      // per-frame block section root
		Section *_i_help_dummy;   // internal use for special help
		int _RegisterParams();
		int _RegisterFrameInfoParams(PerFrameTaskInfo *fi,int show_in_help=0);
		// overriding virtual from par::SectionParameterHandler: 
		int parse(const Section *s,PAR::SPHInfo *info);
		int PrintSectionHelp(const Section *sect,RefStrList *dest,int when);
		
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
