/*
 * ctask.cpp
 * 
 * CompleteTask class implementation. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "tasksource.hpp"

#include <ctype.h>

#include <lib/mymath.hpp>  /* for NAN */

#include <assert.h>


// Static vars: 
int CompleteTask::n_complete_tasks=0;


bool CompleteTask::WasSuccessfullyRendered() const
{
	if(!rt)  return(false);
	return(rtes.tes.IsCompleteSuccess());
}


bool CompleteTask::IsPartlyRenderedTask() const
{
	//if(td && td->GetFactory()->DType()==DTRender)  return(true);
	assert(!d.any());
	if(!rtes.tes.WasKilledByUs())
	{  return(false);  }
	if(rtes.tes.outfile_status==OFS_Resume || 
	   rtes.tes.outfile_status==OFS_Complete )  // <-- to be sure but will not happen
	{  return(true);  }
	return(false);
}

bool CompleteTask::IsPartlyFilteredTask() const
{
	// Categorically false. Because filtering cannot be resumed. 
	assert(!d.any());
	return(false);
}

int CompleteTask::DidRenderTaskFail() const
{
	if(!rt | rtes.tes.rflags==TTR_Unset)  return(-1);
	if(rtes.tes.IsCompleteSuccess())  return(0);
	if(IsPartlyRenderedTask())  return(1);
	return(2);
}

int CompleteTask::DidFilterTaskFail() const
{
	if(!ft | ftes.tes.rflags==TTR_Unset)  return(-1);
	if(ftes.tes.IsCompleteSuccess())  return(0);
	if(IsPartlyFilteredTask())  return(1);
	return(2);
}


char *CompleteTask::Completely_Partly_Not_Processed(int *level) const
{
	if(state==CompleteTask::TaskDone)
	{
		if(level)  *level=2;
		return("completely");
	}
	if(ProcessedTask() ||          // e.g. rendered but but not filtered
	   IsPartlyRenderedTask() ||
	   IsPartlyFilteredTask() )    // interrupt
	{
		if(level)  *level=1;
		return("partly");
	}
	if(level)  *level=0;
	return("not");
}


void CompleteTask::PrintTaskExecuted(int vl,const TaskParams *tp,
	const TaskStructBase *tsb,const char *binpath,bool was_processed) const
{
	if(!IsVerboseR(vl))  return;
	
	if(!was_processed)
	{  VerboseR(vl,"    Executed: [not processed]\n");  return;  }
	
	char tmpA[32];
	if(tp)
	{
		if(tp->niceval==TaskParams::NoNice)  strcpy(tmpA,"(none)");
		else  snprintf(tmpA,32,"%d",tp->niceval);
	}
	else
	{  strcpy(tmpA,"??");  }
	
	long timeout=-1;
	char timeout_char='\0';
	if(tp && tp->timeout>0)
	{  timeout=tp->timeout;  timeout_char='L';  }
	if(tsb->timeout>0 && (timeout>tsb->timeout || timeout<0))
	{  timeout=tsb->timeout;  timeout_char='T';  }  // Another timeout. Take shorter one. 
	
	char tmpB[32];
	if(timeout<=0)  strcpy(tmpB,"(none)");
	else
	{  snprintf(tmpB,32,"%ld sec (%s)",(timeout+500)/1000,
		timeout_char=='L' ? "loc" : "TS");  }
	
	VerboseR(vl,"    Executed: %s\n",binpath);
	VerboseR(vl,"              (nice value: %s; timeout: %s; tty: %s)\n",
		tmpA,tmpB,
		tp ? (tp->call_setsid ? "no" : "yes") : "??");
}

// Call this if the task is new and the data in TaskParams is not 
// yet set up. 
void CompleteTask::PrintTaskToBeExecuted(int vl,const TaskStructBase *tsb,
	const char *binpath) const
{
	char tmpB[32];
	if(tsb->timeout<=0)  strcpy(tmpB,"none");
	else
	{  snprintf(tmpB,32,"%ld sec",(tsb->timeout+500)/1000);  }
	
	// TS -> "task source"
	VerboseR(vl,"    Execute: %s (timeout TS: %s)\n",
		binpath,tmpB);
}


void CompleteTask::PrintTaskExecStatus(int vl,const TES *ct_tes,
	TaskDriverType dtype) const
{
	if(!IsVerboseR(vl))  return;
	
	const TaskExecutionStatus *tes=&ct_tes->tes;
	
	if(tes->rflags==TTR_Unset && tes->outfile_status==OFS_Unset)  return;
	
	if((tes->rflags==TTR_Exit && tes->signal==0) || tes->rflags==TTR_Unset)
	{
		// "Normal" case: successful exit and not killed by rendview. 
		// OR, not processed. 
		assert(!tes->WasKilledByUs());   // Can be left away. 
		
		VerboseR(vl,"      Status: %s;  Output file: %s\n",
			tes->TermStatusString(),
			TaskExecutionStatus::OFS_String(tes->outfile_status));
	}
	else
	{
		VerboseR(vl,"      Exit status: %s\n",tes->TermStatusString());
		if(tes->WasKilledByUs())
		{  VerboseR(vl,"        Killed by %s: %s;",
			::prg_name,TaskExecutionStatus::TTR_JK_String(tes->rflags));  }
		VerboseR(vl,"%s  Output file: %s\n",
			tes->WasKilledByUs() ? "" : "      ",
			TaskExecutionStatus::OFS_String(tes->outfile_status));
	}
	if(tes->rflags==TTR_Unset)  return;
	VerboseR(vl,"      %c%sed by: %s\n",
		toupper(*DTypeString(dtype)),DTypeString(dtype)+1,
		ct_tes->processed_by.str() ? ct_tes->processed_by.str() : "[n/a]");
	VerboseR(vl,"      Started: %s\n",tes->starttime.PrintTime(1,1));
	VerboseR(vl,"      Done:    %s\n",tes->endtime.PrintTime(1,1));
	HTime duration=tes->endtime-tes->starttime;
	
	char cpu_tmp[24];
	cpu_tmp[0]='\0';
	double dur_sec=duration.GetD(HTime::seconds);
	if(dur_sec>=0.01)
	{
		double dur_pc=100.0*(tes->utime.GetD(HTime::seconds)+
			tes->stime.GetD(HTime::seconds))/dur_sec;
		if(dur_pc<10000.0)
		{  snprintf(cpu_tmp,24,"  (%.2f%% CPU)",dur_pc);  }
	}
	VerboseR(vl,"      Elapsed: %s%s\n",duration.PrintElapsed(),cpu_tmp);
	
	char tmp[48];
	snprintf(tmp,48,"%s",tes->utime.PrintElapsed());
	VerboseR(vl,"      Time in mode: user: %s; system: %s\n",
		tmp,tes->stime.PrintElapsed());
}


