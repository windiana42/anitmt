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

#include <message/message.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "afdbase.hpp"

#include "parser_functions.hpp"	// help functions for the parser
				// including nessessary defines 
#include "stdextend.hpp"


#define MAX_OLD_POSITIONS 	10

// open namespaces
namespace funcgen
{

%}

//*********
// Options
//*********
   
%pure_parser			// parser may be called recursive

//********
// Tokens
//********

// keywords
%token TAFD_include TAFD_declaration TAFD_header TAFD_avoid_recursion 
%token TAFD_priority_list TAFD_base_types TAFD_serial TAFD_type TAFD_abstract 
%token TAFD_node TAFD_provides TAFD_extends TAFD_properties TAFD_aliases 
%token TAFD_operands TAFD_common TAFD_constraints TAFD_solvers TAFD_actions 
%token TAFD_push TAFD_condition_push TAFD_default TAFD_contains TAFD_max1 
%token TAFD_min1 TAFD_provide TAFD_resulting TAFD_requires TAFD_this TAFD_prev 
%token TAFD_next TAFD_first TAFD_last TAFD_parent TAFD_child TAFD_first_child 
%token TAFD_last_child TAFD_start_param TAFD_end_param TAFD_true TAFD_false
%token TAFD_return TAFD_return_prop TAFD_return_fail TAFD_return_if_fail 
%token TAFD_operators TAFD_versions TAFD_BB_left TAFD_PT_CONCAT
// lexer error
%token TAFD_ERROR 
// multi character operators
%token TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%token TAFD_NS_CONCAT
// special tokens
%token TAFD_CODE
// type tokens
%token <string> TAFD_IDENTIFIER
%token <u.scalar> TAFD_SCALAR
%token <string> TAFD_QSTRING

%type <string>  operator_version_name
%type <string>  opt_provider_type
%type <string>  priority_level
%type <u.boolean> opt_abstract
%type <u.boolean> opt_max1
%type <u.boolean> opt_min1
%type <u.boolean> opt_serial
%type <string> opt_second_identifier
%type <string> opt_fail_bool_var
%type <u.exp> bool_op_expression
%type <u.exp> op_expression
%type <u.exp> op_expression_list
%type <string> Cxx_type_identifier_element
%type <string> Cxx_type_identifier
%type <string> Cxx_type_identifier_list
%type <string> Cxx_identifier
%type <string> Cxx_complex_identifier
%type <string> Cxx_expression
%type <string> Cxx_expression_list
//****************
// precicion table
//****************

//********************
// normal precedences
%left '?' ':'
%nonassoc '<' '>' TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%left '+' '-'
%left '*' '/'
%left '&' '|' '^' // !!! check this precedence !!!
%nonassoc UMINUS
%left '.' ID_CONCAT
%left NS_CONCAT

//****************
// Parser Rules
//****************
%%
statements: 
    /* nothing */
  | statements statement
;
statement:
    include_declaration
  | include_header
  | avoid_recursion
  | priority_list_declaration
  | base_types_declaration
  | type_declaration
  | node_declaration
  | operators_declaration
;
include_declaration:
    TAFD_include TAFD_declaration TAFD_QSTRING ';' 
					{ include_declaration( info, $3 ); }
;
include_header:
    TAFD_include TAFD_header TAFD_QSTRING ';'	{ include_header( info, $3 ); }
;
avoid_recursion:
    TAFD_avoid_recursion TAFD_IDENTIFIER ';'	
	{ if( avoid_recursion(info,$2) ) return 0; /* return if last file */ }
;
priority_list_declaration:
    TAFD_priority_list '{' priority_list_statements '}'
					{ priority_list_defined( info ); }
;
priority_list_statements: /* optional */
  | priority_list_statements priority_list_statement
;
priority_list_statement:
    TAFD_IDENTIFIER ';'			{ priority_label_add( info, $1 ); }
