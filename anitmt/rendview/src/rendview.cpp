/*
 * rendview.hpp
 * 
 * Implementation of RendView's main routine. 
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


#include "taskmanager.hpp"
#include "tsource/tsfactory.hpp"
#include "tdriver/tdfactory.hpp"

#include <hlib/timeoutmanager.h>
#include <hlib/timeoutbase.h>
#include <hlib/cpmanager.h>


char *prg_name=NULL;


static const char *fti="Failed to initialize %s.\n";

static void _color_setup(int &argc,char **argv,char ** /*envp*/)
{
	int color_arg=0;
	for(int i=1; i<argc; i++)
	{
		const char *a=argv[i];
		if(*a!='-')  continue;
		++a;  while(*a=='-')  ++a;
		if(*a=='n' && !strcmp(a,"nocolor"))
		{
			color_arg=-1;
			for(int j=i+1; j<argc; j++)
			{  argv[j-1]=argv[j];  }
			--i; --argc;
		}
		else if(*a=='c' && !strcmp(a,"color"))
		{
			if(!color_arg)  color_arg=+1;
			for(int j=i+1; j<argc; j++)
			{  argv[j-1]=argv[j];  }
			--i; --argc;
		}
	}
	if(!color_arg)
	{
		if(GetTerminalSize(fileno(stdout),NULL,NULL)==0)
		{  do_colored_output_stdout=1;  }
		if(GetTerminalSize(fileno(stderr),NULL,NULL)==0)
		{  do_colored_output_stderr=1;  }
	}
	else if(color_arg==1)
	{
		do_colored_output_stdout=1;
		do_colored_output_stderr=1;
	}
}


static volatile void _SeedRandom()
{
	HTime tmp(HTime::Curr);
	srandom(tmp.Get(HTime::seconds));
}

