/*
 * lexer.hpp
 * 
 * Expression lexer/parser include file. 
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

#ifndef _NS_ExParse_Lexer_HPP_
#define _NS_ExParse_Lexer_HPP_ 1

#ifndef exparse_in_flex_source
	#undef yyFlexLexer
	#define yyFlexLexer exparse_FlexLexer
	#include <FlexLexer.h>
#endif

#include "basicparser.hpp"
#include "function.hpp"
#include "tree.hpp"

namespace exparse {

enum Token
{
	/*** General: ***/
	TIdentifier,
	TWSpace,
	TCComment,      // "/*"
	TCCommentEnd,   // "*/"
	TCppComment,    // "//"
	TUnknown,       // any unmached char
	TEOF,           // end of file
	/*** Values: ***/
	TVScalar,
	TVVector,
	TVString,
	TVFlag,
	/*** For expressions: ***/
	TEOpBr,TEClBr,      // "(", ")"
	TENot,              // "!"
	TEPlus,TEMinus,     // "+", "-"
	TEPow,              // "^"
	TEMul,TEDiv,        // "*", "/"
	TELE,TEGE,TEL,TEG,  // "<=", ">=", "<", ">"
	TEEq,TENEq,         // "==", "!="
	TEAnd,TEOr,         // "&&", "||"
	TECondQ,TECondC,    // "?", ":"
	TESep,              // ","
	/*** LAST ONE: ***/
	_TLast
};


struct ParserConfig : BasicParserConfig
{
	ParserConfig();
	~ParserConfig() { }
};

//! I recommend using a typedef for this:
//! typedef exparse::Parser ExpressionParser;
class Parser : public BasicParserGlue<exparse_FlexLexer>
{
	private:
		//! NEVER call yylex() and yyinput() directly; 
		//! use NextTok() and NextChar() (from class BasicParser) 
		//! instead. 
		
		ParserConfig config;
		
		// Go into expression start condition and parse the 
		// expression: 
		void _ParseExpression(LinkedList<OperatorNode> *exp);
		
		const FunctionDesc *_ParseFunctionName();
		
		// May only be called for Tolken TVxxx: 
		void _ParseValue(Value *rv,Token valtok);
		// For lex-reported tokens only: 
		void _ParseScalar(Scalar *rv);
		void _ParseVector(Vector *rv);
		int  _ParseFlag(Flag *rv);
		
		void _ResetParser();
	public:
		Parser();
		~Parser();
		
		//! Parse in a complete expression (without expression 
		//! terminator like `;'). 
		//! The expression is parsed into a linked list. Then, 
		//! this list is supplied to tree->Create() (after calling 
		//! tree->Clear()). 
		//! Returns 0 on success, 
		//! <0 on parse error(s) 
		//! >0 on tree creation errors (ExpressionTree::Create())
		//! You may wish to call tree->Optimize() lateron. 
		//! NOTE: You should check if the tree is empty (which is 
		//!       the case if the expression was empty, and also 
		//!       when parse error occured). 
		int Parse(const char *expr,ExpressionTree *tree);
};

}  // end of namespace exparse

#endif  /* _NS_ExParse_Lexer_HPP_ */

