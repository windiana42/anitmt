/*
 * sindentcout.cc
 * 
 * Implementation of simple class allowing you to do simple formatting with 
 * the console output (output to some FILE*) like inserting newlines before 
 * the line is over keeping works together and doing indention on the output. 
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

#include "sindentcout.h"
#include <string.h>

#include <sys/ioctl.h>

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on (uses assert())
#include <assert.h>
#else
#define assert(x) 
#endif

namespace par
{

void SimpleIndentConsoleOutput::SetIndent(int _indent)
{
	if(_indent>columns-10)  _indent=columns-10;
	if(_indent<0)  _indent=0;
	indent=_indent;
}


void SimpleIndentConsoleOutput::SetFile(FILE *_out)
{
	if(out!=_out)
	{
		out=_out;
		lpos=0;
		SetIndent(0);
		columns=_GetTermCols(fileno(out));
	}
}


void SimpleIndentConsoleOutput::Puts(const char *str,
	int _indent)
{
	if(_indent>=0)
	{  SetIndent(_indent);  }
	if(!str)  return;  // emergency break 
	
	if(lpos>=columns)  // must use a new line
	{  fprintf(out,"\n");  lpos=0;  }
	
	char buf[columns+1];
	int bufuse=0;
	while(*str)
	{
		const char *spc=str;
		for(; *spc; spc++)
		{  if(*spc=='\n' || *spc==' ')  break;  }
		int wordlen=spc-str;
		int ignspc=0;
		
		// Now, we must print the word beginning with str. 
		if(lpos<indent)
		{  // must write indent: 
			int len=indent-lpos;
			assert(bufuse+len<columns);
			memset(buf,' ',len);
			bufuse+=len;
			lpos+=len;
		}
		if(lpos+wordlen>columns)
		{  // word does not fit into this line 
			if(lpos>indent)
			{  // Flush the line: 
				assert(bufuse<=columns);
				buf[bufuse]='\0';
				fprintf(out,"%s\n",buf);
				bufuse=lpos=0;
			}
			if(lpos<indent)
			{  // must fill up with indent
				int len=indent-lpos;
				memset(buf+bufuse,' ',len);
				bufuse+=len;
				lpos+=len;
			}
			// Now, write word: 
			if(lpos+wordlen>columns)
			{
				// Word does not fit into empty line? 
				// So what?! -- It simply HAS to...
				assert(bufuse<=columns);
				buf[bufuse]='\0';  // okay. 
				fprintf(out,"%s%.*s\n",buf,wordlen,str);
				bufuse=lpos=0;
				ignspc=(*spc!='\n');
			}
			else goto jumpdown;
		}
		else  // (lpos+wordlen<=columns)
		{  // word fits into this line
			jumpdown:;
			assert(bufuse+wordlen<=columns);
			memcpy(buf+bufuse,str,wordlen);
			bufuse+=wordlen;
			lpos+=wordlen;
			if(lpos+1>=columns || *spc=='\n')
			{
				assert(bufuse<=columns);
				buf[bufuse]='\0';  // okay. 
				fprintf(out,"%s\n",buf);
				bufuse=lpos=0;
				ignspc=(*spc!='\n');
			}
			else
			{
				buf[bufuse++]=' ';
				++lpos;   // lpos<columns here. 
			}
		}
		
		// Okay, now skip the spaces: 
		if(!(*spc))  break;
		++spc;
		while(*spc==' ' || *spc=='\n')
		{
			while(*spc==' ')
			{
				if(lpos<columns && !ignspc)
				{
					assert(bufuse<columns);  // <- okay!!!
					buf[bufuse++]=' ';
					++lpos;
				}
				if(lpos>=columns)  ignspc=1;
				++spc;
			}
			while(*spc=='\n')
			{
				assert(bufuse<=columns);
				buf[bufuse]='\0';  // okay. 
				fprintf(out,"%s\n",buf);
				bufuse=lpos=0;
				++spc;
				ignspc=0;
			}
		}
		
		str=spc;
	}
	
	// Flush buffer if there is something left: 
	if(bufuse)
	{
		assert(bufuse<=columns);
		buf[bufuse]='\0';  // okay. 
		fprintf(out,"%s",buf);
		bufuse=0;
	}
}


int SimpleIndentConsoleOutput::_Print(const char *str,va_list ap)
{
	char buf[256];
	int rv=vsnprintf(buf,256,str,ap);
	#if TESTING
	if(rv<0)
	{  fprintf(stderr,"You need C99-compatible snprintf(). "
		"(glibc>=2.1)\n");  abort();  }
	#endif
	if(rv<255)
	{
		Puts(buf);
		return(0);
	}
	
	// Re-try, this time with large enough buffer: 
	char *tmp=(char*)LMalloc(rv+1);
	if(!tmp)
	{  return(1);  }
	
	int rv2=vsnprintf(tmp,rv+1,str,ap);
	assert(rv2==rv);  // else -> you need C99-compatible snprintf() (glibc>=2.1)
	
	Puts(tmp);
	
	LFree(tmp);
	return(0);
}


int SimpleIndentConsoleOutput::Print(const char *str,...)
{
	va_list ap;
	va_start(ap,str);
	int rv=_Print(str,ap);
	va_end(ap);
	return(rv);
}

int SimpleIndentConsoleOutput::operator()(const char *str,...)
{
	va_list ap;
	va_start(ap,str);
	int rv=_Print(str,ap);
	va_end(ap);
	return(rv);
}


int SimpleIndentConsoleOutput::_GetTermCols(int fd)
{
	// Use ioctl: 
	struct winsize wsz;
	if(ioctl(fd,TIOCGWINSZ,&wsz))
	{  return(80);  }
	
	int cols=wsz.ws_col;
	if(cols>1024)  cols=1024;
	if(cols<10)    cols=10;
	return(cols);
}


SimpleIndentConsoleOutput::SimpleIndentConsoleOutput(int * /*failflag*/)
{
	lpos=0;
	columns=80;
	indent=0;
	out=stdout;
	
	#if 0
	char *ev=getenv("COLUMNS");
	if(ev)
	{
		while(*ev && *ev!='=')  ++ev;
		if(*ev=='=')  ++ev;
		if(*ev)
		{
			int n=atoi(ev);
			if(n>1024)  columns=1024;
			else if(n>=10)  columns=n;
		}
	}
	#endif
	
	columns=_GetTermCols(fileno(out));
	
	/*int failed=0;*/
	
	/*if(failflag)
	{  *failflag+=failed;  }
	else if(failed)
	{  ConstructorFailedExit("SICO");  }*/
}

