/*
 * boutstr.hpp
 * 
 * Buffered output stream using Output_Stream. 
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

#ifndef _Inc_IO_BufOutStr_H_
#define _Inc_IO_BufOutStr_H_ 1

#include "outstr.hpp"

namespace output_io
{

// See outstr.hpp (class Output_Stream) for explanations. 

class Buffered_Output_Stream : public Output_Stream
{
	private:
		char *buf;
		size_t bsize;    // size of buffer
		size_t blen;     // used bytes in buffer
		
		// overriding virtuals: 
		void file_opened();
		void file_close();
		
		inline void _flush();
		inline void _write_buf(const char *obuf,size_t olen) throw();
	public:
		Buffered_Output_Stream(const std::string &path) throw();
		~Buffered_Output_Stream() throw();
		
		// Returns buf size or 0 if !IsOpen(). 
		size_t Get_Buf_Size()  {  return(bsize);  }
		
		// overriding virtuals: 
		size_t Off() const  {  return(off+blen);  }
		void write(const char *buf,size_t len);
}; 

extern size_t Get_Preferred_IO_Size(int fd);

}  /* namespace end */

#endif  /* _Inc_IO_BufOutStr_H_ */
