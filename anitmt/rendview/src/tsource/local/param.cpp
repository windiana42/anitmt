/*
 * param.cpp
 * 
 * Implementation of parameter & factory class of local task source. 
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


#include "local.hpp"
#include "param.hpp"

#include <assert.h>
#include <ctype.h>


#define UnsetNegMagic  (-985430913)


static volatile void __CheckAllocFailFailed()
{
	Error("Allocation failure.\n");
	abort();
}

static inline void _CheckAllocFail(int failed)
{
	if(failed)
	{  __CheckAllocFailFailed();  }
}


// Create a local TaskSource (TaskSource_Local): 
TaskSource *TaskSourceFactory_Local::Create()
{
	return(NEW1<TaskSource_Local>(this));
}


const char *TaskSourceFactory_Local::TaskSourceDesc() const
{
	return("Local task source is used to process frames on "
		"your local hard drive.");
}


// Used for errors so that you know which frame info block 
// it is talking about: 
const char *TaskSourceFactory_Local::_FrameInfoLocationString(
	const PerFrameTaskInfo *fi)
{
	if(fi==&master_fi)
	{  return("[master]");  }
	static char tmp[64];
	snprintf(tmp,64,"[frames %d:%d]",fi->first_frame_no,fi->nframes);
	return(tmp);
}


// Used by TaskSource_Local: Get PerFrameTaskInfo for specified frame: 
const TaskSourceFactory_Local::PerFrameTaskInfo *TaskSourceFactory_Local::
	GetPerFrameTaskInfo(int frame_no)
{
	if(frame_no<startframe || 
	   (nframes>=0 && frame_no>=startframe+nframes) )
	{  return(NULL);  }
	
	int next_pfbs;
	PerFrameTaskInfo *fi=_DoGetPerFrameTaskInfo(frame_no,&next_pfbs);
	// See if there is something to do for that frame. 
	if(fi->rdesc || fi->fdesc)  return(fi);
	
	// Nothing to do for that frame here. 
	// FIXME: maywe we want a per-block jump option and do real JUMPing 
	//        (to next block which has rdesc || fdesc). 
	// If we have unlimited frames and this is the master block, 
	// then we may run into trouble. 
	if(fi==&master_fi)
	{
		// See if there are non-empty blocks following: 
		if(next_pfbs<startframe)  return(NULL);  // no more tasks to do
	}
	
	return(fi);
}

// next_pfbs: saves next per-frame block start frame number (or startframe-1). 
// Only valid of return value is &master_fi. 
TaskSourceFactory_Local::PerFrameTaskInfo *TaskSourceFactory_Local::
	_DoGetPerFrameTaskInfo(int frame_no,int *next_pfbs)
{
	*next_pfbs=startframe-1;
	
	// NOTE!! last_looked_up may !!NEVER!! be master_fi. 
	if(!last_looked_up)  last_looked_up=fi_list.first();
	if(!last_looked_up)  return(&master_fi);
	
	// Check if it is still the same info block: 
	if(frame_no>=last_looked_up->first_frame_no && 
	   frame_no<(last_looked_up->first_frame_no+last_looked_up->nframes) )
	{  return(last_looked_up);  }
	
	// Check previous and next ones: 
	if(frame_no<last_looked_up->first_frame_no)
	{
		for(PerFrameTaskInfo *fi=last_looked_up->prev; fi; fi=fi->prev)
		{
			if(frame_no<fi->first_frame_no)  continue;
			last_looked_up=fi;
			if(frame_no>=(fi->first_frame_no+fi->nframes))  break;
			return(fi);
		}
	}
	else // frame_no>=(last_looked_up->first_frame_no+last_looked_up->nframes)
	{
		for(PerFrameTaskInfo *fi=last_looked_up->next; fi; fi=fi->next)
		{
			if(frame_no>=(fi->first_frame_no+fi->nframes))  continue;
			if(frame_no<fi->first_frame_no)
			{  last_looked_up=fi->prev;  break;  }
			last_looked_up=fi;
			return(fi);
		}
	}
	
	assert(last_looked_up);
	if(last_looked_up->next)
	{
		*next_pfbs=last_looked_up->next->first_frame_no;
		assert(*next_pfbs>frame_no);
	}
	
	return(&master_fi);
}


// Check if the passed pattern is something containing EXACTLY ONE 
// %d - modifier, e.g. "%123d" or "%07x" or "%03X". 
int TaskSourceFactory_Local::_CheckFramePattern(
	RefString *s,const char *name,const PerFrameTaskInfo *fi)
{
	const char *ptr=s->str();
	do {
		if(!ptr)  break;
		ptr=strchr(ptr,'%');
		if(!ptr)  break;
		++ptr;
		while(isdigit(*ptr))  ++ptr;
		if(*ptr!='x' && *ptr!='d' && *ptr!='X')
		{  ptr=NULL;  break;  }
		if(strchr(ptr,'%'))
		{  ptr=NULL;  break;  }
	}  while(0);
	if(!ptr)
	{
		Error("Local: %s: Invalid %s frame pattern \"%s\".\n",
			_FrameInfoLocationString(fi),name,s->str());
		return(1);
	}
	return(0);
}


// Check the output patterns or set reasonable defaults according to 
// input frame pattern: 
int TaskSourceFactory_Local::_SetUpAndCheckOutputFramePatterns(
	PerFrameTaskInfo *fi)
{
	int mfailed=0;
	
	// render output: 
	if(!fi->routfpattern.str())
	{
		// No output pattern. 
		// Use input pattern without extension: 
		const char *ip=fi->rinfpattern.str();
		const char *es=strrchr(ip,'.');
		if(es)
		{
			// Make sure we're right of the `%´: 
			const char *pp=strrchr(ip,'%');
			if(es<pp)  es=NULL;
		}
		if(!es)  es=ip+strlen(ip);
		_CheckAllocFail(fi->routfpattern.set0(ip,es-ip));
	}
	mfailed+=_CheckFramePattern(&fi->routfpattern,"render output",fi);
	// NOTE: The extension is appended below so that we can use it without 
	//       extension for the foutfpattern. 
	// filter output: 
	if(!fi->foutfpattern.str())
	{
		// No output pattern. 
		// Use render output pattern and append -f. 
		fi->foutfpattern=fi->routfpattern;
		assert(fi->oformat);  // MUST have been set above. ALWAYS. 
		char ext_tmp[strlen(fi->oformat->file_extension)+3+1];
		strcpy(ext_tmp,"-f.");
		strcpy(ext_tmp+3,fi->oformat->file_extension);
		_CheckAllocFail(fi->foutfpattern.append(ext_tmp));
	}
	mfailed+=_CheckFramePattern(&fi->foutfpattern,"filter output",fi);
	// Okay, now append extension to render output file pattern: 
	assert(fi->oformat);  // MUST have been set above. ALWAYS. 
	{
		char ext_tmp[strlen(fi->oformat->file_extension)+1+1];
		ext_tmp[0]='.';
		strcpy(ext_tmp+1,fi->oformat->file_extension);
		_CheckAllocFail(fi->routfpattern.append(ext_tmp));
	}
	
	return(mfailed);
}


// Return value: 0 -> OK; 1 -> failed. 
int TaskSourceFactory_Local::_Param_ParseInSizeString(PerFrameTaskInfo *fi)
{
	const char *ptr=fi->ii->size_string.str();
	if(!ptr)
	{  return(0);  }   // using default
	
	char *end;
	int valid=1;
	fi->width=strtol(ptr,&end,0);
	if(end<=ptr)  valid=0;
	ptr=end;
	if(*ptr=='x' || *ptr==',')  ++ptr;
	else  valid=0;
	fi->height=strtol(ptr,&end,0);
	if(end<=ptr || *end)  valid=0;
	
	if(!valid || fi->width<=0 || fi->height<=0)
	{
		Error("Local: %s: Illegal size spec \"%s\".\n",
			_FrameInfoLocationString(fi),fi->ii->size_string.str());
		return(1);
	}
	return(0);
}

int TaskSourceFactory_Local::_Param_ParseOutputFormat(PerFrameTaskInfo *fi)
{
	const char *str=fi->ii->oformat_string.str();
	if(!str)
	{  return(0);  }  // useing default
	
	fi->oformat=component_db()->FindImageFormatByName(str);
	if(!fi->oformat)
	{
		Error("Local: %s: Image (output) format \"%s\" not recognized "
			"(try -list-imgfmt).\n",_FrameInfoLocationString(fi),str);
		return(1);
	}
	return(0);
}

int TaskSourceFactory_Local::_Param_ParseRenderDesc(
	PerFrameTaskInfo *fi,int warn_unspecified)
{
	const char *rstr=fi->ii->rdesc_string.str();
	if(!rstr)
	{
		if(warn_unspecified)
		{  Warning("Local: %s: No renderer (DESC) specified; "
			"maybe you forgot that?\n",_FrameInfoLocationString(fi));  }
		return(0);   // using default or none
	}
	
	// Parse renderer to use: 
	fi->rdesc=component_db()->FindRenderDescByName(rstr);
	if(!fi->rdesc)
	{
		Error("Local: %s: Unknown render DESC named \"%s\".\n",
			_FrameInfoLocationString(fi),rstr);
		return(1);
	}
	
	if(!IsSet(fi->ii->render_resume_pi))
	{
		// Default: yes, if rdesc supports it: 
		if(fi==&master_fi)
		{  fi->render_resume_flag=fi->rdesc->can_resume_render;  }
		else
		{  fi->render_resume_flag=master_fi.render_resume_flag;  }
	}
	if(fi->render_resume_flag && !fi->rdesc->can_resume_render)
	{
		Warning("Local: %s: Disabled render resume feature (-rcont) "
			"(no support by renderer / %s driver).",
			_FrameInfoLocationString(fi),
			fi->rdesc->dfactory->DriverName());
		fi->render_resume_flag=false;
	}
	
	return(0);
}	

int TaskSourceFactory_Local::_Param_ParseFilterDesc(PerFrameTaskInfo *fi)
{
	const char *fstr=fi->ii->fdesc_string.str();
	if(!fstr)
	{  return(0);  }   // using default or none
	
	// Parse filter to use: 
	fi->fdesc=component_db()->FindFilterDescByName(fstr);
	if(!fi->fdesc)
	{
		Error("Local: %s: Unknown filter DESC named \"%s\".\n",
			_FrameInfoLocationString(fi),fstr);
		return(1);
	}
	
	return(0);
}


void TaskSourceFactory_Local::_VPrintFrameInfo_DumpListIfNeeded(
	const char *title,const RefStrList *compare_to,const RefStrList *list)
{
	if(compare_to)
	{
		// This is a simple compare algo; the lists have to match 
		// also in order. 
		for(const RefStrList::Node *i=list->first(),*j=compare_to->first(); 
			(i || j) /* <== REALLY! */; i=i->next,j=j->next)
		{
			if(!(i && j))  goto doit;
			if((*i)!=(*j))  goto doit;
		}
		return; doit:;
	}
	
	Verbose(TSI,"%s:",title);
	for(const RefStrList::Node *i=list->first(); i; i=i->next)
	{  Verbose(TSI," %s",i->str());  }
	Verbose(TSI,"%s\n",list->is_empty() ? " [none]" : "");
}


