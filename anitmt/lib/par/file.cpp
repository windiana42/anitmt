/*
 * file.cpp
 * 
 * Implementation of functions reading in parameters from a file. 
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

#include <stdio.h>
#include <errno.h>
#include "params.hpp"


namespace anitmt
{

// How deep include statements in files may be nested. 
// 0 -> never read in a file
// 1 -> read in one file but follow no include= statement 
static const int max_include_depth=32;   // should be enough

//*************************************************************
// ARRG!! I hate those damn file streams! To hell with them! 
// After playing around with them wasting far too much time 
// without getting desired results I'm through with them. 
// Definiteve through. 
// So, I got back to standard C stuff which (at least) works 
// as I expect it. Just to read in line by line it should do. 
//*************************************************************

// Reads in one line from the stream; 
// If the last char is a `\', the next line is read in, too. 
// Return value: 
//   0 -> eof reached
//  -1 -> parse error (line too long)
//  -2 -> read error
// else: number of real lines read in (normally 1)
int Animation_Parameters::Read_One_Line(FILE *is,char *buf,size_t len)
{
	if(feof(is))
		return(0);
	
	char *store=buf;
	char *end=buf+len;
	int nlines=0;
	
	*store='\0';
	for(;;)
	{
		size_t avail=end-store;
		if(avail<2)   // one char and `\0'
		{
			Error() << ": Multi-line parameter too long (>" << len << 
				" bytes)" << std::endl;
			return(-1);
		}
		errno=0;
		fgets(store,avail,is);
		size_t sl=strlen(store);
		if(!sl)
		{
			if(feof(is))
				return(nlines);
			int errv=errno;
			Error() << ": Read error: ";
			if(errv)
			{  EStream() << strerror(errv) << std::endl;  }
			else
			{  EStream() << "Hey, did you pass a binary file??" << std::endl;  }
			return(-2);
		}
		if(sl>=avail-1 && store[sl-1]!='\n')  // failsafe
		{
			Error() << ": " << 
				(nlines ? "Multi-line parameter" : "Line") << 
				"too long (>" << len << " bytes)" << std::endl;
			return(-1);
		}
		++nlines;
		// Cut off terminating newline: 
		while(sl)
		{
			char *c=&store[sl-1];
			if(*c=='\n' || *c=='\r')
			{  *c='\0';  --sl;  }
			else break;
		}
		// Check for `\' at the end of the line: 
		if(sl<=0)  break;
		if(feof(is))  break;  // DO NOT return(0) 
		if(store[sl-1]!='\\')  break;
		store[--sl]='\0';
		store+=sl;
	}
	return(nlines);
}


inline bool is_assign(char c)
{  return(c=='=' || c==':');  }


int Animation_Parameters::Do_Read_In(FILE *is)
{
	size_t linelen=4096;  // max length of a line
	char line[linelen];
	int linecnt=0,linecnt2=0;  // line counter
	int errors=0;
	
	Set_File_Mode();
	nextarg_is_next=false;
	
	for(;;)
	{
		int readstat=Read_One_Line(is,line,linelen);
		if(readstat<0)  return(readstat);  // error
		if(!readstat)   break;  // eof
		linecnt=linecnt2+1;
		linecnt2+=readstat;
		
		// Tell String_Value_Converter where we are in the file: 
		Set_Line(linecnt,linecnt2);
		
		char *start=line;
		while(isspace(*start))  ++start;
		if(!(*start))  continue;  // empty line
		if(*start==';' || *start=='#')  continue;  // comment
		
		// Identifier starts with *start. 
		char *val=start;
		while(!isspace(*val) && *val && !is_assign(*val) && *val!='#')  ++val;
		char *parse_val=NULL;
		if(*val=='#')  // comment during line
		{  *val='\0';  }
		else if(*val)
		{
			char *endit=val;
			while(isspace(*val))  ++val;
			if(*val)
			{
				// skip assignment char (not needed)
				if(is_assign(*val))  ++val;
				parse_val=val;
			}
			*endit='\0';
		}
		
		// Tell String_Value_Converter the currently parsed parameter: 
		Set_Option(start);
		
		// Look up the parameter name and parse the value: 
		if(!strcmp(start,"include"))
		{
			// recursive include...
			// Get path: 
			if(!parse_val)
			{
				Error() << ": Required path omitted" << std::endl;
				++errors;
			}
			else
			{
				std::string path;
				if(Str_To_Value(parse_val,path))
				{
					// This is probably not necessary. 
					char tmp[path.length()+1];
					strncpy(tmp,path.data(),path.length());
					tmp[path.length()]='\0';
					errors+=Read_File(tmp);
				}
				else 
				{  ++errors;  }
			}
		}
		else switch(Parse_Setting(start,parse_val))
		{
			case 0:  /* identifier not found; must output error */
				Error() << ": Illegal identifier \"" << start << "\"." << std::endl;
				++errors;
				break;
			case 1:  /* used identifier but not value */
				if(parse_val)   // ...but the user specified a value
				{
					Error() << ": Illegal value \"" << parse_val << "\"." << std::endl;
					++errors;
				}
				break;
			case -1: /* invalid argument */  break;
			case 2:  /* OK (normal case) */  break;
		}
	}
	
	return(errors ? 2 : 0);
}


int Animation_Parameters::Read_In(FILE *is)
{
	if(include_depth>=max_include_depth)
	{
		Error() << ": Max file include depth (" << max_include_depth << 
			") reached." << std::endl;
		Error() << ": (Are you sure you did not build a closed loop?)" << std::endl;
		return(1);
	}
	
	++include_depth;
	int rv=Do_Read_In(is);
	--include_depth;
	
	return(rv);
}


// Reads in the specified file and takes the parameters from 
// there. 
// Return value: 
//   0 -> success
//  >0 -> error (1 -> failed to open/read file; 2 -> parse error)
int Animation_Parameters::Read_File(const char *path)
{
	std::string save_filename;
	save_filename.assign(Get_Filename());
	
	FILE *is=fopen(path,"r");
	if(!is)
	{
		int errv=errno;
		Error() << ": Failed to open file \"" << path << "\": " << 
			strerror(errv) << std::endl;
		Set_File(save_filename);
		return(1);
	}
	
	Set_File(path);
	int rv=Read_In(is);
	fclose(is);
	Set_File(save_filename);
	return(rv);
}

} /* end of namespace anitmt */
