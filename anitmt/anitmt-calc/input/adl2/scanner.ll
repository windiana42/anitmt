%{
  /* lexical analyzer */

  #include <stdlib.h>

  #include "adlparser.hpp"		// token type and parser info

  #include "tokens.h"			// token values
 
  // forward declaration
  inline Token get_identifier( char *s, adlparser_info *info );
  inline std::string strip_quotes( std::string text );

#define YYSTYPE adlparser::Token

#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)
%}

%option noyywrap
%option c++
%option prefix="adlparser_"
%pointer	// define yytext as pointer (variable length)

%x ML_COMMENT

id	[a-zA-Z_][a-zA_Z0-9_]*(\.[a-zA-Z_][a-zA_Z0-9_]*)*
scal	(([0-9]+)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)
qstring \"([^\"\n]|\\\")*\"
%%

"//".*\n  { /* ignore one line comment */ }
"/*"	  { BEGIN ML_COMMENT; }
<ML_COMMENT>"*/" { BEGIN 0; /* end of comment */ }
<ML_COMMENT>"/*" { /* ???count nested comments??? */ }
<ML_COMMENT>\n { /*info...column=0; info...line++;*/ }
<ML_COMMENT>.* { /* ingore multiline comment */ }

[ ]	  { /*info...column++;*/ }    //keep track of position
[\t]	  { /*info...column =...;*/ } //(!!!should be stored in info object!!!)
[\n]	  { /*info...column=0; info...line++;*/ }

{scal}	  { yylval->scalar() = atof(yytext); return TOK_SCALAR; }
{qstring} { yylval->string() = strip_quotes(yytext); return TOK_STRING; }

"=="	  { return TOK_IS_EQUAL; }	/* multi character operators */
"!="	  { return TOK_NOT_EQUAL; }
">="	  { return TOK_MORE_EQUAL; }
"<="	  { return TOK_LESS_EQUAL; }

sin	  { return TOK_FUNC_SIN; } /* sin() function keyword */

{id}	  { Token tok = get_identifier(yytext,info);
	    *yylval = tok; return tok.get_type(); } /* identifiers */


.	return yytext[0];	/* one charater tokens */
%%
// asks all id_resolver defined in info, whether they may resolve the 
// identifier. In case they don't it returns TOK_INVALID_ID 
inline Token get_identifier( char *s,adlparser_info *info )
{
  Token tok;
  if( info->id_resolver ) tok = info->id_resolver->get_identifier(s); 
  // if resolver didn't know what the string means?
  if( tok.get_type() == TOK_INVALID_ID )
  {
    tok.identifier() = s; //... return it as id string
  }
  return tok;
}

// strips strings from quotes
// (has a lot of overhead, but no problems with dynamic memory at pointers)
std::string strip_quotes( std::string text )
{
  assert(text[0] == '"');
  assert(text[text.length()-1] == '"');
  return text.substr(1,text.length()-1);
}
/* end of lexical analizer */