;
base_types_declaration: 
    TAFD_base_types '{' base_type_statements '}'
;
base_type_statements: /*optional*/
  | base_type_statements base_type_statement
;
base_type_statement:
    TAFD_IDENTIFIER '=' Cxx_type_identifier ';'			
      { declare_base_type( info, $1, $3 ); }
  | TAFD_IDENTIFIER '=' '{' 
      { declare_base_type_structure( info, $1 ); }
      base_type_structure '}' ';'
;
base_type_structure: 
    base_type_structure_element
  | base_type_structure ',' base_type_structure_element
;
base_type_structure_element:
    TAFD_IDENTIFIER TAFD_IDENTIFIER 	/*type name*/		
      { base_type_structure_element( info, $1, $2 ); }
;

type_declaration: 
    opt_serial TAFD_type TAFD_IDENTIFIER 
      { start_provider_type_declaration( info, $1, $3 ); }
      '{' provider_type_statements '}'	
;
opt_serial: /*optional*/		{ $$ = false; }
  | TAFD_serial				{ $$ = true; }
;
provider_type_statements: /*optional*/
  |  provider_type_statements provider_type_statement
;
provider_type_statement:
    TAFD_provides TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')' ';'	
      { add_provided_result_type( info, $2, $4 ); }
;
operators_declaration:
    TAFD_operators '{' operator_statements '}'
;
operator_statements: /*optional*/
  | operator_statements operator_statement
;
operator_statement:
    TAFD_IDENTIFIER TAFD_IDENTIFIER { start_operator_declaration(info,$1,$2); }
      '{' operator_body_statements '}'
;
operator_body_statements: /*optional*/
  | operator_body_statements operator_body_statement
;
operator_body_statement:
    operator_functions_declaration
  | operator_versions_declaration
;
operator_functions_declaration:
    TAFD_IDENTIFIER	      { start_operator_function_declaration(info,$1); }
      '(' operator_parameter_type_list ')' 
			      { start_operator_function_code(info); }
      '{' operator_function_code '}'
	      		      { finish_operator_function_code(info); }
;
operator_function_code:
			      { start_code_block(info); } 
    operator_code_statements 
			      { finish_code_block(info); }
;
operator_code_statements: /*optional*/
  | operator_code_statements operator_code_statement
;
operator_code_statement:
    TAFD_CODE
  | TAFD_BB_left              { yyerr(info,0) << "no \"[[\" allowed in this code"; 
				continue_code_mode(info); }
;
operator_versions_declaration:
    TAFD_versions '{' operator_version_statements '}'
;
operator_version_statements: /*optional*/
  | operator_version_statements operator_version_statement
;
operator_version_statement:
    TAFD_IDENTIFIER operator_version_name  
					{ start_operator_version(info,$1,$2); }
      '(' operator_version_type_list ')' ';'
					{ finish_operator_version(info); }
				
;
operator_version_name:
    TAFD_IDENTIFIER				{ $$ = $1; }
  | TAFD_IDENTIFIER '!'				{ check_id_operator(info,$1);
						  $$ = $1 + '!'; }
  | TAFD_IDENTIFIER '+'				{ check_id_operator(info,$1);
						  $$ = $1 + '+'; }
  | TAFD_IDENTIFIER '-'				{ check_id_operator(info,$1);
						  $$ = $1 + '-'; }
  | TAFD_IDENTIFIER '*'				{ check_id_operator(info,$1);
						  $$ = $1 + '*'; }
  | TAFD_IDENTIFIER '/'				{ check_id_operator(info,$1);
						  $$ = $1 + '/'; }
  | TAFD_IDENTIFIER '<'				{ check_id_operator(info,$1);
						  $$ = $1 + '<'; }
  | TAFD_IDENTIFIER '>'				{ check_id_operator(info,$1);
						  $$ = $1 + '>'; }
  | TAFD_IDENTIFIER TAFD_IS_EQUAL		{ check_id_operator(info,$1);
						  $$ = $1 + "=="; }
  | TAFD_IDENTIFIER TAFD_NOT_EQUAL		{ check_id_operator(info,$1);
						  $$ = $1 + "!="; }
  | TAFD_IDENTIFIER TAFD_MORE_EQUAL		{ check_id_operator(info,$1);
						  $$ = $1 + ">="; }
  | TAFD_IDENTIFIER TAFD_LESS_EQUAL		{ check_id_operator(info,$1);
						  $$ = $1 + "<="; }
    // ... when adding operators see [bool_]op_expression ...
