/*
 * searchpath.cpp
 * 
 * Implementation of search path class. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   May 2001   started writing
 *
 */


#include "searchpath.hpp"
#include "instr.hpp"

namespace output_io
{

bool Search_Path::try_open(class Input_Stream *is,
	int search_current,
	const std::string &current_file,
	bool honor_search_path)
{
	const char *thisfile=is->CPath();
	// Absolute paths get no search path check. 
	if(*thisfile=='/' || *thisfile=='\0')
	{  return(is->open(/*print_error=*/false));  }
	
	std::string curr_path;
	if(!current_file.length())
	{  search_current=0;  }
	if(search_current)
	{
		// check if current_file has a path component: 
		std::string tmp; tmp.assign(current_file); // !! tmp must exist longer than currfile !!
#warning why assign? what about tmp = current_file?

		const char *currfile=tmp.c_str();
		const char *cfend=strrchr(currfile,'/');
		if(cfend)  // current_file has a path component; use it: 
		{  curr_path.assign(currfile,cfend-currfile+1);  }
		else  // current_file has no path component; assume ``./''
		{  curr_path.assign("./");  }
		curr_path.append(is->Path());
	}
	
	if(search_current<0)
	{
		if(is->open(curr_path))  // will not print error 
		{  return(true);  }
	}
	
	// Do search path tests: 
	if(honor_search_path)
	{
		std::string path;
		for(std::list<std::string>::const_iterator i=spath.begin();
			i!=spath.end(); i++)
		{
			path=(*i)+is->Path();
			if(is->open(path))  // will not print error 
			{  return(true);  }
		}
	}
	
	if(search_current>0)
	{
		if(is->open(curr_path))  // will not print error 
		{  return(true);  }
	}
	
	return(false);
}


int Search_Path::add(const std::string &dir)
{
	if(!dir.length())
	{  return(2);  }
	
	std::string path=dir;
	if(path[path.length()-1]!='/')
	{  path.append("/");  }
	
	// Check if the path is already in the list
	for(std::list<std::string>::const_iterator i=spath.begin();
		i!=spath.end(); i++)
	{
		if((*i) == path)
		{  return(1);  }
	}
	
	spath.push_back(path);
	return(0);
}

}  // namespace end
