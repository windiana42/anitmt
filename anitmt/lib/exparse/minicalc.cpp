/*
 * minicalc.cpp
 * 
 * Expression parser test program which can be used as interactive 
 * or command line calculator. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "lexer.hpp"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

using namespace exparse;

#if 0
char *expressions[]=
{
	"sin(0.77)^2+cos(0.77)^2-1",
	"sqrt(3^2+4^2)-5+1",
	"sin(asin(cos(acos(tan(atan(0.3))))))-0.3+2",
	NULL
};
#endif

int main(int argc,char **arg)
{
	// Make sure to call the initialisation routines: 
	exparse::InitData();
	
	Parser fl;
	ExpressionTree etree;
	Value val;
	
	int errors=0;
	for(int i=1; i<argc; i++)
	{
		fl.Parse(arg[i],&etree);
		int rv=etree.Compute(&val);
		cerr << "Compute errors: " << rv << "; Result: " << val << std::endl;
		errors+=rv;
	}
	
	if(argc==1)
	{
		cerr << "Entering interactive mode (you may use this program as" << std::endl;
		cerr << "command line calculator by simply adding expressions on" << std::endl;
		cerr << "the command line; don't forget to mask them with \'\')." << std::endl;
		cerr << "Enter expressions termianted by newline (don't use `;')." << std::endl;
		cerr.flush();
		#warning this is crippled as it uses a fixed-width buffer. FIXME.
		for(;;)
		{
			size_t buflen=16384;
			char buf[buflen];
			write(1,"minicalc> ",10);
			ssize_t rd=read(0,buf,buflen-1);
			if(!rd)
			{  write(1,"\n",1);  break;  }
			if(rd<0)
			{  fprintf(stderr,"Read error: %s\n",strerror(errno));
				return(1);  }
			buf[rd]='\0';
			fl.Parse(buf,&etree);
			int rv=etree.Compute(&val);
			cerr << "Compute errors: " << rv << "; Result: " << val << std::endl;
		}
		errors=0;  // we always return with 0 from interactive mode. 
	}
	
	exparse::CleanupData();
	
	return(errors ? 1 : 0);
}

