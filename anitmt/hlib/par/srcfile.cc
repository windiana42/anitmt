/*
 * srcfile.cc
 * 
 * Implementation of parameter source capable of reading parameters 
 * from a file. 
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

#define HLIB_IN_HLIB 1
#include "srcfile.h"

#include <string.h>
#include <ctype.h>


#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on (using assert())
#include <assert.h>
#else
#define assert(x) do{}while(0)
#endif


namespace par
{

int ParameterSource_File::_ReadLine(FILE *fp,int *linecnt)
{
	if(!linebuf)
	{
		// alloc a basic line buffer; we inlarge it if it is too small. 
		linebuf=(char*)LMalloc(256);
		if(!linebuf)  return(-1);
		bufsize=256;
	}
	linelen=0;
	*linebuf='\0';
	
	if(feof(fp))  return(1);
	
	for(;;)
	{
		char *ptr=fgets(linebuf+linelen,bufsize-linelen,fp);
		if(!ptr || !linebuf[linelen])
		{
			// nothing read. 
			if(feof(fp))
			{  return(linelen ? 0 : 1);  }
			return(-2);
		}
		
		linelen=strlen(linebuf);   // linelen>0
		if(linebuf[linelen-1]=='\n')
		{
			++(*linecnt);
			// Read a real line.
			char *prevchar=&linebuf[linelen-1];
			for(;;)
			{
				if(prevchar<linebuf)  goto done;
				if(*prevchar!='\n' && *prevchar!='\r')  break;
				--prevchar;
			}
			
			if(*prevchar!='\\')  break;
			
			// Cut the `\' off and read on 
			*prevchar='\0';
			linelen=prevchar-linebuf;
		}
		
		// Must extend line buffer. 
		size_t newsize=bufsize*2;
		char *newbuf=(char*)LRealloc(linebuf,newsize);
		if(!newbuf)  return(-1);
		linebuf=newbuf;
		bufsize=newsize;
	}
	done:;
	return(0);
}


// If allow_recursion=-1, *include will be ignored. 
// Returns no of errors or -1 -> malloc failed. 
int ParameterSource_File::_ReadFile(const char *_file,int allow_recursion)
{
	int fflag=0;
	RefString file(_file,&fflag);
	if(fflag)
	{
		FailedFileOp(file,-1,/*path refstring alloc*/-1);
		return(-1);
	}
	
	FILE *fp=fopen(file.str(),"r");
	if(!fp)
	{  FailedFileOp(file.str(),-1,/*open*/0);  return(1);  }
	
	// This constructor will never fail: 
	ParamArg::Origin origin(ParamArg::FromFile,file,/*line number=*/0);
	
	int errors=0;
	int *lineno_ptr=&origin.opos;
	
	// Section depth stack: 
	// Stores number of sections we went down at the last "section" 
	// command. 
	int *sdstack=NULL;   // array of size [sds_dim]
	int sds_dim=0;  // stack size
	int sds_top=-1;  // top index sdstack[sds_top]
	int ignore_above=-2;  // ignore params in non-existant sections
	
	for(;;)
	{
		int rls=_ReadLine(fp,lineno_ptr);
		if(rls==1)   // EOF
		{  assert(linelen==0);  break;  }
		if(rls<0)  // read error
		{
			FailedFileOp(file,*lineno_ptr,-rls);
			++errors;
			break;
		}
		assert(!rls);
		
		// strip off trailing garbage: 
		while(linelen)
		{
			--linelen;
			if(!isspace(linebuf[linelen]))
			{  ++linelen;  break;  }
		}
		linebuf[linelen]='\0';
		
		// skip whitespace at the beginning: 
		char *line=linebuf;
		while(*line && isspace(*line))  ++line;
		
		// Check for empty lines: 
		if(!(*line))  continue;
		
		// Check for include statement and 
		// check if this is a section beginning/end: 
		char *lcmd=line;
		int special=0;
		int stype=0;
		if(*lcmd!='*')  goto spdone;  // *section, *end, ...
		// Find command in line: 
		++lcmd;
		while(*lcmd && isspace(*lcmd))  ++lcmd;
		if(!(*lcmd) || *lcmd=='#')  // empty or comment
		{  goto spdone;  }  // no special line
		
		// We are only here if the line begins with preprocessor prefix. 
		stype=-1;
		if(!strncmp(lcmd,"include",7))
		{
			if(allow_recursion<0)
			{  PreprocessorError(PPWarningIgnoringInclude,&origin,
				curr_sect,NULL);  }
			else
			{  special=7;  stype=1;  }
		}
		else if(!strncmp(lcmd,"section",7))
		{  special=7;  stype=2;  }
		else if(!strncmp(lcmd,"end",3))
		{  special=3;  stype=3;  }
		if(stype>=0)
		{
			if(lcmd[special] && !isspace(lcmd[special]))
			{  stype=-1;  }  // error
		}
		if(stype==1 || stype==2)
		{
			char *sname=&lcmd[special];
			while(isspace(*sname))  ++sname;
			if(*sname=='#' || !(*sname))
			{  PreprocessorError(PPArgOmitted,&origin,curr_sect,NULL);
				++errors;  goto spdone;  }
			
			size_t slen=0;
			char *end=NULL;
			if(_ParseString(&sname,&slen,&end,&origin))
			{  ++errors;  goto spdone;  }
			while(isspace(*end))  ++end;
			if(*end && *end!='#')
			{  PreprocessorError(PPWarningIgnoringGarbageAtEol,&origin,
				curr_sect,NULL);  }
			sname[slen]='\0';
			if(stype==1)  // include
			{
				// include sname of size slen. 
				if(!allow_recursion)
				{  PreprocessorError(PPIncludeNestedTooOften,&origin,
					curr_sect,NULL);  ++errors;  goto spdone;  }
				int rf=_ReadFile(sname,allow_recursion-1);
				if(rf<0)  // malloc failure
				{  errors=-1;  break;  }
				errors+=rf;
			}
			else if(stype==2)  // section
			{
				// Go to subsection with specified name: 
				// This must be an immediate subsection of the current 
				// section (newsect->up==curr_sect). 
				ParamArg::Origin origin(ParamArg::FromFile,file,*lineno_ptr);
				Section *down=manager->FindSection(sname,curr_sect,
					/*tell_section_handler=*/&origin);
				if(!down)
				{
					PreprocessorError(PPUnknownSection,&origin,
						curr_sect,sname);
					++errors;
					if(ignore_above==-2)  ignore_above=sds_top;
				}
				// Alyways put it on the stack because that is more 
				// user-friendly (aviod error for too many "*end" 
				// statements because of misspelled section name. 
				// Check subsection depth: 
				int depth=0;
				if(down)
				{
					Section *si=down;
					for(; si && si!=curr_sect; si=si->up,depth++);
					assert(si && depth>0);  // Otherwise: down not below curr_sect. 
				}
				if(sds_top+1>=sds_dim)  // need to realloc stack
				{
					sds_dim+=16;
					int *oldptr=sdstack;
					sdstack=(int*)LRealloc(sdstack,sds_dim*sizeof(int));
					if(!sdstack)
					{
						sdstack=oldptr;  sds_dim-=16;
						errors=-1;  // alloc failure
						break;
					}
				}
				sdstack[++sds_top]=depth;  // "push"
				if(down)
				{  curr_sect=down;  }
			}
		}
		else if(stype==3)  // end
		{
			char *sname=&lcmd[special];
			while(isspace(*sname))  ++sname;
			if(*sname!='#' && (*sname))
			{  PreprocessorError(PPWarningIgnoringGarbageAtEol,&origin,
				curr_sect,NULL);  }
			// Process end statement: 
			// We must go one section up but not beyond the file top 
			// section (file_top_sect)
			if(sds_top<0)
			{
				too_many_ends:;
				PreprocessorError(PPTooManyEndStatements,&origin,
					curr_sect,NULL);
				++errors;
				ignore_above=-2;
				continue;
			}
			for(int depth=sdstack[sds_top--]; depth; depth--)
			{
				if(curr_sect==file_top_sect)
				{  goto too_many_ends;  }
				curr_sect=curr_sect->up;
				assert(curr_sect);
			}
			if(ignore_above!=-2 && sds_top<=ignore_above)
			{  ignore_above=-2;  }
		}
		else  // stype==-1
		{
			lcmd[-1]='*';
			PreprocessorError(PPIllegalPPCommand,&origin,
				curr_sect,lcmd-1);
			++errors;
		}
		spdone:;
		
		// Nothing more to do if that was a pp command: 
		if(stype)  continue;
		
		// Ignore params in unknown sections to prevents lots 
		// of unecessary errors. 
		if(ignore_above!=-2)  continue;
		
		// Check for comments: 
		if(*line=='#')  continue;
		
		int fflag=0;
		ParamArg tmp(line,file,*lineno_ptr,&fflag);
		if(fflag)
		{
			FailedFileOp(file,/*param arg alloc failed*/3,-1);
			errors=-1;
			break;
		}
		int rv=Parse(&tmp,curr_sect);
		if(rv<0)
		{  ++errors;  }
	}
	
	LFree(sdstack);
	
	fclose(fp);
	return(errors);
}


