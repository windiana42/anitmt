/*
 * rendview.cpp
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

#include <hlib/fdmanager.h>
#include <hlib/timeoutmanager.h>
#include <hlib/srcenv.h>

char *prg_name=NULL;


static const char *fti="Failed to initialize %s.\n";


static volatile void _SeedRandom()
{
	HTime tmp(HTime::Curr);
	srandom(tmp.Get(HTime::seconds));
}


// Helper for the environment vars accepted by RendView: 
class RendViewEnvVars : public par::ParameterConsumer_Overloaded
{
	private:
		RefStrList addargs;
		ParamInfo *addargs_pi;
		
	public:  _CPP_OPERATORS_FF
		// failflag may NOT be NULL. 
		RendViewEnvVars(par::ParameterManager *parman,int *failflag);
		~RendViewEnvVars();
		
		// Feed the args passed via environment var into passed 
		// param source. 
		// Return value: 0 -> OK; else error
		int ParseAddArgs(par::ParameterSource_CmdLine *cmd_src);
};


int RendViewEnvVars::ParseAddArgs(par::ParameterSource_CmdLine *cmd_src)
{
	static const char *_afail="Alloc failure\n";
	
	// Make sure, this is no longer known: 
	DelParam(addargs_pi);
	addargs_pi=NULL;
	
	int nargs=addargs.count();
	if(!nargs)  return(0);
	
	par::ParamArg *pa_array=NEWarray<par::ParamArg>(nargs);
	if(!pa_array)
	{  Error(_afail);  return(-1);  }
	
	int cnt=0;
	int rv=0;
	for(const RefStrList::Node *i=addargs.first(); i; i=i->next)
	{
		int fflag=0;
		par::ParamArg tmp(i->str(),-cnt-1,&fflag);
		if(fflag)
		{  Error(_afail);  rv=-1;  break;  }
		
		pa_array[cnt++].Assign(tmp);
	}
	
	// Dump them: 
	//for(int i=0; i<cnt; i++)
	//{  fprintf(stderr,"<%s> ",pa_array[i].name);  }
	
	if(!rv)
	{  rv=cmd_src->ReadCmdLine(pa_array,cnt);  }
	
	DELarray(pa_array);
	
	return(rv);
}


RendViewEnvVars::RendViewEnvVars(par::ParameterManager *parman,int *failflag) : 
	par::ParameterConsumer_Overloaded(parman,failflag),
	addargs(failflag)
{
	// failflag may NOT be NULL. 
	
	addargs_pi=NULL;
	
	do {
		if(SetSection(NULL))
		{  --(*failflag);  break;  }
		
		// Our ENV VARS: 
		const char *args_env_var=NULL;
		switch(GetRendViewOpMode(prg_name))
		{
			case RVOM_None:       /* args_env_var stays NULL */  break;
			case RVOM_RendView:   args_env_var="RENDVIEWARGS";   break;
			case RVOM_LDRClient:  args_env_var="LDRCLIENTARGS";  break;
			case RVOM_LDRServer:  args_env_var="LDRSERVERARGS";  break;
		}
		if(args_env_var)
		{
			addargs_pi=AddParam(args_env_var,
				"pass these args as if they were specified on the cmd line",
				&addargs,PEnvironVar|PNoDefault);
		}
		
		if(add_failed)
		{  --(*failflag);  break;  }
	} while(0);
}

RendViewEnvVars::~RendViewEnvVars()
{
	addargs_pi=NULL;
}


