/*****************************************************************************/
/**   This file defines a lexical analyser for the ADL language             **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

%{
  #include <stdlib.h>
  #include <message/message.hpp>

  #include "adlparser.hpp"		// token type and parser info

  #include "tokens.h"			// token values
 
  // forward declaration
  inline Token get_identifier( char *s, adlparser_info *info );
  inline std::string strip_quotes( std::string text );

#define YYSTYPE adlparser::Token

#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)

  inline message::Message_Stream llerr( adlparser_info *info );
  inline message::Message_Stream llwarn ( adlparser_info *info );
  inline message::Message_Stream llverbose( adlparser_info *info, 
					    bool with_position=true, 
					    int vlevel=1, int detail=2 );

  // !! should be called in any rule !!
#define inc_col() ( info->file_pos.inc_column( yyleng ) )

%}

%option noyywrap
%option c++
%option prefix="adlparser_"
%pointer	// define yytext as pointer (variable length)

%x ML_COMMENT

id	[a-zA-Z_][a-zA_Z0-9_]*(\.[a-zA-Z_][a-zA_Z0-9_]*)*
scal	(([0-9]+)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)
qstring \"([^\"\n]|\\\")*\"
qstring_err \"([^\"\n]|\\\")*
%%

"//".*\n  { /* ignore one line comment */ }
"/*"	  { BEGIN ML_COMMENT; }
<ML_COMMENT>"*/" { BEGIN 0; /* end of comment */ }
<ML_COMMENT>"/*" { /* ???count nested comments??? */ }
<ML_COMMENT>\n { info->file_pos.inc_line(); }
<ML_COMMENT>.* { /* ingore multiline comment */ }

" "+	  { inc_col(); }    
"\t"	  { info->file_pos.tab_inc_column(); } 
"\n"	  { info->file_pos.inc_line(); }
"\r"	  ; /*ignore DOS specific line end*/

{scal}	  { inc_col(); yylval->scalar() = atof(yytext); return TOK_SCALAR; }
{qstring} { inc_col(); yylval->string() = strip_quotes(yytext); 
	    return TOK_STRING; }
{qstring_err} { inc_col(); 
		llerr(info) << "unterminated string"; return TOK_ERROR; }

"=="	  { inc_col(); return TOK_IS_EQUAL; } /* multi character operators */
"!="	  { inc_col(); return TOK_NOT_EQUAL; }
">="	  { inc_col(); return TOK_MORE_EQUAL; }
"<="	  { inc_col(); return TOK_LESS_EQUAL; }

sin	  { inc_col(); return TOK_FUNC_SIN; } /* sin() function keyword */
sqrt	  { inc_col(); return TOK_FUNC_SQRT; } /* sqrt() function keyword */

{id}	  { inc_col(); Token tok = get_identifier(yytext,info);
	    *yylval = tok; return tok.get_type(); } /* identifiers */


.	{ inc_col(); return yytext[0]; }	/* one charater tokens */
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
  return text.substr(1,text.length()-2);
}

//*************************
// interfaces to messages
//*************************

inline message::Message_Stream llerr( adlparser_info *info )
{ 
  
  message::Message_Stream msg(message::noinit);
  info->msg.error( &info->file_pos ).copy_to(msg);
  return msg;
}

inline message::Message_Stream llwarn( adlparser_info *info )
{
  message::Message_Stream msg(message::noinit);
  info->msg.warn( &info->file_pos ).copy_to(msg);
  return msg;
}

inline message::Message_Stream llverbose( adlparser_info *info, 
					  bool with_position, 
					  int vlevel, int detail )
{
  message::Message_Stream msg(message::noinit);
  if( with_position )
    info->msg.verbose( vlevel, &info->file_pos, detail ).copy_to(msg);
  else
    info->
      msg.verbose( vlevel, message::GLOB::no_position, detail ).copy_to(msg);

  return msg;
}

/* end of lexical analizer */