;
operator_parameter_type_list: /*optional*/
  | TAFD_IDENTIFIER		{ add_operator_function_parameter(info,$1);}
  | operator_parameter_type_list ',' TAFD_IDENTIFIER	
				{ add_operator_function_parameter(info,$3);}
;
operator_version_type_list: /*optional*/
  | TAFD_IDENTIFIER		{ add_operator_version_parameter(info,$1);}
  | operator_version_type_list ',' TAFD_IDENTIFIER	
				{ add_operator_version_parameter(info,$3);}
;
node_declaration:
    opt_abstract TAFD_node TAFD_IDENTIFIER 
      { start_node_declaration( info, $1, $3 ); }
      opt_extends opt_provides '{' node_statements '}'
;
opt_abstract: /*optional*/	{$$ = false;}
  | TAFD_abstract		{$$ = true;}
;
opt_extends: /*optional*/
  | TAFD_extends extend_list
;
extend_list: 
    TAFD_IDENTIFIER			{ node_extends( info, $1 ); }
  | extend_list ',' TAFD_IDENTIFIER	{ node_extends( info, $3 ); }
;
opt_provides: /*optional*/
  | TAFD_provides provided_type_list
;
provided_type_list: provided_type
  | provided_type_list ',' provided_type
;
provided_type:
    TAFD_IDENTIFIER 			{ node_provides( info, $1 ); }
;
node_statements: /*optional*/
  | node_statements node_statement
;
node_statement:
    properties_declaration
  | aliases_declaration
  | operands_declaration
  | common_declaration
  | first_declaration
  | last_declaration
  | contains_declaration
  | provide_declaration
;
properties_declaration:
    TAFD_properties '{' property_types '}'
;
property_types: /*optional*/
  | property_types property_type
property_type:
    TAFD_type TAFD_IDENTIFIER 
      { node_start_property_type( info, $2 ); }
      '{' property_names '}'	
  | TAFD_IDENTIFIER TAFD_IDENTIFIER ';' 
      { node_declare_property( info, $1, $2 ); }
;
property_names: /*optional*/
  | property_names property_name
;
property_name:
    TAFD_IDENTIFIER ';'			{ node_declare_property( info, $1 ); }
;
aliases_declaration:
    TAFD_aliases '{' alias_statements '}'
;
alias_statements: /*optional*/
  | alias_statements alias_statement
;
alias_statement:
    TAFD_IDENTIFIER '=' TAFD_IDENTIFIER ';'	
      { node_declare_alias( info, $1, $3 ); }
;
operands_declaration:
    TAFD_operands '{' operand_types '}'
;
operand_types: /*optional*/
  | operand_types operand_type
operand_type:
    TAFD_type TAFD_IDENTIFIER 
      { node_start_operand_type( info, $2 ); }
      '{' operand_names '}'	
  | TAFD_IDENTIFIER TAFD_IDENTIFIER ';' 
      { node_declare_operand( info, $1, $2 ); }
;
operand_names: /*optional*/
  | operand_names operand_name
;
operand_name:
    TAFD_IDENTIFIER ';'			{ node_declare_operand( info, $1 ); }
;
common_declaration:
    TAFD_common 
      { node_start_common_declaration( info ); }
      '{' solve_system_statements '}'
;
first_declaration:
    TAFD_first opt_provider_type 
      { node_start_first_declaration( info, $2 ); }
      '{' solve_system_statements '}'
