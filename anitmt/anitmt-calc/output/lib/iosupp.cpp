/*
 * iosupp.cpp
 * 
 * IO support routines. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to > wwieser -a- gmx -*- de <
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

#include "lproto.hpp"

#include "binstr.hpp"
#include "boutstr.hpp"


namespace output_io
{

size_t Get_Preferred_IO_Size(int fd)
{
	// Get preferred file IO size: 
	struct stat st;
	if(!fstat(fd,&st))
	{
		if(st.st_blksize>32768)  // won't make too large buffers
		{  return(32768);  }
		else if(st.st_blksize<1024)  // and won't make too small buffers
		{  return(1024);  }
		else
		{  return(st.st_blksize);  }
	}
	// fstat() failed. Use default: 
	return(4096);
}

}  // namespace end 
