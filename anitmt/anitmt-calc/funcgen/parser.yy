/*****************************************************************************/
/**   This file defines a parser/compiler for the AFD language              **/
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

// ***********
// Codeblock
// ***********

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

// include config switches like YYDEBUG
#include <config.h>

#define MAX_OLD_POSITIONS 	10

// open namespaces
namespace funcgen
{

%}

// *********
// Options
// *********
   
%pure_parser			// parser may be called recursive

// ********
// Tokens
// ********

// keywords
%token TAFD_include TAFD_declaration TAFD_header TAFD_avoid_recursion 
%token TAFD_priority_list TAFD_base_types TAFD_serial TAFD_type TAFD_abstract 
%token TAFD_node TAFD_provides TAFD_extends TAFD_properties TAFD_aliases 
%token TAFD_operands TAFD_common TAFD_constraints TAFD_solvers TAFD_actions 
%token TAFD_push TAFD_condition_push TAFD_default TAFD_contains TAFD_max1 
%token TAFD_min1 TAFD_provide TAFD_resulting TAFD_requires TAFD_this TAFD_prev 
%token TAFD_next TAFD_first TAFD_last TAFD_parent TAFD_child TAFD_first_child 
%token TAFD_last_child TAFD_start_param TAFD_end_param TAFD_true TAFD_false
%token TAFD_return TAFD_return_fail TAFD_return_if_fail 
%token TAFD_operators TAFD_versions TAFD_BB_left TAFD_PT_CONCAT
%token TAFD_declarations TAFD_init_operands TAFD_init_code TAFD_events 
%token TAFD_container TAFD_group TAFD_event TAFD_test_run
%token TAFD_final TAFD_reset TAFD_set TAFD_try TAFD_try_reject
%token TAFD_trial_failed TAFD_is_solved_in_try TAFD_is_just_solved 
%token TAFD_operand TAFD_property TAFD_solver TAFD_first_index TAFD_last_index
%token TAFD_for_each TAFD_element
// lexer error
%token TAFD_ERROR 
// multi character operators
%token TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%token TAFD_AND TAFD_OR TAFD_NS_CONCAT
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
%type <string> opt_fail_bool_var
%type <string> opt_identifier

%type <u.exp> op_expression
%type <u.exp> op_expression_list
%type <string> cxx_type_identifier
%type <string> cxx_type_identifier_base
%type <string> cxx_type_identifier_element
%type <string> cxx_type_identifier_list
%type <string> cxx_identifier
%type <string> cxx_complex_identifier
%type <string> cxx_expression
%type <string> opt_cxx_expression_list
%type <string> cxx_expression_list

%type <string> operand property prop_op provider_type base_type variable
%type <string> special_variable container_name operator solver solver_name

%type <string> operand_dot property_dot prop_op_dot
%type <string> base_type_dot container_name_dot
%type <string> variable_dot special_variable_dot
%type <string> provider_type_dot solver_dot
%type <string> solver_name_dot operator_dot

%type <string> operand_bracket property_bracket prop_op_bracket
%type <string> base_type_bracket container_name_bracket
%type <string> variable_bracket special_variable_bracket
%type <string> provider_type_bracket solver_bracket
%type <string> solver_name_bracket operator_bracket

%type <string> operand_sq_bracket property_sq_bracket prop_op_sq_bracket
%type <string> base_type_sq_bracket container_name_sq_bracket
%type <string> variable_sq_bracket special_variable_sq_bracket
%type <string> provider_type_sq_bracket solver_sq_bracket
%type <string> solver_name_sq_bracket operator_sq_bracket
// ****************
// precicion table
// ****************

// ********************
// normal precedences
%left '?' ':'
%left TAFD_AND TAFD_OR
%nonassoc '<' '>' TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%left '+' '-'
%left '*' '/'
%left '&' '|' '^' // !!! check this precedence !!!
%nonassoc UMINUS '!'
%left '.' ID_CONCAT
%left NS_CONCAT

// ****************
// Parser Rules
// ****************
%%

// ***********************************************************
// starting rules
// ***********************************************************

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
  | event_solvers_declaration
;

// ***********************************************************
// include rules
// ***********************************************************

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

// ***********************************************************
// priority list declaration rules
// ***********************************************************

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

// ***********************************************************
// base type declaration rules
// ***********************************************************

base_types_declaration: 
    TAFD_base_types '{' base_type_statements '}'
;
base_type_statements: /*optional*/
  | base_type_statements base_type_statement
;
base_type_statement:
    TAFD_IDENTIFIER '=' cxx_type_identifier ';'			
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
    base_type TAFD_IDENTIFIER 	/*type name*/		
      { base_type_structure_element( info, $1, $2 ); }
;

// ***********************************************************
// provider type declaration rules
// ***********************************************************

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
    TAFD_provides base_type '(' base_type ')' ';'	
      { add_provided_result_type( info, $2, $4 ); }
;

