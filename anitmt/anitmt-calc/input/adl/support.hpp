#ifndef __PARSER_SUPPORT_HPP
#define __PARSER_SUPPORT_HPP
//-----------------------------------------------------------------------------
// AniTMT ADL-Parser support header file
/* Copyright Onno Kortmann 
   License: GNU GPL */
//-----------------------------------------------------------------------------
#include <message/message.hpp>
#include <string>
//-----------------------------------------------------------------------------
//! List of adl tokens 
enum ADLTokens {
	OP_SECTION=256,	// Open section "{"
	CL_SECTION, 	// Close section "}"
	SEMICOLON,	// ;
	PLUS,		// +
	MINUS,		// -
	ASTERISK,	// *
	SLASH,		// /
	OP_VECTOR,	// <
	CL_VECTOR,	// >
	DOT,		// .
	COMMA,		// ,
	N_A,		// n/a
	IDENTIFIER,	// Blablabla, blablabla, bla_blabla, bla123, scene, scalar, ...
	STRING,		// "Blablabla"
	NUMBER,		// 1.4142
	END_OF_FILE
};

//! Returns a human readable description of a token
std::string NameOfToken(const int tok);

//! VADLFlexLexer is a ADLFlexLexer with values for each returned token and
//! warning/error-handling.
class VADLFlexLexer : public ADLFlexLexer, public message::Message_Reporter {
public:
	VADLFlexLexer(const std::string fn,
		      std::istream *is,
		      message::Message_Consultant *c);
	//! Not a very efficient way to handle values... but it works.
	struct {
		std::string str;
		double num;
	} yylval;
	//! Print a warning from the lexer into the output stream
	void Warning(const std::string msg);

	/*! Get next token out of the stream
	  \return next token :) */
	int GetNext();
	
	/*! Get last token, undefined result if called before a first "GetNext" call! */
	int tok();
private:
	const std::string fn;
	int _t; // The current token
};
//-----------------------------------------------------------------------------
#endif
