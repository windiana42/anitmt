/*
 * bzip2_cinfile.h
 * 
 * Header containing a bzip2 compressed file read access class. 
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

#ifndef _HLIB_BZip2CompressedFileReader_H_
#define _HLIB_BZip2CompressedFileReader_H_ 1

#include "base_cinfile.h"

#if HAVE_BZLIB_H
// This avoids including bzlib.h here: 
typedef void BZFILE;
#endif


class BZip2CompressedFileReader : public CompressedFileReader
{
	private:
		// This is NULL if no file is open: 
		FILE *fp;
		BZFILE *bzf;
		ErrorState sticky_error : 31;
		int eof_reached : 1;
		
	public:
		BZip2CompressedFileReader(int *failflag=NULL);
		~BZip2CompressedFileReader();
		
		// Please see CompressedFileReader (base_cinfile.h) 
		// for more info. 
		ErrorState open(const char *filename);
		ssize_t read(char *buf,size_t len);
		ErrorState close();
};

#endif  /* _HLIB_BZip2CompressedFileReader_H_ */
