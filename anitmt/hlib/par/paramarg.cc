/*
 * paramarg.cc
 * 
 * Basic parameter argument as read in by parameter sources. 
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

#include "paramarg.h"

#include <string.h>
#include <ctype.h>


namespace par
{

static const char *_three_qm="???";

// Convert origin spec into a RefString: 
RefString ParamArg::Origin::OriginStr() const
{
	RefString str;
	switch(otype)
	{
		case FromFile:
			str.sprintf(0,"%s:%d",origin.str(),opos);
			break;
		case FromCmdLine:
			str.sprintf(0,"[cmdline]:%d",opos);
			break;
		default:
			str.sprintf(0,_three_qm);
			break;
	}
	return(str);
}


static inline bool is_trim(char c)
{  return(isspace(c));  }

ParamArg::Type ParamArg::Classify(const char *arg)
{
	if(!arg)  return(Unknown);
	if((arg[0]=='.' && arg[1]==DirSepChar) || *arg==DirSepChar)
	{  return(Filename);  }
	if(strchr(arg,'='))
	{  return(Assignment);  }
	if(*arg=='-')
	{  return(Option);  }
	if(strchr(arg,DirSepChar))
	{  return(Filename);  }
	return(Unknown);
}


void ParamArg::_Assign(const ParamArg &src)
{
	origin.otype=src.origin.otype;
	origin.origin=src.origin.origin;
	origin.opos=src.origin.opos;
	
	next=src.next;
	atype=src.atype;
	
	arg=src.arg;       // Note: we only reference it, so all the pointers 
	name=src.name;     //       keep their values...
	value=src.value;
	namelen=src.namelen;
	
	assmode=src.assmode;
	pdone=src.pdone;
}


ParamArg::ParamArg(const char *cmd_arg,int argnum,int *failflag) : 
	arg(cmd_arg,failflag),
	origin(FromCmdLine,NULL,argnum,failflag)
{
	#if TESTING
	if(!failflag)
	{  fprintf(stderr,"Oops: ParamArg::ParamArg(CMD) called with failflag=NULL\n");  }
	#endif
	
	next=NULL;  // must be set later
	atype=Classify(cmd_arg);
	
	name=NULL;
	value=NULL;
	namelen=0;
	assmode='\0';
	const char *argstr=arg.str();
	if(argstr)
	{
		const char *a=argstr;
		while(*a=='-')  ++a;
		name=a;
		if(atype==Assignment)
		{
			value=strchr(a,'=');
			if(value)
			{
				const char *tst=value-1;
				namelen=value-name;
				++value;
				if(tst>name) if(*tst=='+' || *tst=='-')
				{  assmode=*tst;  --namelen;  }   // <-- is okay. 
			}
		}
		else
		{  namelen=strlen(name);  }
	}
	
	if(!namelen)
	{  name=NULL;  }
	
	pdone=0;
}


ParamArg::ParamArg(const char *par,const RefString &_file,int _line,
	int *failflag) : 
	arg(par,failflag),
	origin(FromFile,_file,_line,failflag)
{
	#if TESTING
	if(!failflag)
	{  fprintf(stderr,"Oops: ParamArg::ParamArg(PAR) called with failflag=NULL\n");  }
	#endif
	
	next=NULL;
	atype=Assignment;
	
	name=NULL;
	value=NULL;
	namelen=0;
	assmode='\0';
	const char *argstr=arg.str();
	if(argstr)
	{
		const char *a=argstr;
		while(*a && isspace(*a))  ++a;
		if(*a)
		{  name=a;  namelen=strlen(name);  }
		else
		{  name=NULL;  }
	}
	if(name)
	{
		const char *n=name;
		while(*n && *n!='=' && *n!=':')  ++n;
		if(*n)  // n='='
		{
			value=n+1;
			--n;
			if(n<name)
			{  namelen=0;  }
			else
			{
				if(*n=='+' || *n=='-')  assmode=*n;
				while(n>name && is_trim(*n))  --n;
				if(!is_trim(*n))  ++n;
				namelen=n-name;
			}
			
			while(*value && is_trim(*value))  ++value;
			//if(!(*value))  value=NULL;
		}
		else
		{
			n=name;
			while(*n && !is_trim(*n))  ++n;
			if(*n)
			{
				namelen=n-name;
				while(*n && is_trim(*n))  ++n;
				//if(*n)
				{  value=n;  }
			}
		}
	}
	
	if(name)
	{
		const char *nameend=name+namelen;
		for(const char *n=name; n<nameend; n++)
		{
			if(*n=='#')
			{
				if(n==name)
				{  name=NULL;  namelen=0;  value=NULL;  break;  }
				--n;
				while(n>name && is_trim(*n))  --n;
				if(!is_trim(*n))  ++n;
				namelen=n-name;
				value=NULL;
				break;
			}
		}
	}
	
	if(!namelen)
	{  name=NULL;  }
	
	pdone=0;
}


ParamArg::ParamArg(int *failflag) : 
	arg(/*cannot fail*/),
	origin(ParamArg::_FromNowhere,NULL,-1,failflag)
{
	name=value=NULL;
	namelen=0;
	assmode='\0';
	pdone=0;
	atype=Unknown;
	next=NULL;
}


void ParamArg::_Cleanup()
{
	arg.deref();  // set arg=NULL. 
	name=NULL;
	value=NULL;
	next=NULL;
	atype=Unknown;
	origin.origin.deref();
	origin.opos=-1;
	origin.otype=_FromNowhere;
}

ParamArg::~ParamArg()
{
	_Cleanup();
}

}  // namespace end 
