/*****************************************************************************/
/**   This file defines a parser/compiler for the ADL language              **/
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

//***********
// Codeblock
//***********

%{
#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>
#include <functionality/solver.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "adlparser.hpp"

#include "parser_functions.hpp"	// help functions for the parser
				// including nessessary defines 

#include <iostream>
#include <string>
#include <math.h>

// include config switches like YYDEBUG
#include <config.h>

  // open namespaces
  namespace anitmt
  {
    namespace adlparser
    {

#define MAX_OLD_POSITIONS 	10

%}

//*********
// Options
//*********
   
%pure_parser		// parser may be called recursive
//%expect 79		// expect 7 shift/reduce and 27 reduce/reduce conflicts

//********
// Tokens
//********

%token TOK_INVALID_ID TOK_ERROR 
// tokens in dummy expression (without semantic)
%token TOK_DUMMY_OPERAND TOK_DUMMY_OPERATOR
// multi character operators
%token TOK_IS_EQUAL TOK_NOT_EQUAL TOK_MORE_EQUAL TOK_LESS_EQUAL 
// type tokens
%token <identifier()> TOK_IDENTIFIER
%token <flag()>   TOK_FLAG
%token <scalar()> TOK_SCALAR
%token <vector()> TOK_VECTOR
%token <matrix()> TOK_MATRIX
%token <string()> TOK_STRING
// operand tokens for creating expression trees
%token <op_flag(info)>   TOK_OP_FLAG
%token <op_scalar(info)> TOK_OP_SCALAR
%token <op_vector(info)> TOK_OP_VECTOR
%token <op_matrix(info)> TOK_OP_MATRIX
%token <op_string(info)> TOK_OP_STRING
// property tokens
%token <prop_flag()>   TOK_PROP_FLAG
%token <prop_scalar()> TOK_PROP_SCALAR
%token <prop_vector()> TOK_PROP_VECTOR
%token <prop_matrix()> TOK_PROP_MATRIX
%token <prop_string()> TOK_PROP_STRING
// types of rules
%type <flag()>	 flag_exp 
%type <scalar()> scalar_exp 
%type <vector()> vector_exp 
%type <matrix()> matrix_exp 
%type <string()> string_exp 
// meta operands allow assigning without
%type <meta_op_flag(info)>   op_flag_exp 
%type <meta_op_scalar(info)> op_scalar_exp 
%type <meta_op_vector(info)> op_vector_exp 
%type <meta_op_matrix(info)> op_matrix_exp 
%type <meta_op_string(info)> op_string_exp 
// mixed types so it returns simply a token
%type <tok()> any_flag_exp 
%type <tok()> any_scalar_exp 
%type <tok()> any_vector_exp 
%type <tok()> any_matrix_exp 
%type <tok()> any_string_exp 

// functions
%token TOK_FUNC__debug
%token TOK_FUNC_abs
%token TOK_FUNC_abs2
%token TOK_FUNC_ceil
%token TOK_FUNC_choose
%token TOK_FUNC_cross
%token TOK_FUNC_floor
%token TOK_FUNC_get_element
%token TOK_FUNC_get_rotate_component
%token TOK_FUNC_get_scale_component
%token TOK_FUNC_get_translate_component
%token TOK_FUNC_mat_inverse
%token TOK_FUNC_mat_rotate
%token TOK_FUNC_mat_rotate_around
%token TOK_FUNC_mat_rotate_pair_pair
%token TOK_FUNC_mat_rotate_spherical_pair
%token TOK_FUNC_mat_rotate_vect_vect
%token TOK_FUNC_mat_rotate_vect_vect_up
%token TOK_FUNC_mat_rotate_x
%token TOK_FUNC_mat_rotate_y
%token TOK_FUNC_mat_rotate_z
%token TOK_FUNC_mat_scale
%token TOK_FUNC_mat_scale_x
%token TOK_FUNC_mat_scale_y
%token TOK_FUNC_mat_scale_z
%token TOK_FUNC_mat_translate
%token TOK_FUNC_mat_translate_x
%token TOK_FUNC_mat_translate_y
%token TOK_FUNC_mat_translate_z
%token TOK_FUNC_dot
%token TOK_FUNC_pass_if
%token TOK_FUNC_pass_if_equal
%token TOK_FUNC_pass_if_not_equal
%token TOK_FUNC_plus_minus
%token TOK_FUNC_reject_if
%token TOK_FUNC_reject_if_equal
%token TOK_FUNC_reject_if_not_equal
%token TOK_FUNC_round
%token TOK_FUNC_sqrt
%token TOK_FUNC_to_angle
%token TOK_FUNC_to_vector
%token TOK_FUNC_trunc
%token TOK_FUNC_vec_angle
%token TOK_FUNC_vec_mirror
%token TOK_FUNC_vec_normalize
%token TOK_FUNC_vec_rotate_x
%token TOK_FUNC_vec_rotate_y
%token TOK_FUNC_vec_rotate_z
%token TOK_FUNC_vec_scale
%token TOK_FUNC_vec_to_rectangular
%token TOK_FUNC_vec_to_spherical
%token TOK_FUNC_vec_translate

//****************
// precicion table
//****************

// special precedences for simplified cases
%nonassoc lowest_precedence

//********************
// normal precedences
// !! value to operand conversion must be last !!
%nonassoc OP_CONVERTION 

%left '?' ':'
%left TAFD_AND TAFD_OR
%nonassoc '<' '>' TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%left '+' '-'
%left '*' '/'
%left '&' '|' '^' // !!! check this precedence !!!
%nonassoc UMINUS '!'

// special precedences for simplified cases
%left left_associated
%left right_associated

%nonassoc lower_precedence
%nonassoc higher_precedence
%nonassoc highest_precedence

//****************
// Parser Rules
//****************
%%
tree_node_block: /* empty */
  | tree_node_block statement
  | error			{ yyerrok; /* error recovery */ }
;
    
statement: 
    child_declaration
  | flag_statement ';'
  | scalar_statement ';'
  | vector_statement ';'
  | matrix_statement ';'
  | string_statement ';'
  | no_statement ';'		// causes some bison conficts
  | error ';' %prec lowest_precedence { yyerrok; /* error recovery */ }
;

child_declaration: 
    TOK_IDENTIFIER '{'			{ change_current_child(info,$1); } 
      tree_node_block '}'		{ change_to_parent(info); }
  | TOK_IDENTIFIER TOK_IDENTIFIER '{'	{ change_current_child(info,$1,$2);} 
      tree_node_block '}'		{ change_to_parent(info); }
;

no_statement:
    TOK_IDENTIFIER	{ yyerr(info,2) << "unknown property " << $1; }
      error 		{ yyerrok; }
;

flag_statement: 
    TOK_PROP_FLAG	{ prop_declaration_start($1,info); } 
      any_flag_exp	{ flag_prop_declaration_finish($1,$3,info); }
;
any_flag_exp:      
    flag_exp		{ $<flag()>$ = $1; }
  | op_flag_exp		{ $<meta_op_flag(info)>$ = $1(); }
  | receive_dummy_exp	{ $$ = Token(); }
  | error		{ yyerr(info,2) 
			    << "invalid flag expression: operand expected"; 
			  yyerrok; $$ = Token(); }
;

scalar_statement: 
    TOK_PROP_SCALAR	{ prop_declaration_start($1,info); } 
      any_scalar_exp	{ scalar_prop_declaration_finish($1,$3,info); }
;
any_scalar_exp:      
    scalar_exp 		{ $<scalar()>$ = $1; }
  | op_scalar_exp 	{ $<meta_op_scalar(info)>$ = $1(); }
  | receive_dummy_exp	{ $$ = Token(); }
  | error		{ yyerr(info,2) 
			    << "invalid scalar expression: operand expected"; 
			  yyerrok; $$ = Token(); }
;
 
vector_statement: 
    TOK_PROP_VECTOR	{ prop_declaration_start($1,info); } 
      any_vector_exp	{ vector_prop_declaration_finish($1,$3,info); }
;
any_vector_exp:      
    vector_exp		{ $<vector()>$ = $1; }
  | op_vector_exp	{ $<meta_op_vector(info)>$ = $1(); }
  | receive_dummy_exp	{ $$ = Token(); }
  | error		{ yyerr(info,2) 
			    << "invalid vector expression: operand expected"; 
			  yyerrok; $$ = Token(); }
;
 
matrix_statement: 
    TOK_PROP_MATRIX	{ prop_declaration_start($1,info); } 
      any_matrix_exp	{ matrix_prop_declaration_finish($1,$3,info); }
;
any_matrix_exp:      
    matrix_exp		{ $<matrix()>$ = $1; }
  | op_matrix_exp	{ $<meta_op_matrix(info)>$ = $1(); }
  | receive_dummy_exp	{ $$ = Token(); }
  | error		{ yyerr(info,2) 
			    << "invalid matrix expression: operand expected"; 
			  yyerrok; $$ = Token(); }
;
 
string_statement: 
    TOK_PROP_STRING	{ prop_declaration_start($1,info); } 
      any_string_exp	{ string_prop_declaration_finish($1,$3,info); }
;
any_string_exp:      
    string_exp		{ $<string()>$ = $1; }
  | op_string_exp	{ $<meta_op_string(info)>$ = $1(); }
  | receive_dummy_exp	{ $$ = Token(); }
  | error		{ yyerr(info,2) 
			    << "invalid string expression: operand expected"; 
			  yyerrok; $$ = Token(); }
;
 
flag_exp:
    '(' flag_exp ')'					{ $$ = $2; }
  | flag_exp TAFD_IS_EQUAL flag_exp			{ $$ = $1 == $3; }
  | scalar_exp TAFD_IS_EQUAL scalar_exp			{ $$ = $1 == $3; }
  | vector_exp TAFD_IS_EQUAL vector_exp			{ $$ = $1 == $3; }
  | matrix_exp TAFD_IS_EQUAL matrix_exp			{ $$ = $1 == $3; }
  | string_exp TAFD_IS_EQUAL string_exp			{ $$ = $1 == $3; }
  | flag_exp TAFD_AND flag_exp				{ $$ = $1 && $3; }
  | flag_exp TAFD_OR flag_exp				{ $$ = $1 || $3; }
  | scalar_exp '>' scalar_exp				{ $$ = $1 > $3; }
  | vector_exp '>' vector_exp				{ $$ = $1 > $3; }
  | string_exp '>' string_exp				{ $$ = $1 > $3; }
  | scalar_exp TAFD_MORE_EQUAL scalar_exp		{ $$ = $1 >= $3; }
  | vector_exp TAFD_MORE_EQUAL vector_exp		{ $$ = $1 >= $3; }
  | string_exp TAFD_MORE_EQUAL string_exp		{ $$ = $1 >= $3; }
  | scalar_exp '<' scalar_exp				{ $$ = $1 < $3; }
  | vector_exp '<' vector_exp				{ $$ = $1 < $3; }
  | string_exp '<' string_exp				{ $$ = $1 < $3; }
  | scalar_exp TAFD_LESS_EQUAL scalar_exp		{ $$ = $1 <= $3; }
  | vector_exp TAFD_LESS_EQUAL vector_exp		{ $$ = $1 <= $3; }
  | string_exp TAFD_LESS_EQUAL string_exp		{ $$ = $1 <= $3; }
  | '!' flag_exp %prec UMINUS				{ $$ = ! $2; }
  | '!' scalar_exp %prec UMINUS				{ $$ = ! $2; }
  | '!' vector_exp %prec UMINUS				{ $$ = ! $2; }
  | '!' matrix_exp %prec UMINUS				{ $$ = ! $2; }
  | '!' string_exp %prec UMINUS				{ $$ = ! $2; }
  | flag_exp TAFD_NOT_EQUAL flag_exp			{ $$ = $1 != $3; }
  | scalar_exp TAFD_NOT_EQUAL scalar_exp		{ $$ = $1 != $3; }
  | vector_exp TAFD_NOT_EQUAL vector_exp		{ $$ = $1 != $3; }
  | matrix_exp TAFD_NOT_EQUAL matrix_exp		{ $$ = $1 != $3; }
  | string_exp TAFD_NOT_EQUAL string_exp		{ $$ = $1 != $3; }
  | TOK_FLAG						{ $$ = $1; }

    // additional operators
  | flag_exp '?' flag_exp ':' flag_exp  
        { $$ = $1 ? $3 : $5; }
;
scalar_exp:
    '(' scalar_exp ')'					{ $$ = $2; }
  | scalar_exp '+' scalar_exp				{ $$ = $1 + $3; }
  | scalar_exp '/' scalar_exp				{ $$ = $1 / $3; }
  | scalar_exp '*' scalar_exp				{ $$ = $1 * $3; }
  | vector_exp '*' vector_exp				{ $$ = $1 * $3; }
  | scalar_exp '-' scalar_exp				{ $$ = $1 - $3; }
  | '-' scalar_exp %prec UMINUS				{ $$ = - $2; }
    /*
  | TOK_FUNC_abs '(' scalar_exp  ')'
        { $$ = abs ( $3  ); }
  | TOK_FUNC_abs '(' vector_exp  ')'
        { $$ = abs ( $3  ); }
  | TOK_FUNC_abs2 '(' vector_exp  ')'
        { $$ = abs2 ( $3  ); }
  | TOK_FUNC_ceil '(' scalar_exp  ')'
        { $$ = ceil ( $3  ); }
  | TOK_FUNC_floor '(' scalar_exp  ')'
        { $$ = floor ( $3  ); }
  | TOK_FUNC_get_element '(' matrix_exp ',' scalar_exp ',' 
      scalar_exp  ')'
        { $$ = $3 [ $5 ] [ $7 ]; }
  | TOK_FUNC_dot '(' vector_exp ',' vector_exp  ')'
        { $$ = dot ( $3 , $5  ); }
  | TOK_FUNC_vec_angle '(' vector_exp ',' vector_exp  ')'
        { $$ = vec_angle ( $3 , $5  ); }
  | TOK_FUNC_round '(' scalar_exp  ')'
        { $$ = round ( $3  ); }
  | TOK_FUNC_sqrt '(' scalar_exp  ')'
        { $$ = sqrt ( $3  ); }
//  | TOK_FUNC_to_angle '(' scalar_exp  ')'
//        { $$ = to_angle ( $3  ); } // Wolfgang will implement this
  | TOK_FUNC_trunc '(' scalar_exp  ')'
        { $$ = trunc ( $3  ); }
  | TOK_FUNC_get_element '(' vector_exp ',' scalar_exp  ')'
        { $$ = $3 [ $5 ]; }
    */
  | TOK_SCALAR						{ $$ = $1; }

    // additional operators
  | '+' scalar_exp %prec UMINUS				{ $$ = $2; }
    /*
  | flag_exp '?' scalar_exp ':' scalar_exp  
        { $$ = $1 ? $3 : $5; }
  | vector_exp '[' scalar_exp  ']'		        { $$ = $1 [$3]; }
  | matrix_exp '[' scalar_exp ']' '[' scalar_exp  ']'   { $$ = $1 [$3][$6]; }
    */
;
vector_exp:
    '(' vector_exp ')'					{ $$ = $2; }
  | vector_exp '+' vector_exp				{ $$ = $1 + $3; }
  | vector_exp '/' scalar_exp				{ $$ = $1 / $3; }
  | scalar_exp '*' vector_exp				{ $$ = $1 * $3; }
  | matrix_exp '*' vector_exp				{ $$ = $1 * $3; }
  | '-' vector_exp %prec UMINUS				{ $$ = - $2; }
  | vector_exp '-' vector_exp				{ $$ = $1 - $3; }
    /*
  | TOK_FUNC_cross '(' vector_exp ',' vector_exp  ')'
        { $$ = cross ( $3 , $5  ); }
  | TOK_FUNC_get_rotate_component '(' matrix_exp  ')'
        { $$ = get_rotate_component ( $3  ); }
  | TOK_FUNC_get_scale_component '(' matrix_exp  ')'
        { $$ = get_scale_component ( $3  ); }
  | TOK_FUNC_get_translate_component '(' matrix_exp  ')'
        { $$ = get_translate_component ( $3  ); }
  | TOK_FUNC_to_vector '(' scalar_exp ',' scalar_exp ',' scalar_exp  ')'
        { $$ = values::Vector( $3 , $5 , $7  ); }
  | TOK_FUNC_vec_mirror '(' vector_exp ',' scalar_exp  ')'
        { $$ = vec_mirror ( $3 , $5  ); }
  | TOK_FUNC_vec_mirror '(' vector_exp  ')'
        { $$ = vec_mirror ( $3  ); }
  | TOK_FUNC_vec_normalize '(' vector_exp  ')'
        { $$ = vec_normalize ( $3  ); }
  | TOK_FUNC_vec_rotate_x '(' vector_exp ',' scalar_exp  ')'
        { $$ = vec_rotate_x ( $3 , $5  ); }
  | TOK_FUNC_vec_rotate_y '(' vector_exp ',' scalar_exp  ')'
        { $$ = vec_rotate_y ( $3 , $5  ); }
  | TOK_FUNC_vec_rotate_z '(' vector_exp ',' scalar_exp  ')'
        { $$ = vec_rotate_z ( $3 , $5  ); }
  | TOK_FUNC_vec_scale '(' vector_exp ',' scalar_exp ',' scalar_exp  ')'
        { $$ = vec_scale ( $3, $5, $7 ); }
  | TOK_FUNC_vec_to_rectangular '(' vector_exp  ')'
        { $$ = vec_to_rectangular ( $3  ); }
  | TOK_FUNC_vec_to_spherical '(' vector_exp  ')'
        { $$ = vec_to_spherical ( $3  ); }
  | TOK_FUNC_vec_translate '(' vector_exp ',' scalar_exp ',' scalar_exp  ')'
        { $$ = vec_translate ( $3 , $5 , $7  ); }
    */
  | TOK_VECTOR						{ $$ = $1; }

    // additional operators
  | '+' vector_exp %prec UMINUS				{ $$ = $2; }
    /*
  | flag_exp '?' vector_exp ':' vector_exp  
        { $$ = $1 ? $3 : $5; }
  | '<' scalar_exp ',' scalar_exp ',' scalar_exp  '>'
        { $$ = values::Vector ( $2 , $4 , $6  ); }
    */
;
matrix_exp:
    '(' matrix_exp ')'					{ $$ = $2; }
  | matrix_exp '+' matrix_exp				{ $$ = $1 + $3; }
    /*
  | TOK_FUNC_mat_inverse '(' matrix_exp  ')'
        { $$ = mat_inverse ( $3  ); }
  | TOK_FUNC_mat_rotate '(' vector_exp  ')'
        { $$ = mat_rotate ( $3  ); }
  | TOK_FUNC_mat_rotate_around '(' vector_exp ',' scalar_exp  ')'
        { $$ = mat_rotate_around ( $3 , $5  ); }
  | TOK_FUNC_mat_rotate_pair_pair '(' vector_exp ',' vector_exp ',' vector_exp
      ',' vector_exp  ')'
        { $$ = mat_rotate_pair_pair ( $3 , $5 , $7 , $9  ); }
  | TOK_FUNC_mat_rotate_spherical_pair '(' vector_exp ',' vector_exp ',' 
      vector_exp  ')'
        { $$ = mat_rotate_spherical_pair ( $3 , $5 , $7  ); }
  | TOK_FUNC_mat_rotate_vect_vect '(' vector_exp ',' vector_exp  ')'
        { $$ = mat_rotate_vect_vect ( $3 , $5  ); }
  | TOK_FUNC_mat_rotate_vect_vect_up '(' vector_exp ',' vector_exp ',' 
      vector_exp  ')'
        { $$ = mat_rotate_vect_vect_up ( $3 , $5 , $7  ); }
  | TOK_FUNC_mat_rotate_x '(' scalar_exp  ')'
        { $$ = mat_rotate_x ( $3  ); }
  | TOK_FUNC_mat_rotate_y '(' scalar_exp  ')'
        { $$ = mat_rotate_y ( $3  ); }
  | TOK_FUNC_mat_rotate_z '(' scalar_exp  ')'
        { $$ = mat_rotate_z ( $3  ); }
  | TOK_FUNC_mat_scale '(' vector_exp  ')'
        { $$ = mat_scale ( $3  ); }
  | TOK_FUNC_mat_scale_x '(' scalar_exp  ')'
        { $$ = mat_scale_x ( $3  ); }
  | TOK_FUNC_mat_scale_y '(' scalar_exp  ')'
        { $$ = mat_scale_y ( $3  ); }
  | TOK_FUNC_mat_scale_z '(' scalar_exp  ')'
        { $$ = mat_scale_z ( $3  ); }
  | TOK_FUNC_mat_translate '(' vector_exp  ')'
        { $$ = mat_translate ( $3  ); }
  | TOK_FUNC_mat_translate_x '(' scalar_exp  ')'
        { $$ = mat_translate_x ( $3  ); }
  | TOK_FUNC_mat_translate_y '(' scalar_exp  ')'
        { $$ = mat_translate_y ( $3  ); }
  | TOK_FUNC_mat_translate_z '(' scalar_exp  ')'
        { $$ = mat_translate_z ( $3  ); }
    */
  | scalar_exp '*' matrix_exp				{ $$ = $1 * $3; }
  | matrix_exp '*' matrix_exp				{ $$ = $1 * $3; }
  | '-' matrix_exp %prec UMINUS				{ $$ = - $2; }
  | matrix_exp '-' matrix_exp				{ $$ = $1 - $3; }
  | TOK_MATRIX						{ $$ = $1; }

    // additional operators
  | '+' matrix_exp %prec UMINUS				{ $$ = $2; }
    /*
  | flag_exp '?' matrix_exp ':' matrix_exp  
        { $$ = $1 ? $3 : $5; }
    */
;
string_exp:
    '(' string_exp ')'					{ $$ = $2; }
  | string_exp '+' string_exp				{ $$ = $1 + $3; }
  | TOK_STRING						{ $$ = $1; }

    // additional operators
    /*
  | flag_exp '?' string_exp ':' string_exp  
        { $$ = $1 ? $3 : $5; }
    */
;

op_flag_exp:
    '(' op_flag_exp ')'					{ $$ = $2(); }
  | TOK_FUNC__debug '(' op_flag_exp  ')'
        { $$ = _debug ( $3()  ); }
  | TOK_FUNC_choose '(' op_flag_exp ',' op_flag_exp ',' op_flag_exp  ')'
        { $$ = choose ( $3() , $5() , $7()  ); }
  | op_flag_exp TAFD_IS_EQUAL op_flag_exp		{ $$ = $1() == $3(); }
  | op_scalar_exp TAFD_IS_EQUAL op_scalar_exp		{ $$ = $1() == $3(); }
  | op_vector_exp TAFD_IS_EQUAL op_vector_exp		{ $$ = $1() == $3(); }
  | op_matrix_exp TAFD_IS_EQUAL op_matrix_exp		{ $$ = $1() == $3(); }
  | op_string_exp TAFD_IS_EQUAL op_string_exp		{ $$ = $1() == $3(); }
  | op_flag_exp TAFD_AND op_flag_exp			{ $$ = $1() && $3(); }
  | op_flag_exp TAFD_OR op_flag_exp			{ $$ = $1() || $3(); }
  | op_scalar_exp '>' op_scalar_exp			{ $$ = $1() > $3(); }
  | op_vector_exp '>' op_vector_exp			{ $$ = $1() > $3(); }
  | op_string_exp '>' op_string_exp			{ $$ = $1() > $3(); }
  | op_scalar_exp TAFD_MORE_EQUAL op_scalar_exp		{ $$ = $1() >= $3(); }
  | op_vector_exp TAFD_MORE_EQUAL op_vector_exp		{ $$ = $1() >= $3(); }
  | op_string_exp TAFD_MORE_EQUAL op_string_exp		{ $$ = $1() >= $3(); }
  | op_scalar_exp '<' op_scalar_exp			{ $$ = $1() < $3(); }
  | op_vector_exp '<' op_vector_exp			{ $$ = $1() < $3(); }
  | op_string_exp '<' op_string_exp			{ $$ = $1() < $3(); }
  | op_scalar_exp TAFD_LESS_EQUAL op_scalar_exp		{ $$ = $1() <= $3(); }
  | op_vector_exp TAFD_LESS_EQUAL op_vector_exp		{ $$ = $1() <= $3(); }
  | op_string_exp TAFD_LESS_EQUAL op_string_exp		{ $$ = $1() <= $3(); }
  | '!' op_flag_exp %prec UMINUS			{ $$ = ! $2(); }
  | '!' op_scalar_exp %prec UMINUS			{ $$ = ! $2(); }
  | '!' op_vector_exp %prec UMINUS			{ $$ = ! $2(); }
  | '!' op_matrix_exp %prec UMINUS			{ $$ = ! $2(); }
  | '!' op_string_exp %prec UMINUS			{ $$ = ! $2(); }
  | op_flag_exp TAFD_NOT_EQUAL op_flag_exp		{ $$ = $1() != $3(); }
  | op_scalar_exp TAFD_NOT_EQUAL op_scalar_exp		{ $$ = $1() != $3(); }
  | op_vector_exp TAFD_NOT_EQUAL op_vector_exp		{ $$ = $1() != $3(); }
  | op_matrix_exp TAFD_NOT_EQUAL op_matrix_exp		{ $$ = $1() != $3(); }
  | op_string_exp TAFD_NOT_EQUAL op_string_exp		{ $$ = $1() != $3(); }
  | TOK_FUNC_pass_if '(' op_flag_exp ',' op_flag_exp  ')'
        { $$ = pass_if ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_equal '(' op_flag_exp ',' op_flag_exp  ')'
        { $$ = pass_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_not_equal '(' op_flag_exp ',' op_flag_exp  ')'
        { $$ = pass_if_not_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if '(' op_flag_exp ',' op_flag_exp  ')'
        { $$ = reject_if ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_equal '(' op_flag_exp ',' op_flag_exp  ')'
        { $$ = reject_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_not_equal '(' op_flag_exp ',' op_flag_exp  ')'
        { $$ = reject_if_not_equal ( $3() , $5()  ); }
  | TOK_PROP_FLAG					{ $$ = $1; }
  | TOK_OP_FLAG						{ $$ = $1; }
  | flag_exp %prec OP_CONVERTION
        { $$ = solve::const_op( $1, msg_consultant(info) ); }

    // additional operators
  | op_flag_exp '?' op_flag_exp ':' op_flag_exp  
        { $$ = choose ( $1() , $3() , $5() ); }
;
op_scalar_exp:
    '(' op_scalar_exp ')'				{ $$ = $2(); }
  | TOK_FUNC__debug '(' op_scalar_exp  ')'
        { $$ = _debug ( $3()  ); }
  | TOK_FUNC_abs '(' op_scalar_exp  ')'
        { $$ = abs ( $3()  ); }
  | TOK_FUNC_abs '(' op_vector_exp  ')'
        { $$ = abs ( $3()  ); }
  | TOK_FUNC_abs2 '(' op_vector_exp  ')'
        { $$ = abs2 ( $3()  ); }
  | op_scalar_exp '+' op_scalar_exp			{ $$ = $1() + $3(); }
  | TOK_FUNC_ceil '(' op_scalar_exp  ')'
        { $$ = ceil ( $3()  ); }
  | TOK_FUNC_choose '(' op_flag_exp ',' op_scalar_exp ',' op_scalar_exp  ')'
        { $$ = choose ( $3() , $5() , $7()  ); }
  | op_scalar_exp '/' op_scalar_exp			{ $$ = $1() / $3(); }
  | TOK_FUNC_floor '(' op_scalar_exp  ')'
        { $$ = floor ( $3()  ); }
  | TOK_FUNC_get_element '(' op_matrix_exp ',' op_scalar_exp ',' 
      op_scalar_exp  ')'
        { $$ = get_element ( $3() , $5() , $7()  ); }
  | op_scalar_exp '*' op_scalar_exp			{ $$ = $1() * $3(); }
  | TOK_FUNC_dot '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = dot ( $3() , $5()  ); }
  | op_vector_exp '*' op_vector_exp			{ $$ = $1() * $3(); }
  | TOK_FUNC_vec_angle '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = vec_angle ( $3() , $5()  ); }
  | '-' op_scalar_exp %prec UMINUS			{ $$ = - $2(); }
  | TOK_FUNC_pass_if '(' op_scalar_exp ',' op_flag_exp  ')'
        { $$ = pass_if ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_equal '(' op_scalar_exp ',' op_scalar_exp  ')'
        { $$ = pass_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_not_equal '(' op_scalar_exp ',' op_scalar_exp  ')'
        { $$ = pass_if_not_equal ( $3() , $5()  ); }
  | TOK_FUNC_plus_minus '(' op_scalar_exp  ')'
        { $$ = plus_minus ( $3()  ); }
  | TOK_FUNC_reject_if '(' op_scalar_exp ',' op_flag_exp  ')'
        { $$ = reject_if ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_equal '(' op_scalar_exp ',' op_scalar_exp  ')'
        { $$ = reject_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_not_equal '(' op_scalar_exp ',' op_scalar_exp  ')'
        { $$ = reject_if_not_equal ( $3() , $5()  ); }
  | TOK_FUNC_round '(' op_scalar_exp  ')'
        { $$ = round ( $3()  ); }
  | TOK_FUNC_sqrt '(' op_scalar_exp  ')'
        { $$ = sqrt ( $3()  ); }
  | op_scalar_exp '-' op_scalar_exp			{ $$ = $1() - $3(); }
  | TOK_FUNC_to_angle '(' op_scalar_exp  ')'
        { $$ = to_angle ( $3()  ); }
  | TOK_FUNC_trunc '(' op_scalar_exp  ')'
        { $$ = trunc ( $3()  ); }
  | TOK_FUNC_get_element '(' op_vector_exp ',' op_scalar_exp  ')'
        { $$ = get_element ( $3() , $5()  ); }
  | TOK_PROP_SCALAR					{ $$ = $1; }
  | TOK_OP_SCALAR					{ $$ = $1; }
  | scalar_exp %prec OP_CONVERTION
        { $$ = solve::const_op( $1, msg_consultant(info) ); }

    // additional operators
  | '+' op_scalar_exp %prec UMINUS			{ $$ = $2; }
  | op_flag_exp '?' op_scalar_exp ':' op_scalar_exp  
        { $$ = choose ( $1() , $3() , $5() ); }
  | op_matrix_exp '[' op_scalar_exp ']' '[' op_scalar_exp  ']'
        { $$ = get_element ( $1() , $3() , $6()  ); }
  | op_vector_exp '[' op_scalar_exp ']' 
        { $$ = get_element ( $1() , $3() ); }
;
op_vector_exp:
    '(' op_vector_exp ')'				{ $$ = $2(); }
  | TOK_FUNC__debug '(' op_vector_exp  ')'
        { $$ = _debug ( $3()  ); }
  | op_vector_exp '+' op_vector_exp			{ $$ = $1() + $3(); }
  | TOK_FUNC_choose '(' op_flag_exp ',' op_vector_exp ',' op_vector_exp  ')'
        { $$ = choose ( $3() , $5() , $7()  ); }
  | TOK_FUNC_cross '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = cross ( $3() , $5()  ); }
  | op_vector_exp '/' op_scalar_exp			{ $$ = $1() / $3(); }
  | TOK_FUNC_get_rotate_component '(' op_matrix_exp  ')'
        { $$ = get_rotate_component ( $3()  ); }
  | TOK_FUNC_get_scale_component '(' op_matrix_exp  ')'
        { $$ = get_scale_component ( $3()  ); }
  | TOK_FUNC_get_translate_component '(' op_matrix_exp  ')'
        { $$ = get_translate_component ( $3()  ); }
  | op_scalar_exp '*' op_vector_exp			{ $$ = $1() * $3(); }
  | op_matrix_exp '*' op_vector_exp			{ $$ = $1() * $3(); }
  | '-' op_vector_exp %prec UMINUS			{ $$ = - $2(); }
  | TOK_FUNC_pass_if '(' op_vector_exp ',' op_flag_exp  ')'
        { $$ = pass_if ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_equal '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = pass_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_not_equal '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = pass_if_not_equal ( $3() , $5()  ); }
  | TOK_FUNC_plus_minus '(' op_vector_exp  ')'
        { $$ = plus_minus ( $3()  ); }
  | TOK_FUNC_reject_if '(' op_vector_exp ',' op_flag_exp  ')'
        { $$ = reject_if ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_equal '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = reject_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_not_equal '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = reject_if_not_equal ( $3() , $5()  ); }
  | op_vector_exp '-' op_vector_exp			{ $$ = $1() - $3(); }
  | TOK_FUNC_to_vector '(' op_scalar_exp ',' op_scalar_exp ',' 
	op_scalar_exp  ')'
        { $$ = to_vector ( $3() , $5() , $7()  ); }
  | TOK_FUNC_vec_mirror '(' op_vector_exp ',' op_scalar_exp  ')'
        { $$ = vec_mirror ( $3() , $5()  ); }
  | TOK_FUNC_vec_mirror '(' op_vector_exp  ')'
        { $$ = vec_mirror ( $3()  ); }
  | TOK_FUNC_vec_normalize '(' op_vector_exp  ')'
        { $$ = vec_normalize ( $3()  ); }
  | TOK_FUNC_vec_rotate_x '(' op_vector_exp ',' op_scalar_exp  ')'
        { $$ = vec_rotate_x ( $3() , $5()  ); }
  | TOK_FUNC_vec_rotate_y '(' op_vector_exp ',' op_scalar_exp  ')'
        { $$ = vec_rotate_y ( $3() , $5()  ); }
  | TOK_FUNC_vec_rotate_z '(' op_vector_exp ',' op_scalar_exp  ')'
        { $$ = vec_rotate_z ( $3() , $5()  ); }
  | TOK_FUNC_vec_scale '(' op_vector_exp ',' op_scalar_exp ',' op_scalar_exp
      ')'
        { $$ = vec_scale ( $3(), $5(), $7() ); }
  | TOK_FUNC_vec_to_rectangular '(' op_vector_exp  ')'
        { $$ = vec_to_rectangular ( $3()  ); }
  | TOK_FUNC_vec_to_spherical '(' op_vector_exp  ')'
        { $$ = vec_to_spherical ( $3()  ); }
  | TOK_FUNC_vec_translate '(' op_vector_exp ',' op_scalar_exp ',' 
      op_scalar_exp  ')'
        { $$ = vec_translate ( $3() , $5() , $7()  ); }
  | TOK_PROP_VECTOR					{ $$ = $1; }
  | TOK_OP_VECTOR					{ $$ = $1; }
  | vector_exp %prec OP_CONVERTION
        { $$ = solve::const_op( $1, msg_consultant(info) ); }

    // additional operators
  | '+' op_vector_exp %prec UMINUS			{ $$ = $2; }
  | op_flag_exp '?' op_vector_exp ':' op_vector_exp  
        { $$ = choose ( $1() , $3() , $5() ); }
  | '[' op_scalar_exp ',' op_scalar_exp ',' op_scalar_exp  ']'
        { $$ = to_vector ( $2() , $4() , $6()  ); }
;
op_matrix_exp:
    '(' op_matrix_exp ')'				{ $$ = $2(); }
  | TOK_FUNC__debug '(' op_matrix_exp  ')'
        { $$ = _debug ( $3()  ); }
  | op_matrix_exp '+' op_matrix_exp			{ $$ = $1() + $3(); }
  | TOK_FUNC_choose '(' op_flag_exp ',' op_matrix_exp ',' op_matrix_exp  ')'
        { $$ = choose ( $3() , $5() , $7()  ); }
  | op_matrix_exp '/' op_scalar_exp			{ $$ = $1() / $3(); }
  | TOK_FUNC_mat_inverse '(' op_matrix_exp  ')'
        { $$ = mat_inverse ( $3()  ); }
  | TOK_FUNC_mat_rotate '(' op_vector_exp  ')'
        { $$ = mat_rotate ( $3()  ); }
  | TOK_FUNC_mat_rotate_around '(' op_vector_exp ',' op_scalar_exp  ')'
        { $$ = mat_rotate_around ( $3() , $5()  ); }
  | TOK_FUNC_mat_rotate_pair_pair '(' op_vector_exp ',' op_vector_exp ',' 
      op_vector_exp ',' op_vector_exp  ')'
        { $$ = mat_rotate_pair_pair ( $3() , $5() , $7() , $9()  ); }
  | TOK_FUNC_mat_rotate_spherical_pair '(' op_vector_exp ',' op_vector_exp ','
      op_vector_exp  ')'
        { $$ = mat_rotate_spherical_pair ( $3() , $5() , $7()  ); }
  | TOK_FUNC_mat_rotate_vect_vect '(' op_vector_exp ',' op_vector_exp  ')'
        { $$ = mat_rotate_vect_vect ( $3() , $5()  ); }
  | TOK_FUNC_mat_rotate_vect_vect_up '(' op_vector_exp ',' op_vector_exp ','
      op_vector_exp  ')'
        { $$ = mat_rotate_vect_vect_up ( $3() , $5() , $7()  ); }
  | TOK_FUNC_mat_rotate_x '(' op_scalar_exp  ')'
        { $$ = mat_rotate_x ( $3()  ); }
  | TOK_FUNC_mat_rotate_y '(' op_scalar_exp  ')'
        { $$ = mat_rotate_y ( $3()  ); }
  | TOK_FUNC_mat_rotate_z '(' op_scalar_exp  ')'
        { $$ = mat_rotate_z ( $3()  ); }
  | TOK_FUNC_mat_scale '(' op_vector_exp  ')'
        { $$ = mat_scale ( $3()  ); }
  | TOK_FUNC_mat_scale_x '(' op_scalar_exp  ')'
        { $$ = mat_scale_x ( $3()  ); }
  | TOK_FUNC_mat_scale_y '(' op_scalar_exp  ')'
        { $$ = mat_scale_y ( $3()  ); }
  | TOK_FUNC_mat_scale_z '(' op_scalar_exp  ')'
        { $$ = mat_scale_z ( $3()  ); }
  | TOK_FUNC_mat_translate '(' op_vector_exp  ')'
        { $$ = mat_translate ( $3()  ); }
  | TOK_FUNC_mat_translate_x '(' op_scalar_exp  ')'
        { $$ = mat_translate_x ( $3()  ); }
  | TOK_FUNC_mat_translate_y '(' op_scalar_exp  ')'
        { $$ = mat_translate_y ( $3()  ); }
  | TOK_FUNC_mat_translate_z '(' op_scalar_exp  ')'
        { $$ = mat_translate_z ( $3()  ); }
  | op_scalar_exp '*' op_matrix_exp			{ $$ = $1() * $3(); }
  | op_matrix_exp '*' op_matrix_exp			{ $$ = $1() * $3(); }
  | '-' op_matrix_exp %prec UMINUS			{ $$ = - $2(); }
  | TOK_FUNC_pass_if '(' op_matrix_exp ',' op_flag_exp  ')'
        { $$ = pass_if ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_equal '(' op_matrix_exp ',' op_matrix_exp  ')'
        { $$ = pass_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_not_equal '(' op_matrix_exp ',' op_matrix_exp  ')'
        { $$ = pass_if_not_equal ( $3() , $5()  ); }
  | TOK_FUNC_plus_minus '(' op_matrix_exp  ')'
        { $$ = plus_minus ( $3()  ); }
  | TOK_FUNC_reject_if '(' op_matrix_exp ',' op_flag_exp  ')'
        { $$ = reject_if ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_equal '(' op_matrix_exp ',' op_matrix_exp  ')'
        { $$ = reject_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_not_equal '(' op_matrix_exp ',' op_matrix_exp  ')'
        { $$ = reject_if_not_equal ( $3() , $5()  ); }
  | op_matrix_exp '-' op_matrix_exp			{ $$ = $1() - $3(); }
  | TOK_PROP_MATRIX					{ $$ = $1; }
  | TOK_OP_MATRIX					{ $$ = $1; }
  | matrix_exp %prec OP_CONVERTION
        { $$ = solve::const_op( $1, msg_consultant(info) ); }

    // additional operators
  | '+' op_matrix_exp %prec UMINUS			{ $$ = $2; }
  | op_flag_exp '?' op_matrix_exp ':' op_matrix_exp  
        { $$ = choose ( $1() , $3() , $5() ); }
;
op_string_exp:
    '(' op_string_exp ')'				{ $$ = $2(); }
  | TOK_FUNC__debug '(' op_string_exp  ')'
        { $$ = _debug ( $3()  ); }
  | op_string_exp '+' op_string_exp			{ $$ = $1() + $3(); }
  | TOK_FUNC_choose '(' op_flag_exp ',' op_string_exp ',' op_string_exp  ')'
        { $$ = choose ( $3() , $5() , $7()  ); }
  | TOK_FUNC_pass_if '(' op_string_exp ',' op_flag_exp  ')'
        { $$ = pass_if ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_equal '(' op_string_exp ',' op_string_exp  ')'
        { $$ = pass_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_pass_if_not_equal '(' op_string_exp ',' op_string_exp  ')'
        { $$ = pass_if_not_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if '(' op_string_exp ',' op_flag_exp  ')'
        { $$ = reject_if ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_equal '(' op_string_exp ',' op_string_exp  ')'
        { $$ = reject_if_equal ( $3() , $5()  ); }
  | TOK_FUNC_reject_if_not_equal '(' op_string_exp ',' op_string_exp  ')'
        { $$ = reject_if_not_equal ( $3() , $5()  ); }
  | TOK_PROP_STRING					{ $$ = $1; }
  | TOK_OP_STRING					{ $$ = $1; }
  | string_exp %prec OP_CONVERTION
        { $$ = solve::const_op( $1, msg_consultant(info) ); }

    // additional operators
  | op_flag_exp '?' op_string_exp ':' op_string_exp  
        { $$ = choose ( $1() , $3() , $5() ); }
;

receive_dummy_exp:
    dummy_exp
  | dummy_exp error   { yyerr(info,2) << "operator or ';' expected"; 
			initialize_lexer(info); yyerrok; }
  | dummy_exp dummy_operator dummy_operator 
		      { yyerr(info,2)<< "operand expected instead of operator";
 			initialize_lexer(info); yyerrok; }
//  | error %prec lowest_precedence // might be reported by any_..._exp
//		      { yyerr(info,2) << "operand expected"; 
//			initialize_lexer(info); yyerrok; }
;
dummy_exp:
    dummy_exp dummy_operator dummy_exp %prec left_associated
  | dummy_exp '>' dummy_exp %prec left_associated
  | TOK_DUMMY_OPERAND '(' dummy_exp_comma_list ')'
  | '(' dummy_exp ')'
  | '!' dummy_exp %prec highest_precedence
  | '-' dummy_exp %prec highest_precedence
  | '+' dummy_exp %prec highest_precedence
  | '[' dummy_exp_comma_list ']' 
  | TOK_DUMMY_OPERAND
;
    /*
reduced_dummy_exp:
    reduced_dummy_exp dummy_operator reduced_dummy_exp %prec left_associated
  | TOK_DUMMY_OPERAND '(' dummy_exp_comma_list ')'
  | '(' dummy_exp ')'
  | '!' reduced_dummy_exp %prec highest_precedence
  | '-' reduced_dummy_exp %prec highest_precedence
  | '+' reduced_dummy_exp %prec highest_precedence
  | '[' reduced_dummy_exp_comma_list ']' 
  | TOK_DUMMY_OPERAND
;
    */
dummy_exp_comma_list:
    dummy_exp
  | dummy_exp_comma_list ',' dummy_exp
;
    /*
reduced_dummy_exp_comma_list:
    reduced_dummy_exp
  | reduced_dummy_exp_comma_list ',' reduced_dummy_exp
;
    */
dummy_operator:
    TOK_DUMMY_OPERATOR
  | '+' | '-' | '*' | '/' | '<' 
;
%%
    int parse_adl( proptree::Prop_Tree_Node *node, 
		   message::Message_Consultant *c, 
		   std::string filename, pass_type pass )
    {
      adlparser_info info(c);
      info.set_max_old_positions(MAX_OLD_POSITIONS);
      info.id_resolver = &info.res_property;
      
      info.open_file( filename );
      info.set_new_tree_node( node );
      info.set_pass(pass);
      int ret = yyparse( static_cast<void*>(&info) );
      info.close_file();
      
      return ret;
    }

  } // close namespace adlparser
} // close namespace anitmt
