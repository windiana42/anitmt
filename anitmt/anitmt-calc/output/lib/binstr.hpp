/*
 * binstr.hpp
 * 
 * Buffered input stream using Input_Stream. 
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

#ifndef _Inc_IO_BufInStr_H_
#define _Inc_IO_BufInStr_H_ 1

#include "instr.hpp"

namespace output_io
{

// See instr.hpp (class Input_Stream) for explanations. 

class Buffered_Input_Stream : public Input_Stream
{
	private:
		char *buf;
		size_t bsize;    // size of buffer
		size_t blen;     // used bytes in buffer
		
		// overriding virtuals: 
		void file_opened();
		void file_close();
		
		inline void _buf_read(char *dest,size_t len) throw();
		inline void _fill_buf();
	public:
		Buffered_Input_Stream(const std::string &path) throw();
		~Buffered_Input_Stream() throw();
		
		// Returns buf size or 0 if !IsOpen(). 
		size_t Get_Buf_Size()  {  return(bsize);  }
		
		// overriding virtuals: 
		size_t Off() const  {  return(off-blen);  }
		size_t read(char *buf,size_t len);
		//void seek(size_t off) throw();
		
		/* Only in Buffered_Input_Stream: */
		// Skip nbytes bytes instead of reading them. 
		// Returns the number of actually skipped bytes 
		// which is <nbytes only if EOF was reached. 
		size_t skip(size_t nbytes);
};

extern size_t Get_Preferred_IO_Size(int fd);

}  /* namespace end */

#endif  /* _Inc_IO_BufInStr_H_ */

