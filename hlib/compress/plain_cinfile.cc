/*
 * plain_cinfile.cc
 * 
 * Implementation of a plain non-compressed file read access class 
 * as wrapper with the same interface as the compressed file readers. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include <hlib/prototypes.h>  /* MUST BE FIRST */

#include "plain_cinfile.h"

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif


NotCompressedFileReader::ErrorState 
	NotCompressedFileReader::open(const char *filename)
{
	if(fd>=0)
	{
		ErrorState es=close();
		if(es)
		{  return(es);  }
	}
	
	errno=0;
	fd=::open(filename,O_RDONLY);
	if(fd<0)
	{  return(ES_Errno);  }
	
	return(ES_Success);
}


ssize_t NotCompressedFileReader::read(char *buf,size_t len)
{
	if(fd<0)
	{  return(ES_NotOpened);  }
	if(len==0)
	{  return(0);  }
	char *dest=buf;
	size_t left=len;
	for(;;)
	{
		retry:;
		errno=0;
		ssize_t rd=::read(fd,dest,left);
		if(rd<0)
		{
			if(errno==EINTR)
			{  goto retry;  }
			return(dest-buf);
		}
		if(!rd) break;
		dest+=rd;
		left-=rd;
		if(left<=0)  break;
	}
	return(dest-buf);
}


NotCompressedFileReader::ErrorState 
	NotCompressedFileReader::close()
{
	if(fd>=0)
	{
		int rv=::close(fd);
		fd=-1;
		if(rv<0)
		{  return(ES_Errno);  }
	}
	return(ES_Success);
}


NotCompressedFileReader::NotCompressedFileReader(int *failflag) : 
	CompressedFileReader(failflag)
{
	fd=-1;
}

NotCompressedFileReader::~NotCompressedFileReader()
{
	if(fd>=0)
	{  close();  }
}
