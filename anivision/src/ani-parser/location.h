/*
 * ani-parser/location.h
 * 
 * Source code location (file/line) representation. 
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

#ifndef _ANIVISION_ANI_LOCATION_H_
#define _ANIVISION_ANI_LOCATION_H_

#include <sourcepos/sourcepos.h>

namespace ANI
{

struct TNLocation  /* C++ save */
{
	enum _NullLoc { NullLoc };
	
	SourcePosition pos0;   // start pos
	SourcePosition pos1;   // end pos
	
	TNLocation(_NullLoc = NullLoc) : 
		pos0(SourcePosition::NullPos),pos1(SourcePosition::NullPos) {}
	TNLocation(const SourcePosition &p0,const SourcePosition &p1) : 
		pos0(p0),pos1(p1) {}
	TNLocation(const TNLocation &loc) : pos0(loc.pos0),pos1(loc.pos1) {}
	~TNLocation() {}
	
	TNLocation &operator=(const TNLocation &loc)
		{  pos0=loc.pos0;  pos1=loc.pos1;  return(*this);  }
	TNLocation &operator=(_NullLoc)
		{  pos0=SourcePosition::NullPos;  pos1=SourcePosition::NullPos;
			return(*this);  }
	
	// Rerurn string representation (without include hierarchy) 
	// of this location (= position range). 
	RefString PosRangeString() const;
	// Return relative position string of *this i.e. without file 
	// name if the file names are equal. 
	RefString PosRangeStringRelative(const TNLocation &rloc) const;
}__attribute__((__packed__));


// Standard error reporting functions. 
extern void Error(const ANI::TNLocation &loc,const char *fmt,...)
	__attribute__ ((__format__ (__printf__, 2, 3)));
extern void Warning(const ANI::TNLocation &loc,const char *fmt,...)
	__attribute__ ((__format__ (__printf__, 2, 3)));

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_LOCATION_H_ */
