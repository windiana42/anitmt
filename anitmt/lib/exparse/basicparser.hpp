/*
 * basicparser.hpp
 * 
 * Header for basic parser routines used by several 
 * flex-based parsers. 
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

#ifndef _NS_ExParse_BasicParser_HPP_
#define _NS_ExParse_BasicParser_HPP_ 1

#include <string>
#include <message/message.hpp>

namespace exparse
{

struct BasicParserConfig
{
	// 0 -> no nested cmts (like C); 1 -> yes & warn; 2 -> yes & dont warn
	int allow_nested_comments;
	// Width of tab char (defaults to 8) 
	int tab_char_width;
	
	BasicParserConfig();
	~BasicParserConfig() { }
};

template<class T>class BasicParserGlue;

//! BasicParser is a class which can be used for multiple flex-based 
//! parsers to prevent duplication of code dealing with C/C++-style 
//! comments, string parsing and updating file position. 
//! If you use it, do NOT use yylex() or yyparse() but use 
//! NextTok() and NextChar() instead. 
//! You will normally derive classes from BasicParserGlue 
//! (see below) and not of BasicParser. 
class BasicParser : 
	public message::Message_Reporter
{
	private:
		//! NEVER call these two directly 
		//! (use NextTok() and NextChar() instead)
		virtual int yy_lex()=0;
		virtual int yy_input()=0;
		char **yy_text;
		int *yy_leng;
		
		// Current position: 
		struct Position
		{
			std::string file;  // MUST BE CHANGED!! (handle) 
			int line;
			int lpos;
		} p,delta_p;
		
		inline void _UpdatePos(int c,Position *p);
		inline void _UpdatePos(char *str,int len,Position *p);
		inline void _AddPos(Position *p,Position *delta);
		
		// Pointer to some class derived from BasicParserConfig 
		// in the derived class. 
		const BasicParserConfig *config;
	protected:
		//! Internal use only: 
		// Please email me if you know how to make the 
		// BasicParserGlue template a friend of BasicParser. 
		void _InternalSetYYPointers(char **t,int *l)
			{  yy_text=t;  yy_leng=l;  }
		
		//! Token values initialized to -1 used by NextChar() 
		//! to test if SkipC[pp]Cpmment() has to be called. 
		//! Set token val to illegal value to disable its 
		//! function:
		int tok_WSpace;        //! whitespace
		int tok_CComment;      //! "/*"
		int tok_CppComment;    //! "//"
		int tok_CCommentEnd;   //! "*/" (stray comment end)
		int tok_Unknown;       //! unknown 
		
		//! Counts parse errors: 
		int errors;
		
		//! Can be used to discart comments from input: 
		//! These are automatically called by NextTok(). 
		void SkipCComment();  // for "/*"-comments
		void SkipCppComment();  // for "//"-comments
		
		//! Used to parse in a string: 
		void ParseString(std::string *rv);
		
		//! Get next char (NEVER call yyinput()): 
		int NextChar();
		//! Get next token; will skip comments. 
		int NextTok();
		int currtok;   // current token (initialized to -1)
		
		void SetConfig(BasicParserConfig *cfg)
			{  config=cfg;  }
		
		const Position *currpos()
			{  return(&p);  }
		
		//! Reset the parser; sets errors=0 and currtok=-1. 
		//! FIXME: should allow position to be set. 
		void ResetParser();
	public:
		//! You should call SetConfig() after construction. 
		BasicParser();
		~BasicParser();
};

//! Used to glue the calls of yylex() and yyinput() to the 
//! correct yyFlexLexer passed as yy. 
template<class yy> class BasicParserGlue : 
	public yy,
	public BasicParser
{
	private:
		//! Overriding virtuals: 
		int yy_lex()    {  return(yy::yylex());    }
		int yy_input()  {  return(yy::yyinput());  }
	protected:
		//! Never call these (these are assert(0)-traps): 
		int yylex()    {  assert(0);  return(-1);  }
		int yyinput()  {  assert(0);  return(-1);  }
	public:
		BasicParserGlue() : yy(),BasicParser()
		{  BasicParser::_InternalSetYYPointers(&yytext,&yyleng);  }
		~BasicParserGlue() { }
};
	
}  // namespace end 

#endif  /* _NS_ExParse_BasicParser_HPP_ */
