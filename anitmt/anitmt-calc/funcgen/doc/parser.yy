// keywords
%token TAFD_include TAFD_declaration  TAFD_base_types TAFD_type TAFD_node
%token TAFD_provides TAFD_extends TAFD_seriatim TAFD_properties TAFD_aliases
%token TAFD_operands TAFD_common TAFD_constraints TAFD_solvers TAFD_actions 
%token TAFD_contains TAFD_max1 TAFD_min1 TAFD_provide TAFD_resulting 
%token TAFD_requires TAFD_this TAFD_prev TAFD_next TAFD_first TAFD_last 
%token TAFD_parent TAFD_child TAFD_first_child TAFD_last_child 
%token TAFD_start_param TAFD_end_param TAFD_true TAFD_false
// lexer error
%token TAFD_ERROR 
// multi character operators
%token TAFD_IS_EQUAL TAFD_NOT_EQUAL TAFD_MORE_EQUAL TAFD_LESS_EQUAL 
%token TAFD_NS_CONCAT
// special tokens
%token TAFD_CODE
// type tokens
%token <string> TAFD_IDENTIFIER
%token <scalar> TAFD_SCALAR
%token <string> TAFD_QSTRING

%type <string> CXX_identifier

//****************
// precicion table
//****************

//********************
// normal precedences
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
;
base_types_declaration: 
    TAFD_base_types '{' base_type_statements '}'
;
base_type_statements: /*optional*/
  | base_type_statements base_type_statement
;
base_type_statement:
    TAFD_IDENTIFIER '=' CXX_identifier ';'			
  | TAFD_IDENTIFIER '=' '{' base_type_structure '}' ';'		
;
CXX_identifier:
    TAFD_IDENTIFIER					
  | CXX_identifier TAFD_NS_CONCAT TAFD_IDENTIFIER	
;
base_type_structure: 
    base_type_structure_element
  | base_type_structure ',' base_type_structure_element
;
base_type_structure_element:
    TAFD_IDENTIFIER TAFD_IDENTIFIER 	/*type name*/		
;

type_declaration: 
    TAFD_type TAFD_IDENTIFIER '{' provider_type_statements '}'	
;
provider_type_statements: /*optional*/
  |  provider_type_statements provider_type_statement
;
provider_type_statement:
    TAFD_provides TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')' ';'	
;
node_declaration:
    TAFD_node TAFD_IDENTIFIER 
      opt_extends opt_provides '{' node_statements '}'
;
opt_extends: /*optional*/
  | TAFD_extends extend_list
;
extend_list: 
    TAFD_IDENTIFIER				{}
  | extend_list ',' TAFD_IDENTIFIER		{}
;
opt_provides: /*optional*/
  | TAFD_provides provided_type_list
;
provided_type_list: provided_type
  | provided_type_list ',' provided_type
;
provided_type:
    TAFD_IDENTIFIER opt_seriatim	{}
;
opt_seriatim: /*optional*/
  | TAFD_seriatim
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
property_types:
    TAFD_type TAFD_IDENTIFIER '{' property_names '}'	{}
;
property_names: /*optional*/
  | property_names property_name
;
property_name:
    TAFD_IDENTIFIER ';'	{}
;
aliases_declaration:
    TAFD_aliases '{' alias_statements '}'
;
alias_statements: /*optional*/
  | alias_statements alias_statement
;
alias_statement:
    TAFD_IDENTIFIER '=' TAFD_IDENTIFIER ';'	{}
;
operands_declaration:
    TAFD_operands '{' operand_types '}'
;
operand_types:
    TAFD_type TAFD_IDENTIFIER '{' operand_names '}'	{}
;
operand_names: /*optional*/
  | operand_names operand_name
;
operand_name:
    TAFD_IDENTIFIER ';'	{}
;
common_declaration:
    TAFD_common '{' solve_system_statements '}'
;
first_declaration:
    TAFD_first opt_provider_type '{' solve_system_statements '}'
;
last_declaration:
    TAFD_last opt_provider_type '{' solve_system_statements '}'
;
opt_provider_type: /*optional*/
  | TAFD_IDENTIFIER	{}
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
    bool_expression ';'	{}
;
solvers_declaration:
    TAFD_solvers '{' solver_statements '}'
