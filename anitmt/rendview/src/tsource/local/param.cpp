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


		
// Create a local TaskSource (TaskSource_Local): 
TaskSource *TaskSourceFactory_Local::Create()
{
	return(NEW1<TaskSource_Local>(this));
}


// Check if the passed pattern is something containing ONE 
// "%123d" or "%07x". 
int TaskSourceFactory_Local::_CheckFramePattern(const char *name,RefString *s)
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
		Error("Local: Invalid %s frame pattern \"%s\".\n",name,s->str());
		return(1);
	}
	return(0);
}


int TaskSourceFactory_Local::FinalInit()
{
	int failed=0;
	
	// Parse output format.
	if(oformat_string.str())
	{
		oformat=component_db()->FindImageFormatByName(oformat_string.str());
		if(!oformat)
		{  Error("Local: Image (output) format \"%s\" not recognized "
			"(try -list-imgfmt).\n",oformat_string.str());  ++failed;  }
		oformat_string.set(NULL);
	}
	
	if(!oformat)
	{
		oformat=component_db()->FindImageFormatByName("png");
		// PNG must be present: 
		assert(oformat);
	}
	
	// Parse renderer to use: 
	rdesc=component_db()->FindRenderDescByName(rdesc_string.str());
	if(!rdesc)
	{
		Error("Local: Unknown render DESC named \"%s\".\n",rdesc_string.str());
		++failed;
	}
	else
	{
		// Default: yes, if rdesc supports it: 
		if(!IsSet(render_resume_pi))
		{  render_resume_flag=rdesc->can_resume_render;  }
		if(render_resume_flag && !rdesc->can_resume_render)
		{
			Warning("Disabled render resume feature (-rcont) "
				"(no support by renderer / %s driver)",
				rdesc->dfactory->DriverName());
			render_resume_flag=false;
		}
	}
	
	// Check frame pattern: 
	failed+=_CheckFramePattern("input",&inp_frame_pattern);
	if(outp_frame_pattern.str())
	{
		failed+=_CheckFramePattern("output",&outp_frame_pattern);
		if(!failed)
		{
			outp_frame_pattern.append(".");
			outp_frame_pattern.append(oformat->file_extension);
		}
	}
	else if(!failed)
	{
		// No output pattern. 
		// Use input pattern without extension: 
		const char *ip=inp_frame_pattern.str();
		const char *es=strrchr(ip,'.');
		if(!es)  es=ip+strlen(ip);
		outp_frame_pattern.sprintf(0,"%.*s.%s",es-ip,ip,
			oformat->file_extension);
	}
	
	if(!failed)
	{
		// Only reason for this assertion to fail would be alloc failure or 
		// internal error. 
		assert(outp_frame_pattern.str());
	}
	
	if(!failed)
	{
		char nf_tmp[24];
		if(nframes>=0)  snprintf(nf_tmp,24,"%d",nframes);
		else  strcpy(nf_tmp,"[unlimited]");
		Verbose("Local task source: jump: %d; nframes: %s; startframe: %d\n",
			fjump,nf_tmp,startframe);
		Verbose("  Continuing: %s; cont operation: %s\n",
			cont_flag ? "yes" : "no",
			render_resume_flag ? "resume (-rcont)" : "re-render (-no-rcont)");
		Verbose("  Renderer: %s (%s driver); output format: %s (%d bpp)\n",
			rdesc->name.str(),rdesc->dfactory->DriverName(),
			oformat->name,oformat->bitspp);
		Verbose("  Frame patterns: input: \"%s\"; output: \"%s\"\n",
			inp_frame_pattern.str(),outp_frame_pattern.str());
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_Local::CheckParams()
{
	int failed=0;
	
	// Check size: 
	if(size_string.str())
	{
		const char *ptr=size_string.str();
		char *end;
		int valid=1;
		width=strtol(ptr,&end,0);
		if(end<=ptr)  valid=0;
		ptr=end;
		if(*ptr=='x' || *ptr==',')  ++ptr;
		else  valid=0;
		height=strtol(ptr,&end,0);
		if(end<=ptr || *end)  valid=0;
		if(!valid || width<=0 || height<=0)
		{  Error("Local: Illegal size spec \"%s\".\n",size_string.str());
			++failed;  }
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
	render_resume_pi=AddParam("rcont",
		"resume cont; This switch enables/disables render continue "
		"feature: If enabled, interrupted files are named *-unfinished "
		"and rendering resumes at the position where it stopped; "
		"if disabled, a not-completely rendered frame is deleted and has "
		"to be rendered completely the next time.\nNOTE: this just selects "
		"the operation mode; you must use -cont to actuallly continue. "
		"(Default: yes, if driver supports it)",
		&render_resume_flag,PNoDefault);
	
	AddParam("size|s","size (WWWxHHH) of created images",&size_string);
	
	AddParam("ifpattern","input frame pattern (e.g. f%07d.pov)",
		&inp_frame_pattern);
	AddParam("ofpattern","output frame pattern (e.g. f%07d); extension "
		"added according to output format",
		&inp_frame_pattern);
	AddParam("oformat","image output format (must be supported by "
		"render/filter driver)",&oformat_string);
	
	AddParam("renderer|rd","renderer to use (render DESC name)",
		&rdesc_string);
	AddParam("rargs","additional args to be passed to the renderer",
		&radd_args);
	
	AddParam("response-delay","local task source response delay in msec; "
		"mainly useful in debugging",&response_delay);
	
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
	inp_frame_pattern(failflag),
	outp_frame_pattern(failflag),
	size_string(failflag),
	oformat_string(failflag),
	rdesc_string(failflag),
	radd_args(failflag)
{
	nframes=-1;  // unlimited
	startframe=0;
	fjump=1;
	
	width=320;
	height=240;
	
	oformat=NULL;
	rdesc=NULL;
	
	cont_flag=false;
	#warning default could be 'yes if supported'
	render_resume_flag=false;
	
	response_delay=0;
	
	int failed=0;
	
	if(inp_frame_pattern.set("f%07d.pov"))
	{  ++failed;  }
	if(rdesc_string.set("povray3.1g"))
	{  ++failed;  }
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSourceFactory_Local");  }
}

TaskSourceFactory_Local::~TaskSourceFactory_Local()
{

}
