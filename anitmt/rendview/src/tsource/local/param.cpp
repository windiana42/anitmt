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
	if(fi==&master_fi && nframes<0)
	{
		// See if there are non-empty blocks following: 
		// (startframe-1 is a special value)
		if(next_pfbs<startframe)  return(NULL);  // no more tasks to do
	}
	
	return(fi);
}

// next_pfbs: saves next per-frame block start frame number (or startframe-1). 
// Only valid if return value is &master_fi. 
TaskSourceFactory_Local::PerFrameTaskInfo *TaskSourceFactory_Local::
	_DoGetPerFrameTaskInfo(int frame_no,int *next_pfbs)
{
	*next_pfbs=startframe-1;
	
	// NOTE!! last_looked_up may !!NEVER!! be master_fi. 
	
	// First, the fast path: 
	if(last_looked_up)
	{
		// Check if it is still the same info block: 
		bool above_start = (frame_no>=last_looked_up->first_frame_no);
		bool below_end = 
			(frame_no<(last_looked_up->first_frame_no+last_looked_up->nframes));
		if(above_start && below_end)
		{  return(last_looked_up);  }
		// Check neighbouring block: 
		if(above_start && last_looked_up->next && 
		   frame_no>=last_looked_up->next->first_frame_no && 
		   frame_no<(last_looked_up->next->first_frame_no+last_looked_up->next->nframes) )
		{  last_looked_up=last_looked_up->next;  return(last_looked_up);  }
		else if(below_end && last_looked_up->prev && 
		   frame_no>=last_looked_up->prev->first_frame_no && 
		   frame_no<(last_looked_up->prev->first_frame_no+last_looked_up->prev->nframes) )
		{  last_looked_up=last_looked_up->prev;  return(last_looked_up);  }
	}
	else if(!fi_list.first())
	{  return(&master_fi);  }
	
	#if 1  /* NEW METHOD: SLOWER BUT LESS ERROR-PRONE. */
	// Do a linear search: 
	// There are faster ways but they are error-prone. 
	PerFrameTaskInfo *special=NULL;
	for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
	{
		if(frame_no<fi->first_frame_no)   // gone too far
		{  special=fi;  break;  }
		else if(frame_no<(fi->first_frame_no+fi->nframes) )
		{  last_looked_up=fi;  return(fi);  }
	}
	if(special)  // There is a next block to come. 
	{  *next_pfbs=special->first_frame_no;  }
	#else  /* OLD METHOD: FASTER BUT MAYBE BUGGY. */
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
		fprintf(stderr,"<%d,%d %d:%d>\n",*next_pfbs,frame_no,
			last_looked_up->next->first_frame_no,last_looked_up->next->nframes);
		assert(*next_pfbs>frame_no);  OOPS...
	}
	#endif
	
	return(&master_fi);
}


// Check if the passed pattern is something containing EXACTLY ONE 
// %d - modifier, e.g. "%123d" or "%07x" or "%03X". 
// Return value: 0 -> OK; 1 -> error
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
	fi->set_flags|=SF_size;
	return(0);
}

int TaskSourceFactory_Local::_Param_ParseOutputFormat(PerFrameTaskInfo *fi)
{
	const char *str=fi->ii->oformat_string.str();
	if(!str)
	{  return(0);  }  // using default
	
	fi->oformat=component_db()->FindImageFormatByName(str);
	if(!fi->oformat)
	{
		Error("Local: %s: Image (output) format \"%s\" not recognized "
			"(try -list-imgfmt).\n",_FrameInfoLocationString(fi),str);
		return(1);
	}
	fi->set_flags|=SF_oformat;
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
	if(!fi->rdesc && strcmp(rstr,"none"))
	{
		Error("Local: %s: Unknown render DESC named \"%s\".\n",
			_FrameInfoLocationString(fi),rstr);
		return(1);
	}
	fi->set_flags|=SF_rdesc;
	
	if(!IsSet(fi->ii->render_resume_pi))
	{
		// Default: yes, if rdesc supports it: 
		if(fi==&master_fi)
		{  fi->render_resume_flag=fi->rdesc ? fi->rdesc->can_resume_render : 0;  }
		else
		{  fi->render_resume_flag=master_fi.render_resume_flag;  }
	}
	else
	{  fi->set_flags|=SF_r_resume_flag;  }
	
	return(0);
}	

