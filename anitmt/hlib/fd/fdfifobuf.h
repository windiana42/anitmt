/*
 * fdfifobuf.h
 * 
 * Header containing class FDFifoBuffer, a class primarily for 
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

#ifndef _HLIB_FDFifoBuffer_H_
#define _HLIB_FDFifoBuffer_H_ 1

#include <hlib/prototypes.h>

// FDFifoBuffer is a FIFO buffer of fixed size. 
// It supplies read and write functions to read data from an fd or 
// write them to an fd. 
// There are also copy routines to get data copied into a buffer. 
// FDFifoBuffer internally uses a char array of specified size and 
// does no copying but uses readv()/writev() instead if it has to read 
// beyond the end where the buffer wraps around. 

class FDFifoBuffer
{
	private:
		size_t bufsize;  // size of the buffer
		char *bufbase;   // where the LMalloc()'ed chunk is
		char *bufend;    // = bufbase+bufsize
		char *bufheadW;  // current write position (read() WRITES data there)
		char *bufheadR;  // current read position (write() READS data here)
		size_t bufuse;   // number of valid bytes after bufheadR
	public:  _CPP_OPERATORS_FF
		// Must specify buffer size. 
		FDFifoBuffer(size_t size,int *failflag=NULL);
		~FDFifoBuffer();
		
		// Get buffer size: 
		size_t BufSize()  {  return(bufsize);  }
		// Get number of bytes stored in the buffer: 
		size_t BufUse()  {  return(bufuse);  }
		// Number of unused bytes in the buffer: 
		size_t BufFree()  {  return(bufsize-bufuse);  }
		
		// Delete buffer contents. 
		void Clear()
			{  bufuse=0;  bufheadW=bufbase;  bufheadR=bufbase;  }
		
		// Actually read in data into buffer. 
		// Read data from fd; will never read more than maxlen bytes 
		// and never more than fits into the buffer. maxlen is for 
		// limiting the number of bytes read at one call to read(); use 
		// maxlen=0 for "unlimited". 
		// Return value: like read() or readv(). 
		ssize_t ReadFD(int fd,size_t maxlen);
		// Similar function, but it reads data from the passed buffer 
		// and simply copies it. Return value is the number of bytes 
		// actually copied. 
		size_t ReadBuf(const char *ibuf,size_t ibuflen);
		
		// Actually write data from buffer: 
		// Write data to fd; will never attempt to write more than 
		// maxlen bytes and never write more than there is in the 
		// buffer. 
		// Set maxlen=0 for "unlimited" (i.e. as much as possible). 
		// Return value: like write() or writev(). 
		ssize_t WriteFD(int fd,size_t maxlen);
		// Similar function, but writes data to a buffer (i.e. simply 
		// copies it). 
		// You may pass NULL as obuf to simply skip the next obuflen 
		// bytes (or BufUse() many bytes if BufUse()<obuflen)
		// Returns number of bytes actually copied. 
		size_t WriteBuf(char *obuf,size_t obuflen);
		
		// This can be used to resize the buffer. 
		// Return value: 
		//    0 -> OK; buffer resized
		//   -1 -> alloc failure; buffer unchanged
		//    1 -> buffer not resized becuase iformation would get lost
		int ResizeBuf(size_t newlen);
};

#endif  /* _HLIB_FDFifoBuffer_H_ */
