/*
 * instr.cpp
 * 
 * Input stream wrapper/interface. 
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
 *   Apr 2001   started writing
 *
 */

#include <fcntl.h>
#include <errno.h>

#include "instr.hpp"

#include <iostream>

namespace output_io
{

bool Input_Stream::open(bool print_error) throw()
{
	if(IsOpen())  return(true);
	fd=::open(path.c_str(),O_RDONLY);
	off=0;
	eof_reached=false;
	if(fd<0)
	{
		if(print_error)
		{  std::cerr << "While opening " << CPath() << " for reading: " << 
				strerror(errno) << std::endl;  }
		return(false);
	}
	file_opened();
	return(true);
}


bool Input_Stream::open(const std::string &_path) throw()
{
	if(IsOpen())  return(true);
	path.assign(_path);
	return(open(false));
}


void Input_Stream::close()
{
	file_close();
	off=0;
	if(!IsOpen())  return;
	if(::close(fd)<0)
	{
		std::cerr << "While closing " << CPath() << ": " << 
			strerror(errno) << std::endl;
		abort();
	}
	fd=-1;
}


size_t Input_Stream::read(char *buf,size_t len)
{
	if(!IsOpen())  return(0);  // file not open
	if(eof_reached)  return(0);
	size_t done=0;
	while(done<len)
	{
		ssize_t rd=::read(fd,buf+done,len-done);
		if(rd<0)
		{
			std::cerr << "While reading " << CPath() << ": " << 
				strerror(errno) << std::endl;
			abort();
		}
		if(!rd)  // eof
		{
			eof_reached=true;
			return(done);
		}
		done+=rd;
		off+=rd;
	}
	return(done);
}

#if 0
void Input_Stream::seek(size_t _off)
{
	if(!IsOpen())  return;  // file not open
	if(off==_off)  return;
	off=_off;
	eof_reached=false;
	if(::lseek(fd,off,SEEK_SET)<0)
	{
		std::cerr << "While seeking in " << CPath() << " (off=" << off << "): " << 
			strerror(errno) << std::endl;
		abort();
	}
}
#endif

Input_Stream::Input_Stream(const std::string &_path) throw()
{
	path.assign(_path);
	fd=-1;
	off=0;
}

Input_Stream::~Input_Stream() throw()
{
	if(IsOpen())  // important as derived classes must close on destruct or 
	{  close();  }  // the destructor will call a virtual function (file_close())
}

}  // namespace end 