// Only print info if it is different from the one in compare_to. 
// Pass compare_to=NULL to get it all dumped. 
void TaskSourceFactory_Local::_VPrintFrameInfo(PerFrameTaskInfo *fi,
	const PerFrameTaskInfo *compare_to)
{
	if(!compare_to || 
	   (fi->rdesc!=compare_to->rdesc || fi->oformat!=compare_to->oformat) )
	{
		Verbose(TSI,"    Renderer: ");
		if(fi->rdesc)
		{  Verbose(TSI,"%s (%s driver)",fi->rdesc->name.str(),
			fi->rdesc->dfactory->DriverName());  }
		else
		{  Verbose(TSI,"[not rendering]");  }
		// YES! It makes sense to dump oformat if renderer is unspecified. 
		// Because it is used e.g. for frame pattern names. 
		Verbose(TSI,"; output format: ");
		if(fi->oformat)  // pbc = bits per channel
		{  Verbose(TSI,"%s (%d bpc)\n",fi->oformat->name,
			fi->oformat->bits_p_rgb);  }
		else
		{  Verbose(TSI,"[unspecified]\n");  }
	}
	
	if(fi->rdesc)
	{
		if(!compare_to || 
		   (fi->render_resume_flag!=compare_to->render_resume_flag || 
		    fi->width!=compare_to->width || fi->height!=compare_to->height ) )
		{  Verbose(TSI,"      Size: %dx%d; cont operation: %s\n",
			fi->width,fi->height,
			fi->render_resume_flag ? 
				"resume (-rcont)" : "re-render (-no-rcont)");  }
		if(!compare_to || fi->rdir!=compare_to->rdir)
		{  Verbose(TSI,"      Render dir: %s\n",fi->rdir.str() ? fi->rdir.str() : "[cwd]");  }
		if(!compare_to || 
		   (fi->rtimeout!=compare_to->rtimeout) )
		{
			char tmp[32];
			if(fi->rtimeout<=0)
			{  strcpy(tmp,"[none]");  }
			else
			{  snprintf(tmp,32,"%ld seconds",fi->rtimeout/1000);  }
			Verbose(TSI,"      Render timeout: %s\n",tmp);
		}
		_VPrintFrameInfo_DumpListIfNeeded("      Add args",
			compare_to ? &compare_to->radd_args : NULL,&fi->radd_args);
		_VPrintFrameInfo_DumpListIfNeeded("      Add files",
			compare_to ? &compare_to->radd_files : NULL,&fi->radd_files);
	}
	
	if(!compare_to || 
	   (fi->fdesc!=compare_to->fdesc) )
	{
		Verbose(TSI,"    Filter: ");
		if(fi->fdesc)
		{  Verbose(TSI,"%s (%s driver)\n",fi->fdesc->name.str(),
			fi->fdesc->dfactory->DriverName());  }
		else
		{  Verbose(TSI,"[not filtering]\n");  }
	}
	
	if(fi->fdesc)
	{
		if(!compare_to || fi->fdir!=compare_to->fdir)
		{  Verbose(TSI,"      Filter dir: %s\n",fi->fdir.str() ? fi->fdir.str() : "[cwd]");  }
		if(!compare_to || 
		   (fi->ftimeout!=compare_to->ftimeout) )
		{
			char tmp[32];
			if(fi->ftimeout<=0)
			{  strcpy(tmp,"[none]");  }
			else
			{  snprintf(tmp,32,"%ld seconds",fi->ftimeout/1000);  }
			Verbose(TSI,"      Filter timeout: %s\n",tmp);
		}
		_VPrintFrameInfo_DumpListIfNeeded("      Add args",
			compare_to ? &compare_to->fadd_args : NULL,&fi->fadd_args);
		_VPrintFrameInfo_DumpListIfNeeded("      Add files",
			compare_to ? &compare_to->fadd_files : NULL,&fi->fadd_files);
	}
	
	if(!compare_to || 
	   (fi->rinfpattern!=compare_to->rinfpattern || 
	    fi->routfpattern!=compare_to->routfpattern || 
	    fi->foutfpattern!=compare_to->foutfpattern ) )
	{
		Verbose(TSI,"    Frame patterns: render: \"%s\" -> \"%s\"\n",
			fi->rinfpattern.str(),fi->routfpattern.str());
		Verbose(TSI,"                    filter: ... -> \"%s\"\n",
			fi->foutfpattern.str());
	}
}