// ***********************************************************
// node declaration rules
// ***********************************************************

node_declaration:
    opt_abstract TAFD_node TAFD_IDENTIFIER 
        { start_node_declaration( info, $1, $3 ); }
      opt_extends opt_provides '{' node_statements '}'
        { finish_node_declaration(info); }
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
    provider_type 			{ node_provides( info, $1 ); }
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
;
property_type:
    TAFD_type base_type
      { node_start_property_type( info, $2 ); }
      '{' property_names '}'	
  | base_type TAFD_IDENTIFIER ';' 
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
    TAFD_IDENTIFIER '=' property ';'	
      { node_declare_alias( info, $1, $3 ); }
;
operands_declaration:
    TAFD_operands '{' operand_types '}'
;
operand_types: /*optional*/
  | operand_types operand_type
;
operand_type:
    TAFD_type base_type
      { start_operand_type( info, $2 ); }
      '{' operand_names '}'	
  | base_type TAFD_IDENTIFIER ';' 
      { declare_operand( info, $1, $2 ); }
;
operand_names: /*optional*/
  | operand_names operand_name
;
operand_name:
    TAFD_IDENTIFIER ';'			{ declare_operand( info, $1 ); }
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
  | provider_type			{$$ = $1;}
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
    op_expression ';'			{ node_solve_constraint( info, $1 ); }
;
solvers_declaration:
    TAFD_solvers '{' solver_statements '}'
;
solver_statements: /*optional*/
  | solver_statements solver_statement
;
solver_statement:
//  solver would be too restrictive here, as C++ coded solvers weren't allowed
    TAFD_IDENTIFIER opt_identifier
        { node_start_solver( info, $1, $2 ); }
      opt_solver_parameter_list ';'	
        { node_finish_solver( info ); }
  | TAFD_IDENTIFIER '=' op_expression ';'
        { node_solve_expression( info,$1,$3 ); }
;
opt_solver_parameter_list: /*optional*/
  | '(' solver_parameter_list ')'
  | '(' ')'
;
solver_parameter_list: 
    solver_parameter
  | solver_parameter_list ',' solver_parameter
;
solver_parameter:		// cxx expression compatible to property ref
    property_reference
      { node_add_solver_parameter( info ); }
  | TAFD_true
      { node_add_solver_const_parameter( info, true ); }
  | TAFD_false
      { node_add_solver_const_parameter( info, false ); }
  | TAFD_SCALAR
      { node_add_solver_const_parameter( info, $1 ); }
  | TAFD_QSTRING
      { node_add_solver_const_parameter( info, $1 ); }
  | cxx_identifier '(' cxx_expression_list ')'
      { node_add_solver_const_parameter( info, $1, $3 ); }
  | TAFD_container '.' container_name
      { solver_code_add_container_parameter( info, $3 ); }
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
		     cxx_expression ')' ';'
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
		  op_expression ')' ';'
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
/* action_parameter_list: property_reference
  | action_parameter_list ',' property_reference
      { node_add_action_parameter_ref( info ); }
; */

contains_declaration:
    TAFD_contains '{' contain_statements '}'
;
contain_statements: /*optional*/
  | contain_statements contain_statement
;
contain_statement:
    opt_max1 opt_min1 provider_type ';'	
      { node_contains( info, $1, $2, $3 ); }
;
opt_max1: /*optional*/	{$$ = false; }
  | TAFD_max1		{$$ = true; }
;
opt_min1: /*optional*/	{$$ = false; }
  | TAFD_min1		{$$ = true; }
;
provide_declaration:
    TAFD_provide provider_type
      { node_start_provide( info, $2 ); }
      '{' result_code_definitions '}'	
;
result_code_definitions: /*optional*/
  | result_code_definitions result_code_definition
;
result_code_definition:
    TAFD_resulting base_type '(' base_type TAFD_IDENTIFIER ')'
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
    prop_op
        { node_result_essential_prop( info, $1 ); }
  | TAFD_operand '.' operand
        { node_result_essential_prop( info, $3 ); }
  | TAFD_property '.' property
        { node_result_essential_prop( info, $3 ); }
  | TAFD_child '.' essential_children
  | TAFD_this '.' essential_this
  | solver_name_dot TAFD_IDENTIFIER 
        { node_result_essential_solver_function( info, $1, $2 ); }
  | TAFD_solver '.' solver_name_dot TAFD_IDENTIFIER 
        { node_result_essential_solver_function( info, $3, $4 ); }
;
essential_children:
    container_name
        { node_result_essential_child_result( info, $1 ); }
  | container_name '.' base_type '(' base_type ')'
        { node_result_essential_child_result( info, $1, $3, $5 ); }
;
essential_this:
    provider_type
        { node_result_essential_result( info, $1 ); }
  | base_type_bracket base_type ')'
        { node_result_essential_result( info, "", $1, $2 ); }
  | provider_type_dot base_type '(' base_type ')'
        { node_result_essential_result( info, $1, $2, $4 ); }
