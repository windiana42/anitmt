/*
 * cmdline.cc
 * 
 * Implementation of routines for the command line class. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>

#include "cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


namespace par
{

CmdLineArgs::CmdLineArgs(int _argc,char **_argv,char **_envp,
	int *failflag) : 
	CommandLine(_argc,_argv,_envp)
{
	int failed=0;
	
	args=NEWarray<ParamArg>((argc>0) ? argc : 1);
	if(!args)
	{  ++failed;  }
	else
	{
		// arg[0] is special:
		int fflag=0;
		ParamArg tmp((argc>0) ? argv[0] : "???",0,&fflag);
		if(fflag)
		{  ++failed;  }
		else
		{
			args->Assign(tmp);
			args->name=strrchr(args->arg.str(),DirSepChar);
			if(args->name)  ++args->name;
			else  args->name=args->arg.str();
			args->namelen=strlen(args->name);
			args->atype=ParamArg::Filename;
			args->pdone=1;
		}
	}
	
	if(argc>0 && !failed)
	{
		// Now the ``real'' args...
		for(int i=1; i<argc; i++)
		{
			int fflag=0;
			ParamArg tmp(argv[i],i,&fflag);
			if(fflag)
			{  ++failed;  break;  }
			else
			{
				ParamArg *arg=&args[i];
				arg->Assign(tmp);
				// must link them...
				(arg-1)->next=arg;
			}
		}
	}
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("CmdLineArgs");  }
}


CmdLineArgs::~CmdLineArgs()
{
	args=DELarray(args);
}

}  // namespace end 