;
last_declaration:
    TAFD_last opt_provider_type 
      { node_start_last_declaration( info, $2 ); }
      '{' solve_system_statements '}'
;
opt_provider_type: /*optional*/		{$$ = "";}
  | TAFD_IDENTIFIER			{$$ = $1;}
;
solve_system_statements: /*optional*/
  | solve_system_statements solve_system_statement
;
solve_system_statement:
    constraints_declaration
  | solvers_declaration
  | actions_declaration
;
constraints_declaration:
    TAFD_constraints '{' constraint_statements '}'
;
constraint_statements: /*optional*/
  | constraint_statements constraint_statement
;
constraint_statement:
    bool_op_expression ';'		{ node_solve_constraint( info, $1 ); }
;
solvers_declaration:
    TAFD_solvers '{' solver_statements '}'
;
solver_statements: /*optional*/
  | solver_statements solver_statement
;
solver_statement:
    TAFD_IDENTIFIER 
      { node_start_solver( info, $1 ); }
      '(' solver_parameter_list ')' ';'	
      { node_finish_solver( info ); }
  | TAFD_IDENTIFIER '=' op_expression ';'
      { node_solve_expression( info,$1,$3 ); }
;
solver_parameter_list: 
    property_reference
      { node_add_solver_parameter( info ); }
  | solver_parameter_list ',' property_reference
      { node_add_solver_parameter( info ); }
;
actions_declaration:
    TAFD_actions '{' action_statements '}'
;
action_statements: /*optional*/
  | action_statements action_statement
;
action_statement:
    TAFD_default '(' priority_level ',' 
      { node_start_action( info, "default", $3 ); }
	             property_reference ',' 
      { node_add_action_parameter_ref( info ); }
		     Cxx_expression ')' ';'
      { node_add_action_parameter_str( info, $9 );
	node_finish_action( info ); }
  | TAFD_push '(' priority_level ',' 
      { node_start_action( info, "push", $3 ); }
	          property_reference ','  
      { node_add_action_parameter_ref( info ); }
		  property_reference ')' ';'  
      { node_add_action_parameter_ref( info ); // store second parameter
	node_finish_action( info ); }
  | TAFD_condition_push '(' priority_level ',' 
      { node_start_action( info, "condition_push", $3 ); }
	          property_reference ','  
      { node_add_action_parameter_ref( info ); }
		  property_reference ',' 
      { node_add_action_parameter_ref( info ); }// store second parameter
		  bool_op_expression ')' ';'
      { node_add_action_parameter_exp( info, $12 ); 
	node_finish_action( info ); }
/*| TAFD_IDENTIFIER '(' priority_level ',' 
      { node_start_action( info, $1, $3 ); }
      action_parameter_list ')' ';' 
      { node_finish_action( info ); } //!!! obsolete? */
;
priority_level:
    TAFD_IDENTIFIER		// only priority level labels allowed
;
action_parameter_list: property_reference
  | action_parameter_list ',' property_reference
      { node_add_action_parameter_ref( info ); }
;     

contains_declaration:
    TAFD_contains '{' contain_statements '}'
;
contain_statements: /*optional*/
  | contain_statements contain_statement
;
contain_statement:
    opt_max1 opt_min1 TAFD_IDENTIFIER ';'	
      { node_contains( info, $1, $2, $3 ); }
;
opt_max1: /*optional*/	{$$ = false; }
  | TAFD_max1		{$$ = true; }
;
opt_min1: /*optional*/	{$$ = false; }
  | TAFD_min1		{$$ = true; }
;
provide_declaration:
    TAFD_provide TAFD_IDENTIFIER 
      { node_start_provide( info, $2 ); }
      '{' result_code_definitions '}'	
;
result_code_definitions: /*optional*/
  | result_code_definitions result_code_definition
