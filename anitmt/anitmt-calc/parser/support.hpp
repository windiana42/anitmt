#ifndef __PARSER_SUPPORT_HPP
#define __PARSER_SUPPORT_HPP
//-----------------------------------------------------------------------------
// AniTMT ADL-Parser support header file
// (C)opyright Onno Kortmann
// License: GNU GPL - see file "COPYING" for details (hmm. lgpl?!)
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
	IDENTIFIER,	// Blablabla
	STRING,		// "Blablabla"
	NUMBER		// 1.4142
};
//-----------------------------------------------------------------------------
#endif