void CompleteTask::DumpTask(int vl,int when) const
{
	if(!IsVerboseR(vl))  return;
	
	VerboseR(vl,"  Task state: %s\n",
		when==0 ? "new" : StateString(state));
	
	if(rt)
	{
		VerboseR(vl,"  Render task: %s (%s driver) -rcont=%s%s\n",
			rt->rdesc->name.str(),rt->rdesc->dfactory->DriverName(),
			rt->resume_flag_set ? "yes" : "no",
			rt->resume ? " [continue]" : "");
		#if 0  	/* FrameClockVal: UNUSED */
		char tmp[32];
		if(!finite(rt->frame_clock))
		{  strcpy(tmp,"[none]");  }
		else
		{  snprintf(tmp,32,"%g",rt->frame_clock);  }
		VerboseR(vl,"    Input file:  hd path: %s; frame clock: %s\n",
			!!rt->infile ? rt->infile.HDPathRV().str() : NULL,tmp);
		#else
		char tmp[32];
		if(!rt->use_frame_clock)
		{  strcpy(tmp,"[none]");  }
		else
		{  snprintf(tmp,32,"frame %d",rt->frame_no);  }
		VerboseR(vl,"    Input file:  hd path: %s; clock: %s\n",
			!!rt->infile ? rt->infile.HDPathRV().str() : NULL,tmp);
		#endif
		VerboseR(vl,"    Output file: hd path: %s; size %dx%d; format: %s (%d bpc)\n",
			!!rt->outfile ? rt->outfile.HDPathRV().str() : NULL,
			rt->width,rt->height,
			rt->oformat ? rt->oformat->name : NULL,
			rt->oformat ? rt->oformat->bits_p_rgb : 0);
		#warning more info? (HDPathJob?)
		if(when==0)
		{  PrintTaskToBeExecuted(vl,rt,rt->rdesc->binpath.str());  }
		else
		{
			PrintTaskExecuted(vl,rtp,rt,rt->rdesc->binpath.str(),
				rtes.tes.rflags!=TTR_Unset);
			PrintTaskExecStatus(vl,&rtes,DTRender);
		}
	}
	else
	{  VerboseR(vl,"  Render task: [none]\n");  }
	
	if(ft)
	{
		VerboseR(vl,"  Filter task: %s (%s driver)\n",
			ft->fdesc->name.str(),ft->fdesc->dfactory->DriverName());
		VerboseR(vl,"    Input file:  hd path: %s\n",
			!!ft->infile ? ft->infile.HDPathRV().str() : NULL);
		VerboseR(vl,"    Output file: hd path: %s\n",
			!!ft->outfile ? ft->outfile.HDPathRV().str() : NULL);
		#warning more info?
		if(when==0)
		{  PrintTaskToBeExecuted(vl,ft,ft->fdesc->binpath.str());  }
		else
		{
			PrintTaskExecuted(vl,ftp,ft,ft->fdesc->binpath.str(),
				ftes.tes.rflags!=TTR_Unset);
			PrintTaskExecStatus(vl,&ftes,DTFilter);
		}
	}
	else
	{  VerboseR(vl,"  Filter task: [none]\n");  }
}


const char *CompleteTask::StateString(State s)
{
	switch(s)
	{
		case TaskDone:      return("done");
		case ToBeRendered:  return("to be rendered");
		case ToBeFiltered:  return("to be filtered");
		// default: see below
	}
	return("???");
}


CompleteTask::CompleteTask(int * failflag) : 
	LinkedListBase<CompleteTask>(),
	rtes(failflag),
	ftes(failflag)
{
	state=TaskDone;  // okay. 
	frame_no=-1;
	task_id=0;
	d.td=NULL;
	d.ldrc=NULL;
	d.shall_render=0;
	d.shall_filter=0;
	assert(!d.any());
	rt=NULL;   ft=NULL;
	rtp=NULL;  ftp=NULL;
	radd.nfiles=0;    fadd.nfiles=0;
	radd.tfile=NULL;  fadd.tfile=NULL;
	
	++n_complete_tasks;
}

CompleteTask::~CompleteTask()
{
	assert(!d.any());
	DELETE(rt);
	DELETE(ft);
	DELETE(rtp);
	DELETE(ftp);
	assert(rt==NULL);  // can be removed 
	radd.tfile=DELarray(radd.tfile);
	fadd.tfile=DELarray(fadd.tfile);
	
	--n_complete_tasks;
	Verbose(DBGV,"~CompleteTask[%d] (left: %d)\n",frame_no,n_complete_tasks);
}


CompleteTask::TES::TES(int *failflag) : 
	tes(failflag),
	processed_by(failflag)
{
	
}
