/*
 * anivision.h
 * 
 * Main AniVision program class. 
 * This is part of the AniVision project. 
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

#ifndef _ANIVISION_ANIVISION_H_
#define _ANIVISION_ANIVISION_H_ 1

#include <hlib/cplusplus.h>

#include <hlib/valuehandler.h>
#include <hlib/parconsumerovl.h>
#include <hlib/secthdl.h>

#include <ani-parser/ani_parser.h>
#include <ani-parser/treereg.h>
#include <core/animation.h>
#include <calccore/ccif.h>

#include <stdio.h>
#include <stdlib.h>

//#include <ani-parser/exprtype.h>

class AniVisionMaster : 
	public par::ParameterConsumer_Overloaded,
	public par::SectionParameterHandler
{
	private:
		// File name to parse: 
		RefString prim_file;
		bool more_than_one_error;
		
		// Animation name to execute. 
		RefString ani_name;
		
		// Initialize parameters: 
		// Return value: 0 -> OK; 1 -> error
		int _SetUpParams();
		
		// Overriding virtuial from ParameterConsumer: 
		int CheckParams();
		// [overriding virtual from SectionParameterHandler:]
		int parse(const Section *,SPHInfo *);
		
	public:  _CPP_OPERATORS_FF
		AniVisionMaster(par::ParameterManager *parman,int *failflag=NULL);
		~AniVisionMaster();
		
		// Run AniVision. Return value: program exit code. 
		int Run();
};

#endif  /* _ANIVISION_ANIVISION_H_ */