;
result_code_definition:
    TAFD_resulting TAFD_IDENTIFIER '(' TAFD_IDENTIFIER TAFD_IDENTIFIER ')'
      { node_start_result_code( info, $2, $4, $5 ); }
      opt_requires '{' result_code_block '}'
;
opt_requires: /*optional*/
  | TAFD_requires essentials_list
;
essentials_list: 
    essential
  | essentials_list ',' essential
;
essential:
    TAFD_IDENTIFIER
      { node_result_essential_prop( info, $1 ); }
  | TAFD_child '.' TAFD_IDENTIFIER
      { node_result_essential_child_result( info, $3 ); }
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'
      { node_result_essential_child_result( info, $3, $5, $7 ); }
  | TAFD_this '.' TAFD_IDENTIFIER
      { node_result_essential_result( info, $3 ); }
  | TAFD_this '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'
      { node_result_essential_result( info, "", $3, $5 ); }
  | TAFD_this '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'
      { node_result_essential_result( info, $3, $5, $7 ); }
;
result_code_block:
    { start_code_block(info); } code_statements { finish_code_block(info); }
;
code_statements: /*optional*/
  | code_statements code_statement
;
code_statement:
    TAFD_CODE	
// closing brackets may be single to avoid C++ code conflicts with tokens
  | TAFD_BB_left result_reference ']' ']' { continue_code_mode(info); }
  | TAFD_BB_left TAFD_return_prop	  { res_ref_start_return_prop(info); }
      result_property_reference ']' ']'   { res_ref_finish_return_prop(info);
					    continue_code_mode(info); }
  | TAFD_BB_left TAFD_return		  { res_ref_start_return(info); }
      Cxx_expression ']' ']'		  { write_code(info,$4);
					    res_ref_finish_return(info);
					    continue_code_mode(info); }
  | TAFD_BB_left TAFD_return		  { res_ref_start_return_res(info); }
      strict_result_function_reference ']' ']'   
					  { res_ref_finish_return_res(info);
					    continue_code_mode(info); }
  | TAFD_BB_left TAFD_return_fail ']' ']' { res_ref_return_fail(info);
					    continue_code_mode(info); }
  | TAFD_BB_left TAFD_return_if_fail ']' ']' 
					  { res_ref_return_if_fail(info);
					    continue_code_mode(info); }
  | TAFD_BB_left error ']' ']'		  { continue_code_mode(info); }
;
result_reference:
    result_property_reference
  | result_function_reference
;
result_property_reference:
    TAFD_IDENTIFIER		{ res_ref_property(info,$1); }
;
result_function_reference:
    TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '('
    TAFD_IDENTIFIER TAFD_IDENTIFIER ')' opt_fail_bool_var
				{ res_ref_child(info,$3,$5,$7,$8,$10); }
  | opt_this TAFD_IDENTIFIER opt_second_identifier '(' 
    TAFD_IDENTIFIER TAFD_IDENTIFIER ')' opt_fail_bool_var
				{ res_ref_this(info,$2,$3,$5,$6,$8); }
;
strict_result_function_reference: // for return statements
    TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '('
    TAFD_IDENTIFIER TAFD_IDENTIFIER ')'
				{ res_ref_ret_child(info,$3,$5,$7,$8); }
  | TAFD_this '.' TAFD_IDENTIFIER opt_second_identifier '(' 
    TAFD_IDENTIFIER TAFD_IDENTIFIER ')'
				{ res_ref_ret_this(info,$3,$4,$6,$7); }
;
opt_this: /*optional*/
  | TAFD_this '.'
;
opt_second_identifier: /*optional*/ { $$ = ""; }
  | '.' TAFD_IDENTIFIER 	    { $$ = $2; }
;
opt_fail_bool_var: /*optional*/     { $$ = ""; }
  | ',' TAFD_IDENTIFIER 	    { $$ = $2; }
