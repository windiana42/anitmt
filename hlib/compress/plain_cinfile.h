/*
 * plain_cinfile.h
 * 
 * Header containing a plain non-compressed file read access class 
 * as wrapper with the same interface as the compressed file readers. 
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

#ifndef _HLIB_NotCompressedFileReader_H_
#define _HLIB_NotCompressedFileReader_H_ 1

#include "base_cinfile.h"


class NotCompressedFileReader : public CompressedFileReader
{
	private:
		int fd;
		
	public:
		NotCompressedFileReader(int *failflag=NULL);
		~NotCompressedFileReader();
		
		// Please see CompressedFileReader (base_cinfile.h) 
		// for more info. 
		ErrorState open(const char *filename);
		ssize_t read(char *buf,size_t len);
		ErrorState close();
};

#endif  /* _HLIB_NotCompressedFileReader_H_ */
