/*
 * imgfmt.hpp
 * 
 * Image format virtual interface...
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <lib/prototypes.hpp>
#include <hlib/linkedlist.h>


class ComponentDataBase;

enum ImageFormatID
{
	IF_None=0,
	IF_PNG,
	IF_PPM,
	IF_TGA
};

// Linked list hold by ComponentDataBase. 
struct ImageFormat : LinkedListBase<ImageFormat>
{
	ImageFormatID fmtid;  //               e.g.     PNG
	const char *name;     //               e.g. "png", "png6"
	int bits_p_rgb;       // bits per RGB, e.g. 8,     6
	const char *file_extension;   //      e.g. "png", "png"
	
	// NEED MORE HERE (query/verify routines/depth)
	
	_CPP_OPERATORS_FF
	ImageFormat(int *failflag=NULL);
	~ImageFormat();
	
	// Called om startup to set up list of known image formats at 
	// component data base. Return value as usuas: 0 -> OK; >0 -> error
	static int init(ComponentDataBase *cdb);
};

#endif  /* _RNDV_IMGFMT_IMGFMT_HPP_ */
