/*
 * base_cinfile.h
 * 
 * Header containing a compressed file read access base class. 
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

#ifndef _HLIB_CompressedFileReader_H_
#define _HLIB_CompressedFileReader_H_ 1

#include <hlib/prototypes.h>


// This is the base class for GZipCompressedFileReader,... 
// This class is abstract only providing pure virtual interface. 
// You can use this class directly (without deriving any compressor) 
// to use no compression and simple file IO. 
// This is useful if the available compressors depend on precence of 
// libraries (zlib,...). 
// NOTE: THESE CLASSES INTERNALLY USE THE STANDARD ALLOCATION FUNCTIONS, 
//       AND *NOT* LMalloc(). 
class CompressedFileReader
{
	public:
		typedef u_int64_t datalen_t;
		
		enum ErrorState
		{
			// ALL NON-SUCCESS STATES MUST BE NEGATIVE. 
			ES_Success=0,         // success; MUST be 0
			ES_Allocfail=-999,    // allocation failure
			ES_Errno,             // check errno for details 
			ES_NotOpened,         // no file open (calling read) 
			ES_UnexpectedEOF,     // read saw unexpected EOF
			ES_DataCorrupt,       // read corrupt data
			ES_InternalError      // all the rest...
		};
		
	public:  _CPP_OPERATORS_FF
		CompressedFileReader(int * /*failflag*/=NULL)
			{  ;  }
		virtual ~CompressedFileReader()
			{  ;  }
		
		// Open file for reading. 
		// Use class GZipCompressedFile for reading of gzip compressed 
		//   files; that class can also transparently read 
		//   uncompressed files. 
		// Use BZip2CompressedFile for reading of bzip2 compressed files. 
		virtual ErrorState open(const char * /*filename*/)
			HL_PureVirt(ES_InternalError);
		
		// Read from the file
		// Return value: 
		//    number of bytes read or 
		//    0 -> EOF
		//   <0 -> cast to ErrorState and interprete ErrorState. 
		virtual ssize_t read(char * /*buf*/,size_t /*len*/)
			HL_PureVirt(ES_InternalError);
		
		// Close file again. 
		virtual ErrorState close() HL_PureVirt(ES_InternalError);
};

#endif  /* _HLIB_CompressedFileReader_H_ */
