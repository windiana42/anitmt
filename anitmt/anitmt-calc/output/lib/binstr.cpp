/*
 * binstr.cpp
 * 
 * Buffered input stream using Input_Stream. 
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


#include "binstr.hpp"

namespace output_io
{

// Gets called after a file has been opened. 
void Buffered_Input_Stream::file_opened()
{
	file_close();  // be sure...
	
	bsize=Get_Preferred_IO_Size(fd);
	buf=new char[bsize];
	blen=0;
}


// Gets called before a file is closed. 
void Buffered_Input_Stream::file_close()
{
	if(buf)
	{  delete[] buf;  buf=NULL;  }
	bsize=0;
	blen=0;
}


// May only be called if the whole request (len bytes) can be 
// copied from the buffer. 
void Buffered_Input_Stream::_buf_read(char *dest,size_t destlen) throw()
{
	assert(blen>=destlen);
	if(destlen)
	{
		memcpy(dest,buf,destlen);
		blen-=destlen;
		if(blen)
		{  memmove(buf,buf+destlen,blen);  }
	}
}


// !!Should only be called when the buffer is completely empty!!
inline void Buffered_Input_Stream::_fill_buf()
{
	assert(blen==0);
	blen=Input_Stream::read(buf,bsize);
	// blen<bsize means `EOF reached'. 
}


size_t Buffered_Input_Stream::read(char *ibuf,size_t ilen)
{
	if(!IsOpen())
	{  return(0);  }
	
	if(!blen && ilen==bsize)
	{
		// Want to read one chunk which is as long as the buffer 
		// and the buffer is empty. Do it directly. 
		size_t rd=Input_Stream::read(ibuf,ilen);
		return(rd);
	}
	
	if(blen>=ilen)
	{
		// whole request can be read from buffer. 
		_buf_read(ibuf,ilen);
		return(ilen);
	}
	
	// The request is larger than the amount we have in the buffer. 
	// First, copy the buffer. 
	char *dest=ibuf;
	size_t destlen=ilen;
	size_t _blen=blen;
	_buf_read(dest,_blen);
	dest+=_blen;
	if(eof_reached)  // That's all. 
	{  goto done;  }
	destlen-=_blen;
	
	// Now, read directly from the file something of multiple 
	// bsize length. 
	{
		size_t rlen=destlen-(destlen%bsize);
		size_t rd=Input_Stream::read(dest,rlen);
		dest+=rd;
		if(rd<rlen)
		{
			assert(eof_reached);
			goto done;
		}
		destlen-=rd;
	}
	
	if(!destlen)
	{  goto done;  }
	
	// We still have to read destlen bytes (destlen<bsize). 
	// The buffer is empty. We first fill the buffer, then copy it. 
	assert(destlen<bsize);  // NOT <= 
	
	_fill_buf();
	if(blen>destlen)  // NOT >=
	{
		_buf_read(dest,destlen);
		dest+=destlen;
		goto done;
	}
	
	// Hmmm... seems we reached EOF. 
	assert(eof_reached);
	// Copy as much as is left: 
	memcpy(dest,buf,blen);
	dest+=blen;
	blen=0;
	
done:
	return(dest-ibuf);
}


#if 0
void Buffered_Input_Stream::seek(size_t off)
{
	// Not implemented yet as not needed.
	// Seek is a bad idea as it is possible to seek beyond EOF. 
	assert(0);
}
#endif


size_t Buffered_Input_Stream::skip(size_t nbytes)
{
	if(!nbytes)  return(0);
	if(nbytes<blen)   // nbytes==blen treated below 
	{
		// simply skip from buffer
		blen-=nbytes;
		memmove(buf,buf+nbytes,blen);
		return(nbytes);
	}
	
	// First, skip the whole buffer. 
	size_t skipped=blen;
	nbytes-=skipped;
	blen=0;
	if(eof_reached || !nbytes)  return(skipped);
	
	// Then, read in all nbytes bytes to skip. 
	for(;;)
	{
		_fill_buf();
		// There are now blen bytes in the buffer. 
		if(nbytes<blen)
		{
			blen-=nbytes;
			memmove(buf,buf+nbytes,blen);
			return(skipped+nbytes);
		}
		nbytes-=blen;
		skipped+=blen;
		if(blen<bsize)
		{
			assert(eof_reached);
			break;
		}
		blen=0;
	}
	return(skipped);
}


Buffered_Input_Stream::Buffered_Input_Stream(const std::string &_path) throw() : 
	Input_Stream(_path)
{
	buf=NULL;
	bsize=0;
	blen=0;
}

Buffered_Input_Stream::~Buffered_Input_Stream() throw()
{
	close();   // ...or Input_Stream's destructor will call a virtual
	if(buf)
	{  delete[] buf;  buf=NULL;  }
}

}  /* namespace end */
