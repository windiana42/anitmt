/* Scanner/Lexer for the anitmt scene description language 
   Copyright Onno Kortmann 
   License: GNU GPL */


// FIXME: Find a better way to handle this flex include stuff!
#define yyFlexLexer ADLFlexLexer
#include <FlexLexer.h>

#include "support.hpp"
using namespace message;

//-----------------------------------------------------------------------------
struct TokenTableEntry {int index; char *desc;};
// Token --> human readable string table
const TokenTableEntry TokenTable[]= {
	{OP_SECTION,			"{"				},
	{CL_SECTION,			"}"				},
	{SEMICOLON,			";"				},
	{PLUS,				"+"				},
	{MINUS,				"-"				},
	{ASTERISK,			"*"				},
	{SLASH,				"/"				},
	{OP_VECTOR,			"<"				},
	{CL_VECTOR,			">"				},
	{DOT,				"."				},
	{COMMA,				","				},
	{IDENTIFIER,			"identifier"			},
	{STRING,			"string"			},
	{NUMBER,			"number"			},
	{END_OF_FILE,			"end of file"			},
	{0, 0}
};
std::string NameOfToken(const int tok) {
	const TokenTableEntry *lp=TokenTable;
	while (lp->index) {
		if (lp->index==tok) return lp->desc;
		lp++;
	}
	return "unknown token";
}

//-----------------------------------------------------------------------------
VADLFlexLexer::VADLFlexLexer(const std::string _fn,
			     istream *is,
			     message::Message_Consultant *c) :
	ADLFlexLexer(is, &cerr), //FIXME
	message::Message_Reporter(c),
	fn(_fn) {}

void VADLFlexLexer::Warning(const std::string msg) {
	warn(new File_Position(fn, yylineno))
		<< "ADL-Scanner: " << msg;
}

int VADLFlexLexer::GetNext() { return _t=yylex(); }

int VADLFlexLexer::tok() { return _t; }
//-----------------------------------------------------------------------------
