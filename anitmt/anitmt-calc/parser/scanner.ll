%{
#include "support.hpp"
/* Scanner/Lexer for the anitmt scene description language */
%}

%option noyywrap
%option c++
%option yylineno


%%

\{			return (OP_SECTION);
\}			return (CL_SECTION);
\;			return (SEMICOLON);
\+			return (PLUS);
\-			return (MINUS);
\*			return (ASTERISK);
\/			return (SLASH);
\<			return (OP_VECTOR);
\>			return (CL_VECTOR);
\.			return (DOT);
[:alpha:][:alnum:]*	return (IDENTIFIER);
\"[^"]+\"		return (STRING);
[+-]?[:digit:]+(\.[:digit:]+([eE][+-]?[:digit:]+)?)?	{
                        return (NUMBER);
}

%%

