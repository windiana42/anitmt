/*
 * boutstr.cpp
 * 
 * Buffered output stream using Output_Stream. 
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


#include "boutstr.hpp"

namespace output_io
{


// Gets called after a file has been opened. 
void Buffered_Output_Stream::file_opened()
{
	file_close();  // be sure...
	
	bsize=Get_Preferred_IO_Size(fd);
	buf=new char[bsize];
	blen=0;
}


// Gets called before a file is closed. 
void Buffered_Output_Stream::file_close()
{
	// flush it (don't call _flush() because of the assertion) 
	Output_Stream::write(buf,blen);
	blen=0;
	
	if(buf)
	{  delete[] buf;  buf=NULL;  }
	bsize=0;
	blen=0;
}


// ONLY CALL WHEN THE BUFFER IS COMPLETELY FULL. 
inline void Buffered_Output_Stream::_flush()
{
	assert(blen==bsize);
	Output_Stream::write(buf,blen);
	blen=0;
}


// Only call if the passed data fits entirely into the buffer. 
inline void Buffered_Output_Stream::_write_buf(const char *obuf,size_t olen) throw()
{
	size_t bleft=bsize-blen;
	assert(bleft>=olen);
	if(olen)
	{
		memcpy(buf+blen,obuf,olen);
		blen+=olen;
	}
}


void Buffered_Output_Stream::write(const char *obuf,size_t olen)
{
	if(!IsOpen())
	{
		// Call write directly to get an error. 
		Output_Stream::write(obuf,olen);
		return;
	}
	
	size_t bleft=bsize-blen;
	if(olen<=bleft)
	{
		// Output fits completely into the buffer. 
		if(!blen && olen==bsize)
		{
			// Buffer empty and write size equals buffer size. 
			// The, write it directly. 
			Output_Stream::write(obuf,olen);
		}
		else
		{
			// Write to buffer and flush it if necessray 
			_write_buf(obuf,olen);
			if(blen>=bsize)
			{  _flush();  }
		}
		return;
	}
	
	// First, fill up the buffer and flush it:
	const char *src=obuf;
	size_t srclen=olen;
	_write_buf(src,bleft);
	src+=bleft;
	srclen-=bleft;
	_flush();  //bleft=bsize;
	
	// Then, write a multiple of bsize bytes directly to the file: 
	size_t wlen=srclen-(srclen%bsize);
	Output_Stream::write(src,wlen);
	src+=wlen;
	srclen-=wlen;
	
	if(srclen)
	{
		// Then, write the rest to the buffer: 
		assert(srclen<bsize);  // may not be ==bsize. 
		_write_buf(src,srclen);
	}
}


Buffered_Output_Stream::Buffered_Output_Stream(
	const std::string &_path) throw() : 
	Output_Stream(_path)
{
	buf=NULL;
	bsize=0;
	blen=0;
}

Buffered_Output_Stream::~Buffered_Output_Stream() throw()
{
	close();  // Important
	if(buf)
	{  delete[] buf;  buf=NULL;  }
}

}  // namespace end
