/*
 * imgfmt.cpp
 * 
 * Image format virtual interface: implementation. 
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

#include "imgfmt.hpp"




ImageFormat::ImageFormat(int *failflag=NULL)
{
	fmtid=IF_None;
	name=NULL;
	bitspp=-1;
}

ImageFormat::~ImageFormat()
{
	
}