static int MAIN(int argc,char **argv,char **envp)
{
	// First, seed the random generator: 
	_SeedRandom();
	
	// Then, let`s see if we may have color output: 
	_color_setup(argc,argv,envp);
	
	// Okay, then... Here we go...
	Verbose("Setting up managers: [FD] ");
	FDManager *fdman=NEW<FDManager>();
	if(!fdman)
	{
		Error(fti,"FD manager");
		return(1);
	}
	
	Verbose("[timeout] ");
	TimeoutManager *timeoutman=NEW<TimeoutManager>();
	if(!timeoutman)
	{
		Error(fti,"timeout manager");
		return(1);
	}
	
	Verbose("[CP] ");
	FDCopyManager *cpman=NEW<FDCopyManager>();
	if(!cpman)
	{
		Error(fti,"copy manager");
		return(1);
	}
	
	Verbose("[process] ");
	ProcessManager *procman=NEW1<ProcessManager>(envp);
	if(!procman)
	{
		Error(fti,"process manager");
		return(1);
	}
	// FIXME?
	procman->TermKillDelay(1000);
	
	Verbose("[parameter] ");
	par::ParameterManager *parman=NEW<par::ParameterManager>();
	if(!parman)
	{
		Error(fti,"parameter manager");
		return(1);
	}
	parman->SetVersionInfo(
		VERSION,      // version string
		NULL,         // package name
		"rendview");  // program name
	parman->SetLicenseInfo(
		// --lincense--
		"RendView may be distributed under the terms of the GNU General "
		"Public License version 2 as published by the Free Software "
		"Foundation.\n"
		"This program is provided AS IS with NO WARRANTY OF ANY KIND, "
		"INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS "
		"FOR A PARTICULAR PURPOSE.\n"
		"See the GNU GPL for details.",
		// --author--
		"RendView (c) 2001--2002 by Wolfgang Wieser.\n"
		"Bug report instructions: See http://anitmt.sf.net/rendview/");
	parman->AdditionalHelpText(
		"RendView is a utility to render (and postprocess) animations. "
		"While the actual rendering is done by some render application "
		"like e.g. POVRay and the real animation logic is computed by a "
		"animation calculator like aniTMT, rendview can be used to "
		"have the render program actually generate the frames calculated. "
		"RendView has sopthistcated job control and allows you to start "
		"several processes in parallel as well as only processing "
		"changed/not-yet-done frames. Further features include resuming "
		"partly-rendered images, load control and lots of informative "
		"output.\n"
		"NOTE: Pressig ^C (SIGINT) on the terminal will tell RendView to "
		"start no more processes and wait for all current jobs to "
		"exit; pressing ^C twice will kill all jobs. In either way "
		"rendview exits cleanly. Pressing ^C three times makes "
		"RendView abort. SIGTERM acts like two ^C. SIGTSTP (^Z) will "
		"make RendView stop all tasks and then als stop; SIGCONT (fg or "
		"bg in bash) makes them continue all again.\n"
		"NOTE: COLORED OUTPUT is automatically switched off for non-ttys "
		"but you can manually force it off/on using --nocolor/--color."
		"\n\n** JUST DO NOT TRY TO DO ANYTHING WITH FILTERS NOW**");
		// ...as well as only rendering changed frames or resuming operation
		// ...and later create a film of them...
	
	Verbose("[CDB] ");
	ComponentDataBase *cdb=NEW1<ComponentDataBase>(parman);
	if(!cdb)
	{
		Error(fti,"component data base");
		return(1);
	}
	
	Verbose("[Task] ");
	TaskManager *taskman=NEW1<TaskManager>(cdb);
	if(!cdb)
	{
		Error(fti,"task manager");
		return(1);
	}
	
	
	Verbose("OK\n");
	
	int fail=0;
	int will_exit=0;
	fail+=ImageFormat::init(cdb);
	fail+=TaskDriverFactory::init(cdb);
	fail+=TaskSourceFactory::init_factories(cdb);
	fail+=TaskDriverInterfaceFactory::init_factories(cdb);
	
	// NOW IT IS TIME TO LOOK AT THE COMMAND LINE ARGS: 
	par::CmdLineArgs cmdline(argc,argv,envp);
	if(!fail) do {
		par::ParameterSource_CmdLine cmd_src(parman,&fail);
		if(fail) 
		{
			Error(fti,"command line parameter source");
			fail=1;  break;
		}
		
		if(cmd_src.ReadCmdLine(&cmdline))
		{  fail=1;  break;  }
		
		// Check if info query options were passed; exit in this case: 
		if(cmd_src.n_iquery_opts)
		{  will_exit=1;  break;  }
  		
		int rv=cmd_src.WriteParams();
		if(rv)
		{
			Error("Failed to store the command line parameters (%d)\n",rv);
			fail=1;  break;
		}
		
		// Read in config files: 
		par::ParameterSource_File file_src(parman,&fail);
		if(fail) 
		{
			Error(fti,"file parameter source");
			fail=1;  break;
		}
		
		// Read in render and filter descriptions: 
		if(cdb->ReadInDescFiles(&file_src))
		{  fail=1;  break;  }
		
		rv=cmd_src.Override(/*below=*/&file_src);
		if(rv)
		{
			Error("Parameter override errors (%d)\n",rv);
			fail=1;  break;
		}
		
		rv=cmd_src.WriteParams();  // (again)
		if(rv)
		{
			Error("Failed to store the command line parameters (%d)\n",rv);
			fail=1;  break;
		}
	} while(0);
	
	if(!fail && !will_exit) do {
		
		if(parman->CheckParams())
		{  fail=1;  break;  }
		
	} while(0);
		
	// NOW IT IS TIME TO REALLY DO THINGS...
	if(!fail && !will_exit) do {
		
		fail=fdman->MainLoop();
		
	} while(0);
	
	// Cleanup: 
	Verbose("Cleanup:");
    if(taskman)     Verbose(" [Task");        delete taskman;
    if(cdb)         Verbose("] [CDB");        delete cdb;
    if(parman)      Verbose("] [parameter");  delete parman;
    if(procman)     Verbose("] [process");    delete procman;
    if(cpman)       Verbose("] [CP");         delete cpman;
    if(timeoutman)  Verbose("] [timeout");    delete timeoutman;
    if(fdman)    Verbose("] [FD");            delete fdman;
	Verbose("] OK\n");
	
	Verbose("Exiting: status=%s\n",fail ? "failure" : "success");
	return(fail ? 1 : 0);
}


int main(int argc,char **argv,char **envp)
{
	prg_name=GetPrgName(argv[0]);
	if(!prg_name)
	{  fprintf(stderr,"RendView: argv[0] is NULL. Aborting.\n");  abort();  }
	
	int rv=MAIN(argc,argv,envp);
	
	// Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sAlloc: %u bytes in %d chunks; Peak: %u bytes; "
		"(%u/%u/%u)%s\n",
		prg_name,
		lmu.curr_used ? "*** " : "",
		lmu.curr_used,lmu.used_chunks,lmu.max_used,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls,
		lmu.curr_used ? " ***" : "");
	
	if(lmu.curr_used || lmu.used_chunks)
	{
		fprintf(stderr,"\33[1;31m");
		fprintf(stderr,"%s: YOU PROBABLY FOUND A MEMORY LEAK IN %s.\n"
			"%s: PLEASE FIND OUT HOW TO REPRODUCE IT AND REPORT THIS BUG TO THE AUTHOR\n",
			prg_name,prg_name,prg_name);
		fprintf(stderr,"\33[00m");
	}
	
	return(rv);
}
