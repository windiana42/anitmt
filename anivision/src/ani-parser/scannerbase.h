/*
 * ani-parser/scannerbase.h
 * 
 * Base class for language scanner. 
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

#ifndef _ANIVISION_SCANNER_BASE_H_
#define _ANIVISION_SCANNER_BASE_H_

#include <ani-parser/location.h>

/* LOCATION/POSITON: */
typedef ANI::TNLocation yyltype;
#define YYLTYPE yyltype

/* Include tokens and def for YYSTYPE, YYLTYPE: */
#include <ani-parser/treenode.h>
#include <ani-parser/grammar.hh>


// Base class for the lexer; interface to grammar/parser is YYLex(). 
class LexerScannerBase
{
	public:
		enum StartCond
		{
			SC_INITIAL=0,   // do not change (as used by flex)
			SC_ANI=10,  // not 0
			SC_POV
		};
	private:
	public: _CPP_OPERATORS
		LexerScannerBase();
		virtual ~LexerScannerBase();
		
		// Get number of errors: 
		virtual int NErrors() const;
		
		// When YYLex() returns: position of the returned token: 
		virtual const YYLTYPE &GetCLoc() const;
		
		// See AniLexerScanner; need only be overridden if ever used. 
		virtual void SetMainStartCond(StartCond _main_start_cond);
		
		// The lexer function which is called by the 
		// grammar/parser. 
		virtual int YYLex(YYSTYPE *lvalp,YYLTYPE *llocp);
};

#endif  /* _ANIVISION_SCANNER_BASE_H_ */