;
result_code_block:
      { start_code_block(info); } 
  result_code_statements 
      { finish_code_block(info); }
;
result_code_statements: /*optional*/
  | result_code_statements result_code_statement
;
result_code_statement:
    TAFD_CODE	
// closing brackets may be single to avoid C++ code conflicts with tokens
  | TAFD_BB_left result_code_special_command ']' ']' 
        { continue_code_mode(info); }
  | TAFD_BB_left error ']' ']'		  { continue_code_mode(info); }
;
result_code_special_command:
    prop_op
        { user_code_prop_op_try(info,$1); }
  | TAFD_operand '.' operand
        { user_code_prop_op_try(info,$3); }
  | TAFD_property '.' property
        { user_code_prop_op_try(info,$3); }

  | TAFD_child '.' provider_type '.' base_type 
      '(' base_type cxx_expression ')' opt_fail_bool_var
	{ res_ref_child(info,$3,$5,$7,$8,$10); }
  | TAFD_this '.' provider_type '.' base_type 
      '(' base_type cxx_expression ')' opt_fail_bool_var
	{ res_ref_this(info,$3,$5,$7,$8,$10); }
  | solver_name_dot TAFD_IDENTIFIER 
      '(' opt_cxx_expression_list ')' opt_fail_bool_var
	{ user_code_solver_function(info,$1,$2,$4,$6); }
  | TAFD_solver '.' solver_name_dot TAFD_IDENTIFIER 
      '(' opt_cxx_expression_list ')' opt_fail_bool_var
	{ user_code_solver_function(info,$3,$4,$6,$8); }

  | TAFD_return TAFD_operand '.' operand ';'
        { user_code_return_prop_try(info,$4); }
  | TAFD_return TAFD_property '.' property ';'
        { user_code_return_prop_try(info,$4); }
  | TAFD_return cxx_expression ';'
        { user_code_return(info,$2); }
  | TAFD_return TAFD_solver '.' solver_name '.' TAFD_IDENTIFIER
      '(' opt_cxx_expression_list ')' ';'
	{ user_code_return_solver_function(info,$4,$6,$8); }
  | TAFD_return TAFD_child '.' provider_type '.' base_type  '('
    base_type cxx_expression ')' ';'
	{ res_ref_ret_child(info,$4,$6,$8,$9); }
  | TAFD_return TAFD_this '.' provider_type '.' base_type '(' 
    base_type cxx_expression ')' ';'
				{ res_ref_ret_this(info,$4,$6,$8,$9); }
  | TAFD_return_fail ';'
        { user_code_return_fail(info); }
  | TAFD_return_if_fail ';'
        { user_code_return_if_fail(info); }
;
opt_fail_bool_var: /*optional*/     { $$ = ""; }
  | ',' TAFD_IDENTIFIER 	    { $$ = $2; }
;    

// ***********************************************************
// operator declaration rules
// ***********************************************************

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
      '{' cxx_code '}'
	      		      { finish_operator_function_code(info); }
;
operator_versions_declaration:
    TAFD_versions '{' operator_version_statements '}'
;
operator_version_statements: /*optional*/
  | operator_version_statements operator_version_statement
;
operator_version_statement:
    base_type operator_version_name  
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
  | TAFD_IDENTIFIER TAFD_AND			{ check_id_operator(info,$1);
						  $$ = $1 + "&&"; }
  | TAFD_IDENTIFIER TAFD_OR			{ check_id_operator(info,$1);
						  $$ = $1 + "||"; }
    // ... when adding operators see op_expression ...
;
operator_parameter_type_list: /*optional*/
  | TAFD_IDENTIFIER		{ add_operator_function_parameter(info,$1);}
  | operator_parameter_type_list ',' TAFD_IDENTIFIER
				{ add_operator_function_parameter(info,$3);}
;
operator_version_type_list: /*optional*/
  | base_type			{ add_operator_version_parameter(info,$1);}
  | operator_version_type_list ',' base_type
				{ add_operator_version_parameter(info,$3);}
;

// ***********************************************************
// solver declaration rules
// ***********************************************************

event_solvers_declaration:
    TAFD_solvers '{' event_solvers_statements '}'
;
event_solvers_statements: /*optional*/
  | event_solvers_statements event_solver_declaration    
;
event_solver_declaration:
    TAFD_IDENTIFIER '(' 
        { start_event_solver_parameters( info, $1 ); }
      event_solver_parameter_list ')' 
        { start_event_solver_declaration( info ); }
      '{' event_solver_statements '}' 
        { finish_event_solver_declaration( info ); }
;
event_solver_parameter_list: /*optional*/
  | event_solver_parameter					
  | event_solver_parameter_list ',' event_solver_parameter  
