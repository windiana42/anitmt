/*
 * outstr.hpp
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

#ifndef _Inc_IO_OutStr_H_
#define _Inc_IO_OutStr_H_ 1

#include <string>

#include "lproto.hpp"

namespace output_io
{

extern size_t Get_Preferred_IO_Size(int fd);

// Maybe someone wants to implement this using FILE streams 
// or, maybe even worse things like C++ ostream...
// This is thought as a layer of abstraction clearly identifying 
// the required functionality. 
class Output_Stream
{
	protected:
		int fd;
		std::string path;   // path
		size_t off;         // current offset (# of written bytes)
		
		virtual void file_opened()  { }
		virtual void file_close()   { }
	public:
		// Does NOT open the file: 
		Output_Stream(const std::string &path) throw();
		virtual ~Output_Stream() throw();
		
		// Get current offset in the file. 
		virtual size_t Off() const  {  return(off);  }
		// Get path reference. 
		std::string Path() const  {  return(path);  }
		const char *CPath()  {  return(path.c_str());  }
		// File open?
		bool IsOpen() const  {  return((fd<0) ? false : true);  }
		
		// Returns the preferred IO size (for buffering) 
		size_t Preferred_IO_Size()
			{  return(IsOpen() ? Get_Preferred_IO_Size(fd) : 0);  }
		
		// Open the file specified with the constructor. 
		// An error message is written in case opening fails. 
		// Returns true on success and false on failure. 
		bool open(bool allow_overwrite=false) throw();
		
		// Close the file; does nothing if it is not open. 
		void close();
		
		// Write buf of size len 
		virtual void write(const char *buf,size_t len);
};

}  // namespace end

#endif  /* _Inc_IO_OutStr_H_ */
