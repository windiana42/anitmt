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
%token TAFD_include TAFD_declaration  TAFD_base_types TAFD_serial TAFD_type 
%token TAFD_node TAFD_provides TAFD_extends TAFD_properties TAFD_aliases
%token TAFD_operands TAFD_common TAFD_constraints TAFD_solvers TAFD_actions 
%token TAFD_push TAFD_default
%token TAFD_contains TAFD_max1 TAFD_min1 TAFD_provide TAFD_resulting 
%token TAFD_requires TAFD_this TAFD_prev TAFD_next TAFD_first TAFD_last 
%token TAFD_parent TAFD_child TAFD_first_child TAFD_last_child 
%token TAFD_start_param TAFD_end_param TAFD_true TAFD_false
%token TAFD_return_res TAFD_return_prop TAFD_return
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

%type <string>  CXX_identifier
%type <string>  opt_provider_type
%type <u.scalar>  priority_level
%type <u.boolean> opt_max1
%type <u.boolean> opt_min1
%type <u.boolean> opt_serial
%type <string>  opt_res_ref_provider
%type <u.exp>  bool_expression
%type <u.exp>  expression
//****************
// precicion table
//****************

//********************
// normal precedences
%nonassoc '<' '>' TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

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
  | base_types_declaration
  | type_declaration
  | node_declaration
;
include_declaration:
    TAFD_include TAFD_declaration TAFD_QSTRING ';' 
					{ include_declaration( info, $3 ); }
;
base_types_declaration: 
    TAFD_base_types '{' base_type_statements '}'
;
base_type_statements: /*optional*/
  | base_type_statements base_type_statement
;
base_type_statement:
    TAFD_IDENTIFIER '=' CXX_identifier ';'			
      { declare_base_type( info, $1, $3 ); }
  | TAFD_IDENTIFIER '=' '{' 
      { declare_base_type_structure( info, $1 ); }
      base_type_structure '}' ';'
;
CXX_identifier:
    TAFD_IDENTIFIER					{$$ = $1}
  | CXX_identifier TAFD_NS_CONCAT TAFD_IDENTIFIER	{$$ = $1 + "::" + $3}
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
node_declaration:
    TAFD_node TAFD_IDENTIFIER 
      { start_node_declaration( info, $2 ); }
      opt_extends opt_provides '{' node_statements '}'
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
    bool_expression ';'			{ node_solve_constraint( info, $1 ); }
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
  | TAFD_IDENTIFIER '=' expression ';' { node_solve_expression( info,$1,$3 ); }
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
		     expression ')' ';'
      { node_add_action_parameter_exp( info, $9 );
	node_finish_action( info ); }
  | TAFD_push '(' priority_level ',' 
      { node_start_action( info, "push", $3 ); }
	          property_reference ','  
      { node_add_action_parameter_ref( info ); }
		  property_reference ')' ';'  
      { node_add_action_parameter_ref( info ); // store second parameter
	node_finish_action( info ); }
  | TAFD_IDENTIFIER '(' priority_level ',' 
      { node_start_action( info, $1, $3 ); }
      action_parameter_list ')' ';' 
      { node_finish_action( info ); } //!!! obsolete?
;
priority_level:
    TAFD_SCALAR	
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
  | '[' result_reference ']' 	{ continue_code_mode(info); }
  | '[' TAFD_SCALAR ']' 	{ write_code(info,"[");
				  write_code(info,$2);
				  write_code(info,"]");
				  continue_code_mode(info); }
  | '[' TAFD_return_res		{ res_ref_start_return_res(info); }
      result_function_reference ']' 	
				{ res_ref_finish_return_res(info);
				  continue_code_mode(info); }
  | '[' TAFD_return_prop	{ res_ref_start_return_prop(info); }
      result_property_reference ']' 	
				{ res_ref_finish_return_prop(info);
				  continue_code_mode(info); }
  | '[' TAFD_return		{ res_ref_start_return(info); }
      TAFD_IDENTIFIER ']'	{ write_code(info,$4);
				  res_ref_finish_return(info);
				  continue_code_mode(info); }
  | '[' TAFD_return		{ res_ref_start_return(info); }
      TAFD_SCALAR ']'		{ write_code(info,$4);
				  res_ref_finish_return(info);
				  continue_code_mode(info); }
  | '[' error ']' 		{ continue_code_mode(info); }
;
result_reference:
    result_property_reference
  | result_function_reference
;
result_property_reference:
    TAFD_IDENTIFIER		{ res_ref_property(info,$1); }
;
result_function_reference:
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '('
    TAFD_IDENTIFIER TAFD_IDENTIFIER ')'
				{ res_ref_child(info,$3,$5,$7,$8); }
  | TAFD_this '.' opt_res_ref_provider TAFD_IDENTIFIER '(' 
    TAFD_IDENTIFIER TAFD_IDENTIFIER ')'
				{ res_ref_this(info,$3,$4,$6,$7); }
;
opt_res_ref_provider: /*optional*/ { $$ = ""; }
  | TAFD_IDENTIFIER '.'		{ $$ = $1; }
;
bool_expression:
    expression TAFD_IS_EQUAL expression	  {$$ = bool_expr($1,"==",$3);}
  | expression TAFD_NOT_EQUAL expression  {$$ = bool_expr($1,"!=",$3);}
  | expression TAFD_MORE_EQUAL expression {$$ = bool_expr($1,">=",$3);}
  | expression TAFD_LESS_EQUAL expression {$$ = bool_expr($1,"<=",$3);}
  | expression '>' expression		  {$$ = bool_expr($1,">",$3);}
  | expression '<' expression		  {$$ = bool_expr($1,"<",$3);}
;
expression:
    property_reference			  {$$ = expr_from_ref(info);}
  | TAFD_SCALAR				  {$$ = expr_scalar($1);}
  | '(' expression ')'			  {$$ = expr($2);}
  | expression '+' expression		  {$$ = expr($1,"+",$3);}
  | expression '-' expression		  {$$ = expr($1,"-",$3);}
  | expression '*' expression		  {$$ = expr($1,"*",$3);}
  | expression '/' expression		  {$$ = expr($1,"/",$3);}
  | TAFD_IDENTIFIER '(' expression ')'	  {$$ = expr_function($1,$3);}
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
    provider_type '.' TAFD_prev		{ ref_node_local_prev(info); }
  | provider_type '.' TAFD_next		{ ref_node_local_next(info); }
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
      return -1;
    int ret = yyparse( static_cast<void*>(&info) );
      
    return ret;
  }
} // close namespace afd
