/*
 * init.cpp
 * 
 * Initialize known image formats. 
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

#include "../database.hpp"


static int _SetupIFmt(ComponentDataBase *cdb,
	ImageFormatID fmtid, const char *name, int bitspp,const char *ext)
{
	Verbose("[%s] ",name);
	ImageFormat *fmt=NEW<ImageFormat>();
	if(!fmt)  return(1);
	fmt->fmtid=fmtid;
	fmt->name=name;
	fmt->bitspp=bitspp;
	fmt->file_extension=ext;
	if(cdb->RegisterImageFormat(fmt))
	{  delete fmt;  return(1);  }
	return(0);
}

int ImageFormat::init(ComponentDataBase *cdb)
{
	int failed=0;
	Verbose("Setting up image formats: ");
	failed+=_SetupIFmt(cdb,IF_PNG,"PNG",8,"png");
	failed+=_SetupIFmt(cdb,IF_PNG,"PNG6",6,"png");
	failed+=_SetupIFmt(cdb,IF_PPM,"PPM",8,"ppm");
	failed+=_SetupIFmt(cdb,IF_TGA,"TGA",8,"tga");
	Verbose(failed ? "FAILED\n" : "OK\n");
	return(failed ? 1 : 0);
}

