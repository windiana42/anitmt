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
			if(frame_no>=(fi->first_frame_no+fi->nframes))
			{  return(&master_fi);  }
			return(fi);
		}
	}
	else // frame_no>=(last_looked_up->first_frame_no+last_looked_up->nframes)
	{
		for(PerFrameTaskInfo *fi=last_looked_up->next; fi; fi=fi->next)
		{
			if(frame_no>=(fi->first_frame_no+fi->nframes))  continue;
			if(frame_no<fi->first_frame_no)
			{
				last_looked_up=fi->prev;
				return(&master_fi);
			}
			last_looked_up=fi;
			return(fi);
		}
	}
	return(&master_fi);
}


// Check if the passed pattern is something containing EXACTLY ONE 
// %d - modifier, e.g. "%123d" or "%07x". 
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
		if(*ptr!='x' && *ptr!='d')
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


// Only print info if it is different from the one in compare_to. 
// Pass compare_to=NULL to get it all dumped. 
void TaskSourceFactory_Local::_VPrintFrameInfo(PerFrameTaskInfo *fi,
	const PerFrameTaskInfo *compare_to)
{
	if(!compare_to || 
	   (fi->rdesc!=compare_to->rdesc || fi->oformat!=compare_to->oformat) )
	{
		Verbose("    Renderer: ");
		if(fi->rdesc)
		{  Verbose("%s (%s driver)",fi->rdesc->name.str(),
			fi->rdesc->dfactory->DriverName());  }
		else
		{  Verbose("[not rendering]");  }
		// YES! It makes sense to dump oformat if renderer is unspecified. 
		// Because it is used e.g. for frame pattern names. 
		Verbose("; output format: ");
		if(fi->oformat)
		{  Verbose("%s (%d bpp)\n",fi->oformat->name,fi->oformat->bitspp);  }
		else
		{  Verbose("[unspecified]\n");  }
	}
	
	if(fi->rdesc)
	{
		if(!compare_to || 
		   (fi->render_resume_flag!=compare_to->render_resume_flag || 
		    fi->width!=compare_to->width || fi->height!=compare_to->height ) )
		{  Verbose("      Size: %dx%d; cont operation: %s\n",
			fi->width,fi->height,
			fi->render_resume_flag ? 
				"resume (-rcont)" : "re-render (-no-rcont)");  }
		if(!compare_to || fi->rdir!=compare_to->rdir)
		{  Verbose("      Render dir: %s\n",fi->rdir.str() ? fi->rdir.str() : "[cwd]");  }
		#warning radd_args not dumped. 
	}
	
	if(!compare_to || 
	   (fi->fdesc!=compare_to->fdesc) )
	{
		Verbose("    Filter: ");
		if(fi->fdesc)
		{  Verbose("%s (%s driver)\n",fi->fdesc->name.str(),
			fi->fdesc->dfactory->DriverName());  }
		else
		{  Verbose("[not filtering]\n");  }
	}
	
	if(fi->fdesc)
	{
		if(!compare_to || fi->fdir!=compare_to->fdir)
		{  Verbose("      Filter dir: %s\n",fi->fdir.str() ? fi->fdir.str() : "[cwd]");  }
		#warning fadd_args not dumped. 
	}
	
	if(!compare_to || 
	   (fi->rinfpattern!=compare_to->rinfpattern || 
	    fi->routfpattern!=compare_to->routfpattern || 
	    fi->foutfpattern!=compare_to->foutfpattern ) )
	{
		Verbose("    Frame patterns: render: \"%s\" -> \"%s\"\n",
			fi->rinfpattern.str(),fi->routfpattern.str());
		Verbose("                    filter: ... -> \"%s\"\n",
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
	// render output: 
	if(!master_fi.routfpattern.str())
	{
		// No output pattern. 
		// Use input pattern without extension: 
		const char *ip=master_fi.rinfpattern.str();
		const char *es=strrchr(ip,'.');
		if(es)
		{
			// Make sure we're right of the `%´: 
			const char *pp=strrchr(ip,'%');
			if(es<pp)  es=NULL;
		}
		if(!es)  es=ip+strlen(ip);
		_CheckAllocFail(master_fi.routfpattern.set0(ip,es-ip));
	}
	mfailed+=_CheckFramePattern(&master_fi.routfpattern,
		"render output",&master_fi);
	// NOTE: The extension is appended below so that we can use it without 
	//       extension for the foutfpattern. 
	// filter output: 
	if(!master_fi.foutfpattern.str())
	{
		// No output pattern. 
		// Use render output pattern and append -f. 
		master_fi.foutfpattern=master_fi.routfpattern;
		assert(master_fi.oformat);  // MUST have been set above. ALWAYS. 
		char ext_tmp[strlen(master_fi.oformat->file_extension)+3+1];
		strcpy(ext_tmp,"-f.");
		strcpy(ext_tmp+3,master_fi.oformat->file_extension);
		_CheckAllocFail(master_fi.foutfpattern.append(ext_tmp));
	}
	mfailed+=_CheckFramePattern(&master_fi.foutfpattern,
		"filter output",&master_fi);
	// Okay, now append extension to render output file pattern: 
	assert(master_fi.oformat);  // MUST have been set above. ALWAYS. 
	{
		char ext_tmp[strlen(master_fi.oformat->file_extension)+1+1];
		ext_tmp[0]='.';
		strcpy(ext_tmp+1,master_fi.oformat->file_extension);
		_CheckAllocFail(master_fi.routfpattern.append(ext_tmp));
	}
	
	if(!mfailed)
	{
		// Only reason for these assertion to fail would be alloc failure or 
		// internal error. 
		assert(master_fi.rinfpattern.str());
		assert(master_fi.routfpattern.str());
		assert(master_fi.foutfpattern.str());
	}
	
	if(mfailed)
	{  Verbose("Local: Errors in master frame info spec; "
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
			
			if((nframes>=0 && fi->first_frame_no>=(startframe+nframes)) || 
			   (fi->first_frame_no+fi->nframes)<startframe )
			{  ++never_used;  goto cdelete;  }
			
			continue;
			cfailed:  ++failed;
			cdelete:  delete fi_list.dequeue(fi);
		}
		if(never_used)
		{  Verbose("Local: Deleted %d per-frame info blocks as they "
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
		
		// Finally, check for overlapping regions: 
		if(fi_list.first() && fi_list.first()->next)  // need at least 2 elems
		{
			for(PerFrameTaskInfo *fi=fi_list.first(); fi->next; fi=fi->next)
			{
				// Collision detection: Check if fi collides with fi->next. 
				// NOTE: fi->next not NULL here (see for() condition). 
				if(fi->first_frame_no+fi->nframes>fi->next->first_frame_no)
				{
					Error("Local: %s: Per-frame info overlaps with %d:%d.\n",
						_FrameInfoLocationString(fi),
						fi->next->first_frame_no,fi->next->nframes);
					++failed;
				}
			}
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
		_DeleteStrListFirstNULLRef(&master_fi.fadd_args);
		
		// Okay, now set up the rest of the members in the per-frame 
		// info structures: 
		for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
		{
			// First, the easy part: 
			if(!fi->rdir)  fi->rdir=master_fi.rdir;
			else  _PathAppendSlashIfNeeded(&fi->rdir);
			if(!fi->fdir)  fi->fdir=master_fi.fdir;
			else  _PathAppendSlashIfNeeded(&fi->fdir);
			
			// Okay, not so easy... 
			// If the first NULL-string is still there, we prepend 
			// the master params, else we override. 
			_AppendOverrideStrListArgs(&fi->radd_args,&master_fi.radd_args);
			_AppendOverrideStrListArgs(&fi->fadd_args,&master_fi.fadd_args);
			
			// Hard part: file name patterns...
			Error("Handle file name patterns!! (to be hacked; aborting)\n");
			abort();
			//rinfpattern
			//routfpattern
			//foutfpattern
		}
	}
	
	if(!failed)
	{
		char nf_tmp[24];
		if(nframes>=0)  snprintf(nf_tmp,24,"%d",nframes);
		else  strcpy(nf_tmp,"[unlimited]");
		Verbose("Local task source: jump: %d; nframes: %s; startframe: %d\n",
			fjump,nf_tmp,startframe);
		Verbose("  Continuing: %s\n",
			cont_flag ? "yes" : "no");
		
		Verbose("  Master frame info:\n");
		_VPrintFrameInfo(&master_fi,NULL);
		
		for(PerFrameTaskInfo *fi=fi_list.first(); fi; fi=fi->next)
		{
			Verbose("  Frame info difference for frames %d--%d (%d frames):\n",
				fi->first_frame_no,fi->first_frame_no+fi->nframes-1,
				fi->nframes);
			_VPrintFrameInfo(fi,&master_fi);
		}
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
	
	if(nframes<0 && fjump<0)
	{
		Error("Local: Cannot use unlimited nframes value (nframes not specified\n"
			"Local:   or negative value) combined with reverse jumps (%d).\n",
			fjump);
		++failed;
	}
	
	if(response_delay<0)
	{  response_delay=0;  }
	
	if(response_delay)
	{  Warning("Local: You set resonse delay %ld msec.\n",response_delay);  }
	
	return(failed ? 1 : 0);
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
	
	// Master frame info params: 
	AddParam("size|s","size (WWWxHHH) of created images",
		&master_fi.ii->size_string);
	AddParam("oformat","image output format (must be supported by "
		"render/filter driver)",&master_fi.ii->oformat_string);
	AddParam("renderer|rd","renderer to use (render DESC name)",
		&master_fi.ii->rdesc_string);
	AddParam("rargs","additional args to be passed to the renderer",
		&master_fi.radd_args);
	master_fi.ii->render_resume_pi=AddParam("rcont",
		"resume cont; This switch enables/disables render continue "
		"feature: If enabled, interrupted files are named *-unfinished "
		"and rendering resumes at the position where it stopped; "
		"if disabled, a not-completely rendered frame is deleted and has "
		"to be rendered completely the next time.\nNOTE: this just selects "
		"the operation mode; you must use -cont to actuallly continue. "
		"(Default: yes, if driver supports it)",
		&master_fi.render_resume_flag,PNoDefault);
	
	AddParam("rdir","chdir to this directory before calling renderer",
		&master_fi.rdir);
	AddParam("rifpattern","render input file pattern (e.g. f%07d.pov) "
		"(relative to rdir)",
		&master_fi.rinfpattern);
	AddParam("rofpattern","render output file pattern (e.g. f%07d); extension "
		"added according to output format (relative to rdir)",
		&master_fi.routfpattern);
	
	AddParam("filter|fd","filter to use (filter DESC name)",
		&master_fi.ii->fdesc_string);
	AddParam("fargs","additional args to be passed to the filter",
		&master_fi.fadd_args);
	AddParam("fdir","chdir to this directory before calling filter",
		&master_fi.fdir);
	AddParam("fofpattern","filter output file pattern (e.g. f%07d-f.png) "
		"(relative to fdir)",
		&master_fi.foutfpattern);
	
	return(add_failed ? (-1) : 0);
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
	Verbose("[local] ");
	return(0);
}


TaskSourceFactory_Local::TaskSourceFactory_Local(
	ComponentDataBase *cdb,int *failflag) : 
	TaskSourceFactory("local",cdb,failflag),
	master_fi(failflag),
	fi_list(failflag)
{
	nframes=-1;  // unlimited
	startframe=0;
	fjump=1;
	
	last_looked_up=NULL;
	
	cont_flag=false;
	
	response_delay=0;
	
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
	radd_args(failflag),
	rdir(failflag),
	rinfpattern(failflag),
	routfpattern(failflag),
	fadd_args(failflag),
	fdir(failflag),
	foutfpattern(failflag)
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
	
	fdesc=NULL;
	
	ii=NEW<PerFrameTaskInfo_Internal>();
	if(!ii)
	{  --(*failflag);  }
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
