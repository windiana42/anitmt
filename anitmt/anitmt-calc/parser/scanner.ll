%{
#include "support.hpp"
/* Scanner/Lexer for the anitmt scene description language 
   Copyright Onno Kortmann 
   License: GNU GPL

   FIXMEs:
   The ugly typecast into VADLFlexLexer ... is there a better solution?
   *NEVER* create objects of type ADLFlexLexer!!
*/
	
%}

%option noyywrap
%option c++
%option yylineno
%option debug

%%

^\#.+				; /* Comment ... */
[\t\n\r\b ]			; /* Whitespace */

\{	return (OP_SECTION);
\}	return (CL_SECTION);
\;	return (SEMICOLON);
\+	return (PLUS);
\-	return (MINUS);
\*	return (ASTERISK);
\/	return (SLASH);
\<	return (OP_VECTOR);
\>	return (CL_VECTOR);
\.	return (DOT);
\,	return (COMMA);
"n/a"|\""n/a"\" return (N_A);
scene|scalar|linear {
	((VADLFlexLexer*)this)->yylval.str=yytext; return (NODE);
}

[[:alpha:]_][[:alnum:]_]*	{
	((VADLFlexLexer*)this)->yylval.str=yytext;
	return (IDENTIFIER);
}
\"[^"\n]*\"	{	/*" this hyphen brings the emacs colorizer back into normal mode :)*/
	((VADLFlexLexer*)this)->yylval.str=string(yytext+1).substr(0, yyleng-2);
	return (STRING);
}
[+-]?[[:digit:]]+(\.[[:digit:]]+([eE][+-]?[[:digit:]]+)?)? {
        ((VADLFlexLexer*)this)->yylval.num=atof(yytext);
        return (NUMBER);
}

<<EOF>>	return (END_OF_FILE);
.	((VADLFlexLexer*)this)->Warning(string("Invalid character: ")+yytext);


%%