static void _DeleteStrListFirstNULLRef(RefStrList *l)
{
	if(!l->first())  return;
	if(!l->first()->str())
	{  l->popfirst();  }
}
static void _AppendOverrideStrListArgs(RefStrList *l,RefStrList *m)
{
	if(l->first() && !l->first()->str())
	{
		// First NULL-ref is still there. Delete it and 
		// prepend list m: 
		l->popfirst();
		_CheckAllocFail(l->insert(m));
	}
}

static void _PathAppendSlashIfNeeded(RefString *s)
{
	if(!s->str())  return;
	if(s->str()[0]=='\0')
	{  s->set(NULL);  return;  }
	// s.len()>0 here. 
	if(s->str()[s->len()-1]!='/')
	{  _CheckAllocFail(s->append("/"));  }
}

static const char *_ltsrt_str="local task source render timeout";
static const char *_ltsft_str="local task source filter timeout";

// timeout: timeout in SECONDS or -1 or UnsetNegMagic. 
// master_timeout: override timeout for UnsetNegMagic in MSEC
// Returned new timeout in timeout in MSEC. 
static void _FrameBlockTimeoutOverride(long *timeout,TaskDriverType dtype,
	long master_timeout)
{
	if((*timeout)==UnsetNegMagic)
	{  *timeout=master_timeout;  }  // master_timeout already in msec
	else
	{  ConvertTimeout2MSec(timeout,dtype==DTRender ? _ltsrt_str : _ltsft_str);  }
}