;
event_solver_parameter:
    base_type TAFD_IDENTIFIER
        { event_solver_parameter_operand( info, $1, $2 ); }
  | TAFD_container '.' provider_type TAFD_IDENTIFIER
        { event_solver_parameter_container( info, $3, $4 ); }
;
event_solver_statements: /*optional*/
  | event_solver_statements event_solver_statement
;
event_solver_statement:
    operands_declaration
  | event_solver_member_declaration
  | event_solver_init_operands_declaration
  | event_solver_init_code_declaration
  | event_solver_events_declaration
  | event_solver_provide_declaration
;

// ********
// operands

// same code for node operands declaration

// *************
// declarations

event_solver_member_declaration:
    TAFD_declarations '{' variable_declarations '}'
;
variable_declarations: /*optional*/
  | variable_declarations variable_declaration
;
variable_declaration:
    cxx_type_identifier		{ start_variable_declaration( info, $1 ); }
      variable_name_list ';'       
;
variable_name_list:
    variable_name_identifier
  | variable_name_list ',' variable_name_identifier
;
variable_name_identifier:
    TAFD_IDENTIFIER		{ variable_declaration_name( info, $1 ); }
;

// **************
// init operands

event_solver_init_operands_declaration:
    TAFD_init_operands '{' event_solver_init_operand_statements '}'
;
event_solver_init_operand_statements: /*optional*/
  | event_solver_init_operand_statements 
      event_solver_init_operand_statement
;
event_solver_init_operand_statement:
    TAFD_solvers '{' 
        { start_event_solver_init_solvers_block(info); } 
      solver_statements 
        { finish_event_solver_init_solvers_block(info); } 
      '}'
  | TAFD_constraints '{' 
        { start_event_solver_init_constraints_block(info); } 
      constraint_statements 
        { finish_event_solver_init_constraints_block(info); } 
      '}'
;
  

// **********
// init code

event_solver_init_code_declaration:
    TAFD_init_code			{ start_event_solver_init_code(info); }
      '{' cxx_code '}'			{ finish_event_solver_init_code(info);}
;

// ********
// events

event_solver_events_declaration:
    TAFD_events '{' event_solver_events_statements '}'
;
event_solver_events_statements: /*optional*/
  | event_solver_events_statements event_solver_events_statement
;
event_solver_events_statement:
    event_solver_event_block
      { finish_event_solver_event_group(info); /* close single event group */ }
  | TAFD_group opt_identifier '{'
      { start_event_solver_event_group(info,$2); }
    event_solver_event_group_statements '}'
      { finish_event_solver_event_group(info); }
;
event_solver_event_group_statements: /*optional*/
  | event_solver_event_group_statements event_solver_event_group_statement
;
event_solver_event_group_statement:
    event_solver_event_block
  | event_solver_event_group_reset_block
;
event_solver_event_group_reset_block: 
    TAFD_reset				{ start_event_group_reset_code(info); }
      '{' cxx_code '}'			{ finish_event_group_reset_code(info);}
;
event_solver_event_block:
    TAFD_event opt_identifier TAFD_requires 
        { start_event_solver_event(info, $2); }
    event_solver_event_condition 
    '{' event_solver_event_statement '}'
;
event_solver_event_condition:
    event_solver_event_condition_element
  | event_solver_event_condition ',' 
      event_solver_event_condition_element
;
event_solver_event_condition_element:
    opt_prefix_operand operand
        { event_condition(info,$2); }
  | TAFD_container '.' event_solver_event_container_condition
  | TAFD_event '.' TAFD_IDENTIFIER
        { event_condition_event(info, $3); }
  | TAFD_group '.' TAFD_IDENTIFIER
        { event_condition_event_group(info, $3); }
  | TAFD_solver '.' solver_name '.' TAFD_IDENTIFIER
        { event_condition_solver(info, $3, $5); }
;
event_solver_event_container_condition:
    container_name
        { event_condition_container(info, $1); }
  | container_name_dot base_type '(' base_type ')'
        { event_condition_container_function(info, $1, $2, $4); }
;
event_solver_event_statement:
    event_solver_event_test_run_block
    opt_event_solver_event_final_block
    opt_event_solver_event_reset_block
;
opt_event_solver_event_final_block: /*optional*/
  | TAFD_final				{ start_event_final_code(info); }
      '{' cxx_code '}'			{ finish_event_final_code(info); }
;
opt_event_solver_event_reset_block: /*optional*/
  | TAFD_reset				{ start_event_reset_code(info); }
      '{' cxx_code '}'			{ finish_event_reset_code(info); }
;
event_solver_event_test_run_block:
    TAFD_test_run			{ start_event_test_run_code(info); }
      '{' test_run_code_block '}'	{ finish_event_test_run_code(info); }
;
test_run_code_block:
    { start_code_block(info); } 
  test_run_code_statements 
    { finish_code_block(info); }
;
test_run_code_statements: /*optional*/
  | test_run_code_statements test_run_code_statement
;
test_run_code_statement:
    TAFD_CODE	
