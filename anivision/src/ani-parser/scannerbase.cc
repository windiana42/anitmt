/*
 * ani-parser/scannerbase.cc
 * 
 * Basic routines for language scanner. 
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

#include "scannerbase.h"

#include <assert.h>


int LexerScannerBase::NErrors() const
{
	// Must be overridden. 
	assert(0);
	return(100000);  // <-- ...at least! :)
}


const YYLTYPE &LexerScannerBase::GetCLoc() const
{
	// Must be overridden. 
	assert(0);
	//return YYLTYPE();
}


void LexerScannerBase::SetMainStartCond(StartCond /*_main_start_cond*/)
{
	// Must be overridden if ever used. 
	assert(0);
}


int LexerScannerBase::YYLex(YYSTYPE * /*lvalp*/,YYLTYPE * /*llocp9*/)
{
	// Must be overridden. 
	assert(0);
	return(0);
}


LexerScannerBase::LexerScannerBase()
{
}

LexerScannerBase::~LexerScannerBase()
{
	// Not inline because virtual. 
}