/*SimpleIndentConsoleOutput::~SimpleIndentConsoleOutput()
{
	is inline
}*/

}  // namespace end


#if 0
int main(int argc,char **arg)
{
	par::SimpleIndentConsoleOutput sico;
	sico.SetFile(stderr);
	for(int i=1; i<argc; i++)
	{
		sico.Puts(arg[i],(i-1)*8);
	}
	fprintf(stderr,"\n");
	
	sico.SetIndent(10);
	sico("this is the first text to be displayed here bkdjf lksdfdjlsdjf lsd "
		"skdf sdl lsdf jlsdkf lsdjf lsdkjf sdjflsd flkdasjflkdsj kdsjf sdkjf "
		"sdkfh sldkj lsdjf ldsf sdf lsdkf lksdjf lsdkjf lsj ls lsdjf lsdkfj l\n");
	sico.SetIndent(5);
	sico("this should not be indented as much aslkdlakjasldkj asd asdj k "
		"lksd ldaf lsdkf lsdf lsdjf sdljf osd lsgh lsdghlsdk lksdgj sdkj lfs"
		" jdfh sdhf kjsdhf kjsdhf kjsdhf ksdjhf kdjsh fksd hksdh ksdh "
		"skd jksdj sdlj \n");
	sico.SetIndent(10);
	/* test buffer code: 
	char buf[512];
	memset(buf,'x',511);
	buf[511]='\0';
	sico("okay %s %d",buf,10); */
	
	return(0);
}
#endif

