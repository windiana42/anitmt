/*
 * gzip_cinfile.cc
 * 
 * Implementation of a gzip compressed file read access class. 
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

#include "gzip_cinfile.h"

#if HAVE_ZLIB_H
#  include <zlib.h>
#endif


GZipCompressedFileReader::ErrorState 
	GZipCompressedFileReader::open(const char *filename)
{
	if(gzf)
	{
		ErrorState es=close();
		if(es)
		{  return(es);  }
	}
	
	errno=0;
	gzf=gzopen(filename,"rb");
	if(!gzf)
	{
		if(errno==0)
		{  return(ES_Allocfail);  }
		return(ES_Errno);
	}
	
	return(ES_Success);
}


ssize_t GZipCompressedFileReader::read(char *buf,size_t len)
{
	if(!gzf)
	{  return(ES_NotOpened);  }
	if(len==0)
	{  return(0);  }
	#warning "Add support for ES_UnexpectedEOF, ES_DataCorrupt."
	return(gzread(gzf,buf,len));
}


GZipCompressedFileReader::ErrorState 
	GZipCompressedFileReader::close()
{
	if(gzf)
	{
		int rv=gzclose(gzf);
		gzf=NULL;
		switch(rv)
		{
			case Z_ERRNO:      return(ES_Errno);
			case Z_MEM_ERROR:  return(ES_Allocfail);
		}
	}
	return(ES_Success);
}


GZipCompressedFileReader::GZipCompressedFileReader(int *failflag) : 
	CompressedFileReader(failflag)
{
	gzf=NULL;
}

GZipCompressedFileReader::~GZipCompressedFileReader()
{
	if(gzf)
	{  close();  }
}
