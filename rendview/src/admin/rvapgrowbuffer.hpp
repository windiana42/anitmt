/*
 * rvapgrowbuffer.hpp
 * 
 * Header for growing buffer which can store data written using 
 * standard RVAPGrowBuffer::printf() calls. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_ADMIN_GROWBUFFER_HPP_
#define _RNDV_ADMIN_GROWBUFFER_HPP_ 1

#include <lib/prototypes.hpp>
#include <hlib/growbuffer.h>


class RVAPGrowBuffer : public GrowBuffer
{
	public:
		RVAPGrowBuffer(size_t reserv_len,int *failflag=NULL) : 
			GrowBuffer(reserv_len,failflag) {}
		~RVAPGrowBuffer() {}
		
		// Use this to append something to the buffer: 
		// Alloc failure will append an alloc fail message. 
		// Return value: 
		//  0 -> OK
		// -1 -> alloc failure
		int printf(const char *fmt,...)
			__attribute__ ((__format__ (__printf__, 2, 3)));
		int vprintf(const char *fmt,va_list ap);
};

#endif  /* _RNDV_ADMIN_GROWBUFFER_HPP_ */