;    
bool_op_expression:
    op_expression	 			{$$ = bool_expr($1);}
  | '!' bool_op_expression 			{$$ = bool_expr("!",$2);}
  | '(' bool_op_expression ')'		  	{$$ = bool_expr($2);}
  | op_expression TAFD_IS_EQUAL op_expression	{$$ = bool_expr($1,"==",$3);}
  | op_expression TAFD_NOT_EQUAL op_expression  {$$ = bool_expr($1,"!=",$3);}
  | op_expression TAFD_MORE_EQUAL op_expression {$$ = bool_expr($1,">=",$3);}
  | op_expression TAFD_LESS_EQUAL op_expression {$$ = bool_expr($1,"<=",$3);}
  | op_expression '>' op_expression		{$$ = bool_expr($1,">",$3);}
  | op_expression '<' op_expression		{$$ = bool_expr($1,"<",$3);}
    // ... when adding operators see operator_version_name ...
;
op_expression:
    property_reference			  {$$ = expr_from_ref(info);}
  | TAFD_SCALAR				  {$$ = expr_scalar($1);}
  | '(' op_expression ')'		  {$$ = expr($2);}
  | '!' op_expression 			  {$$ = expr("!",$2);}
  | '-' op_expression 			  {$$ = expr("-",$2);}
  | op_expression '+' op_expression	  {$$ = expr($1,"+",$3);}
  | op_expression '-' op_expression	  {$$ = expr($1,"-",$3);}
  | op_expression '*' op_expression	  {$$ = expr($1,"*",$3);}
  | op_expression '/' op_expression	  {$$ = expr($1,"/",$3);}
    // ... when adding operators see operator_version_name ...
  | TAFD_IDENTIFIER '(' ')'		  {$$ = expr_function($1);}
  | TAFD_IDENTIFIER '(' op_expression_list ')'
					  {$$ = expr_function($1,$3);}
;
op_expression_list: 
    op_expression				  {$$ = expr($1);}
  | op_expression_list ',' op_expression	  {$$ = expr($1,",",$3);}
;
Cxx_type_identifier_element:
    TAFD_IDENTIFIER				  {$$ = $1;}
  | TAFD_IDENTIFIER '<'Cxx_type_identifier_list'>'{$$ = $1 + "<" + $3 + ">";}
;
Cxx_type_identifier: /* C++ Type Identifier like values::Scalar */
    Cxx_type_identifier_element			  {$$ = $1;}
  | Cxx_type_identifier TAFD_NS_CONCAT Cxx_type_identifier_element 
      %prec NS_CONCAT				  {$$ = $1 + "::" + $3;}
;
Cxx_type_identifier_list: 
    Cxx_type_identifier				  {$$ = $1;}
  | Cxx_type_identifier_list ',' Cxx_type_identifier
						  {$$ = $1 + "," + $3;}
;
Cxx_identifier:
    TAFD_IDENTIFIER				  {$$ = $1;}
  | Cxx_type_identifier TAFD_NS_CONCAT TAFD_IDENTIFIER %prec NS_CONCAT
						  {$$ = $1 + "::" + $3;}
;
Cxx_complex_identifier: /* C++ Identifier like values::value.x->y.z */
    Cxx_identifier					{$$ = $1}
  | Cxx_complex_identifier '.' TAFD_IDENTIFIER		{$$ = $1 + "." + $3}
  | Cxx_complex_identifier TAFD_PT_CONCAT TAFD_IDENTIFIER %prec ID_CONCAT  
							{$$ = $1 + "->" + $3}
