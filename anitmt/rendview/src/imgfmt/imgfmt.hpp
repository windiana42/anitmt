/*
 * imgfmt.hpp
 * 
 * Image format virtual interface...
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_IMGFMT_IMGFMT_HPP_
#define _RNDV_IMGFMT_IMGFMT_HPP_ 1

#include <hlib/prototypes.h>
#include <hlib/linkedlist.h>

enum ImageFormatID
{
	IF_None=0,
	IF_PNG,
	IF_PPM
};

struct ImageFormat : LinkedListBase<ImageFormat>
{
	ImageFormatID fmtid;
	const char *name;   // e.g. "PNG"
	int bitspp;         // bits per pixel
	
	// NEED MORE HERE (query/verify routines/depth)
	
	_CPP_OPERATORS_FF
	ImageFormat(int *failflag=NULL);
	~ImageFormat();
};

#endif  /* _RNDV_IMGFMT_IMGFMT_HPP_ */
