/*
 * bzip2_cinfile.cc
 * 
 * Implementation of a bzip2 compressed file read access class. 
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

#include "bzip2_cinfile.h"

#if HAVE_BZLIB_H
#  include <bzlib.h>
#endif


BZip2CompressedFileReader::ErrorState 
	BZip2CompressedFileReader::open(const char *filename)
{
	if(bzf)
	{
		ErrorState es=close();
		if(es)
		{  return(es);  }
	}
	
	fp=fopen(filename,"rb");
	if(!fp)
	{  return(ES_Errno);  }
	
	int err=0;
	bzf=BZ2_bzReadOpen(&err,fp,/*small=*/0,/*verbosity=*/0,/*unused=*/NULL,0);
	if(err!=BZ_OK)
	{
		if(bzf)
		{
			int dummy;
			BZ2_bzReadClose(&dummy,bzf);
		}
		fclose(fp);  // Needed. 
		
		if(err==BZ_MEM_ERROR)
		{  return(ES_Allocfail);  }
		else if(err==BZ_IO_ERROR)
		{  return(ES_Errno);  }
		return(ES_InternalError);
	}
	if(!bzf)  // This should never happen if we reach here. 
	{
		fclose(fp);  // Needed. 
		return(ES_InternalError);
	}
	
	return(ES_Success);
}


ssize_t BZip2CompressedFileReader::read(char *buf,size_t len)
{
	if(!bzf)
	{  return(ES_NotOpened);  }
	if(sticky_error)
	{
		ErrorState tmp=sticky_error;
		sticky_error=ES_Success;
		return(tmp);
	}
	
	if(len==0 || eof_reached)
	{  return(0);  }
	
	int err=0;
	ssize_t rd=BZ2_bzRead(&err,bzf,buf,len);
	
	// If rd was returned, then rd bytes were read and are 
	// valid. I then return rd and a possible error is reported the 
	// next time read() gets called. Otherwise, return the error 
	// immediately. 
	
	switch(err)
	{
		case BZ_OK:                sticky_error=ES_Success;        break;
		case BZ_UNEXPECTED_EOF:    sticky_error=ES_UnexpectedEOF;  break;
		case BZ_DATA_ERROR:
		case BZ_DATA_ERROR_MAGIC:  sticky_error=ES_DataCorrupt;    break;
		case BZ_MEM_ERROR:         sticky_error=ES_Allocfail;      break;
		case BZ_STREAM_END:        sticky_error=ES_Success;
		                           eof_reached=1;                  break;
		default:                   sticky_error=ES_InternalError;  break;
	}
	
	if(rd)
	{  return(rd);  }
	ErrorState tmp=sticky_error;
	sticky_error=ES_Success;
	return(tmp);
}


BZip2CompressedFileReader::ErrorState 
	BZip2CompressedFileReader::close()
{
	ErrorState state=ES_Success;
	
	if(bzf)
	{
		int err=0;
		BZ2_bzReadClose(&err,bzf);
		switch(err)
		{
			case BZ_OK:  state=ES_Success;        break;
			default:     state=ES_InternalError;  break;
		}
	}
	
	if(fp)
	{
		if(fclose(fp)<0)
		{
			if(!state)
			{  state=ES_Errno;  }
		}
	}
	
	fp=NULL;
	bzf=NULL;
	sticky_error=ES_Success;
	eof_reached=0;
	
	return(state);
}


BZip2CompressedFileReader::BZip2CompressedFileReader(int *failflag) : 
	CompressedFileReader(failflag)
{
	fp=NULL;
	bzf=NULL;
	sticky_error=ES_Success;
	eof_reached=0;
}

BZip2CompressedFileReader::~BZip2CompressedFileReader()
{
	if(bzf)
	{  close();  }
}
