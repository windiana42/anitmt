/*
 * fdfifobuf.cc
 * 
 * Implementation of class FDFifoBuffer, a class primarily for 
 * buffering a data stream copied from one FD to another. 
 *
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/fdfifobuf.h>

#include <string.h>

#if HAVE_SYS_UIO_H
#  include <sys/uio.h>
#endif

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#warning Using assert()
#include <assert.h>
#else
#define assert(x) do{}while(0)
#endif


// Read from fd into this buffer. 
ssize_t FDFifoBuffer::ReadFD(int fd,size_t maxlen)
{
	// See how much free space is in the buffer: 
	size_t avail=bufsize-bufuse;
	
	// Okay, see how much we want to read: 
	if(maxlen && avail>maxlen)
	{  avail=maxlen;  }
	
	// Now, set up read io vector: 
	struct iovec rio[2];  // never need more than 2
	int need_readv=0;
	rio->iov_base=bufheadW;
	rio->iov_len=bufend-bufheadW;
	if(rio->iov_len>=avail)   // >= is CORRECT. 
	{  rio->iov_len=avail;  }
	else
	{
		need_readv=1;
		rio[1].iov_base=bufbase;
		rio[1].iov_len=avail-rio->iov_len;
	}
	
	// Actually read the stuff in: 
	ssize_t rd = need_readv ? 
		readv(fd,rio,2) : read(fd,rio->iov_base,rio->iov_len);
	if(rd>0)
	{
		// Read in data; update buffer stuff: 
		size_t rrd=rd;
		size_t to_end=bufend-bufheadW;
		if(rrd<to_end)
		{  bufheadW+=rrd;  }
		else
		{  bufheadW=bufbase+(rrd-to_end);  }
		bufuse+=rrd;
		assert(bufuse<=bufsize);
	}
	
	return(rd);
}


// Copy from passed ibuf to this buffer. 
size_t FDFifoBuffer::ReadBuf(const char *ibuf,size_t ibuflen)
{
	size_t avail=bufsize-bufuse;
	if(avail>ibuflen)
	{  avail=ibuflen;  }
	
	size_t tail_room=bufend-bufheadW;
	bufuse+=avail;   // OK. 
	if(tail_room<=avail)
	{
		memcpy(bufheadW,ibuf,tail_room);
		bufheadW=bufbase;
		avail-=tail_room;
		ibuf+=tail_room;
	}
	if(avail)
	{
		memcpy(bufheadW,ibuf,avail);
		bufheadW+=avail;
	}
	
	return(avail);
}


// Write from this buffer into fd. 
ssize_t FDFifoBuffer::WriteFD(int fd,size_t maxlen)
{
	// See how much data is available in the buffer
	size_t avail=bufuse;
	
	// Okay, see how much we may write: 
	if(maxlen && avail>maxlen)
	{  avail=maxlen;  }
	
	// Now, set up write io vector: 
	struct iovec wio[2];  // never need more than 2
	int need_writev=0;
	wio->iov_base=bufheadR;
	wio->iov_len=bufend-bufheadR;
	if(wio->iov_len>=avail)   // >= is CORRECT. 
	{  wio->iov_len=avail;  }
	else
	{
		need_writev=1;
		wio[1].iov_base=bufbase;
		wio[1].iov_len=avail-wio->iov_len;
	}
	
	// Actually write the stuff out: 
	ssize_t wr = need_writev ? 
		writev(fd,wio,2) : write(fd,wio->iov_base,wio->iov_len);
	if(wr>0)
	{
		// Wrote out data; update buffer stuff: 
		size_t wwd=wr;
		size_t to_end=bufend-bufheadR;
		if(wwd<to_end)
		{  bufheadR+=wwd;  }
		else
		{  bufheadR=bufbase+(wwd-to_end);  }
		assert(bufuse>=wwd);
		bufuse-=wwd;
		if(!bufuse)
		{
			// Buffer is empty; we can start at the beginning (avoiding 
			// readv()/writev() overhead: 
			bufheadR=bufbase;
			bufheadW=bufbase;
		}
	}
	
	return(wr);
}


// Copy from this buffer to passed obuf. 
size_t FDFifoBuffer::WriteBuf(char *obuf,size_t obuflen)
{
	size_t avail=bufuse;
	if(avail>obuflen)
	{  avail=obuflen;  }
	
	size_t tail_room=bufend-bufheadR;
	bufuse-=avail;   // OK. 
	if(tail_room<=avail)
	{
		if(obuf)
		{
			memcpy(obuf,bufheadR,tail_room);
			obuf+=tail_room;
		}
		bufheadR=bufbase;
		avail-=tail_room;
	}
	if(avail)
	{
		if(obuf)
		{  memcpy(obuf,bufheadR,avail);  }
		bufheadR+=avail;
	}
	if(!bufuse)
	{
		// Buffer is empty; we can start at the beginning (avoiding 
		// readv()/writev() overhead: 
		bufheadR=bufbase;
		bufheadW=bufbase;
	}
	
	return(avail);
}


int FDFifoBuffer::ResizeBuf(size_t newlen)
{
	if(newlen<bufuse)
	{  return(1);  }
	
	// Move contents to the beginning of the buffer so that 
	// bufheadR=bufbase and bufheadW=bufbase+bufuse. 
	if(bufuse)
	{
		// NOT IMPLEMENTED. 
		assert(0);
	}
	
	char *oldbase=bufbase;
	bufbase=(char*)LRealloc(bufbase,newlen);
	if(!bufbase)
	{
		bufbase=oldbase;
		return(-1);
	}
	
	bufsize=newlen;
	bufend=bufbase+bufsize;
	bufheadR=bufbase;
	bufheadW=bufheadR+bufuse;
	return(0);
}


FDFifoBuffer::FDFifoBuffer(size_t size,int *failflag)
{
	int failed=0;
	
	bufsize=size;
	if(size)
	{
		bufbase=(char*)LMalloc(size);
		if(!bufbase)
		{  ++failed;  }
	}
	else
	{  bufbase=NULL;  }
	
	bufend=bufbase+bufsize;
	bufheadW=bufbase;
	bufheadR=bufbase;
	bufuse=0;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("FifoBuf");  }
}

FDFifoBuffer::~FDFifoBuffer()
{
	bufsize=0;
	bufbase=(char*)LFree(bufbase);
	bufheadW=NULL;
	bufheadR=NULL;
}
