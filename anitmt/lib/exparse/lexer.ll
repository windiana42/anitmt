%{
/*
 * lexer.ll
 * 
 * Expression lexer (flex source) 
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

// Include expression tokens: 
#define exparse_in_flex_source 1
#include "lexer.hpp"

// Grrr #@/&$+\*!!! Opening a namespace here will NOT 
// put the generated class into this namespace. Damn! 
using namespace exparse;

%}

%pointer
%option prefix="exparse_"
%option noyywrap
%option c++

wSPACE      [\f\n\r\t\v ]
/* unsigned float value */
UfLOATVAL   ((\.[[:digit:]]+)|([[:digit:]]+(\.[[:digit:]]+)?))([eE][+-]?[[:digit:]]+)?
/* signed float value */
SfLOATVAL   [+-]?{UfLOATVAL}

iDENTIFIER  [[:alpha:]_][[:alnum:]_]*
vECTOR      \<{wSPACE}*{SfLOATVAL}{wSPACE}*\,{wSPACE}*{SfLOATVAL}{wSPACE}*\,{wSPACE}*{SfLOATVAL}{wSPACE}*\>
/*fLAG        (on|off|yes|no|true|false|1|0)/[^[:alnum:]]*/

%%

{wSPACE}+           return(TWSpace);  /* whitespace */
{iDENTIFIER}        return(TIdentifier);
{UfLOATVAL}         return(TVScalar);
{vECTOR}            return(TVVector);
	/*{fLAG}              return(TVFlag);*/
\"                  return(TVString);
\/\*                return(TCComment);
\/\/                return(TCppComment);
\*\/                return(TCCommentEnd);
<<EOF>>             return(TEOF);

\(                  return(TEOpBr);
\)                  return(TEClBr);
\!                  return(TENot);
\+                  return(TEPlus);
\-                  return(TEMinus);
\^                  return(TEPow);
\*                  return(TEMul);
\/                  return(TEDiv);
"<="                return(TELE);
">="                return(TEGE);
\<                  return(TEL);
\>                  return(TEG);
"=="                return(TEEq);
"!="                return(TENEq);
"&&"                return(TEAnd);
"||"                return(TEOr);
\?                  return(TECondQ);
\:                  return(TECondC);
\,                  return(TESep);

.                   return(TUnknown);

%%