// closing brackets may be single to avoid C++ code conflicts with tokens
  | TAFD_BB_left test_run_special_command ']' ']' { continue_code_mode(info); }
;
test_run_special_command:
    TAFD_set prop_op '=' cxx_expression ';'	
        { event_solver_code_set(info,$2,$4); }
  | TAFD_try prop_op '=' cxx_expression ';'
        { event_solver_code_try(info,$2,$4); }
  | TAFD_try_reject operand_container_solver_comma_list ';'
        { event_solver_code_try_reject(info); }
  | TAFD_is_solved_in_try '(' prop_op ')'	
        { event_solver_code_is_solved_in_try(info,$3); }
  | TAFD_is_just_solved   '(' prop_op ')'	
        { event_solver_code_is_just_solved(info,$3); }

  | operand
        { user_code_prop_op_try(info,$1); }
  | TAFD_operand '.' operand
        { user_code_prop_op_try(info,$3); }
  | TAFD_solver '.' solver_name '.' TAFD_IDENTIFIER 
      '(' opt_cxx_expression_list ')' opt_fail_bool_var
	{ user_code_solver_function(info,$3,$5,$7,$9); }
  | test_run_container_special_command
  | TAFD_container '.' test_run_container_special_command

  | TAFD_for_each TAFD_IDENTIFIER in TAFD_IDENTIFIER
        { user_code_for_each_container_element(info,$2,$4); }
  | TAFD_element '.' special_variable '.' base_type
      '(' base_type cxx_expression ')' opt_fail_bool_var
	{ user_code_element_function(info,$3,$5,$7,$8,$10); }
;
test_run_container_special_command:
    container_name_dot 
        { user_code_store_container_name(info,$1); }
      test_run_container_dot_special_command
  | container_name_sq_bracket TAFD_SCALAR ']' '.' base_type
      '(' base_type cxx_expression ')' opt_fail_bool_var
	{ user_code_container_element_function(info,$1,$2,$5,$7,$8,$10); }
;
test_run_container_dot_special_command:
  // serial container functions
    base_type '(' base_type cxx_expression ')' opt_fail_bool_var
	{ user_code_container_function(info,$1,$3,$4,$6); }
  // parallel container (loop) functions
  | TAFD_first_index
	{ user_code_container_first_index(info); }
  | TAFD_last_index 
	{ user_code_container_last_index(info); }
;
// ********
// provides

event_solver_provide_declaration:
    TAFD_provide '{' event_solver_provide_statements '}'
;
event_solver_provide_statements: /*optional*/
  | event_solver_provide_statements event_solver_function_declaration
;
event_solver_function_declaration:
    cxx_type_identifier TAFD_IDENTIFIER
				{ start_event_solver_function(info,$1,$2); }
      '(' cxx_function_parameters ')' 
      opt_event_solver_function_requirements
      '{' event_solver_function_code_block '}' 
				{ finish_event_solver_function(info);}
;
cxx_function_parameters: /*optional*/
  | cxx_function_parameter
  | cxx_function_parameters ',' cxx_function_parameter
;
cxx_function_parameter:
    cxx_type_identifier TAFD_IDENTIFIER	{ function_parameter(info,$1,$2); }
;
opt_event_solver_function_requirements: /*optional*/
  | TAFD_requires event_solver_function_require_statements
;
event_solver_function_require_statements:
    event_solver_function_require_statement
  | event_solver_function_require_statements ',' 
      event_solver_function_require_statement
;
event_solver_function_require_statement:
    operand
        { event_solver_require_operand(info,$1); }
  | TAFD_operand '.' operand
        { event_solver_require_operand(info,$3); }
  | TAFD_this '.' TAFD_IDENTIFIER	
	{ event_solver_require_function(info,$3); }
  | solver_name_dot TAFD_IDENTIFIER	
        { event_solver_require_solver_func(info,$1,$2); }
  | TAFD_solver '.' solver_name_dot TAFD_IDENTIFIER	
        { event_solver_require_solver_func(info,$3,$4); }
  | TAFD_event '.' TAFD_IDENTIFIER	
	{ event_solver_require_event(info,$3); }
  | TAFD_group '.' TAFD_IDENTIFIER
	{ event_solver_require_event_group(info,$3); }
  | TAFD_container '.' event_solver_function_require_container
;
event_solver_function_require_container:
    container_name
        { event_solver_require_container(info, $1); }
  | container_name_dot base_type '(' base_type ')'
        { event_solver_require_container_function(info, $1, $2, $4); }
;
event_solver_function_code_block:
    { start_code_block(info); } 
  event_solver_function_code_statements 
    { finish_code_block(info); }
;
event_solver_function_code_statements: /*optional*/
  | event_solver_function_code_statements event_solver_function_code_statement
;
event_solver_function_code_statement:
    TAFD_CODE	
// closing brackets may be single to avoid C++ code conflicts with tokens
  | TAFD_BB_left event_solver_function_special_command ']' ']' 
    { continue_code_mode(info); }
