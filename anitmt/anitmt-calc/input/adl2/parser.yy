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
#include <iostream>
#include <string>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "adlparser.hpp"

#include "parser_functions.hpp"	// help functions for the parser
				// including nessessary defines 

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
%expect 7		// expect 7 shift/reduce and 24 reduce/reduce conflicts

//********
// Tokens
//********

%token TOK_INVALID_ID TOK_ERROR 
// tokens in dummy expression (without semantic)
%token TOK_DUMMY_OPERAND TOK_DUMMY_OPERATOR
// multi character operators
%token TOK_IS_EQUAL TOK_NOT_EQUAL TOK_MORE_EQUAL TOK_LESS_EQUAL 
// functions
%token TOK_FUNC_SIN TOK_FUNC_SQRT
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

//****************
// precicion table
//****************

// special precedences for simplified cases
%nonassoc lowest_precedence

//********************
// normal precedences
// !! value to operand conversion must be last !!
%nonassoc OP_CONVERTION 

%left '+' '-'
%left '*' '/'
//%right '^'
%nonassoc UMINUS

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
  | receive_dummy_exp	{ $$; }
  | error		{ yyerr(info,2) << "invalid flag expression: operand expected"; 
			  yyerrok; $$; }
;

scalar_statement: 
    TOK_PROP_SCALAR	{ prop_declaration_start($1,info); } 
      any_scalar_exp	{ scalar_prop_declaration_finish($1,$3,info); }
;
any_scalar_exp:      
    scalar_exp 		{ $<scalar()>$ = $1; }
  | op_scalar_exp 	{ $<meta_op_scalar(info)>$ = $1(); }
  | receive_dummy_exp	{ $$; }
  | error		{ yyerr(info,2) << "invalid scalar expression: operand expected"; 
			  yyerrok; $$; }
;
 
vector_statement: 
    TOK_PROP_VECTOR	{ prop_declaration_start($1,info); } 
      any_vector_exp	{ vector_prop_declaration_finish($1,$3,info); }
;
any_vector_exp:      
    vector_exp		{ $<vector()>$ = $1; }
  | op_vector_exp	{ $<meta_op_vector(info)>$ = $1(); }
  | receive_dummy_exp	{ $$; }
  | error		{ yyerr(info,2) << "invalid vector expression: operand expected"; 
			  yyerrok; $$; }
;
 
matrix_statement: 
    TOK_PROP_MATRIX	{ prop_declaration_start($1,info); } 
      any_matrix_exp	{ matrix_prop_declaration_finish($1,$3,info); }
;
any_matrix_exp:      
    matrix_exp		{ $<matrix()>$ = $1; }
  | op_matrix_exp	{ $<meta_op_matrix(info)>$ = $1(); }
  | receive_dummy_exp	{ $$; }
  | error		{ yyerr(info,2) << "invalid matrix expression: operand expected"; 
			  yyerrok; $$; }
;
 
string_statement: 
    TOK_PROP_STRING	{ prop_declaration_start($1,info); } 
      any_string_exp	{ string_prop_declaration_finish($1,$3,info); }
;
any_string_exp:      
    string_exp		{ $<string()>$ = $1; }
  | op_string_exp	{ $<meta_op_string(info)>$ = $1(); }
  | receive_dummy_exp	{ $$; }
  | error		{ yyerr(info,2) << "invalid string expression: operand expected"; 
			  yyerrok; $$; }
;
 

flag_exp: 
    '(' TOK_FLAG ')'				{ $$ = $2; }
  | TOK_FLAG					{ $$ = $1; }
;

scalar_exp: 
    scalar_exp '+' scalar_exp			{ $$ = $1 + $3; } 
  | scalar_exp '-' scalar_exp			{ $$ = $1 - $3; }
  | scalar_exp '*' scalar_exp			{ $$ = $1 * $3; }
  | scalar_exp '/' scalar_exp			{ $$ = $1 / $3; }
  | '-' scalar_exp %prec UMINUS			{ $$ = -$2; }
  | '+' scalar_exp %prec UMINUS			{ $$ = $2; }
  | '(' scalar_exp ')'				{ $$ = $2; }
  | TOK_FUNC_SIN '(' scalar_exp ')'		{ $$ = sin($3); } 
  | TOK_FUNC_SQRT '(' scalar_exp ')'		{ $$ = sqrt($3); }
  | TOK_SCALAR					{ $$ = $1; }
