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
  #include <message/message.hpp>

  #include "tokens.h"			// token values
  #include "token.hpp"			// token type
  #include "parsinfo.hpp"		// parser/lexer info
  #include "parser_functions.hpp"	// parser/lexer help functions
 
  // forward declaration
  inline std::string strip_quotes( std::string text );

#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (afd_info&)

  inline message::Message_Stream llerr( afd_info *info );
  inline message::Message_Stream llwarn ( afd_info *info );
  inline message::Message_Stream llverbose( afd_info *info, 
					    bool with_position=true, 
					    int vlevel=1, int detail=2 );

  // !! one of the following defines should be called in any rule !!
  // increase file position according to yyleng
#define inc_col() ( info->file_pos.inc_column( yyleng ) )
  // increase file position and store begin and end position
#define tok_pos() ( {info->store_pos(); info->file_pos.inc_column( yyleng ); \
		    info->store_pos();} )

%}

%option noyywrap
%option c++
%option prefix="funcgen_"
%pointer	// define yytext as pointer (variable length)

%x ML_COMMENT COPY_CODE CC_COMMENT

id		([a-zA-Z_][a-zA-Z0-9_]*)
integer 	([0-9]+)
float   	((({integer}\.)|([0-9]*\.[0-9]+))([eE][-+]?{integer})?)
scal		({integer}|{float})
qstring 	(\"([^\"\n]|\\\")*\")
qstring_err 	(\"([^\"\n]|\\\")*)

%%
%{
// allow the usage of state stack functions yy_pop_state and yy_push_state 
#undef YY_NO_PUSH_STATE 
#undef YY_NO_POP_STATE 
#undef YY_NO_TOP_STATE 
%}

"//".*\n  { info->file_pos.inc_line(); /* ignore one line comment */ }
"/*"	  { inc_col(); yy_push_state(ML_COMMENT); }
<ML_COMMENT>"*/" 	{ inc_col(); yy_pop_state(); }
<ML_COMMENT>"/*" 	{ inc_col(); yy_push_state(ML_COMMENT); /*nested?*/ } 
<ML_COMMENT>\n         	{ info->file_pos.inc_line(); }
<ML_COMMENT>\r        	{ ; /*ignore DOS specific line end*/ }
<ML_COMMENT>[^\n\*/]+ 	{ inc_col(); /* ingore multiline comment */ }
<ML_COMMENT>[^\n\*]*\n 	{ info->file_pos.inc_line(); /*optimized*/ }
<ML_COMMENT>.	 	{ inc_col(); /* single '*' would fail otherwise */ }
<ML_COMMENT><<EOF>>	{ llerr(info) << "unexpected end of file within "
				      << "comment"; 
		  	  finished_file(info); 
			  if( info->close_file() ) return 0; }

" "+	  { inc_col(); }    
"\t"	  { info->file_pos.tab_inc_column(); } 
"\n"	  { info->file_pos.inc_line(); }
"\r"	  { ; /*ignore DOS specific line end*/ }

{scal}	      { tok_pos(); yylval->u.scalar=atof(yytext);return TAFD_SCALAR; }
{qstring}     { tok_pos(); yylval->string = strip_quotes(yytext); 
	        return TAFD_QSTRING; }
{qstring_err} { llerr(info) << "unterminated string"; tok_pos();
		return TAFD_ERROR; }

	/* multi character operators */
"=="	  		{ tok_pos(); return TAFD_IS_EQUAL; } 
"!="	  		{ tok_pos(); return TAFD_NOT_EQUAL; }
">="	  		{ tok_pos(); return TAFD_MORE_EQUAL; }
"<="	  		{ tok_pos(); return TAFD_LESS_EQUAL; }
"&&"	  		{ tok_pos(); return TAFD_AND; }
"||"			{ tok_pos(); return TAFD_OR; }

"::"			{ tok_pos(); return TAFD_NS_CONCAT; }
"->"    		{ tok_pos(); return TAFD_PT_CONCAT; }

include			{ tok_pos(); return TAFD_include; }
declaration		{ tok_pos(); return TAFD_declaration; }
header			{ tok_pos(); return TAFD_header; }
avoid_recursion		{ tok_pos(); return TAFD_avoid_recursion; }
priority_list		{ tok_pos(); return TAFD_priority_list; }
base_types		{ tok_pos(); return TAFD_base_types; }
serial			{ tok_pos(); return TAFD_serial; }
type			{ tok_pos(); return TAFD_type; }
abstract		{ tok_pos(); return TAFD_abstract; }
node			{ tok_pos(); return TAFD_node; }
provides		{ tok_pos(); return TAFD_provides; }
extends			{ tok_pos(); return TAFD_extends; }
properties		{ tok_pos(); return TAFD_properties; }
aliases			{ tok_pos(); return TAFD_aliases; }
operands		{ tok_pos(); return TAFD_operands; }
common			{ tok_pos(); return TAFD_common; }
constraints		{ tok_pos(); return TAFD_constraints; }
solvers			{ tok_pos(); return TAFD_solvers; }
actions			{ tok_pos(); return TAFD_actions; }
push			{ tok_pos(); return TAFD_push; }
condition_push		{ tok_pos(); return TAFD_condition_push; }
default			{ tok_pos(); return TAFD_default; }
contains		{ tok_pos(); return TAFD_contains; }
max1			{ tok_pos(); return TAFD_max1; }
min1			{ tok_pos(); return TAFD_min1; }
provide			{ tok_pos(); return TAFD_provide; }
resulting		{ tok_pos(); return TAFD_resulting; }
requires		{ tok_pos(); return TAFD_requires; }
this			{ tok_pos(); return TAFD_this; }
prev			{ tok_pos(); return TAFD_prev; }
next			{ tok_pos(); return TAFD_next; }
first			{ tok_pos(); return TAFD_first; }
last			{ tok_pos(); return TAFD_last; }
parent			{ tok_pos(); return TAFD_parent; }
child			{ tok_pos(); return TAFD_child; }
first_child		{ tok_pos(); return TAFD_first_child; }
last_child		{ tok_pos(); return TAFD_last_child; }
start_param		{ tok_pos(); return TAFD_start_param; }
end_param		{ tok_pos(); return TAFD_end_param; }
true			{ tok_pos(); return TAFD_true; }
false			{ tok_pos(); return TAFD_false; }
return			{ tok_pos(); return TAFD_return; }
return_prop		{ tok_pos(); return TAFD_return_prop; }
return_fail		{ tok_pos(); return TAFD_return_fail; }
return_if_fail		{ tok_pos(); return TAFD_return_if_fail; }
operators		{ tok_pos(); return TAFD_operators; }
versions		{ tok_pos(); return TAFD_versions; }
declarations		{ tok_pos(); return TAFD_declarations; }
init_operands		{ tok_pos(); return TAFD_init_operands; }
init_code		{ tok_pos(); return TAFD_init_code; }
events			{ tok_pos(); return TAFD_events; }
container		{ tok_pos(); return TAFD_container; }
serial_container	{ tok_pos(); return TAFD_serial_container; }
group			{ tok_pos(); return TAFD_group; }
event			{ tok_pos(); return TAFD_event; }
test_run		{ tok_pos(); return TAFD_test_run; }
final			{ tok_pos(); return TAFD_final; }
reset			{ tok_pos(); return TAFD_reset; }
set			{ tok_pos(); return TAFD_set; }
try			{ tok_pos(); return TAFD_try; }
try_reject		{ tok_pos(); return TAFD_try_reject; }
trial_failed		{ tok_pos(); return TAFD_trial_failed; }
is_solved_in_try	{ tok_pos(); return TAFD_is_solved_in_try; }
is_just_solved		{ tok_pos(); return TAFD_is_just_solved; }
solver			{ tok_pos(); return TAFD_solver; }
first_index		{ tok_pos(); return TAFD_first_index; }
last_index		{ tok_pos(); return TAFD_last_index; }
for_each		{ tok_pos(); return TAFD_for_each; }
element			{ tok_pos(); return TAFD_element; }

{id}	  { tok_pos(); yylval->string = yytext; return TAFD_IDENTIFIER; }

.	  { tok_pos(); return yytext[0]; }	/* one charater tokens */

<COPY_CODE>"\t"	{ info->file_pos.tab_inc_column(); 
		  copy_code_line( info, yytext,yyleng ); } 
<COPY_CODE>"\n"	{ info->file_pos.inc_line(); 
		  copy_code_line( info, yytext,yyleng ); }
<COPY_CODE>"\r"	{ ; /*ignore DOS specific line end*/ }
<COPY_CODE>"{"	{ inc_col(); ++info->depth_counter; 
		  copy_code_line( info, yytext, yyleng ); }
<COPY_CODE>"}"	{ if( --info->depth_counter < 0 ) 
		  { unput('}'); yy_pop_state(); return TAFD_CODE;} 
		  else
		  { inc_col(); copy_code_line( info, yytext, yyleng ); }
                }
<COPY_CODE>[^\n\t\[{}/]+  {inc_col(); copy_code_line( info, yytext, yyleng ); }
<COPY_CODE>[^\n\[{}/]+"\n" {info->file_pos.inc_line(); 
			   copy_code_line( info, yytext,yyleng ); }
<COPY_CODE>"//".*\n { info->file_pos.inc_line(); 
		      copy_code_line( info, yytext,yyleng ); }
<COPY_CODE>"[["	{ tok_pos(); code_block_escape(info); yy_pop_state(); 
		  return TAFD_BB_left; }
<COPY_CODE>"/*"	{ inc_col(); yy_push_state(CC_COMMENT); 
		  copy_code_line( info, yytext,yyleng ); }
<COPY_CODE>.    { inc_col(); copy_code_line( info, yytext, yyleng ); }

<CC_COMMENT>"*/" 	{ inc_col(); yy_pop_state(); 
			  copy_code_line( info, yytext, yyleng ); }
<CC_COMMENT>\r        	{ ; /*ignore DOS specific line end*/ }
<CC_COMMENT>[^\n\*]+ 	{ inc_col(); copy_code_line( info, yytext, yyleng ); }
<CC_COMMENT>[^\n\*]*\n  { info->file_pos.inc_line();
			   copy_code_line( info, yytext, yyleng ); }
<CC_COMMENT>.	 	{ inc_col(); copy_code_line( info, yytext, yyleng ); }
<CC_COMMENT><<EOF>>	{ llerr(info) << "unexpected end of file within "
				      << "comment";
			  finished_file(info); 
			  if( info->close_file() ) return 0; }

<<EOF>>		{ finished_file(info); if( info->close_file() ) return 0; }

%%

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

inline message::Message_Stream llerr( afd_info *info )
{ 
  
  message::Message_Stream msg(message::noinit);
  info->msg.error( &info->file_pos ).copy_to(msg);
  return msg;
}

inline message::Message_Stream llwarn( afd_info *info )
{
  message::Message_Stream msg(message::noinit);
  info->msg.warn( &info->file_pos ).copy_to(msg);
  return msg;
}

inline message::Message_Stream llverbose( afd_info *info, 
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

//******************
// public functions
//******************

void yyFlexLexer::goto_initial_state() 
{
  BEGIN INITIAL;
}

void yyFlexLexer::goto_code_copy_mode()
{
  yy_push_state(COPY_CODE);
}

void yyFlexLexer::finish_mode()
{
  yy_pop_state();
}

void yyFlexLexer::set_input_stream( std::istream &in ) 
{
  yyin = &in;
}

/* end of lexical analizer */