;
event_solver_function_special_command:
    operand
        { user_code_prop_op_try(info,$1); }
  | TAFD_operand '.' operand
        { user_code_prop_op_try(info,$3); }
  | TAFD_solver '.' solver_name '.' TAFD_IDENTIFIER 
      '(' opt_cxx_expression_list ')' opt_fail_bool_var
	{ user_code_solver_function(info,$3,$5,$7,$9); }
  | test_run_container_special_command
  | TAFD_container '.' test_run_container_special_command

  | TAFD_for_each TAFD_IDENTIFIER in TAFD_IDENTIFIER
        { user_code_for_each_container_element(info,$2,$4); }
  | TAFD_element '.' special_variable '.' base_type
      '(' base_type cxx_expression ')' opt_fail_bool_var
	{ user_code_element_function(info,$3,$5,$7,$8,$10); }

  | TAFD_return TAFD_operand '.' TAFD_IDENTIFIER ';'
        { user_code_return_prop_try(info,$4); }
  | TAFD_return cxx_expression ';'
        { user_code_return(info,$2); }
  | TAFD_return TAFD_solver '.' solver_name '.' TAFD_IDENTIFIER
      '(' opt_cxx_expression_list ')' ';'
	{ user_code_return_solver_function(info,$4,$6,$8); }
  | TAFD_return_fail ';'
        { user_code_return_fail(info); }
  | TAFD_return_if_fail ';'
        { user_code_return_if_fail(info); }
;

// ***********************************************************
// common rules
// ***********************************************************

in:
    TAFD_IDENTIFIER		{ require_identifier(info,$1,"in"); }
;

opt_identifier: /*optional*/	{ $$=""; }
  | TAFD_IDENTIFIER		
;

/*
opt_identifier_comma_list: /*optional* /
  | identifier_comma_list
;

operand_parameter_list: /*optional* /
  | operand_parameter					
  | operand_parameter_list ',' operand_parameter  
;
operand_parameter:
    TAFD_IDENTIFIER TAFD_IDENTIFIER { declare_operand( info, $1, $2 ); }
;

identifier_comma_list: /*optional/
  | TAFD_IDENTIFIER			      {common_add_identifier(info,$1);}
  | identifier_comma_list ',' TAFD_IDENTIFIER {common_add_identifier(info,$3);}
;
*/

//!!! todo !!!
operand_container_solver_comma_list: /*optional*/
  | TAFD_IDENTIFIER			      {common_add_identifier(info,$1);}
  | operand_container_solver_comma_list ',' TAFD_IDENTIFIER 
					      {common_add_identifier(info,$3);}
;

opt_prefix_operand: /*optional*/
  | TAFD_operand '.'
;

// ***********************************************************
// context checked identifiers
// ***********************************************************

operand:
    TAFD_IDENTIFIER		{ require_operand(info,$1); $$=$1; }
;

property:
    TAFD_IDENTIFIER		{ require_property(info,$1); $$=$1; }
;

prop_op:
    TAFD_IDENTIFIER		{ require_prop_op(info,$1); $$=$1; }
;

provider_type:
    TAFD_IDENTIFIER		{ require_provider_type(info,$1); $$=$1; }
;

container_name:
    TAFD_IDENTIFIER		{ require_container_name(info,$1); $$=$1; }
;

variable:
    TAFD_IDENTIFIER		{ require_variable(info,$1); $$=$1; }
;

special_variable:
    TAFD_IDENTIFIER		{ require_special_variable(info,$1); $$=$1; }
;

base_type:
    TAFD_IDENTIFIER		{ require_base_type(info,$1); $$=$1; }
;

operator:
    TAFD_IDENTIFIER		{ require_operator(info,$1); $$=$1; }
;

solver:
    TAFD_IDENTIFIER		{ require_solver(info,$1); $$=$1; }
;

solver_name:
    TAFD_IDENTIFIER		{ require_solver_name(info,$1); $$=$1; }
;

// identifier with '.' following 

operand_dot:
    TAFD_IDENTIFIER '.'		{ require_operand(info,$1); $$=$1; }
;

property_dot:
    TAFD_IDENTIFIER '.'		{ require_property(info,$1); $$=$1; }
;

prop_op_dot:
    TAFD_IDENTIFIER '.'		{ require_prop_op(info,$1); $$=$1; }
;

provider_type_dot:
    TAFD_IDENTIFIER '.'		{ require_provider_type(info,$1); $$=$1; }
;

container_name_dot:
    TAFD_IDENTIFIER '.'		{ require_container_name(info,$1); $$=$1; }
;

variable_dot:
    TAFD_IDENTIFIER '.'		{ require_variable(info,$1); $$=$1; }
;

special_variable_dot:
    TAFD_IDENTIFIER '.'		{ require_special_variable(info,$1); $$=$1; }
;