;
  
vector_exp: 
    vector_exp '+' vector_exp			{ $$ = $1 + $3; } 
  | '<' scalar_exp ',' scalar_exp ',' scalar_exp '>' 
				{ $$ = values::Vector( $2, $4, $6 ); }
  | '(' vector_exp ')'				{ $$ = $2; }
  | TOK_VECTOR					{ $$ = $1; }
;

matrix_exp: 
    '(' TOK_MATRIX ')'				{ $$ = $2; }
  | TOK_MATRIX					{ $$ = $1; }
;

string_exp: 
    '(' TOK_STRING ')'				{ $$ = $2; }
  | TOK_STRING					{ $$ = $1; }
;

op_flag_exp: 
    '(' TOK_OP_FLAG ')'				{$$ = $2;}
  | TOK_PROP_FLAG				{$$ = $1;}
  | TOK_OP_FLAG					{$$ = $1;}

;

op_scalar_exp: 
    op_scalar_exp '+' op_scalar_exp		{ $$ = $1() + $3(); } 
  | op_scalar_exp '-' op_scalar_exp		{ $$ = $1() - $3(); }
  | op_scalar_exp '*' op_scalar_exp		{ $$ = $1() * $3(); }
  | op_scalar_exp '/' op_scalar_exp		{ $$ = $1() / $3(); }
  | '-' op_scalar_exp %prec UMINUS		{ $$ = -$2(); }
  | '+' op_scalar_exp %prec UMINUS		{ $$ = $2(); }
  | '(' op_scalar_exp ')'			{ $$ = $2(); }
  | TOK_FUNC_SQRT '(' op_scalar_exp ')'		{ $$ = sqrt($3()); }
  | scalar_exp %prec OP_CONVERTION // should be done if nothing else works
      {$$ = solve::const_op($1,msg_consultant(info));}
  | TOK_PROP_SCALAR				{ $$ = $1; } 
  | TOK_OP_SCALAR				{ $$ = $1; }
;

op_vector_exp: 
    op_vector_exp '+' op_vector_exp		{ $$ = $1() + $3(); } 
  | '<' op_scalar_exp ',' op_scalar_exp ',' op_scalar_exp '>' 
      { /*$$ = combine_Vector( $2, $4, $6 ); not supported yet*/ 
	yyerr(info) << "vector creation from operands not supported yet!";
	assert(0); }
  | '(' TOK_OP_VECTOR ')'			{ $$ = $2; }
  | TOK_PROP_VECTOR				{ $$ = $1; }
  | TOK_OP_VECTOR				{ $$ = $1; }
;

op_matrix_exp: 
    '(' TOK_OP_MATRIX ')'			{$$ = $2;}
  | TOK_PROP_MATRIX				{$$ = $1;}
  | TOK_OP_MATRIX				{$$ = $1;}
;

op_string_exp: 
    '(' TOK_OP_STRING ')'			{$$ = $2;}
  | TOK_PROP_STRING				{$$ = $1;}
  | TOK_OP_STRING				{$$ = $1;}
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
  | TOK_DUMMY_OPERAND '(' dummy_exp_comma_list ')'
  | '(' dummy_exp ')'
  | '-' TOK_DUMMY_OPERAND %prec highest_precedence
  | '+' TOK_DUMMY_OPERAND %prec highest_precedence
  | '<' dummy_exp_comma_list '>'
  | TOK_DUMMY_OPERAND
;
dummy_exp_comma_list:
    dummy_exp
  | dummy_exp_comma_list ',' dummy_exp
;
dummy_operator:
    TOK_DUMMY_OPERATOR
  | '+' | '-' | '*' | '/'
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
