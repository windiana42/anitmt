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

%{
#include <iostream>
#include <string>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>

#include "adlparser.hpp"

#include "parser_functions.hpp"

#define YYPARSE_PARAM info
#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)

// open namespaces
namespace anitmt
{
  namespace adlparser
  {

%}

%pure_parser

%token TOK_INVALID_ID TOK_ERROR
// multi character operators
%token TOK_IS_EQUAL TOK_NOT_EQUAL TOK_MORE_EQUAL TOK_LESS_EQUAL 
// functions
%token TOK_FUNC_SIN
// type tokens
%token <identifier()> TOK_IDENTIFIER
%token <flag()>   TOK_FLAG
%token <scalar()> TOK_SCALAR
%token <vector()> TOK_VECTOR
%token <matrix()> TOK_MATRIX
%token <string()> TOK_STRING
%token <op_flag()>   TOK_OP_FLAG
%token <op_scalar()> TOK_OP_SCALAR
%token <op_vector()> TOK_OP_VECTOR
%token <op_matrix()> TOK_OP_MATRIX
%token <op_string()> TOK_OP_STRING
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
%type <op_flag()>   op_flag_exp 
%type <op_scalar()> op_scalar_exp 
%type <op_vector()> op_vector_exp 
%type <op_matrix()> op_matrix_exp 
%type <op_string()> op_string_exp 
%type <tok()> any_flag_exp 
%type <tok()> any_scalar_exp 
%type <tok()> any_vector_exp 
%type <tok()> any_matrix_exp 
%type <tok()> any_string_exp 

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%nonassoc OP_CONVERTION 

%%
tree_node_block: /* empty */
	| tree_node_block statement
	;

statement: flag_statement ';'
	 | scalar_statement ';'
	 | vector_statement ';'
	 | matrix_statement ';'
	 | string_statement ';'
	 | child_declaration
	;
	
child_declaration: TOK_IDENTIFIER '{'    { change_current_child(info,$1); } 
		     tree_node_block '}' { change_to_parent(info); }
	| TOK_IDENTIFIER TOK_IDENTIFIER '{' { change_current_child(info,$1,$2);} 
	    tree_node_block '}' 	    { change_to_parent(info); }
	;

flag_statement: TOK_PROP_FLAG { resolve_references(info); } any_flag_exp 
 { 
   if( $3.get_type() == TOK_FLAG )
   {
     if( !$1.set_value( $3.flag() ) )
     {
       yyerr(info) << "error while setting property " << $1.get_name() << " to "
	     << $3.flag();
     }
   }
   else 
   {
     assert( $3.get_type() == TOK_OP_FLAG );
     solve::explicite_reference( $1, $3.op_flag() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 }
	;
any_flag_exp:     flag_exp {Token res;res.flag() = $1; $$ = res;}
		| op_flag_exp {Token res;res.set_op_flag($1); $$ = res;}
	;

scalar_statement: TOK_PROP_SCALAR { resolve_references(info); } any_scalar_exp 
 { 
   if( $3.get_type() == TOK_SCALAR )
   {
     if( !$1.set_value( $3.scalar() ) )
     {
       yyerr(info) << "error while setting property " << $1.get_name() << " to "
	     << $3.scalar();
     }
   }
   else 
   {
     assert( $3.get_type() == TOK_OP_SCALAR );
     solve::explicite_reference( $1, $3.op_scalar() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 }
	;
any_scalar_exp:      scalar_exp {Token res;res.scalar() = $1; $$ = res;}
		| op_scalar_exp {Token res;res.set_op_scalar($1); $$ = res;}
	;
 
vector_statement: TOK_PROP_VECTOR { resolve_references(info); } any_vector_exp 
 { 
   if( $3.get_type() == TOK_VECTOR )
   {
     if( !$1.set_value( $3.vector() ) )
     {
       yyerr(info) << "error while setting property " << $1.get_name() << " to "
	     << $3.vector();
     }
   }
   else 
   {
     assert( $3.get_type() == TOK_OP_VECTOR );
     solve::explicite_reference( $1, $3.op_vector() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 }
	;
any_vector_exp:      vector_exp {Token res;res.vector() = $1; $$ = res;}
		| op_vector_exp {Token res;res.set_op_vector($1); $$ = res;}
	;
 
matrix_statement: TOK_PROP_MATRIX { resolve_references(info); } any_matrix_exp 
 { 
   if( $3.get_type() == TOK_MATRIX )
   {
     if( !$1.set_value( $3.matrix() ) )
     {
       yyerr(info) << "error while setting property " << $1.get_name() << " to "
	     << $3.matrix();
     }
   }
   else 
   {
     assert( $3.get_type() == TOK_OP_MATRIX );
     solve::explicite_reference( $1, $3.op_matrix() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 }
	;
any_matrix_exp:      matrix_exp {Token res;res.matrix() = $1; $$ = res;}
		| op_matrix_exp {Token res;res.set_op_matrix($1); $$ = res;}
	;

string_statement: TOK_PROP_STRING { resolve_references(info); } any_string_exp 
 { 
   if( $3.get_type() == TOK_STRING )
   {
     if( !$1.set_value( $3.string() ) )
     {
       yyerr(info) << "error while setting property " << $1.get_name() << " to "
	     << $3.string();
     }
   }
   else 
   {
     assert( $3.get_type() == TOK_OP_STRING );
     solve::explicite_reference( $1, $3.op_string() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 }
	;
any_string_exp:      string_exp {Token res;res.string() = $1; $$ = res;}
		| op_string_exp {Token res;res.set_op_string($1); $$ = res;}
	;


flag_exp: TOK_FLAG {$$ = $1;}
	;
scalar_exp: scalar_exp '+' scalar_exp {$$ = $1 + $3;} 
	  | scalar_exp '-' scalar_exp {$$ = $1 - $3;}
	  | scalar_exp '*' scalar_exp {$$ = $1 * $3;}
  	  | scalar_exp '/' scalar_exp {$$ = $1 / $3;}
	  | '-' scalar_exp %prec UMINUS {$$ = -$2;}
	  | '+' scalar_exp %prec UMINUS {$$ = $2;}
	  | TOK_FUNC_SIN '(' scalar_exp ')' {$$ = sin($3);}
          | TOK_SCALAR {$$ = $1;}
	;

vector_exp: vector_exp '+' vector_exp {$$ = $1 + $3;} 
	  | '<' scalar_exp ',' scalar_exp ',' scalar_exp '>' 
		{ $$ = values::Vector( $2, $4, $6 ); }
	  | TOK_VECTOR {$$ = $1;}
	;

matrix_exp: TOK_MATRIX {$$ = $1;}
	;
string_exp: TOK_STRING {$$ = $1;}
	;


op_flag_exp: TOK_OP_FLAG {$$ = $1;}
	;

op_scalar_exp: op_scalar_exp '+' op_scalar_exp {$$ = $1 + $3;} 
	  | op_scalar_exp '-' op_scalar_exp {$$ = $1 - $3;}
	  | op_scalar_exp '*' op_scalar_exp {$$ = $1 * $3;}
  	  | op_scalar_exp '/' op_scalar_exp {$$ = $1 / $3;}
	  | '-' op_scalar_exp %prec UMINUS  {$$ = -$2;}
	  | '+' op_scalar_exp %prec UMINUS  {$$ = $2;}
          | scalar_exp %prec OP_CONVERTION {$$ = solve::const_op($1);}
          | TOK_OP_SCALAR {$$ = $1;}
	;

op_vector_exp: op_vector_exp '+' op_vector_exp {$$ = $1 + $3;} 
	  | '<' op_scalar_exp ',' op_scalar_exp ',' op_scalar_exp '>' 
	{ /*$$ = values::Vector( $2, $4, $6 ); not supported yet*/ 
	  yyerr(info) << "vector creation from operands not supported yet!";
	  assert(0); }
	  | TOK_OP_VECTOR {$$ = $1;}
	;

op_matrix_exp: TOK_OP_MATRIX {$$ = $1;}
	;
op_string_exp: TOK_OP_STRING {$$ = $1;}
	;

%%
    int parse_adl( Prop_Tree_Node *node, adlparser_info *info,
		   std::string filename )
    {
      if( filename != "" ) info->open_file( filename );
      info->set_new_tree_node( node );
      info->id_resolver = &info->res_property;
      return yyparse( static_cast<void*>(info) );
    }

  } // close namespace adlparser
} // close namespace anitmt
