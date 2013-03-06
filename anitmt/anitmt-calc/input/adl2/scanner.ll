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

  // !! one of the following defines should be called in any rule !!
  // increase file position according to yyleng
#define inc_col() ( info->file_pos.inc_column( yyleng ) )
  // increase file position and store begin and end position
#define tok_pos() ( {info->store_pos(); info->file_pos.inc_column( yyleng ); \
		    info->store_pos();} )

  inline int yywrap() { return 1; }
%}

%option c++
%option prefix="adlparser_"
%pointer	// define yytext as pointer (variable length)

%x ML_COMMENT DUMMY_STATEMENT DUMMY_ML_COMMENT

id	([a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*)
integer ([0-9]+)
float   ((({integer}\.)|([0-9]*\.[0-9]+))([eE][-+]?{integer})?)
scal	({integer}|{float})
qstring (\"([^\"\n]|\\\")*\")
qstring_err (\"([^\"\n]|\\\")*)

operator ("=="|"!="|"<="|">=")
operand  ({scal}|{qstring}|{id})
%%
%{
#undef YY_NO_PUSH_STATE 
#undef YY_NO_POP_STATE 
#undef YY_NO_TOP_STATE 
%}

"//".*\n  { info->file_pos.inc_line(); /* ignore one line comment */ }
"/*"	  { inc_col(); yy_push_state(ML_COMMENT); }
<ML_COMMENT>{
  "*/"		{ inc_col(); yy_pop_state(); }
  "/*"		{ inc_col(); yy_push_state(ML_COMMENT); /*nested?*/ } 
  \n         	{ info->file_pos.inc_line(); }
  \r        	{ ; /*ignore DOS specific line end*/ }
  [^\n\*]* 	{ inc_col(); /* ingore multiline comment */ }
  [^\n\*]*\n 	{ info->file_pos.inc_line(); /*optimized*/ }
  .	 	{ inc_col(); /* singel '*' would fail otherwise */ }
}

<DUMMY_STATEMENT>{
  " "	  	{ inc_col(); }    
  "\t"	  	{ info->file_pos.tab_inc_column(); } 
  "\n"	  	{ info->file_pos.inc_line(); }
  "\r"	  	{ ; /*ignore DOS specific line end*/ }
  ";"   	{ BEGIN INITIAL; tok_pos(); return yytext[0]; }
  {operand}  	{ tok_pos(); return TOK_DUMMY_OPERAND; }
  {operator}	{ tok_pos(); return TOK_DUMMY_OPERATOR; }
  .	   	{ tok_pos(); return yytext[0]; }

  "//".*\n  { info->file_pos.inc_line(); /* ignore one line comment */ }
  "/*"	    { inc_col(); yy_push_state(DUMMY_ML_COMMENT); }
}
<DUMMY_ML_COMMENT>{
  "*/"		{ inc_col(); yy_pop_state(); }
  "/*"		{ inc_col(); yy_push_state(DUMMY_ML_COMMENT); /*nested?*/ } 
  \n         	{ info->file_pos.inc_line(); }
  \r        	{ ; /*ignore DOS specific line end*/ }
  [^\n\*]* 	{ inc_col(); /* ingore multiline comment */ }
  [^\n\*]*\n 	{ info->file_pos.inc_line(); /*optimized*/ }
  .	 	{ inc_col(); /* singel '*' would fail otherwise */ }
}

" "+	  { inc_col(); }    
"\t"	  { info->file_pos.tab_inc_column(); } 
"\n"	  { info->file_pos.inc_line(); }
"\r"	  { ; /*ignore DOS specific line end*/ }

{scal}	      { tok_pos(); yylval->scalar() = atof(yytext);return TOK_SCALAR; }
{qstring}     { tok_pos(); yylval->string() = strip_quotes(yytext); 
	        return TOK_STRING; }
{qstring_err} { llerr(info) << "unterminated string"; tok_pos();
		return TOK_ERROR; }

"=="	  { tok_pos(); return TOK_IS_EQUAL; } /* multi character operators */
"!="	  { tok_pos(); return TOK_NOT_EQUAL; }
">="	  { tok_pos(); return TOK_MORE_EQUAL; }
"<="	  { tok_pos(); return TOK_LESS_EQUAL; }

true	  { tok_pos(); yylval->flag() = true;  return TOK_FLAG; }
false	  { tok_pos(); yylval->flag() = false; return TOK_FLAG; }

_debug			{ tok_pos(); return TOK_FUNC__debug; }
abs			{ tok_pos(); return TOK_FUNC_abs; }
abs2			{ tok_pos(); return TOK_FUNC_abs2; }
angle_range		{ tok_pos(); return TOK_FUNC_angle_range; }
ceil			{ tok_pos(); return TOK_FUNC_ceil; }
choose			{ tok_pos(); return TOK_FUNC_choose; }
cross			{ tok_pos(); return TOK_FUNC_cross; }
deg2rad			{ tok_pos(); return TOK_FUNC_deg2rad; }
floor			{ tok_pos(); return TOK_FUNC_floor; }
mat_inverse		{ tok_pos(); return TOK_FUNC_mat_inverse; }
get_element		{ tok_pos(); return TOK_FUNC_get_element; }
get_rotate_component	{ tok_pos(); return TOK_FUNC_get_rotate_component; }
get_scale_component	{ tok_pos(); return TOK_FUNC_get_scale_component; }
get_translate_component	{ tok_pos(); return TOK_FUNC_get_translate_component; }
mat_rotate		{ tok_pos(); return TOK_FUNC_mat_rotate; }
mat_rotate_around	{ tok_pos(); return TOK_FUNC_mat_rotate_around; }
mat_rotate_pair_pair	{ tok_pos(); return TOK_FUNC_mat_rotate_pair_pair; }
mat_rotate_spherical_pair { tok_pos(); return TOK_FUNC_mat_rotate_spherical_pair; }
mat_rotate_vect_vect	{ tok_pos(); return TOK_FUNC_mat_rotate_vect_vect; }
mat_rotate_vect_vect_up	{ tok_pos(); return TOK_FUNC_mat_rotate_vect_vect_up; }
mat_rotate_x		{ tok_pos(); return TOK_FUNC_mat_rotate_x; }
mat_rotate_y		{ tok_pos(); return TOK_FUNC_mat_rotate_y; }
mat_rotate_z		{ tok_pos(); return TOK_FUNC_mat_rotate_z; }
mat_scale		{ tok_pos(); return TOK_FUNC_mat_scale; }
mat_scale_x		{ tok_pos(); return TOK_FUNC_mat_scale_x; }
mat_scale_y		{ tok_pos(); return TOK_FUNC_mat_scale_y; }
mat_scale_z		{ tok_pos(); return TOK_FUNC_mat_scale_z; }
mat_translate		{ tok_pos(); return TOK_FUNC_mat_translate; }
mat_translate_x		{ tok_pos(); return TOK_FUNC_mat_translate_x; }
mat_translate_y		{ tok_pos(); return TOK_FUNC_mat_translate_y; }
mat_translate_z		{ tok_pos(); return TOK_FUNC_mat_translate_z; }
dot			{ tok_pos(); return TOK_FUNC_dot; }
pass_if			{ tok_pos(); return TOK_FUNC_pass_if; }
pass_if_equal		{ tok_pos(); return TOK_FUNC_pass_if_equal; }
pass_if_not_equal	{ tok_pos(); return TOK_FUNC_pass_if_not_equal; }
plus_minus		{ tok_pos(); return TOK_FUNC_plus_minus; }
rad2deg			{ tok_pos(); return TOK_FUNC_rad2deg; }
reject_if		{ tok_pos(); return TOK_FUNC_reject_if; }
reject_if_equal		{ tok_pos(); return TOK_FUNC_reject_if_equal; }
reject_if_not_equal	{ tok_pos(); return TOK_FUNC_reject_if_not_equal; }
round			{ tok_pos(); return TOK_FUNC_round; }
sqrt			{ tok_pos(); return TOK_FUNC_sqrt; }
to_vector		{ tok_pos(); return TOK_FUNC_to_vector; }
trunc			{ tok_pos(); return TOK_FUNC_trunc; }
vec_angle		{ tok_pos(); return TOK_FUNC_vec_angle; }
vec_mirror		{ tok_pos(); return TOK_FUNC_vec_mirror; }
vec_normalize		{ tok_pos(); return TOK_FUNC_vec_normalize; }
vec_rotate_x		{ tok_pos(); return TOK_FUNC_vec_rotate_x; }
vec_rotate_y		{ tok_pos(); return TOK_FUNC_vec_rotate_y; }
vec_rotate_z		{ tok_pos(); return TOK_FUNC_vec_rotate_z; }
vec_scale		{ tok_pos(); return TOK_FUNC_vec_scale; }
vec_to_rectangular	{ tok_pos(); return TOK_FUNC_vec_to_rectangular; }
vec_to_spherical	{ tok_pos(); return TOK_FUNC_vec_to_spherical; }
vec_translate		{ tok_pos(); return TOK_FUNC_vec_translate; }

{id}	  { tok_pos(); Token tok = get_identifier(yytext,info);
	    *yylval = tok; return tok.get_type(); } /* identifiers */


.	{ tok_pos(); return yytext[0]; }	/* one charater tokens */
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

#undef tok_pos
#undef inc_col

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

void yyFlexLexer::goto_initial_state() 
{
  BEGIN INITIAL;
}
void yyFlexLexer::dummy_statement_follows() 
{
  BEGIN DUMMY_STATEMENT;
}
void yyFlexLexer::set_input_stream( std::istream &in ) 
{
  yyin = &in;
}

/* end of lexical analizer */
