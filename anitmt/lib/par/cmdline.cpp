/*
 * cmdline.cpp
 * 
 * Implementation of functions dealing with command line args. 
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs, suggestions to wwieser@gmx.de. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 * 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * See the GNU General Public License for details.
 * If you have not received a copy of the GNU General Public License,
 * write to the 
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * Revision History:
 *   Jan 2001   started writing
 *
 */

#include "params.hpp"

#ifndef VERSION
#warning "VERSION not defined."
#define VERSION "???"
#endif

namespace anitmt
{

// Anitmt version string (as passed to the compiler): 
static const char *aniTMT_version=VERSION;

// Config environment name: (including `=' at the end) 
static const char *conf_env_var="ANITMT_CONF=";

// Built-in default config file path (set to NULL for none)
static const char *conf_builtin_default=NULL;


static const int FD_Max_Synonymes=2;

static const char *opt_argname_conf[FD_Max_Synonymes]={"conf","cfg"};
static const char *opt_argname_ini[FD_Max_Synonymes]= {"ini","opt" };
static const char *opt_argname_rif[FD_Max_Synonymes]= {"rif"       };

struct File_Description   // Needed nowhere else. 
{
	const char **names;  // one of the opt_argname_XX above. 
	Animation_Parameters *ap;
	int counter;
};

// Just prints version (for --version)
static void Print_Version(std::ostream &os)
{
	os << "aniTMT version " << aniTMT_version << std::endl;
}

static void _Help_Helper(std::ostream &os,
	const char **opt_argname,const char *ftype)
{
	os << "    " << Arg_Prefix(opt_argname[0]) << opt_argname[0];
	for(int i=1; i<FD_Max_Synonymes; i++)
	{
		if(!opt_argname[i])  continue;
		os << ", " << Arg_Prefix(opt_argname[i]) << opt_argname[i];
	}
	os << "  specify " << ftype << " file to read" << std::endl;
}

// Prints help on file options...
static void Print_Help(std::ostream &os)
{
	os << "** This is aniTMT, version " << aniTMT_version << " **" << std::endl;
	os << std::endl;
	os << "NOTE: - You may specify any number of options in any order." << std::endl;
	os << "      - All options can be passed with one or two leading `-\'." << std::endl;
	os << "      - Option values can either be specified as next argument (e.g. --fps 24)" << std::endl;
	os << "        or as assignment using the same argument (e.g. --fps=24)" << std::endl;
	os << std::endl;
	os << "Supported standard options:" << std::endl;
	os << "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯" << std::endl;
	os << "    --help     prints this phenomenous help" << std::endl;
	os << "    --version  prints version string (and exits)" << std::endl;
	os << "    --ovrwarn  print a message for each overridden parameter" << std::endl;
	os << "    --dumppar  dump parameters; optional argument specifies which ones:" << std::endl;
	os << "         --dumppar dcirlf for Default,Config,Ini,Rif,cmd Line,Final" << std::endl;
	os << std::endl;
	os << "Supported file options:" << std::endl;
	os << "¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯" << std::endl;
	_Help_Helper(os,opt_argname_conf,"config");
	os << "      Default built-in config file: " << 
		(conf_builtin_default ? conf_builtin_default : "(none)") << std::endl;
	_Help_Helper(os,opt_argname_ini,"initialisation");
	os << "      Files without leading option are considered as initialisation files." << std::endl;
	_Help_Helper(os,opt_argname_rif,"render information");
	os << std::endl;
	os << "  File format:   parameter = value   with one parameter per line." << std::endl;
	os << "    You may insert spaces/tabs anywhere if it makes sense." << std::endl;
	os << "    The assignment (`=\') is optional. Comments are introduced by `#\' or `;\'." << std::endl;
	os << "    `\\\' at line end concatenates it with the next one (no space inserted)." << std::endl;
	os << "    `#\' makes the rest of the line a comment; don\'t use in combination" << std::endl;
	os << "    with multi-line parameters." << std::endl;
	os << "    include = path   includes specified file at the current position." << std::endl;
	os << std::endl;
}


// Return value: true -> OK; false -> error(s) (quit)
bool Animation_Parameters::Parse_Command_Line(Command_Line *cmd)
{
	// First, we read in files after --ini --conf --rif. 
	// Then, we have to read in all the parameters. 
	// Arguments left unused might be files then. 
	
	Animation_Parameters def_pars;
	def_pars.Set_Defaults(/*keep_old=*/false);
	
	Animation_Parameters conf_pars;
	Animation_Parameters ini_pars;
	Animation_Parameters rif_pars;
	
	File_Description file_desc[]=
	{
		{ opt_argname_conf, &conf_pars, 0 },  // Must be the first one. 
		{ opt_argname_ini,  &ini_pars,  0 },
		{ opt_argname_rif,  &rif_pars,  0 },
		{ NULL, NULL }
	};
	
	int errors=0;
	Set_Cmd_Mode();
	
	// Search for options followed by a file: 
	for(int i=1; i<cmd->argc; i++)
	{
		if(cmd->used[i])  continue;
		char *cmdstart=cmd->argv[i];
		if(*cmdstart!='-')  continue;
		if(*(++cmdstart)=='-')  ++cmdstart;  // skip one or two `-'
		int cmdlen=strlen(cmdstart);
		
		{
			char optname[cmdlen],*d=optname;
			for(char *s=cmdstart; *s; s++,d++)
			{  if(*s=='=')  break;  *d=*s;  }
			*d='\0';
			Set_Option(optname);
		}
		
		char *filearg=NULL;
		File_Description *reader=NULL;
		for(File_Description *fd=file_desc; fd->ap; fd++)
			for(int j=0; j<FD_Max_Synonymes; j++)
			{
				if(!fd->names[j])  continue;
				int namelen=strlen(fd->names[j]);
				if(cmdlen<namelen)  continue;
				if(strncmp(cmdstart,fd->names[j],namelen))  continue;
				
				if(cmdstart[namelen]=='=')
				{
					filearg=&cmdstart[namelen+1];
					reader=fd;
					goto breakboth;
				}
				if(cmdstart[namelen]=='\0')
				{
					if(i<cmd->argc-1)
					{
						cmd->used[i]=strlen(cmd->argv[i]);
						filearg=cmd->argv[++i];
						reader=fd;
						goto breakboth;
					}
					cmd->used[i]=-1;
					Error() << ": Required path missing" << std::endl;
					goto breakboth;
				}
			}
		breakboth:;
		
		if(filearg)
		{
			++reader->counter;
			reader->ap->Set_Cmd_Mode();
			reader->ap->Set_Option(Get_Option());
			int rv=reader->ap->Read_File(filearg);
			cmd->used[i] = rv ? (-1) : (int(strlen(cmd->argv[i])));
		}
	}
	
	// Read in parameter settings from the command line: 
	Animation_Parameters cmd_pars;
	cmd_pars.Read_Command_Line(cmd);
	
	// Now, check for --help and unused file names: 
	bool override_warnings=false;
	char *dump_parameters="";
	char *dump_all_parameters="dcirlf";
	for(int i=1; i<cmd->argc; i++)
	{
		if(cmd->used[i])  continue;
		if(cmd->argv[i][0]=='-')
		{
			char *opt=cmd->argv[i]+1;
			if(*opt=='-')  ++opt;
			
			int argidx=-1;
			char *arg=strchr(opt,'=');
			if(arg)
			{  argidx=i;  ++arg;  }
			else if(i<cmd->argc-1)
				if(!cmd->used[i+1])
				{  argidx=i+1;  arg=cmd->argv[argidx];  }
			
			if(!strcmp(opt,"version"))
			{
				cmd->used[i]=strlen(cmd->argv[i]);
				cmd->quitafter=true;
				Print_Version(std::cerr);
			}
			else if(!strcmp(opt,"help"))
			{
				cmd->used[i]=strlen(cmd->argv[i]);
				cmd->quitafter=true;
				anitmt::Print_Help(std::cout);   // on file options...
				cmd_pars.Print_Help(std::cout);  // on parameters
			}
			else if(!strcmp(opt,"ovrwarn"))
			{
				cmd->used[i]=strlen(cmd->argv[i]);
				override_warnings=true;
			}
			else if(!strncmp(opt,"dumppar",7))
			{
				cmd->used[i]=opt-cmd->argv[i]+7+((argidx==i) ? 1 : 0);
				dump_parameters=dump_all_parameters;
				if(arg)
					if(argidx==i || (*arg!='-' && !strchr(arg,'=') && 
					   strlen(arg)<=strlen(dump_all_parameters)))
					{
						int l=0;
						for(char *c=dump_all_parameters; *c; c++)
							l+=(strchr(arg,*c) ? 1 : 0);
						if(l==strlen(arg) || argidx==i)
						{
							dump_parameters=arg;
							cmd->used[argidx]+=l;
						}
					}
			}
		}
		else //if(cmd->argv[i][0]!='-')
		{
			// Should be an ini-file. 
			ini_pars.Set_Cmd_Mode();
			ini_pars.Set_Option("[implicit]");
			int rv=ini_pars.Read_File(cmd->argv[i]);
			cmd->used[i] = rv ? (-1) : (int(strlen(cmd->argv[i])));
		}
	}
	
	// Relies on fact that file_desc[0] refers to .conf file. 
	if(!file_desc[0].counter)
	{
		// No conf file was specified. 
		// Search for conf file in environment; else use default path. 
		int must_read_default=1;
		
		int len=strlen(conf_env_var);
		for(int i=0; cmd->envp[i]; i++)
		{
			if(strlen(cmd->envp[i])<len)  continue;
			if(!strncmp(cmd->envp[i],conf_env_var,len))
			{
				char *path=cmd->envp[i]+len;
				if(*path)
				{
					conf_pars.Set_Cmd_Mode();
					conf_pars.Set_Option(conf_env_var);
					if(conf_pars.Read_File(path))
					{  ++errors;  }
				}
				else
				{  std::cerr << "Warning: Reading no config file as " << 
					conf_env_var << "is set to \"\"." << std::endl;  }
				must_read_default=0;
				break;
			}
		}
		
		if(must_read_default && conf_builtin_default)
		{
			conf_pars.Set_Cmd_Mode();
			conf_pars.Set_Option("[builtin config]");
			if(conf_pars.Read_File(conf_builtin_default))
			{  ++errors;  }
		}
	}
	
	// Go one last time through the args (check for one letter options) 
	for(int i=1; i<cmd->argc; i++)
	{
		if(cmd->used[i])  continue;
		if(cmd->argv[i][0]!='-')  continue;
		if(cmd->argv[i][1]=='-')  continue;
		if(strchr(cmd->argv[i],'='))  continue;
		
		int verbinc=0;
		int used=0;
		for(char *c=cmd->argv[i]+1; *c; c++)
		{
			switch(*c)
			{
				case 'v':  ++verbinc;  ++used;  break;
				#warning "Any more one letter options without arg (besides -v)?"
			}
		}
		if(verbinc>0)
		{
			if(!cmd_pars.par_int[PID::verbose].is_set)
			{  cmd_pars.par_int[PID::verbose]=0;  }
			cmd_pars.par_int[PID::verbose].val+=verbinc;
		}
		if(used) ++used;   // we also `used' the initial `-' 
		cmd->used[i]=used;
	}
	
	const int n_o_pars=5;
	struct Override_Pars pars[n_o_pars]=
	{
		{ "default",  'd', &def_pars  },
		{ "config",   'c', &conf_pars },   // specified in the 
		{ "ini",      'i', &ini_pars  },   // override order 
		{ "rif",      'r', &rif_pars  },
		{ "cmd line", 'l', &cmd_pars  }
	};
	
	for(int i=0; i<n_o_pars; i++)
		if(strchr(dump_parameters,pars[i].type_short))
		{
			std::cout << "DUMP: " << pars[i].type << " parameters:" << std::endl;
			pars[i].ap->Print_Parameters(std::cerr,"  ");
			std::cout << std::endl;
		}
	
	// Command line argument processing done. 
	if(!cmd->Check_Unused(std::cerr))
		return(false);
	if(errors)
		return(false);
	
	// Now... we must merge all the xx_pars into *this...
	// First, the easy part... 
	if(Simple_Override(pars,n_o_pars,std::cerr,override_warnings))  // copying to *this 
	{
		// Returned >0 -> Some parameter is not set. 
		++errors;
	}
	
	// ... then the hard part: those parameters which have to 
	// get solved (as they depend on others). 
	// Dealing with: start/endtime, start/endframe, duration, frames, fps 
	if(Solve_TimeFrame_Net(pars,n_o_pars,std::cerr,
		par_bool[PID::warnings].is_set ? par_bool[PID::warnings].val : true))
	{
		++errors;
	}
	
	if(strchr(dump_parameters,'f'))
	{
		std::cout << "DUMP: FINAL parameters:" << std::endl;
		Print_Parameters(std::cerr,"  ");
		std::cout << std::endl;
	}
	
	if(!errors)  // check necessary to ensure that all parameters are set. 
	{  errors+=Parameter_Checks();  }
	
	return(errors ? false : true);
}

// Reads in the command line arguments updating 
// the cmd.used[] array. 
void Animation_Parameters::Read_Command_Line(Command_Line *cmd)
{
	Set_Cmd_Mode();
	
	for(int i=1; i<cmd->argc; i++)
	{
		if(cmd->used[i])  continue;
		
		char *arg=cmd->argv[i];
		// Is this an option (and not a file)?
		if(*arg!='-')  continue;
		++arg;
		// Is there a second `-' at the beginning?
		if(*arg=='-')  ++arg;
		// Okay. Now, the option starts with *arg. 
		char *nextarg=strchr(arg,'=');
		
		int arglen = nextarg ? (nextarg-arg) : strlen(arg);
		char tmparg[arglen+1];  // +1 for terminating '\0' 
		strncpy(tmparg,arg,arglen);
		tmparg[arglen]='\0';
		
		// Tell String_Value_Converter the currently parsed option 
		// (for errors/warnings): 
		Set_Option(tmparg);
		
		nextarg_is_next=false;
		if(nextarg)  ++nextarg;  // skip '=' 
		else if((i+1)<cmd->argc)
		{
			if(!cmd->used[i+1])
			{  nextarg=cmd->argv[i+1];  nextarg_is_next=true;  }
			//else nextarg stays NULL;
		}
		
		switch(Parse_Setting(tmparg,nextarg))
		{
			case 0:  /* arg not found */
				// This is not an error as it might be interpreted by 
				// someone else. 
				break;
			case 1:  /* used arg but not nextarg */
				cmd->used[i]=strlen(cmd->argv[i]);
				break;
			case 2:  /* used arg and nextarg */
				cmd->used[i]=strlen(cmd->argv[i]);
				if(nextarg_is_next)
				{  ++i;  cmd->used[i]=strlen(cmd->argv[i]);  }
				break;
			case -1:  /* arg valid but invalid nextarg */
				cmd->used[i] = -1;  // -1 -> definitely invalid
				// ``definitely invalid'' means: you might use the next arg 
				// as the real arg was omitted but we got an error. 
				// We don't increment i here, even if(nextarg_is_next) 
				// and keep the next arg unused. 
				break;
		}
	}
}


// Check for unused arguments. 
// For every unused arg, an error is written to os. 
// Return value: true  -> no unused args (okay, go on)
//               false -> at least one unused arg (error; exit)
bool Command_Line::Check_Unused(std::ostream &os)
{
	static const int max=5;   // report max. 5 illegal args
	int nunused1=0;
	int nunused2=0;
	for(int i=1; i<argc; i++)
	{
		int u=used[i];
		int l=strlen(argv[i]);
		if(u<0)
		{  ++nunused1;  }  // error already written 
		else if(u<l)
		{
			if(nunused2<max)
			{
				if(!u)
				{  os << "Error: Illegal argument \"" << argv[i] << "\"." << std::endl;  }
				else
				{  os << "Error: Argument \"" << argv[i] << "\" contains " 
					<< (l-u) << " illegal option" << ((l-u>1) ? "s." : ".") 
					<< std::endl;  }
			}
			++nunused2;
		}
	}
	if(nunused2>max)
	{  os << (nunused2-max) << " further illegal arguments follow." << std::endl;  }
	if(nunused1 || nunused2)
		return(false);
	if(quitafter)
		return(false);
	return(true);
}


Command_Line::Command_Line(int _argc,char **_argv,char **_envp) : 
	argc(_argc),
	argv(_argv),
	envp(_envp)
{
	used = new int[argc];
	used[0]=strlen(argv[0]);
	for(int i=1; i<argc; i++)
	{  used[i]=0;  }
	quitafter=false;
}

Command_Line::~Command_Line()
{
	delete[] used;
}

} /* end of namespace anitmt */