static par::ParameterManager *SetUpParManager()
{
	par::ParameterManager *parman=NEW<par::ParameterManager>();
	if(!parman)
	{  return(NULL);  }
	
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
		"RENDVIEWARGS/LDRCLIENTARGS/LDRSERVERARGS: Environment var to "
		"pass arguments just as if you put them at the beginning of the "
		"cmd line; which one is used depends on the binary name.\n\n"
		"RendView is a utility to render (and postprocess) animations.  "
		"While the actual rendering is done by some render application "
		"like e.g. POVRay and the real animation logic is computed by a "
		"animation calculator like aniTMT, rendview can be used to "
		"have the render program actually generate the frames calculated.  "
		"RendView has sopthistcated job control, allows you to start "
		"several processes in parallel and supports distributed rendering "
		"(LDR) in local IP networks.  "
		"Further features include only processing changed/not-yet-done "
		"frames, resuming partly-rendered images, load control and lots "
		"of informative output.\n"
		"NOTE: Pressig ^C on the terminal (SIGINT) will tell RendView to "
		"start no more processes and wait for all current jobs to "
		"exit; pressing ^C twice will kill all jobs.  In either way "
		"rendview exits cleanly.  Pressing ^C three times makes "
		"RendView abort.  SIGTERM acts like two ^C.  SIGTSTP (^Z) will "
		"make RendView stop all tasks and then also stop; SIGCONT (fg or "
		"bg in bash) makes them continue all again.\n"
		"NOTE: COLORED OUTPUT is automatically switched off for non-ttys "
		"but you can manually force it off/on using --nocolor/--color."
		"\n\n** LDR is in experimental state **"
		"\n\nExamples:\n"
		"# rendview --rdfile=/dir/to/renderers.par -l-rd=povray3.5 "
		"-l-n=240 -l-f0=10 -l-size=320x200 -l-oformat=ppm -l-cont -l-rcont "
		"-ld-r-quiet -ld-njobs=2\n"
		"# rendview -opmode=ldrclient --rdfile=... -L-port=7001 "
		"-L-servernet=192.168.0.0/8 -L-password=prompt -ld-njobs=2 -ld-r-quiet "
		"-ld-r-nice=15\n"
		"# rendview -opmode=ldrserver --rdfile=... -Ld-clients=\"localhost/7001 "
		"192.168.0.2/7001/PaSsWoRd 192.168.0.3/7001/prompt\" -Ld-ctimeout=5000 "
		"-l-rd=povray3.1g -l-n=245 -l-r-args=\"-display 192.168.0.1:0.0\" "
		"-l-size=320x240 -l-r-files=\"scene.pov colors.inc\" -l-cont\n");
		// ...as well as only rendering changed frames or resuming operation
		// ...and later create a film of them...
	
	if(rv_oparams.enable_color_stdout)
	{
		parman->SetHighlightStrings(
			rv_oparams.console_blue_start,rv_oparams.console_blue_end,
			rv_oparams.console_red_start,rv_oparams.console_red_end);
	}
	
	parman->SetConsolePrintfFunction(&vaError,&vaWarning,NULL);
	
	return(parman);
}


