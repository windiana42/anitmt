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

int cont_on_errors=0;
int optimize=0;
int semicol_term=0;
char *myname=NULL;

int DoExpr(const char *expr,Parser *fl,ExpressionTree *etree)
{
	int errors=0;
	Value val;
	int rv=fl->Parse(expr,etree);
	if(rv)
	{
		cerr << "Parse errors: " << rv << std::endl;
		++errors;
		if(!cont_on_errors)  return(errors);
	}
	
	if(optimize)
	{
		rv=etree->Optimize();
		if(rv)
		{
			cerr << "Optimize errors: " << rv << std::endl;
			++errors;
			if(!cont_on_errors)  return(errors);
		}
	}
	
	rv=etree->Compute(&val);
	if(rv)
	{
		cerr << "Compute errors: " << rv << std::endl;
		++errors;
		if(!cont_on_errors)  return(errors);
	}
	
	cout << "Result: " << val << std::endl;
	return(errors);
}


void PrintHelp(char *name)
{
	cout << name << " [-oi-] [--help] [--] [expression....]" << std::endl;
	cout << "  -o   optimize expression tree before calculating; no practical" << std::endl;
	cout << "       use besides testing." << std::endl;
	cout << "  -i   ignore parse/optimize/compute errors; always print result" << std::endl;
	cout << "       use besides testing." << std::endl;
	cout << "  -t   expressions in interactive mode are terminated by `;' followed" << std::endl;
	cout << "       by a newline rather than simply a newline." << std::endl;
	cout << "  --   expressions follow; you may also append a `-' to an options" << std::endl;
	cout << "       string (e.g. -o- 2*(3+7)); only required if expr. starts with `-'" << std::endl;
	cout << "expression: any expression to compute; if none is given and no `--'" << std::endl;
	cout << "            is specified, enter interactive mode." << std::endl;
	cout << "       Don't forget to mask expressions with \'\' on the shell prompt." << std::endl;
	cout << "Bugs to: Wolfgang Wieser <wwieser@gmx.de>" << std::endl;
}


void OnLineHelp()
{
	cout << 
	  "YOU need help?! Beware; this is an expert-only parser. But well...\n"
	  "Try " << myname << " --help for help on command line options.\n"
	  "Hit ^D to quit interactive mode.\n"
	  "Enter expression here, e.g. acos(sqrt(3^2+4^2)-5)" << 
		  (semicol_term ? ";" : "") << "\n"
	  "If you want more online help implemented, kindly ask my programmer...\n";
}


int main(int argc,char **arg)
{
	myname=strrchr(arg[0],'/');
	if(myname)  ++myname;
	else myname=arg[0];
	
	int expr_start=-1;
	for(int i=1; i<argc; i++)
	{
		if(*arg[i]=='-')
		{
			if(!strcmp(arg[i],"--"))
			{  expr_start=i+1;  break;  }
			else if(!strcmp(arg[i],"--help"))
			{  PrintHelp(myname);  return(0);  }
			else 
			{
				for(char *c=&arg[i][1]; *c; c++)
				{
					switch(*c)
					{
						case 'o':  ++optimize;  break;
						case 'i':  ++cont_on_errors;  break;
						case 't':  ++semicol_term;  break;
						case '-':
							if(!c[1])
							{  expr_start=i+1;  goto breakargs;  }
							// fall through
						default: cerr << "Illegal option `" << *c << "' in arg "
							"\"" << arg[i] << "\"." << std::endl;  return(1);
					}
				}
			}
		}
		else
		{  expr_start=i;  break;  }
	}  breakargs:;
	
	// Make sure to call the initialisation routines: 
	exparse::InitData();
	
	Parser fl;
	ExpressionTree etree;
	
	int errors=0;
	if(expr_start>=0)
	{
		for(int i=expr_start; i<argc; i++)
		{  errors+=DoExpr(arg[i],&fl,&etree);  }
	}
	else
	{
		cerr << "Try " << myname << " --help for info on options." << std::endl;
		cerr << "Entering interactive mode." << std::endl;
		cerr << "Expressions terminated by " << (semicol_term ? "semicolon + " : "") << 
			"newline." << std::endl;
		cerr.flush();
		
		std::string ebuf;
		size_t buflen=4096;   // read buffer; expressions may be longer
		char buf[buflen];
		for(;;)
		{
			ebuf="";
			
			cout << myname << "> ";
			cout.flush();
			for(;;)
			{
				*buf='\0';
				if(!fgets(buf,buflen,stdin))
				{
					if(feof(stdin))
					{  cout << std::endl;  goto breakall;  }
					fprintf(stderr,"read error: %s\n",strerror(errno));
					return(1);
				}
				size_t bl=strlen(buf);
				size_t bl0=bl;
				int term=0;
				int pnl=0;
				while(bl>0 && (buf[bl-1]=='\n' || buf[bl-1]=='\r'))  --bl;
				if(semicol_term)
				{
					if(bl0>bl)  ++pnl;
					while(bl>0 && isspace(buf[bl-1]))  --bl;
					if(bl>0 && buf[bl-1]==';')
					{  --bl;  ++term;  }
				}
				else if(bl0>bl)  ++term;
				if(bl)
				{
					ebuf.append(buf,bl);
					if(ebuf=="help")
					{  OnLineHelp();  ebuf="";  ++term;  }
				}
				if(term)  break;
				if(pnl)
				{  cout << ">> ";  cout.flush();  }
			}
			
			if(ebuf.length())
			{  errors+=DoExpr(ebuf.c_str(),&fl,&etree);  }
		}  breakall:;
		errors=0;  // we always return with 0 from interactive mode. 
	}
	
	exparse::CleanupData();
	
	return(errors ? 1 : 0);
}