int TaskSourceFactory_Local::_Param_ParseFilterDesc(PerFrameTaskInfo *fi)
{
	const char *fstr=fi->ii->fdesc_string.str();
	if(!fstr)
	{  return(0);  }   // using default or none
	
	// Parse filter to use: 
	fi->fdesc=component_db()->FindFilterDescByName(fstr);
	if(!fi->fdesc && strcmp(fstr,"none"))
	{
		Error("Local: %s: Unknown filter DESC named \"%s\".\n",
			_FrameInfoLocationString(fi),fstr);
		return(1);
	}
	fi->set_flags|=SF_fdesc;
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
	
	//if(fi->rdesc)
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
	
	//if(fi->fdesc)
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


// Return value: 0 -> deleted NULL ref; 1 -> NULL ref was not there
static int _DeleteStrListFirstNULLRef(RefStrList *l)
{
	if(l->first() && !l->first()->str())
	{
		delete l->popfirst();
		return(0);
	}
	return(1);
}
static void _AppendOverrideStrListArgs(RefStrList *l,RefStrList *m)
{
	if(l->first() && !l->first()->str())
	{
		// First NULL-ref is still there. Delete it and 
		// prepend list m: 
		delete l->popfirst();
		_CheckAllocFail(l->insert(m));
	}
}
static inline bool _StrListModified(RefStrList *l)
{
	// Not modified if it contains exactly one element which is a NULL ref. 
	return(!(l->first() && !l->first()->next && !l->first()->str()));
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


// Check if the per-frame block f0,n will ever be used (1) or not (0): 
inline int TaskSourceFactory_Local::_CheckWillUseFrameBlock(int f0,int n)
{
	return( !((nframes>=0 && f0>=(startframe+nframes)) || (f0+n)<startframe) );
}


// These are helper functions for _MergePerFrameBlock(): 
// Return value: -1 -> error; 0 -> no value for dest; 1 -> dest assigned 
int TaskSourceFactory_Local::_PFBMergePtr(const void **dest,
	const void *sa,bool set_a,const void *sb,bool set_b)
{
	if(!set_a && !set_b)  return(0);
	if(set_a && set_b && sa!=sb)  return(-1);
	*dest=(set_a ? sa : sb);
	return(1);
}
int TaskSourceFactory_Local::_PFBMergeString(RefString *dest,
	const RefString *sa,bool set_a,const RefString *sb,bool set_b)
{
	assert(sa && sb);
	
	if(!set_a && !set_b)  return(0);
	if(set_a && set_b && (*sa)!=(*sb))  return(-1);
	*dest=(set_a ? *sa : *sb);
	return(1);
}
int TaskSourceFactory_Local::_PFBMergeStrList(RefStrList *dest,
	const RefStrList *sa,bool set_a,const RefStrList *sb,bool set_b)
{
	assert(sa && sb);
	
	if(!set_a && !set_b)  return(0);
	if(set_a && set_b && !sa->exact_compare(sb))  return(-1);
	dest->clear();  // be sure...
	_CheckAllocFail(dest->append(set_a ? sa : sb));
	return(1);
}
int TaskSourceFactory_Local::_PFBMergeInt(long *dest,
	long sa,bool set_a,long sb,bool set_b)
{
	if(!set_a && !set_b)  return(0);
	if(set_a && set_b && sa!=sb)  return(-1);
	*dest=(set_a ? sa : sb);
	return(1);
}
int TaskSourceFactory_Local::_PFBMergeBool(bool *dest,
	bool sa,bool set_a,bool sb,bool set_b)
{
	if(!set_a && !set_b)  return(0);
	if(set_a && set_b && sa!=sb)  return(-1);
	*dest=(set_a ? sa : sb);
	return(1);
}
int TaskSourceFactory_Local::_PFBMergeSize(int *dest_w,int *dest_h,
	int sa_w,int sa_h,bool set_a,int sb_w,int sb_h,bool set_b)
{
	if(!set_a && !set_b)  return(0);
	if(set_a && set_b && (sa_w!=sb_w || sa_h!=sb_h))  return(-1);
	*dest_w=(set_a ? sa_w : sb_w);
	*dest_h=(set_a ? sa_h : sb_h);
	return(1);
}

static inline void _PFBMergeHelper(int merge_retval,int flag,
	int *failed,int *set_flags)
{
	if(merge_retval<0)
	{  *failed|=flag;  }
	else if(merge_retval>0)
	{  *set_flags|=flag;  }
}

// Merge sources *sa and *sb into *dest. 
// {dest,sa,sb}->ii does not get merged or interpreted in any way. 
// Return value: 0 -> OK; else: SF_* mask of failed params 
int TaskSourceFactory_Local::_MergePerFrameBlock(PerFrameTaskInfo *dest,
	const PerFrameTaskInfo *sa,const PerFrameTaskInfo *sb)
{
	assert(dest && sa && sb);
	// In the current concept, the fi->ii stuff should be NULL here. 
	assert(!dest->ii && !sa->ii && !sb->ii);
	
	// NOTE: dest->first_frame_no, dest->nframes is not set here. 
	
	int failed=0;
	dest->set_flags=0;  // be sure...
	
	_PFBMergeHelper(_PFBMergeSize(&dest->width,&dest->height,
		sa->width,sa->height,sa->set_flags & SF_size,
		sb->width,sb->height,sb->set_flags & SF_size),
		SF_size,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergePtr((const void**)&dest->oformat,
		sa->oformat,sa->set_flags & SF_oformat,
		sb->oformat,sb->set_flags & SF_oformat),
		SF_oformat,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergePtr((const void**)&dest->rdesc,
		sa->rdesc,sa->set_flags & SF_rdesc,
		sb->rdesc,sb->set_flags & SF_rdesc),
		SF_rdesc,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeStrList(&dest->radd_args,
		&sa->radd_args,sa->set_flags & SF_radd_args,
		&sb->radd_args,sb->set_flags & SF_radd_args),
		SF_radd_args,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeString(&dest->rdir,
		&sa->rdir,sa->set_flags & SF_rdir,
		&sb->rdir,sb->set_flags & SF_rdir),
		SF_rdir,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeString(&dest->rinfpattern,
		&sa->rinfpattern,sa->set_flags & SF_rinfpattern,
		&sb->rinfpattern,sb->set_flags & SF_rinfpattern),
		SF_rinfpattern,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeString(&dest->routfpattern,
		&sa->routfpattern,sa->set_flags & SF_routfpattern,
		&sb->routfpattern,sb->set_flags & SF_routfpattern),
		SF_routfpattern,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeBool(&dest->render_resume_flag,
		sa->render_resume_flag,sa->set_flags & SF_r_resume_flag,
		sb->render_resume_flag,sb->set_flags & SF_r_resume_flag),
		SF_r_resume_flag,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeInt(&dest->rtimeout,
		sa->rtimeout,sa->set_flags & SF_rtimeout,
		sb->rtimeout,sb->set_flags & SF_rtimeout),
		SF_rtimeout,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeStrList(&dest->radd_files,
		&sa->radd_files,sa->set_flags & SF_radd_files,
		&sb->radd_files,sb->set_flags & SF_radd_files),
		SF_radd_files,&failed,&dest->set_flags);
	
	_PFBMergeHelper(_PFBMergePtr((const void**)&dest->fdesc,
		sa->fdesc,sa->set_flags & SF_fdesc,
		sb->fdesc,sb->set_flags & SF_fdesc),
		SF_fdesc,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeStrList(&dest->fadd_args,
		&sa->fadd_args,sa->set_flags & SF_fadd_args,
		&sb->fadd_args,sb->set_flags & SF_fadd_args),
		SF_fadd_args,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeString(&dest->fdir,
		&sa->fdir,sa->set_flags & SF_fdir,
		&sb->fdir,sb->set_flags & SF_fdir),
		SF_fdir,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeString(&dest->foutfpattern,
		&sa->foutfpattern,sa->set_flags & SF_foutfpattern,
		&sb->foutfpattern,sb->set_flags & SF_foutfpattern),
		SF_foutfpattern,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeInt(&dest->ftimeout,
		sa->ftimeout,sa->set_flags & SF_ftimeout,
		sb->ftimeout,sb->set_flags & SF_ftimeout),
		SF_ftimeout,&failed,&dest->set_flags);
	_PFBMergeHelper(_PFBMergeStrList(&dest->fadd_files,
		&sa->fadd_files,sa->set_flags & SF_fadd_files,
		&sb->fadd_files,sb->set_flags & SF_fadd_files),
		SF_fadd_files,&failed,&dest->set_flags);
	
	return(failed);
}


// InternalInfo (fi->ii) is not compared. 
// first_frame_no and nframes are NOT compared. 
// Return value: operator==(), i.e. true if equal.
bool TaskSourceFactory_Local::_ComparePerFrameInfo(
	const PerFrameTaskInfo *a,const PerFrameTaskInfo *b)
{
	assert(!a->ii && !b->ii);
	
	// first_frame_no and nframes are NOT compared. 
	// set_flags are not compared, of course, because they are 
	// not interesting. 
	return(
		a->width == b->width &&
		a->height == b->height &&
		a->oformat == b->oformat &&
		a->rdesc == b->rdesc &&
		a->radd_args.exact_compare(&b->radd_args) &&
		a->rdir == b->rdir &&
		a->rinfpattern == b->rinfpattern &&
		a->routfpattern == b->routfpattern &&
		a->render_resume_flag == b->render_resume_flag &&
		a->rtimeout == b->rtimeout &&
		a->radd_files.exact_compare(&b->radd_files) &&

		a->fdesc == b->fdesc &&
		a->fadd_args.exact_compare(&b->fadd_args) &&
		a->fdir == b->fdir &&
		a->foutfpattern == b->foutfpattern &&
		a->ftimeout == b->ftimeout &&
		a->fadd_files.exact_compare(&b->fadd_files) );
}


// Fill in the set_flags for all vars not corresponding to 
// param args in fi->ii (for those, this is done by the parsing 
// functions). 
void TaskSourceFactory_Local::_FillIn_SetFlags(PerFrameTaskInfo *fi)
{
	// Note... radd_args and fadd_args contain one NULL ref by 
	//         default to see if they were modified. Delete that. 
	if(_StrListModified(&fi->radd_args))
	{  fi->set_flags|=SF_radd_args;  }
	if(_StrListModified(&fi->radd_files))
	{  fi->set_flags|=SF_radd_files;  }
	if(_StrListModified(&fi->fadd_args))
	{  fi->set_flags|=SF_fadd_args;  }
	if(_StrListModified(&fi->fadd_files))
	{  fi->set_flags|=SF_fadd_files;  }
	
	if(fi->rtimeout!=UnsetNegMagic)
	{  fi->set_flags|=SF_rtimeout;  }
	if(fi->ftimeout!=UnsetNegMagic)
	{  fi->set_flags|=SF_ftimeout;  }
	
	// The "!!"-thingy is due to the fact that I use RefString::operator!(). 
	if(!!fi->rdir)  // KEEP "!!"
	{  fi->set_flags|=SF_rdir;  }
	if(!!fi->fdir)  // KEEP "!!"
	{  fi->set_flags|=SF_fdir;  }
	
	if(!!fi->rinfpattern)  // KEEP "!!"
	{  fi->set_flags|=SF_rinfpattern;  }
	if(!!fi->routfpattern)  // KEEP "!!"
	{  fi->set_flags|=SF_routfpattern;  }
	if(!!fi->foutfpattern)  // KEEP "!!"
	{  fi->set_flags|=SF_foutfpattern;  }
}


static inline const char *_BaseNamePtr(const char *str)
{
	const char *ptr=strrchr(str,'/');
	return(ptr ? (ptr+1) : str);
}

int TaskSourceFactory_Local::_FixupAdditionalFileSpec(RefStrList *add_files,
	PerFrameTaskInfo *fi,TaskDriverType dtype)
{
	// Delete null-entries: 
	for(const RefStrList::Node *_i=add_files->first(); _i; )
	{
		const RefStrList::Node *i=_i;
		_i=_i->next;
		
		if(!i->str() || !i->len())
		{  delete add_files->dequeue((RefStrList::Node*)i);  }
	}

	// See if an additional file is mentioned twice. 
	for(const RefStrList::Node *i=add_files->first(); i; i=i->next)
	{
		const char *in=_BaseNamePtr(i->str());
		int warn=0;
		for(const RefStrList::Node *_j=i->next; _j; )
		{
			const RefStrList::Node *j=_j;
			_j=_j->next;
			
			const char *jn=_BaseNamePtr(j->str());
			if(strcmp(in,jn))  continue;
			++warn;
			delete add_files->dequeue((RefStrList::Node*)j);
		}
		if(warn)
		{
			Warning("Local: %s: %d additional %s files with "
				"basename \"%s\". Keeping only \"%s\".\n",
				_FrameInfoLocationString(fi),warn+1,DTypeString(dtype),
				in,i->str());
		}
	}
	
	return(0);
}


int TaskSourceFactory_Local::_FixupPerFrameBlock(PerFrameTaskInfo *fi)
{
	if(fi->rdesc && fi->render_resume_flag && !fi->rdesc->can_resume_render)
	{
		Warning("Local: %s: Disabled render resume feature (-rcont) "
			"(no support by renderer / %s driver).",
			_FrameInfoLocationString(fi),
			fi->rdesc ? fi->rdesc->dfactory->DriverName() : "[none]");
		fi->render_resume_flag=false;
	}
	
	if(fi->rtimeout<-1)  {  fi->rtimeout=-1;  }
	if(fi->ftimeout<-1)  {  fi->ftimeout=-1;  }
	
	int errors=0;
	errors+=_FixupAdditionalFileSpec(&fi->radd_files,fi,DTRender);
	errors+=_FixupAdditionalFileSpec(&fi->fadd_files,fi,DTFilter);
	
	return(0);
}

// Insert passed PerFrameTaskInfo (which may NOT be in the fi_list) 
// at the correct position so that the list stays sorted. 
inline void TaskSourceFactory_Local::_InsertPFIAtCorrectPos(
	PerFrameTaskInfo *fi)
{
	// Could be optimized because during per-frame block splitting/merging 
	// we know the position pretty well (but not exactly). 
	for(PerFrameTaskInfo *i=fi_list.first(); i; i=i->next)
	{
		if(i->first_frame_no<fi->first_frame_no)  continue;
		if(i->first_frame_no==fi->first_frame_no && 
		   i->nframes<=fi->nframes)  continue;
		fi_list.queuebefore(fi,i);
		return;
	}
	fi_list.append(fi);
}


int TaskSourceFactory_Local::_DeleteNeverUsedPerFrameBlocks()
{
	int never_used_warned=0;
	for(PerFrameTaskInfo *_fi=fi_list.first(); _fi; )
	{
		PerFrameTaskInfo *fi=_fi;
		_fi=_fi->next;

		if(_CheckWillUseFrameBlock(fi->first_frame_no,fi->nframes))
		{  continue;  }

		if(!never_used_warned)
		{  Verbose(TSP,"Local: Deleting never used per-frame info "
			"blocks: %d:%d",fi->first_frame_no,fi->nframes);  }
		else
		{  Verbose(TSP," %d:%d",fi->first_frame_no,fi->nframes);  }
		++never_used_warned;

		delete fi_list.dequeue(fi);
	}
	if(never_used_warned)
	{  Verbose(TSP," [%d blocks]\n",never_used_warned);  }
	return(never_used_warned);
}


int TaskSourceFactory_Local::FinalInit()
{
	int mfailed=0;  // "master failed"
	
	// MASTER FRAME INFO...
	_FillIn_SetFlags(&master_fi);
	
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
		
		mfailed+=_FixupPerFrameBlock(&master_fi);
	}
	
	if(mfailed)
	{  Verbose(TSP,"Local: Errors in master frame info spec; "
		"skipping per-frame info.\n");  }
	int failed=mfailed;
	
	// Get rid of internal info for param system: 
	delete master_fi.ii;
	master_fi.ii=NULL;
	
	// Okay, get rid of per-frame info which will never be used. 
	// AND, sort it in ascenting order. 
	if(!failed)
	{
		// First, eliminite all list elements with illegal range entries 
		// (and those which will never be used): 
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
			
			continue;
			cfailed:  ++failed;
			cdelete:  delete fi_list.dequeue(fi);
		}
		
		_DeleteNeverUsedPerFrameBlocks();
		
		// Then, sort them: 
		LinkedList<PerFrameTaskInfo> srclist;
		fi_list.swap_list(&srclist);
		for(;;)
		{
			PerFrameTaskInfo *fi=srclist.popfirst();
			if(!fi)  break;
			
			// Insert at correct position: 
			_InsertPFIAtCorrectPos(fi);
		}
	}
	
	if(!failed)
	{
		// PER-FRAME FRAME INFO...
		// Get rid of the fi->ii stuff early; it's only used for arg 
		// parsing: 
		// Also check if some params/opts were set or not. 
		for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
		{
			_FillIn_SetFlags(fi);
			
			// These set the apropriate set_flags (SF_*): 
			failed+=_Param_ParseOutputFormat(fi);
			failed+=_Param_ParseRenderDesc(fi,0);
			failed+=_Param_ParseFilterDesc(fi);
			failed+=_Param_ParseInSizeString(fi);
			
			// Delete unneeded internal info: 
			if(fi->ii->section)
			{  RecursiveDeleteParams(fi->ii->section);  }
			delete fi->ii;  fi->ii=NULL;
		}
	}
	
	// Okay, now, the fi->ii stuff now longer exists and we did set 
	// up sf->flags. Best time to do the per-frame block splitting/merging. 
	if(!failed)
	{
		// Good -- check for overlapping blocks and split/merge them: 
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
				// s_type: 
				//   type 0: xxxxxxxx   |  type 1: xxxxxxxx
				//             xxxx     |             xxxxxxx
				PerFrameTaskInfo *fi_next=fi->next;
				int s_type=(fi->first_frame_no+fi->nframes < 
						fi_next->first_frame_no+fi_next->nframes);
				// There can be up to three resulting per-frame blocks: 
				int s0_f0=fi->first_frame_no;
				int s0_n=fi_next->first_frame_no-fi->first_frame_no;
				int s1_f0=fi_next->first_frame_no;
				int s1_n = s_type ? 
					(fi->first_frame_no+fi->nframes-fi_next->first_frame_no) : 
					fi_next->nframes;
				int s2_f0 = s_type ? (fi->first_frame_no+fi->nframes) : 
					(fi_next->first_frame_no+fi_next->nframes);
				int s2_n = (fi_next->first_frame_no+fi_next->nframes-
						(fi->first_frame_no+fi->nframes)) * (s_type ? 1 : (-1));
				int err_flags=0;
				Verbose(TSP,"Local: %s: Per-frame info overlaps with %d:%d; "
					"%s into:",
					_FrameInfoLocationString(fi),
					fi_next->first_frame_no,fi_next->nframes,
					(s0_n<=0 && s2_n<=0) ? "merging" : "splitting");
				// We do not generate blocks which will never ne used. 
				if(s1_n>0 && _CheckWillUseFrameBlock(s1_f0,s1_n))
				{
					Verbose(TSP," %d:%d",s1_f0,s1_n);
					
					PerFrameTaskInfo *s1=NEW<PerFrameTaskInfo>();
					_CheckAllocFail(!s1);
					delete s1->ii;  s1->ii=NULL;
					
					// Okay, actually merge it: 
					s1->first_frame_no=s1_f0;
					s1->nframes=s1_n;
					err_flags=_MergePerFrameBlock(s1,fi,fi_next);
					// Report error if one occured. 
					if(err_flags)
					{
						Verbose(TSP," <- failed\n");
						Error("Local: Conflicts merging per-frame blocks "
							"%d:%d and %d:%d:",
							fi->first_frame_no,fi->nframes,
							fi_next->first_frame_no,fi_next->nframes);
						for(int i=1; i<SF_ALL_FLAGS; i<<=1)
						{
							if(err_flags & i)
							{  Error(" %s",SF_FlagString(i));  }
						}
						Error("\n");
						
						++failed;
					}
					
					_InsertPFIAtCorrectPos(s1);
				}
				// DONT CHANGE THIS ORDER. 
				if(s2_n>0 && _CheckWillUseFrameBlock(s2_f0,s2_n))
				{
					if(!err_flags)  Verbose(TSP," %d:%d",s2_f0,s2_n);
					if(s_type)
					{
						fi_next->first_frame_no=s2_f0;
						fi_next->nframes=s2_n;
					}
					else
					{
						// s2 as copy of fi. 
						PerFrameTaskInfo *s2=NEW1<PerFrameTaskInfo>(fi);
						_CheckAllocFail(!s2);
						delete s2->ii;  s2->ii=NULL;
						
						s2->first_frame_no=s2_f0;
						s2->nframes=s2_n;
						
						_InsertPFIAtCorrectPos(s2);
						delete fi_list.dequeue(fi_next);
					}
				}
				else
				{  delete fi_list.dequeue(fi_next);  }
				// DONT CHANGE THIS ORDER. 
				_fi=fi->prev;  // May be NULL, yes... (see below)
				if(s0_n>0 && _CheckWillUseFrameBlock(s0_f0,s0_n))
				{
					if(!err_flags)  Verbose(TSP," %d:%d",s0_f0,s0_n);
					fi->first_frame_no=s0_f0;
					fi->nframes=s0_n;
				}
				else
				{  delete fi_list.dequeue(fi);  }
				if(!err_flags)  Verbose(TSP,"\n");
				if(!_fi)  _fi=fi_list.first();
				
				// Dump list; very useful in debugging. 
				//fprintf(stderr,"LIST:");
				//for(PerFrameTaskInfo *i=fi_list.first(); i; i=i->next)
				//{  fprintf(stderr," %d:%d",i->first_frame_no,i->nframes);  }
				//fprintf(stderr,"\n");
			}
		}
	}
	
	// Now, after the splitting/merging, we have more per-frame 
	// blocks set up just as if the user had specified them without 
	// any overlapping conditions. Go on setting defaults from the 
	// master frame block. 
	if(!failed)	
	{
		// Note... radd_args and fadd_args contain one NULL ref by 
		//         default to see if they were modified. Delete that 
		//         (NULL ref still there -> append; else: override).
		_DeleteStrListFirstNULLRef(&master_fi.radd_args);
		_DeleteStrListFirstNULLRef(&master_fi.radd_files);
		_DeleteStrListFirstNULLRef(&master_fi.fadd_args);
		_DeleteStrListFirstNULLRef(&master_fi.fadd_files);
		
		// Okay, now set up the rest of the members in the per-frame 
		// info structures: 
		for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
		{
			// Per-frame blocks may no longer overlap. If this fails, there is 
			// a bug in the split/merge code above. 
			assert(!fi->next || fi->first_frame_no+fi->nframes<=fi->next->first_frame_no);
			// Also, they have to stay sorted. 
			assert(!fi->next || fi->next->first_frame_no>=fi->first_frame_no);
			
			// Set "defaults" from master frame info if needed: 
			if(!(fi->set_flags & SF_oformat)) fi->oformat=master_fi.oformat;
			if(!(fi->set_flags & SF_rdesc))   fi->rdesc=master_fi.rdesc;
			if(!(fi->set_flags & SF_r_resume_flag))   // Hmm.... hm...
				fi->render_resume_flag=master_fi.render_resume_flag;
			if(!(fi->set_flags & SF_fdesc))   fi->fdesc=master_fi.fdesc;
			
			if(!(fi->set_flags & SF_size))
			{  fi->width=master_fi.width;  fi->height=master_fi.height;  }
			
			// Check frame patterns (and use defaults from master 
			// if needed): 
			if(!(fi->set_flags & SF_rinfpattern))
			{  fi->rinfpattern=master_fi.rinfpattern;  }
			int pfailed=_CheckFramePattern(&fi->rinfpattern,"input",fi);
			failed+=pfailed;
			if(!pfailed)
			{
				// Check other patterns or assign reasonable defaults: 
				failed+=_SetUpAndCheckOutputFramePatterns(fi);
			}
			
			if(!(fi->set_flags & SF_rdir))  fi->rdir=master_fi.rdir;
			else  _PathAppendSlashIfNeeded(&fi->rdir);
			if(!(fi->set_flags & SF_fdir))  fi->fdir=master_fi.fdir;
			else  _PathAppendSlashIfNeeded(&fi->fdir);
			
			if(fi->set_flags & SF_rtimeout)
			{  ConvertTimeout2MSec(&fi->rtimeout,_ltsrt_str);  }
			else
			{  fi->rtimeout=master_fi.rtimeout;  }  // master_timeout already in msec
			if(fi->set_flags & SF_ftimeout)
			{  ConvertTimeout2MSec(&fi->ftimeout,_ltsft_str);  }
			else
			{  fi->ftimeout=master_fi.ftimeout;  }  // master_timeout already in msec
			
			// If the first NULL-string is still there, we prepend 
			// the master params, else we override. 
			_AppendOverrideStrListArgs(&fi->radd_args,&master_fi.radd_args);
			_AppendOverrideStrListArgs(&fi->radd_files,&master_fi.radd_files);
			_AppendOverrideStrListArgs(&fi->fadd_args,&master_fi.fadd_args);
			_AppendOverrideStrListArgs(&fi->fadd_files,&master_fi.fadd_files);
		}
	}
	
	if(!failed)
	{
		// Tidy up a bit and do some checks: 
		for(PerFrameTaskInfo *_fi=fi_list.first(); _fi; )
		{
			PerFrameTaskInfo *fi=_fi;
			_fi=_fi->next;
			
			// NOTE: We will NOT delete fdesc=rdesc=NULL blocks UNLESS the 
			//       master block is also fdesc=rdesc=NULL. This allows to 
			//       specify blocks which shall not be processed. 
			if(!fi->rdesc && !fi->fdesc && 
			   !master_fi.rdesc && !master_fi.fdesc)
			{
				delete fi_list.dequeue(fi);
				continue;
			}
			
			assert(!fi->ii);
			
			failed+=_FixupPerFrameBlock(fi);
		}
		
		// Okay, merge blocks if we can: 
		int cnt=0;
		for(PerFrameTaskInfo *_fi=fi_list.first(); _fi && _fi->next; )
		{
			PerFrameTaskInfo *fi=_fi;
			_fi=_fi->next;
			
			if(fi->first_frame_no+fi->nframes!=fi->next->first_frame_no || 
			   !_ComparePerFrameInfo(fi,fi->next))
			{  continue;  }
			
			fi->nframes+=fi->next->nframes;
			delete fi_list.dequeue(fi->next);
			++cnt;
			
			_fi=fi->prev;
			if(!_fi)  _fi=fi_list.first();
		}
		if(cnt)
		{  Verbose(TSP,"Local: Merged equal per-frame info blocks "
			"(%d merged).\n",cnt);  }
		
		// Merge per-frame blocks with master if we can: 
		int warned=0;
		for(PerFrameTaskInfo *_fi=fi_list.first(); _fi; )
		{
			PerFrameTaskInfo *fi=_fi;
			_fi=_fi->next;
			
			if(!_ComparePerFrameInfo(fi,&master_fi))  continue;
			
			if(!warned)
			{  Verbose(TSP,"Local: Merging per-frame info blocks with "
				"master: %d:%d",fi->first_frame_no,fi->nframes);  }
			else
			{  Verbose(TSP," %d:%d",fi->first_frame_no,fi->nframes);  }
			++warned;
			
			delete fi_list.dequeue(fi);
		}
		if(warned)
		{  Verbose(TSP,"\n");  }
		
		// This should not be necessary here, but it won't hurt a lot: 
		if(_DeleteNeverUsedPerFrameBlocks())
		{  Error("OOPS! This should not happen. (Please bugreport)\n");  }
	}
	
	// Puh, per-frame block processing is now done. 
	
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
	
	// Dump useful info to the user: 
	if(!failed)
	{
		char nf_tmp[24];
		if(nframes>=0)  snprintf(nf_tmp,24,"%d",nframes);
		else  strcpy(nf_tmp,"[unlimited]");
		Verbose(TSI,"Local task source: jump: %d; nframes: %s; startframe: %d\n",
			fjump,nf_tmp,startframe);
		Verbose(TSI,"  Continuing: %s\n",
			cont_flag ? "yes" : "no");
		
		Verbose(TSI,"  Master frame info (complete dump):\n");
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
		// Okay, there was a `#section xyz´ ot sth like that. 
		int illegal=0;
		int saw_colon=0;
		for(const char *c=info->nend; c<info->name_end; c++)
		{
			//if(*c=='-' || isspace(*c))
			if(*c==':')
			{  ++saw_colon;  }
			else if(!isdigit(*c))
			{  ++illegal;  break;  }
		}
		if(saw_colon!=1)  ++illegal;
		xname=info->nend;
		xnamelen=info->name_end-xname;
		if(info->name_end<=info->nend || illegal)
		{
			Error("%s: in %s: illegal per-frame section spec \"%.*s\".\n",
				prg_name,info->origin->OriginStr().str(),
				info->name_end>xname ? info->name_end-xname : 10,xname);
			return(-1);
		}
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
	
	Verbose(DBG,"Adding new per-frame block %d:%d (last frame: %d).\n",
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
	AddParam("r-args","additional args to be passed to the renderer",
		fi ? &fi->radd_args : NULL,flags);
	AddParam("r-files","additional files needed by the rendering process",
		fi ? &fi->radd_files : NULL,flags);
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
	
	AddParam("r-dir","chdir to this directory before calling renderer",
		fi ? &fi->rdir : NULL,flags);
	AddParam("r-ifpattern","render input file pattern (e.g. f%07d.pov; "
		"can use %d,%x,%X) (relative to rdir)",
		fi ? &fi->rinfpattern : NULL,flags);
	AddParam("r-ofpattern","render output file pattern (e.g. f%07d; "
		"can use %d,%x,%X); extension added according to output "
		"format (relative to rdir)",
		fi ? &fi->routfpattern : NULL,flags);
	AddParam("r-timeout","render job time limit (seconds; -1 for none)",
		fi ? &fi->rtimeout : NULL,flags);
	
	AddParam("filter|fd","filter to use (filter DESC name)",
		fi ? &fi->ii->fdesc_string : NULL,flags);
	AddParam("f-args","additional args to be passed to the filter",
		fi ? &fi->fadd_args : NULL,flags);
	AddParam("f-files","additional files needed by the filtering process",
		fi ? &fi->fadd_files : NULL,flags);
	AddParam("f-dir","chdir to this directory before calling filter",
		fi ? &fi->fdir : NULL,flags);
	AddParam("f-ofpattern","filter output file pattern (e.g. f%07d-f.png; "
		"can use %d,%x,%X) (relative to fdir)",
		fi ? &fi->foutfpattern : NULL,flags);
	AddParam("f-timeout","filter job time limit (seconds; -1 for none)",
		fi ? &fi->ftimeout : NULL,flags);
	
	if(add_failed)
	{
		RecursiveDeleteParams(CurrentSection());
		return(-1);
	}
	return(0);
}

const char *TaskSourceFactory_Local::SF_FlagString(int flag)
{
	switch(flag)
	{
		case SF_size:          return("size");
		case SF_oformat:       return("oformat");
		case SF_rdesc:         return("renderer");
		case SF_radd_args:     return("rargs");
		case SF_rdir:          return("rdir");
		case SF_rinfpattern:   return("rifpattern");
		case SF_routfpattern:  return("rofpattern");
		case SF_r_resume_flag: return("rcont");
		case SF_rtimeout:      return("rtimeout");
		case SF_radd_files:    return("rfiles");
		case SF_fdesc:         return("filter");
		case SF_fadd_args:     return("fargs");
		case SF_fdir:          return("fdir");
		case SF_foutfpattern:  return("fofpattern");
		case SF_ftimeout:      return("ftimeout");
		case SF_fadd_files:    return("ffiles");
	}
	return("???");
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
	// !! DONT FORGET THE COPY CONSTRUCTOR BELOW AND _MergePerFrameBlock() !!
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
	set_flags=0;
	
	width=-1;
	height=-1;
	oformat=NULL;
	rdesc=NULL;
	render_resume_flag=true;
	rtimeout=UnsetNegMagic;
	
	fdesc=NULL;
	ftimeout=UnsetNegMagic;
	
	// Append NULL ref (detection if params have to be appended). 
	RefString nullref;
	if(radd_args.append(nullref))   {  --(*failflag);  }
	if(radd_files.append(nullref))  {  --(*failflag);  }
	if(fadd_args.append(nullref))   {  --(*failflag);  }
	if(fadd_files.append(nullref))  {  --(*failflag);  }
	
	ii=NEW<PerFrameTaskInfo_Internal>();
	if(!ii)
	{  --(*failflag);  }
}

TaskSourceFactory_Local::PerFrameTaskInfo::PerFrameTaskInfo(
	PerFrameTaskInfo *c,int *failflag) : 
	// !! DONT FORGET THE NORMAL CONSTRUCTOR ABOVE AND _MergePerFrameBlock() !!
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
	set_flags=c->set_flags;
	
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