static int MAIN(int argc,char **argv,char **envp)
{
	// Before we do anything, set these up: 
	// This also checks if we may have color output: 
	InitRVOutputParams(argc,argv,envp);
	
	par::ParameterManager *parman=NULL;
	FDManager *fdman=NULL;
	TimeoutManager *timeoutman=NULL;
	ProcessManager *procman=NULL;
	ComponentDataBase *cdb=NULL;
	TaskManager *taskman=NULL;
	TaskFileManager *taskfileman=NULL;
	
	RendViewEnvVars *envvars=NULL;
	par::CmdLineArgs *cmdline=NULL;
	
	// First action: Set up the parameter system...
	Verbose(BasicInit,"Parameter system:");
	int fail=0;
	do {
		parman=SetUpParManager();
		if(!parman)
		{  Error(fti,"parameter manager");  fail=1;  break;  }
		Verbose(BasicInit," [manager]");
		
		Verbose(BasicInit," [environ");
		
		envvars=NEW1<RendViewEnvVars>(parman);
		if(!envvars)
		{
			Error(fti,"environment var handler");
			fail=1;  break;
		}
		Verbose(BasicInit,".");
		
		par::ParameterSource_Environ env_src(parman,&fail);
		if(fail) 
		{
			Error(fti,"environment parameter source");
			fail=1;  break;
		}
		Verbose(BasicInit,".");
		
		cmdline=NEW3<par::CmdLineArgs>(argc,argv,envp);
		if(!cmdline)
		{
			Error(fti,"cmdline buffer");
			fail=1;  break;
		}
		Verbose(BasicInit,".");
		
		int rv=env_src.ReadEnviron(cmdline);
		if(rv)
		{
			Error("Failed to parse the environment vars (%d)\n",rv);
			fail=1;  break;
		}
		Verbose(BasicInit,".");
		
		rv=env_src.WriteParams();
		if(rv)
		{
			Error("Failed to store the environment vars (%d)\n",rv);
			fail=1;  break;
		}
		Verbose(BasicInit,".]");
		
		Verbose(BasicInit," OK\n");
	} while(0);
	
	// It can never hurt to seed the randon number generator...
	_SeedRandom();
	
	// Okay, then... Here we go...
	while(!fail) {
		fail=1;
		Verbose(BasicInit,"Setting up managers: ");
		
		Verbose(BasicInit,"[FD] ");
		fdman=NEW<FDManager>();
		if(!fdman)
		{  Error(fti,"FD manager");  break;  }
		
		Verbose(BasicInit,"[timeout] ");
		timeoutman=NEW<TimeoutManager>();
		if(!timeoutman)
		{  Error(fti,"timeout manager");  break;  }
		
		Verbose(BasicInit,"[process] ");
		procman=NEW1<ProcessManager>(envp);
		if(!procman)
		{  Error(fti,"process manager");  break;  }
		// FIXME?
		procman->TermKillDelay(1000);
		
		Verbose(BasicInit,"[CDB] ");
		cdb=NEW1<ComponentDataBase>(parman);
		if(!cdb)
		{  Error(fti,"component data base");  break;  }
		
		Verbose(BasicInit,"[task] ");
		taskman=NEW1<TaskManager>(cdb);
		if(!taskman)
		{  Error(fti,"task manager");  break;  }
		
		Verbose(BasicInit,"[file] ");
		taskfileman=NEW<TaskFileManager>();
		if(!taskfileman)
		{  Error(fti,"file manager");  break;  }
		
		Verbose(BasicInit,"OK\n");
		fail=0;  break;
	}
	
	int will_exit=0;
	if(!fail)
	{
		fail+=ImageFormat::init(cdb);
		fail+=TaskDriverFactory::init(cdb);
		fail+=TaskSourceFactory::init_factories(cdb);
		fail+=TaskDriverInterfaceFactory::init_factories(cdb);
	}
	
	// NOW IT IS TIME TO LOOK AT THE COMMAND LINE ARGS: 
	if(!fail) do {
		par::ParameterSource_CmdLine cmd_src(parman,&fail);
		if(fail) 
		{
			Error(fti,"command line parameter source");
			fail=1;  break;
		}
		
		// First, parse the args from the ENV VAR if any: 
		if(envvars->ParseAddArgs(&cmd_src))
		{  fail=1;  break;  }
		// Then, parse the args passed via cmd line: 
		if(cmd_src.ReadCmdLine(cmdline))
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
	
	// Get rid of some stuff: 
	if(cmdline)  {  delete cmdline;  cmdline=NULL;  }
	if(envvars)  {  delete envvars;  envvars=NULL;  }
	
	// NOW IT IS TIME TO REALLY DO THINGS...
	if(!fail && !will_exit) do {
		
		fail=fdman->MainLoop();
		
	} while(0);
	
	// Cleanup: 
	Verbose(BasicInit,"Cleanup:");
    if(taskfileman) { Verbose(BasicInit," [file]");       delete taskfileman;  }
    if(taskman)     { Verbose(BasicInit," [task]");       delete taskman;  }
    if(cdb)         { Verbose(BasicInit," [CDB]");        delete cdb;  }
    if(procman)     { Verbose(BasicInit," [process]");    delete procman;  }
    if(timeoutman)  { Verbose(BasicInit," [timeout]");    delete timeoutman;  }
    if(fdman)       { Verbose(BasicInit," [FD]");         delete fdman;  }
    if(parman)      { Verbose(BasicInit," [parameter]");  delete parman;  }
	Verbose(BasicInit," OK\n");
	
	Verbose(BasicInit,"Exiting: status=%s\n",fail ? "failure" : "success");
	return(fail ? 1 : 0);
}


static void abort_handler(int)
{
	Error(
		"The message above describes a bug in RendView.\n"
		"Please submit a full bug report. (See http://anitmt.sf.net/rendview/)\n");
	_exit(1);
}


int main(int argc,char **argv,char **envp)
{
	prg_name=GetPrgName(argv[0]);
	if(!prg_name)
	{  fprintf(stderr,"RendView: argv[0] is NULL. Aborting.\n");  abort();  }
	
	InstallSignalHandler(SIGABRT,&abort_handler,0);
	
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