;
solver_statements: /*optional*/
  | solver_statements solver_statement
;
solver_statement:
    TAFD_IDENTIFIER '(' solver_parameter_list ')' ';'	{}
  | TAFD_IDENTIFIER '=' expression ';'	{}
;
solver_parameter_list: property_reference
  | solver_parameter_list ',' property_reference
;
actions_declaration:
    TAFD_actions '{' action_statements '}'
;
action_statements: /*optional*/
  | action_statements action_statement
;
action_statement:
    TAFD_IDENTIFIER '(' priority_level ',' action_parameter_list ')' ';' {}
;
priority_level:
    TAFD_SCALAR	{}
;
action_parameter_list: property_reference
  | action_parameter_list ',' property_reference
;     

contains_declaration:
    TAFD_contains '{' contain_statements '}'
;
contain_statements: /*optional*/
  | contain_statements contain_statement
;
contain_statement:
    opt_max1 opt_min1 TAFD_IDENTIFIER opt_seriatim ';'	{}
;
opt_max1: /*optional*/
  | TAFD_max1
;
opt_min1: /*optional*/
  | TAFD_min1
;
provide_declaration:
    TAFD_provide TAFD_IDENTIFIER '{' result_code_definitions '}'	{}
;
result_code_definitions: /*optional*/
  | result_code_definitions result_code_definition
;
result_code_definition:
    TAFD_resulting TAFD_IDENTIFIER '(' TAFD_IDENTIFIER TAFD_IDENTIFIER ')'
      opt_requires '{' result_code_block '}'
      {}
;
opt_requires: /*optional*/
  | TAFD_requires essentials_list
;
essentials_list: 
    essential
  | essentials_list essential
;
essential:
    TAFD_IDENTIFIER								{}
  | TAFD_child '.' TAFD_IDENTIFIER						{}
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'	{}
  | TAFD_this '.' TAFD_IDENTIFIER					 	{}
  | TAFD_this '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'			{}
  | TAFD_this '.' TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'	{}
;
result_code_block:
    { start_code_block(info); } code_statements { finish_code_block(info); }
;
code_statements: /*optional*/
  | code_statements code_statement
;
code_statement:
    TAFD_CODE
  | '[' property_reference ']' 	{ continue_code_mode(info); }
  | '[' TAFD_SCALAR ']' 	{ continue_code_mode(info); }
;
bool_expression:
    TAFD_true
  | TAFD_false
  | expression TAFD_IS_EQUAL expression
  | expression TAFD_NOT_EQUAL expression
  | expression TAFD_MORE_EQUAL expression
  | expression TAFD_LESS_EQUAL expression
;
expression:
    property_reference
  | TAFD_SCALAR				{}
  | '(' expression ')'
  | expression '+' expression
  | expression '-' expression
  | expression '*' expression
  | expression '/' expression
  | TAFD_IDENTIFIER '(' expression ')'	{}  /* function */
;
property_reference:
    TAFD_IDENTIFIER			{} /* property or operand */
  | node_reference '.' TAFD_IDENTIFIER	{} /* property of other node */
  | provider_type '.' TAFD_start_param	{} /* start/end parameter of this node */
  | provider_type '.' TAFD_end_param  	{} /*   according to sequence type */
  | TAFD_child '.' provider_result	{} /* get result provided by child */
;
provider_type:
    TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' TAFD_IDENTIFIER ')'	{}
;
provider_result:
    TAFD_IDENTIFIER '.' TAFD_IDENTIFIER '(' expression ')'	{}
;
node_reference:
    local_node_identifier
  | node_identifier
  | node_reference '.' node_identifier
;
local_node_identifier:
    provider_type '.' TAFD_prev
  | provider_type '.' TAFD_next
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_first	{} /* first child */
  | TAFD_child '.' TAFD_IDENTIFIER '.' TAFD_last	{} /* last child */
  | TAFD_child '.' TAFD_IDENTIFIER '[' TAFD_SCALAR ']'	{} /* n th child */
;
node_identifier:
    TAFD_prev
  | TAFD_next
  | TAFD_parent
  | TAFD_first_child
  | TAFD_last_child
  | TAFD_child '[' TAFD_SCALAR ']'	{}
;

%%
