/*
 * outstr.cpp
 * 
 * Output stream wrapper/interface. 
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

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "outstr.hpp"

namespace output_io
{

bool Output_Stream::open(bool allow_overwrite) throw()
{
	if(IsOpen())  return(true);
	fd=::open(path.c_str(),O_TRUNC | O_WRONLY | O_CREAT | 
		(allow_overwrite ? 0 : O_EXCL),0666);
	off=0;
	if(fd<0)
	{
		cerr << "While opening " << CPath() << " for writing: " << 
				strerror(errno) << std::endl;
		return(false);
	}
	file_opened();
	return(true);
}


void Output_Stream::close()
{
	file_close();
	off=0;
	if(!IsOpen())  return;
	if(::close(fd)<0)
	{
		cerr << "While closing " << CPath() << ": " << 
			strerror(errno) << std::endl;
		abort();
	}
	fd=-1;
}


void Output_Stream::write(const char *buf,size_t len)
{
	size_t done=0;
	while(done<len)
	{
		errno=0;
		ssize_t wr;
		if(!IsOpen())
		{  errno=EBADF;  wr=-1;  }
		else
		{  wr=::write(fd,buf+done,len-done);  }
		if(wr<=0)
		{
			cerr << "While writing " << CPath() << ": " << 
				strerror(errno) << std::endl;
			abort();
		}
		done+=wr;
		off+=wr;
	}
}


Output_Stream::Output_Stream(const std::string &_path) throw()
{
	path.assign(_path);
	fd=-1;
	off=0;
}

Output_Stream::~Output_Stream() throw()
{
	if(IsOpen())  // important
	{  close();  }
}

}  // namespace end 