base_type_dot:
    TAFD_IDENTIFIER '.'		{ require_base_type(info,$1); $$=$1; }
;

operator_dot:
    TAFD_IDENTIFIER '.'		{ require_operator(info,$1); $$=$1; }
;

solver_dot:
    TAFD_IDENTIFIER '.'		{ require_solver(info,$1); $$=$1; }
;

solver_name_dot:
    TAFD_IDENTIFIER '.'		{ require_solver_name(info,$1); $$=$1; }
;

// identifier with '(' following 

operand_bracket:
    TAFD_IDENTIFIER '('		{ require_operand(info,$1); $$=$1; }
;

property_bracket:
    TAFD_IDENTIFIER '('		{ require_property(info,$1); $$=$1; }
;

prop_op_bracket:
    TAFD_IDENTIFIER '('		{ require_prop_op(info,$1); $$=$1; }
;

provider_type_bracket:
    TAFD_IDENTIFIER '('		{ require_provider_type(info,$1); $$=$1; }
;

container_name_bracket:
    TAFD_IDENTIFIER '('		{ require_container_name(info,$1); $$=$1; }
;

variable_bracket:
    TAFD_IDENTIFIER '('		{ require_variable(info,$1); $$=$1; }
;

special_variable_bracket:
    TAFD_IDENTIFIER '('		{ require_special_variable(info,$1); $$=$1; }
;

base_type_bracket:
    TAFD_IDENTIFIER '('		{ require_base_type(info,$1); $$=$1; }
;

operator_bracket:
    TAFD_IDENTIFIER '('		{ require_operator(info,$1); $$=$1; }
;

solver_bracket:
    TAFD_IDENTIFIER '('		{ require_solver(info,$1); $$=$1; }
;

solver_name_bracket:
    TAFD_IDENTIFIER '('		{ require_solver_name(info,$1); $$=$1; }
;

// identifier with '[' following 

operand_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_operand(info,$1); $$=$1; }
;

property_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_property(info,$1); $$=$1; }
;

prop_op_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_prop_op(info,$1); $$=$1; }
;

provider_type_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_provider_type(info,$1); $$=$1; }
;

container_name_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_container_name(info,$1); $$=$1; }
;

variable_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_variable(info,$1); $$=$1; }
;

special_variable_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_special_variable(info,$1); $$=$1; }
;

base_type_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_base_type(info,$1); $$=$1; }
;

operator_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_operator(info,$1); $$=$1; }
;

solver_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_solver(info,$1); $$=$1; }
;

solver_name_sq_bracket:
    TAFD_IDENTIFIER '['		{ require_solver_name(info,$1); $$=$1; }
;


// ***********************************************************
// expression rules
// ***********************************************************

op_expression:
    property_reference			  	{$$ = expr_from_ref(info);}
  | TAFD_SCALAR				  	{$$ = expr_scalar(info,$1);}
  | TAFD_QSTRING			  	{$$ = expr_string(info,$1);}
  | '(' op_expression ')'		  	{$$ = expr($2);}
  | '-' op_expression 			  	{$$ = expr("-",$2);}
  | op_expression '+' op_expression	  	{$$ = expr($1,"+",$3);}
  | op_expression '-' op_expression	  	{$$ = expr($1,"-",$3);}
  | op_expression '*' op_expression	  	{$$ = expr($1,"*",$3);}
  | op_expression '/' op_expression	  	{$$ = expr($1,"/",$3);}
  | TAFD_IDENTIFIER '(' ')'		  	{$$ = expr_function($1);}
  | TAFD_IDENTIFIER '(' op_expression_list ')'	{$$ = expr_function($1,$3);}
    // boolean results
  | TAFD_true				  	{$$ = expr_bool(info,true);}
  | TAFD_false					{$$ = expr_bool(info,false);}
  | '!' op_expression				{$$ = expr("!",$2);}
  | op_expression '>' op_expression		{$$ = bool_expr($1,">",$3);}
  | op_expression '<' op_expression		{$$ = bool_expr($1,"<",$3);}
  | op_expression TAFD_IS_EQUAL op_expression	{$$ = bool_expr($1,"==",$3);}
  | op_expression TAFD_NOT_EQUAL op_expression  {$$ = bool_expr($1,"!=",$3);}
  | op_expression TAFD_MORE_EQUAL op_expression {$$ = bool_expr($1,">=",$3);}
  | op_expression TAFD_LESS_EQUAL op_expression {$$ = bool_expr($1,"<=",$3);}
  | op_expression TAFD_AND op_expression	{$$ = bool_expr($1,"&&",$3);}
  | op_expression TAFD_OR op_expression		{$$ = bool_expr($1,"||",$3);}
    //!!! this needs precedences set up correctly
  | op_expression '?' op_expression ':' op_expression
	{$$ = expr_function("choose",expr(expr($1,",",$3),",",$5));}
    // ... when adding operators see operator_version_name ...