// *str=beginning of string
// Return value: 0 -> OK; >0 -> error
// NOT used for parameters, only used for *section, *include. 
int ParameterSource_File::_ParseString(char **str,size_t *retlen,char **end,
	const ParamArg::Origin *origin)
{
	char *s=*str;
	if(*s=='\"')
	{
		++(*str);  ++s;
		while(*s && *s!='\"')  ++s;
		if(!(*s))
		{  PreprocessorError(PPUnterminatedString,origin,
			curr_sect,NULL);  return(1);  }
		*retlen=s-(*str);
		*end=(s+1);
	}
	else
	{
		while(*s && !isspace(*s))  ++s;
		*retlen=s-(*str);
		*end=s;
	}
	return(0);
}


int ParameterSource_File::ReadFile(const char *file,int allow_recursion,
	Section *topsect)
{
	file_top_sect = (topsect ? topsect : manager->TopSection());
	curr_sect=file_top_sect;
	
	int rv=_ReadFile(file,allow_recursion);
	
	_ResetLinebuf();
	
	file_top_sect=NULL;
	curr_sect=NULL;
	return(rv);
}


int ParameterSource_File::Parse(ParamArg *pa,Section *topsect)
{
	return(FindCopyParseParam(pa,NULL,NULL,topsect));
}


void ParameterSource_File::_ResetLinebuf()
{
	LFree(linebuf);
	linebuf=NULL;
	bufsize=0;
	linelen=0;
}

ParameterSource_File::ParameterSource_File(
	ParameterManager *_manager,int *failflag) : 
	ParameterSource(_manager,failflag)
{
	file_top_sect=NULL;
	curr_sect=NULL;
	
	linebuf=NULL;
	bufsize=0;
	linelen=0;
}

ParameterSource_File::~ParameterSource_File()
{
	_ResetLinebuf();
}

}  // namespace end 