// Check if the per-frame block f0,n will ever be used (1) or not (0): 
inline int TaskSourceFactory_Local::_CheckWillUseFrameBlock(int f0,int n)
{
	return( !((nframes>=0 && f0>=(startframe+nframes)) || (f0+n)<startframe) );
}


int TaskSourceFactory_Local::FinalInit()
{
	int mfailed=0;  // "master failed"
	
	// MASTER FRAME INFO...
	// Parse rdesc to use: 
	mfailed+=_Param_ParseRenderDesc(&master_fi,1);
	
	// Parse output format.
	mfailed+=_Param_ParseOutputFormat(&master_fi);
	if(!master_fi.oformat)
	{
		// We always need an oformat (e.g. for default file suffix, etc). 
		master_fi.oformat=component_db()->FindImageFormatByName("png");
		// PNG must be present: 
		assert(master_fi.oformat);
	}
	
	// Parse fdesc to use: 
	mfailed+=_Param_ParseFilterDesc(&master_fi);
	if(!mfailed && !master_fi.rdesc && !master_fi.fdesc)
	{
		Warning("Local: Neither render nor filter desc specified; "
			"master action is to do nothing.\n");
	}
	
	_PathAppendSlashIfNeeded(&master_fi.rdir);
	_PathAppendSlashIfNeeded(&master_fi.fdir);
	
	// Check frame pattern: 
	// NOTE: master_fi.rinfpattern has default set in constructor. 
	mfailed+=_CheckFramePattern(&master_fi.rinfpattern,"input",&master_fi);
	mfailed+=_SetUpAndCheckOutputFramePatterns(&master_fi);
	
	// Convert timeout seconds -> msec: 
	ConvertTimeout2MSec(&master_fi.rtimeout,_ltsrt_str);
	ConvertTimeout2MSec(&master_fi.ftimeout,_ltsft_str);
	
	if(!mfailed)
	{
		// Only reason for these assertion to fail would be alloc failure or 
		// internal error. 
		assert(master_fi.rinfpattern.str());
		assert(master_fi.routfpattern.str());
		assert(master_fi.foutfpattern.str());
	}
	
	if(mfailed)
	{  Verbose(TSP,"Local: Errors in master frame info spec; "
		"skipping per-frame info.\n");  }
	int failed=mfailed;
	
	// Okay: check if the per-frame info overlaps: 
	// Also, get rid of per-frame info which will never be used. 
	// AND, sort it in ascenting order. 
	if(!failed)
	{
		// First, eliminite all list elements with illegal range entries 
		// (and those which will never be used): 
		int never_used=0;
		for(PerFrameTaskInfo *_fi=fi_list.first(); _fi; )
		{
			PerFrameTaskInfo *fi=_fi;
			_fi=_fi->next;
			
			if(fi->first_frame_no<0 || fi->nframes<0)
			{
				Error("Local: Illegal frame range %d:%d in per-frame info.\n",
					fi->first_frame_no,fi->nframes);
				goto cfailed;
			}
			
			if(!fi->nframes)
			{
				Warning("Local: %s: Per-frame info refers to 0 frames.\n",
					_FrameInfoLocationString(fi));
				goto cdelete;
			}
			else if(fi->nframes<0)
			{
				Error("Local: %s: Invalid per-frame range spec.\n",
					_FrameInfoLocationString(fi));
				goto cfailed;
			}
			
			if(!_CheckWillUseFrameBlock(fi->first_frame_no,fi->nframes))
			{  ++never_used;  goto cdelete;  }
			
			continue;
			cfailed:  ++failed;
			cdelete:  delete fi_list.dequeue(fi);
		}
		if(never_used)
		{  Verbose(TSP,"Local: Deleted %d per-frame info blocks as they "
			"will not be used.\n",never_used);  }
		
		// Then, sort them: 
		LinkedList<PerFrameTaskInfo> srclist;
		fi_list.swap_list(&srclist);
		for(;;)
		{
			PerFrameTaskInfo *fi=srclist.popfirst();
			if(!fi)  break;
			
			// Insert at correct position: 
			for(PerFrameTaskInfo *i=fi_list.first(); i; i=i->next)
			{
				if(fi->first_frame_no>=i->first_frame_no)  continue;
				fi_list.queuebefore(fi,i);
				goto enqueued;
			}
			fi_list.append(fi);
			enqueued:;
		}
	}
	
	if(!failed)
	{
		// PER-FRAME FRAME INFO...
		// Also get rid of the fi->ii stuff: 
			
		for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
		{
			// Set defaults...
			fi->oformat=master_fi.oformat;
			fi->rdesc=master_fi.rdesc;
			//NOTE: render_resume_flag treated/copied correctly 
			//      by _Param_ParseRenderDesc()
			fi->fdesc=master_fi.fdesc;
			
			// ...and parse it: 
			failed+=_Param_ParseOutputFormat(fi);
			failed+=_Param_ParseRenderDesc(fi,0);
			failed+=_Param_ParseFilterDesc(fi);
			
			// Check frame patterns: 
			if(!fi->rinfpattern)
			{  fi->rinfpattern=master_fi.rinfpattern;  }
			int pfailed=_CheckFramePattern(&fi->rinfpattern,"input",fi);
			failed+=pfailed;
			if(!pfailed)
			{
				// Check other patterns or assign reasonable defaults: 
				failed+=_SetUpAndCheckOutputFramePatterns(fi);
			}
			
			// Delete unneeded internal info: 
			if(fi->ii->section)
			{  RecursiveDeleteParams(fi->ii->section);  }
			delete fi->ii;  fi->ii=NULL;
		}
	}
	
	if(!failed)
	{
		// Note... radd_args and fadd_args contain one NULL ref by 
		//         default to see if they were modified. Delete that. 
		_DeleteStrListFirstNULLRef(&master_fi.radd_args);
		_DeleteStrListFirstNULLRef(&master_fi.radd_files);
		_DeleteStrListFirstNULLRef(&master_fi.fadd_args);
		_DeleteStrListFirstNULLRef(&master_fi.fadd_files);
		
		// Okay, now set up the rest of the members in the per-frame 
		// info structures: 
		for(PerFrameTaskInfo *_fi=fi_list.first(); _fi; )
		{
			PerFrameTaskInfo *fi=_fi;
			_fi=_fi->next;
			
			// First, the easy part: 
			if(!fi->rdir)  fi->rdir=master_fi.rdir;
			else  _PathAppendSlashIfNeeded(&fi->rdir);
			if(!fi->fdir)  fi->fdir=master_fi.fdir;
			else  _PathAppendSlashIfNeeded(&fi->fdir);
			
			_FrameBlockTimeoutOverride(&fi->rtimeout,DTRender,
				master_fi.rtimeout);
			_FrameBlockTimeoutOverride(&fi->ftimeout,DTFilter,
				master_fi.ftimeout);
			
			// If the first NULL-string is still there, we prepend 
			// the master params, else we override. 
			_AppendOverrideStrListArgs(&fi->radd_args,&master_fi.radd_args);
			_AppendOverrideStrListArgs(&fi->radd_files,&master_fi.radd_files);
			_AppendOverrideStrListArgs(&fi->fadd_args,&master_fi.fadd_args);
			_AppendOverrideStrListArgs(&fi->fadd_files,&master_fi.fadd_files);
			
			// NOTE: We will NOT delete fdesc=rdesc=NULL blocks UNLESS the 
			//       master block is also fdesc=rdesc=NULL. This allows to 
			//       specify blocks which shall not be processed. 
			if(!fi->rdesc && !fi->fdesc && 
			   !master_fi.rdesc && !master_fi.fdesc)
			{
				delete fi_list.dequeue(fi);
				continue;
			}
			
			Error("*** Check if it is possible to create invalid per-frame block if no action for master block (or invalid data for master block such as wicth=-1) ***\n");
			
		}
	}
	
	if(!failed)
	{
		// Finally, check for overlapping blocks and split them: 
		if(fi_list.first() && fi_list.first()->next)  // need at least 2 elems
		{
			for(PerFrameTaskInfo *_fi=fi_list.first(); (_fi && _fi->next); )
			{
				PerFrameTaskInfo *fi=_fi;
				_fi=_fi->next;
				
				// Collision detection: Check if fi collides with fi->next. 
				// NOTE: fi->next not NULL here (see for() condition). 
				if(fi->first_frame_no+fi->nframes<=fi->next->first_frame_no)
				{  continue;  }
				
				// So they overlap. 
				// There can be up to three resulting per-frame blocks: 
				int s0_f0=fi->first_frame_no;
				int s0_n=fi->next->first_frame_no-fi->first_frame_no;
				int s1_f0=fi->next->first_frame_no;
				int s1_n=fi->first_frame_no+fi->nframes-fi->next->first_frame_no;
				int s2_f0=fi->first_frame_no+fi->nframes;
				int s2_n=fi->next->first_frame_no+fi->next->nframes-
					(fi->first_frame_no+fi->nframes);
				Verbose(TSP,"Local: %s: Per-frame info overlaps with %d:%d; "
					"splitting into:",
					_FrameInfoLocationString(fi),
					fi->next->first_frame_no,fi->next->nframes);
				// We do not generate blocks which will neber ne used. 
				if(s1_n>0 && _CheckWillUseFrameBlock(s1_f0,s1_n))
				{
					Verbose(TSP," %d,%d",s1_f0,s1_n);
					
					PerFrameTaskInfo *s1=NEW<PerFrameTaskInfo>();
					_CheckAllocFail(!s1);
					delete s1->ii;  s1->ii=NULL;
					
					// Okay, actually merge it: 
					Error("Merging overlapping per-frame blocks not yet supported.\n");
					hack_assert(0);
					
					s1->first_frame_no=s1_f0;
					s1->nframes=s1_n;
					
					fi_list.queueafter(s1,fi);
				}
				// DONT CHANGE THIS ORDER. 
				if(s2_n>0 && _CheckWillUseFrameBlock(s2_f0,s2_n))
				{
					Verbose(TSP," %d,%d",s2_f0,s2_n);
					fi->next->first_frame_no=s2_f0;
					fi->next->nframes=s2_n;
				}
				else
				{  delete fi_list.dequeue(fi->next);  }
				// DONT CHANGE THIS ORDER. 
				_fi=fi->prev;  // May be NULL, yes... (see below)
				if(s0_n>0 && _CheckWillUseFrameBlock(s0_f0,s0_n))
				{
					Verbose(TSP," %d,%d",s0_f0,s0_n);
					fi->first_frame_no=s0_f0;
					fi->nframes=s0_n;
				}
				else
				{  delete fi_list.dequeue(fi);  }
				Verbose(TSP,"\n");
				if(!_fi)  _fi=fi_list.first();
			}
		}
	}
	
	if(!failed)
	{
		// Okay, last action here: Find the last frame in case 
		// we are doing backwards steps and nframes is not 
		// specified: 
		if(nframes<0 && fjump<0)
		{
			// This algorithm will stop at the first non-existing 
			// frame file: 
			int fno=startframe;
			Verbose(MiscInfo,"Local: Looking for last frame to render... %7d",fno);
			RefString tmp;
			for(int chk=0;;chk++,fno-=fjump)
			{
				// Good that GetPerFrameTaskInfo() is fast...
				const PerFrameTaskInfo *fi=GetPerFrameTaskInfo(fno);
				if(!fi)  break;
				
				// Check file existence: 
				_CheckAllocFail(tmp.sprintf(0,fi->rinfpattern,fno));
				// Prepend rdir if not absolute path: 
				if(fi->rdir.str() && tmp.str()[0]!='/')  // NOTE!!! SIMILAR CODE IN local.cpp 
				{
					assert(fi->rdir.str()[fi->rdir.len()-1]=='/');  // was appended above
					_CheckAllocFail(tmp.prepend(fi->rdir));
				}
				if(!CheckExistFile(&tmp,1))  break;
				
				if(!(chk%50))
				{  Verbose(MiscInfo,"\b\b\b\b\b\b\b%7d",fno);  }
			}
			nframes=fno-startframe;
			Verbose(MiscInfo,"\b\b\b\b\b\b\b%d (nframes=%d)\n",fno,nframes);
		}
	}
	
	if(!failed)
	{
		char nf_tmp[24];
		if(nframes>=0)  snprintf(nf_tmp,24,"%d",nframes);
		else  strcpy(nf_tmp,"[unlimited]");
		Verbose(TSI,"Local task source: jump: %d; nframes: %s; startframe: %d\n",
			fjump,nf_tmp,startframe);
		Verbose(TSI,"  Continuing: %s\n",
			cont_flag ? "yes" : "no");
		
		Verbose(TSI,"  Master frame info:\n");
		_VPrintFrameInfo(&master_fi,NULL);
		
		for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
		{
			Verbose(TSI,"  Frame info difference for frames %d--%d (%d frames):\n",
				fi->first_frame_no,fi->first_frame_no+fi->nframes-1,
				fi->nframes);
			_VPrintFrameInfo(fi,&master_fi);
		}
		
		if(!master_fi.rdesc && !master_fi.fdesc && fi_list.is_empty())
		{  VerboseSpecial("Local: Nothing to do at all. "
			"Try %s --help if you're puzzled.",prg_name);  }
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_Local::CheckParams()
{
	int failed=0;
	
	// Check and parse in size strings: 
	failed+=_Param_ParseInSizeString(&master_fi);
	for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
	{
		// Set default: 
		fi->width=master_fi.width;
		fi->height=master_fi.height;
		failed+=_Param_ParseInSizeString(fi);
	}
	
	if(!fjump)
	{
		fjump=1;
		Warning("Local: Illegal frame jump value 0 corrected to 1.\n");
	}
	
	if(startframe<0)
	{
		Error("Local: Illegal start frame number %d.\n",startframe);
		++failed;
	}
	
	if(response_delay<0)
	{  response_delay=0;  }
	
	if(response_delay)
	{  Warning("Local: You set response delay %ld msec.\n",response_delay);  }
	
	return(failed ? 1 : 0);
}


// Additional help (called via section handler) 
int TaskSourceFactory_Local::PrintSectionHelp(const Section *sect,
	RefStrList * /*dest*/,int when)
{
	if(sect!=fi_topsect)  return(0);
	
	if(when==-1)
	{
		// Register the parameters, so that help text on 
		// them can be written: 
		assert(!_i_help_dummy);
		_CheckAllocFail(SetSection("<fff>:<nnn>",
			"Per-frame block for <nnn> frames beginning with frame <fff>. "
			"Leave away (the <fff>:<nnn>) for the master frame block.",
			fi_topsect));
		if(!_RegisterFrameInfoParams(NULL,1))
		{  _i_help_dummy=CurrentSection();  }
	}
	else if(when==+2)
	{
		// Must clean up the parameters for help text again: 
		if(_i_help_dummy)
		{
			RecursiveDeleteParams(_i_help_dummy);
			_i_help_dummy=NULL;
		}
	}
	
	return(0);
}


// Section handler parse routine:
int TaskSourceFactory_Local::parse(const Section * /*s*/,PAR::SPHInfo *info)
{
	if(info->sect!=fi_topsect)  return(2);
	
	// Okay, we're getting info about a new per-frame info block, so 
	// quickly add the parameters...
	// But first, to some checking. 
	const char *xname=NULL;
	size_t xnamelen=0;
	if(info->arg)
	{
		// Okay, this is more tricky, we have to find where the 
		// name of the section ends. 
		int illegal=1;
		const char *e=NULL;
		const char *saw_colon=NULL;
		for(const char *c=info->nend; c<info->name_end; c++)
		{
			if(isdigit(*c))  continue;
			if(*c==':')
			{
				if(saw_colon || c==info->nend)  break;
				saw_colon=c;
			}
			else if(*c=='-')
			{
				if(saw_colon+1<c)
				{  illegal=0;  e=c;  break;  }
				break;
			}
			else break;
		}
		if(illegal)  return(1);  // go on parsing... (and possibly report error)
		xname=info->nend;
		xnamelen=e-xname;
	}
	else
	{
Error("*** Please check that code if it does what we want. ***\n");
assert(0);
		
		// Okay, there was a `#section xyz´ ot sth like that. 
		int illegal=0;
		for(const char *c=info->nend; c<info->name_end; c++)
		{
			//if(*c=='-' || isspace(*c))
			if(!isdigit(*c) && *c!=':')
			{  ++illegal;  break;  }
		}
		xname=info->nend;
		xnamelen=info->name_end-xname;
		/*if(info->name_end<=info->nend || illegal)
		{
			#warning fixme: cannot report <SOMEWHERE> as location...
			Error("In <SOMEWHERE - FIXME>: %s \"%.*s\".\n",
				ipfss,
				info->name_end>xname ? info->name_end-xname : 10,xname);
			return(-1);
		}*/
		return(1);
	}
	
	int fail_req=0;
	
	// Okay, now we have the per-frame spec. Parse it: 
	PerFrameTaskInfo *new_fi=NULL;
	{
		int illegal=1;
		int f0=-1,nf=-1;
		do {
			if(xnamelen>=256)  break;
			
			char buf[xnamelen+1];
			strncpy(buf,xname,xnamelen);
			buf[xnamelen]='\0';
			
			char *end;
			f0=strtol(buf,&end,10);
			if(end<=buf || *end!=':' || f0<0)  break;
			char *b=end+1;
			nf=strtol(b,&end,10);
			if(end<=buf || *end || nf<0)  break;
			
			illegal=0;
			
			// Do it here because we have buf() here. 
			if(SetSection(buf,NULL,fi_topsect,SSkipInHelp))
			{  ++fail_req;  }
		} while(0);
		if(illegal)  return(1);  // will report illegal parameter...
		
		// Okay, since things seem to be valid, alloc a per frame block: 
		new_fi=NEW<PerFrameTaskInfo>();
		if(!new_fi)  _CheckAllocFail(-1);
		new_fi->first_frame_no=f0;
		new_fi->nframes=nf;
		fi_list.append(new_fi);  // sorted lateron
	}
	
	Verbose(0,"Adding new per-frame block %d:%d (last frame: %d).\n",
		new_fi->first_frame_no,new_fi->nframes,
		new_fi->first_frame_no+new_fi->nframes);
	
	if(_RegisterFrameInfoParams(new_fi))
	{
		delete fi_list.dequeue(new_fi);
		fail_req=1;
	}
	
	if(fail_req)
	{
		Error("Failed to register section for per-frame block %.*s.\n",
			int(xnamelen),xname);
		return(-1);
	}
	
	return(1);  // okay, accepted. 
}


int TaskSourceFactory_Local::_RegisterParams()
{
	if(SetSection("l","loacal task source"))
	{  return(-1);  }
	
	AddParam("fjump|j","frame jump value; use negative values to "
		"process last frames first",&fjump);
	AddParam("nframes|n","number of frames to process; if you do not "
		"specify or use negative value -> unlimited (as long as input "
		"files exist)",&nframes);
	AddParam("startframe|f0","first frame to process",&startframe);
	
	AddParam("cont","continue; don't render frames which have "
		"already been rendered (i.e. image exists and is newer than input) "
		"or (if -rcont is used) continue rendering of partly rendered image",
		&cont_flag);
	
	AddParam("response-delay","local task source response delay in msec; "
		"mainly useful in debugging",&response_delay);
	
	if(add_failed)  return(-1);
	
	// Master frame info params: 
	if(_RegisterFrameInfoParams(&master_fi))
	{  return(-1);  }
	
	fi_topsect=CurrentSection();
	if(SectionParameterHandler::Attach(fi_topsect))
	{  ++add_failed;  }
	
	return(0);
}


int TaskSourceFactory_Local::_RegisterFrameInfoParams(PerFrameTaskInfo *fi,
	int show_in_help)
{
	int flags = show_in_help ? 0 : PSkipInHelp;
	
	add_failed=0;
	
	AddParam("size|s","size (WWWxHHH) of created images",
		fi ? &fi->ii->size_string : NULL,flags);
	AddParam("oformat","image output format (must be supported by "
		"render/filter driver)",fi ? &fi->ii->oformat_string : NULL,flags);
	AddParam("renderer|rd","renderer to use (render DESC name)",
		fi ? &fi->ii->rdesc_string : NULL,flags);
	AddParam("rargs","additional args to be passed to the renderer",
		fi ? &fi->radd_args : NULL,flags);
	//AddParam("rfiles","additional files needed by the rendering process",
	//	fi ? &fi->radd_files : NULL,flags);
	ParamInfo *tmp=AddParam("rcont",
		"resume cont; This switch enables/disables render continue "
		"feature: If enabled, interrupted files are named *-unfinished "
		"and rendering resumes at the position where it stopped; "
		"if disabled, a not-completely rendered frame is deleted and has "
		"to be rendered completely the next time.\nNOTE: this just selects "
		"the operation mode; you must use -cont to actuallly continue. "
		"(Default: yes, if driver supports it)",
		fi ? &fi->render_resume_flag : NULL,PNoDefault | flags);
	if(fi)  fi->ii->render_resume_pi=tmp;
	
	AddParam("rdir","chdir to this directory before calling renderer",
		fi ? &fi->rdir : NULL,flags);
	AddParam("rifpattern","render input file pattern (e.g. f%07d.pov; "
		"can use %d,%x,%X) (relative to rdir)",
		fi ? &fi->rinfpattern : NULL,flags);
	AddParam("rofpattern","render output file pattern (e.g. f%07d; "
		"can use %d,%x,%X); extension added according to output "
		"format (relative to rdir)",
		fi ? &fi->routfpattern : NULL,flags);
	AddParam("rtimeout","render job time limit (seconds; -1 for none)",
		fi ? &fi->rtimeout : NULL,flags);
	
	AddParam("filter|fd","filter to use (filter DESC name)",
		fi ? &fi->ii->fdesc_string : NULL,flags);
	AddParam("fargs","additional args to be passed to the filter",
		fi ? &fi->fadd_args : NULL,flags);
	//AddParam("ffiles","additional files needed by the filtering process",
	//	fi ? &fi->fadd_files : NULL,flags);
	AddParam("fdir","chdir to this directory before calling filter",
		fi ? &fi->fdir : NULL,flags);
	AddParam("fofpattern","filter output file pattern (e.g. f%07d-f.png; "
		"can use %d,%x,%X) (relative to fdir)",
		fi ? &fi->foutfpattern : NULL,flags);
	AddParam("ftimeout","filter job time limit (seconds; -1 for none)",
		fi ? &fi->ftimeout : NULL,flags);
	
	if(add_failed)
	{
		RecursiveDeleteParams(CurrentSection());
		return(-1);
	}
	return(0);
}


// Called on program start to set up the TaskSourceFactory_Local. 
// TaskSourceFactory_Local registers at ComponentDataBase. 
// Return value: 0 -> OK; >0 -> error. 
int TaskSourceFactory_Local::init(ComponentDataBase *cdb)
{
	TaskSourceFactory_Local *s=NEW1<TaskSourceFactory_Local>(cdb);
	if(!s)
	{
		Error("Failed to initialize local task source.\n");
		return(1);
	}
	Verbose(BasicInit,"[local] ");
	return(0);
}


TaskSourceFactory_Local::TaskSourceFactory_Local(
	ComponentDataBase *cdb,int *failflag) : 
	TaskSourceFactory("local",cdb,failflag),
	par::SectionParameterHandler(cdb->parmanager(),failflag),
	master_fi(failflag),
	fi_list(failflag)
{
	nframes=-1;  // unlimited
	startframe=0;
	fjump=1;
	
	last_looked_up=NULL;
	
	cont_flag=false;
	
	response_delay=0;
	
	fi_topsect=NULL;
	_i_help_dummy=NULL;
	
	int failed=0;
	
	// Set up defaults: 
	master_fi.width=320;
	master_fi.height=240;
	if(master_fi.rinfpattern.set("f%07d.pov"))  ++failed;
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSourceFactory_Local");  }
}

TaskSourceFactory_Local::~TaskSourceFactory_Local()
{
	// Make sure, fi_list is cleared: 
	while(!fi_list.is_empty())
	{  delete fi_list.popfirst();  }
}


/******************************************************************************/

TaskSourceFactory_Local::PerFrameTaskInfo::PerFrameTaskInfo(int *failflag) : 
	LinkedListBase<PerFrameTaskInfo>(),
	// !!DONT FORGET THE COPY CONSTRUCTOR BELOW!!
	radd_args(failflag),
	rdir(failflag),
	rinfpattern(failflag),
	routfpattern(failflag),
	radd_files(failflag),
	fadd_args(failflag),
	fdir(failflag),
	foutfpattern(failflag),
	fadd_files(failflag)
{
	// This is always called with passed failflag: 
	assert(failflag);
	
	first_frame_no=-1;
	nframes=0;
	tobe_rendered=0;
	tobe_filtered=0;
	
	width=-1;
	height=-1;
	oformat=NULL;
	rdesc=NULL;
	render_resume_flag=true;
	rtimeout=-1;
	
	fdesc=NULL;
	ftimeout=-1;
	
	ii=NEW<PerFrameTaskInfo_Internal>();
	if(!ii)
	{  --(*failflag);  }
}

TaskSourceFactory_Local::PerFrameTaskInfo::PerFrameTaskInfo(
	PerFrameTaskInfo *c,int *failflag) : 
	// !!DONT FORGET THE NORMAL CONSTRUCTOR ABOVE!!
	LinkedListBase<PerFrameTaskInfo>(),
	radd_args(failflag),
	rdir(failflag),
	rinfpattern(failflag),
	routfpattern(failflag),
	radd_files(failflag),
	fadd_args(failflag),
	fdir(failflag),
	foutfpattern(failflag),
	fadd_files(failflag)
{
	// This is always called with passed failflag: 
	assert(failflag);
	int failed=0;
	
	assert(c->ii==NULL);  // Not copied. 
	ii=NULL;
	
	first_frame_no=c->first_frame_no;
	nframes=c->nframes;
	tobe_rendered=c->tobe_rendered;
	tobe_filtered=c->tobe_filtered;
	
	width=c->width;
	height=c->height;
	oformat=c->oformat;
	rdesc=c->rdesc;
	if(radd_args.append(&c->radd_args))  ++failed;
	rdir=c->rdir;
	rinfpattern=c->rinfpattern;
	routfpattern=c->routfpattern;
	render_resume_flag=c->render_resume_flag;
	rtimeout=c->rtimeout;
	if(radd_files.append(&c->radd_files))  ++failed;
	
	fdesc=c->fdesc;
	if(fadd_args.append(&c->fadd_args))  ++failed;
	fdir=c->fdir;
	foutfpattern=c->foutfpattern;
	ftimeout=c->ftimeout;
	if(fadd_files.append(&c->fadd_files))  ++failed;
	
	(*failflag)-=failed;
}

TaskSourceFactory_Local::PerFrameTaskInfo::~PerFrameTaskInfo()
{
	if(ii)
	{  delete ii;  ii=NULL;  }
	assert(!prev && !next);  // make sure we were dequeued. 
}


/******************************************************************************/

TaskSourceFactory_Local::PerFrameTaskInfo_Internal::PerFrameTaskInfo_Internal(int *failflag) :
	size_string(failflag),
	rdesc_string(failflag),
	oformat_string(failflag),
	fdesc_string(failflag)
{
	render_resume_pi=NULL;
}