;
Cxx_expression:
    Cxx_complex_identifier		  {$$ = $1;}
  | TAFD_SCALAR				  {$$ = to_string($1);}
  | '(' Cxx_expression ')'		  {$$ = '(' + $2 + ')';}
  | Cxx_complex_identifier '(' ')'	  {$$ = $1 + '(' + ')';}
  | Cxx_complex_identifier '(' Cxx_expression_list ')'
					  {$$ = $1 + '(' + $3 + ')';}
  | Cxx_expression '+' Cxx_expression	  {$$ = $1 + "+" + $3;}
  | Cxx_expression '-' Cxx_expression	  {$$ = $1 + "-" + $3;}
  | Cxx_expression '*' Cxx_expression	  {$$ = $1 + "*" + $3;}
  | Cxx_expression '/' Cxx_expression	  {$$ = $1 + "/" + $3;}
  | Cxx_expression '|' Cxx_expression	  {$$ = $1 + "|" + $3;}
  | Cxx_expression '&' Cxx_expression	  {$$ = $1 + "&" + $3;}
  | Cxx_expression '^' Cxx_expression	  {$$ = $1 + "^" + $3;}
//!!! extend this list !!!
  | Cxx_expression '<' Cxx_expression	  {$$ = $1 + "<" + $3;}
  | Cxx_expression '>' Cxx_expression	  {$$ = $1 + ">" + $3;}
  | Cxx_expression TAFD_IS_EQUAL Cxx_expression   {$$ = $1 + "==" + $3;}
  | Cxx_expression TAFD_NOT_EQUAL Cxx_expression  {$$ = $1 + "!=" + $3;}
  | Cxx_expression TAFD_MORE_EQUAL Cxx_expression {$$ = $1 + ">=" + $3;}
  | Cxx_expression TAFD_LESS_EQUAL Cxx_expression {$$ = $1 + "<=" + $3;}
  | Cxx_expression '?' Cxx_expression ':' Cxx_expression
					  {$$ = $1 + "?" + $3 + ":" + $5;}
;
Cxx_expression_list: 
    Cxx_expression				  {$$ = $1;}
  | Cxx_expression_list ',' Cxx_expression	  {$$ = $1 + "," + $3;}
;
property_reference:
    /* property or operand */
    TAFD_IDENTIFIER			{ ref_prop_or_op(info, $1); } 
    /* property of other node */
  | node_reference '.' TAFD_IDENTIFIER	{ ref_node_prop(info,$3); } 
    /* start/end parameter of this node */
  | provider_type '.' TAFD_start_param	{ ref_start_param(info); } 
  | provider_type '.' TAFD_end_param  	{ ref_end_param(info); }
;
provider_type:
    TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'	
					{ ref_provider_type(info,$1,$3,$5); }
;
node_reference:
    local_node_identifier
  | node_identifier
  | node_reference '.' node_identifier
;
local_node_identifier:
    TAFD_IDENTIFIER '.' TAFD_prev	{ ref_node_local_prev(info,$1); }
  | TAFD_IDENTIFIER '.' TAFD_next	{ ref_node_local_next(info,$1); }
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_first	/* first child */
      { ref_node_local_child_first(info,$3); }
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_last	/* last child */
      { ref_node_local_child_last(info,$3); }
  | TAFD_child '.' TAFD_IDENTIFIER '[' TAFD_SCALAR ']'	/* n th child */
      { ref_node_local_child(info,$3,$5); }
;
node_identifier:
    TAFD_prev				{ ref_node_prev(info); }
  | TAFD_next				{ ref_node_next(info); }
  | TAFD_parent				{ ref_node_parent(info); }
  | TAFD_first_child			{ ref_node_first_child(info); }
  | TAFD_last_child			{ ref_node_last_child(info); }
  | TAFD_child '[' TAFD_SCALAR ']'	{ ref_node_child(info, $3); }
;

%%
  int parse_afd( AFD_Root *afd, message::Message_Consultant *c, 
		 std::string filename )
  {
    afd_info info(afd,c);
    info.set_max_old_positions(MAX_OLD_POSITIONS);
     
    if(!info.open_file( filename )) // did an error occur?
    {	
      info.msg.error() << "couldn't open input file " << filename;
      return -1;
    }
    int ret = yyparse( static_cast<void*>(&info) );
      
    return ret;
  }
} // close namespace afd
