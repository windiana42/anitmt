/*
 * ani-parser/location.cc
 * 
 * Source location (file/line) representation for TreeNode. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "location.h"
#include <stdarg.h>


namespace ANI
{

RefString TNLocation::PosRangeString() const
{
	RefString tmp;
	pos0.PosRangeString(pos1,&tmp);
	return(tmp);
}

RefString TNLocation::PosRangeStringRelative(const TNLocation &rloc) const
{
	if(pos0.GetFile()==pos1.GetFile() && 
	   pos0.GetFile()==rloc.pos0.GetFile() && 
	   pos0.GetFile()==rloc.pos1.GetFile() )
	{
		RefString tmp;
		pos0.PosRangeString(pos1,&tmp,/*with_file=*/0);
		return(tmp);
	}
	else
	{  return(PosRangeString());  }
}


static int enable_color_stderr=1;

void _vaError(const TNLocation &loc,const char *fmt,va_list ap)
{
	if(enable_color_stderr)
	{  fprintf(stderr,"\33[1;31m");  }
	
	// Write location: 
	RefString pos=loc.PosRangeString();
	fprintf(stderr,"%s: error: ",pos.str());
	
	// Write error: 
	vfprintf(stderr,fmt,ap);
	
	if(enable_color_stderr)
	{  fprintf(stderr,"\33[00m");  }
}


void Error(const TNLocation &loc,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	_vaError(loc,fmt,ap);
	va_end(ap);
}


void _vaWarning(const TNLocation &loc,const char *fmt,va_list ap)
{
	if(enable_color_stderr)
	{  fprintf(stderr,"\33[0;31m");  }
	
	// Write location: 
	RefString pos=loc.PosRangeString();
	fprintf(stderr,"%s: warning: ",pos.str());
	
	// Write error: 
	vfprintf(stderr,fmt,ap);
	
	if(enable_color_stderr)
	{  fprintf(stderr,"\33[00m");  }
}


static const int enable_warnings=1;
void Warning(const TNLocation &loc,const char *fmt,...)
{
	static int _warned=0;
	if(enable_warnings)
	{
		va_list ap;
		va_start(ap,fmt);
		_vaWarning(loc,fmt,ap);
		va_end(ap);
	}
	else if(!_warned)
	{
		fprintf(stderr,"\33[0;31m** Warnings disabled **\33[00m\n");
		_warned=1;
	}
}

}  // end of namespace ANI
