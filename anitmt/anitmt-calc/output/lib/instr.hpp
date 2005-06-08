/*
 * instr.hpp
 * 
 * Input stream wrapper/interface. 
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
 
#ifndef _Inc_IO_InStr_H_
#define _Inc_IO_InStr_H_ 1

#include "lproto.hpp"

#include <string>


namespace output_io
{

extern size_t Get_Preferred_IO_Size(int fd);

// Maybe someone wants to implement this using FILE streams 
// or, maybe even worse things like C++ istream...
// This is thought as a layer of abstraction clearly identifying 
// the required functionality. 
class Input_Stream
{
	protected:
		int fd;
		bool eof_reached;
		std::string path;   // path
		size_t off;         // current offset
		
		virtual void file_opened()  { }
		virtual void file_close()   { }
	public:
		// Does NOT open the file: 
		Input_Stream(const std::string &path) throw();
		virtual ~Input_Stream() throw();
		
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
		
		// Used to open the file. 
		// Does nothing if file is already open. 
		// Returns false on failure. 
		// print_error: print error on failure or not; 
		//    function will still return (never exit). 
		bool open(bool print_error) throw();
		
		// Change the path and re-try to open the file. 
		// If the file is already open, true is returned 
		// immediately. 
		// (For search path test.) 
		// Returns false on failure; uses open(/*print_error=*/false) 
		bool open(const std::string &path) throw();
		
		// Close an opened file as also done by destructor; 
		// does nothing if not open. 
		void close();
		
		// Reads up to len bytes into buf; returns number of read 
		// bytes which may be smaller than len only on EOF. 
		// Returns 0 if the file is not open. 
		virtual size_t read(char *buf,size_t len);
		
		#if 0
		/* SEEK IS NOT A GOOD IDEA SINCE IT IS POSSIBLE TO SEEK */
		/* BEYOND EOF.                                          */
		// Seek to specified offset (off as counted from the 
		// beginning of the file). 
		virtual void seek(size_t off);
		// BE SURE TO ALSO IMPLEMENT Buffered_Input_Stream::seek(). 
		#endif
};

}  // namespace end 

#endif  /* _Inc_IO_InStr_H_ */
