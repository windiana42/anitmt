/* Scanner/Lexer for the anitmt scene description language 
   Copyright Onno Kortmann 
   License: GNU GPL */


// FIXME: Find a better way to handle this flex include stuff!
#define yyFlexLexer ADLFlexLexer
#include <FlexLexer.h>

#include "support.hpp"
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
	{N_A,				"n/a or \"n/a\""		},
	{NODE,				"scene, scalar or linear etc."	},
	{IDENTIFIER,			"identifier"			},
	{STRING,			"string"			},
	{NUMBER,			"number"			},
	{END_OF_FILE,			"end of file"			},
	{0, 0}
};
string NameOfToken(const int tok) {
	const TokenTableEntry *lp=TokenTable;
	while (lp->index) {
		if (lp->index==tok) return lp->desc;
		lp++;
	}
	return "unknown token";
}

//-----------------------------------------------------------------------------
VADLFlexLexer::VADLFlexLexer(istream *is, ostream *os) {
	
}
void VADLFlexLexer::Warning(const string msg) {
	*yyout << "ADL-Scanner WARNING (line " << yylineno << "): " << msg << '\n';
}

int VADLFlexLexer::GetNext() { return _t=yylex(); }

int VADLFlexLexer::tok() { return _t; }
//-----------------------------------------------------------------------------