;
op_expression_list: 
    op_expression				  {$$ = expr($1);}
  | op_expression_list ',' op_expression	  {$$ = expr($1,",",$3);}
;
cxx_type_identifier: /* C++ Type Identifier like values::Scalar */
    cxx_type_identifier_base			  {$$ = $1;}
  | cxx_type_identifier_base '*'		  {$$ = $1 + "*";}
;
cxx_type_identifier_base:
    cxx_type_identifier_element			  {$$ = $1;}
  | cxx_type_identifier_base TAFD_NS_CONCAT cxx_type_identifier_element 
      %prec NS_CONCAT				  {$$ = $1 + "::" + $3;}
;
cxx_type_identifier_element:
    TAFD_IDENTIFIER				  {$$ = $1;}
  | TAFD_IDENTIFIER '<' cxx_type_identifier_list '>'
						  {$$ = $1 + "<" + $3 + ">";}
;
cxx_type_identifier_list: 
    cxx_type_identifier				  {$$ = $1;}
  | cxx_type_identifier_list ',' cxx_type_identifier
						  {$$ = $1 + "," + $3;}
;
cxx_identifier:
    cxx_type_identifier_base	// types can't be distinguished from variables
;
cxx_complex_identifier: /* C++ Identifier like values::value.x->y.z */
    cxx_identifier					{$$ = $1;}
  | cxx_complex_identifier '.' cxx_identifier		{$$ = $1 + "." + $3;}
  | cxx_complex_identifier TAFD_PT_CONCAT cxx_identifier %prec ID_CONCAT  
							{$$ = $1 + "->" + $3;}
;
cxx_expression:
    cxx_complex_identifier		  {$$ = $1;}
  | TAFD_SCALAR				  {$$ = to_string($1);}
  | TAFD_true				  {$$ = "true";}
  | TAFD_false				  {$$ = "false";}
  | '(' cxx_expression ')'		  {$$ = '(' + $2 + ')';}
  | cxx_complex_identifier '(' ')'	  {$$ = $1 + '(' + ')';}
  | cxx_complex_identifier '(' cxx_expression_list ')'
					  {$$ = $1 + '(' + $3 + ')';}
  | cxx_expression '+' cxx_expression	  {$$ = $1 + "+" + $3;}
  | cxx_expression '-' cxx_expression	  {$$ = $1 + "-" + $3;}
  | cxx_expression '*' cxx_expression	  {$$ = $1 + "*" + $3;}
  | cxx_expression '/' cxx_expression	  {$$ = $1 + "/" + $3;}
  | cxx_expression '|' cxx_expression	  {$$ = $1 + "|" + $3;}
  | cxx_expression '&' cxx_expression	  {$$ = $1 + "&" + $3;}
  | cxx_expression '^' cxx_expression	  {$$ = $1 + "^" + $3;}
//!!! extend this list !!!
  | cxx_expression '<' cxx_expression	  {$$ = $1 + "<" + $3;}
  | cxx_expression '>' cxx_expression	  {$$ = $1 + ">" + $3;}
  | cxx_expression TAFD_IS_EQUAL cxx_expression   {$$ = $1 + "==" + $3;}
  | cxx_expression TAFD_NOT_EQUAL cxx_expression  {$$ = $1 + "!=" + $3;}
  | cxx_expression TAFD_MORE_EQUAL cxx_expression {$$ = $1 + ">=" + $3;}
  | cxx_expression TAFD_LESS_EQUAL cxx_expression {$$ = $1 + "<=" + $3;}
  | cxx_expression '?' cxx_expression ':' cxx_expression
					  {$$ = $1 + "?" + $3 
					     + ":" + $5;}
;
opt_cxx_expression_list: /*optional*/		  {$$ = ""; }
  | cxx_expression_list				  {$$ = $1; }
;

cxx_expression_list: 
    cxx_expression				  {$$ = $1;}
  | cxx_expression_list ',' cxx_expression	  {$$ = $1 + "," + $3;}
;
property_reference:
    /* property or operand */
    TAFD_IDENTIFIER			{ ref_prop_or_op(info, $1); } 
    /* property of other node */
  | node_reference '.' TAFD_IDENTIFIER	{ ref_node_prop(info,$3); } 
    /* start/end parameter of this node */
  | ref_provider_type '.' TAFD_start_param	{ ref_start_param(info); } 
  | ref_provider_type '.' TAFD_end_param  	{ ref_end_param(info); }
;
ref_provider_type:
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

// ***********************************************************
// C++ code rules
// ***********************************************************

cxx_code:
			  { start_code_block(info); } 
    cxx_code_statements 
			  { finish_code_block(info); }
;
cxx_code_statements: /*optional*/
  | cxx_code_statements cxx_code_statement
;
cxx_code_statement:
    TAFD_CODE
  | TAFD_BB_left          { yyerr(info,0) << "no \"[[\" allowed in this code";
			    continue_code_mode(info); }
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
