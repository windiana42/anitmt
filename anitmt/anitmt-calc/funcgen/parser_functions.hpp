/*****************************************************************************/
/**   This file offers functions and macros to shorten the parser           **/
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

#ifndef __afd_parser_functions__
#define __afd_parser_functions__

#include <message/message.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "afdbase.hpp"
#include "base_operators.hpp"

namespace funcgen
{
  // **********************
  // interfaces to lexer
  // **********************
  
  // define token type for parser
#define YYSTYPE Token
  
#define YYPARSE_PARAM info	// parameter to the parser (yyparser(void*))
#define YYLEX_PARAM info	// parameter to the lexer
#define YYLEX_PARAM_TYPE (afd_info&) // -> yylex(afd_info &info)
  inline int yylex( Token *lvalp, void *info );

  //*************************
  // interfaces to messages
  //*************************

  //! returns error message stream
  //! \param vinfo 
  inline message::Message_Stream yyerr( void* vinfo, int back=-1 );
  inline message::Message_Stream yywarn( void* vinfo, int back=-1 );
  inline message::Message_Stream yyverbose( void* vinfo, int back=-1, 
					    bool with_position=true, 
					    int vlevel=1, int detail=2 );


  // redefine error output
#define yyerror( s ) ( yyerr(info) << s, 1 )

  //****************************************
  // functions used by the parser or lexer
  //****************************************

  void start_code_block( void *info );
  void finish_code_block( void *info );
  void continue_code_mode( void *info );

  // for lexer
  void copy_code_line( afd_info *info, char *line, int len );
  void code_block_escape( afd_info *info );
  void finished_file( afd_info *info );

  //! returns a message object for reporting things to the user
  inline message::Message_Reporter &msg( void *info )
  {return static_cast<afd_info*>(info)->msg;}

  //! returns the used message consultant
  inline message::Message_Consultant *msg_consultant( void *info )
  {return static_cast<afd_info*>(info)->msg.get_consultant();}

  //! sets the position of a Property in the adl source
  inline void initialize_lexer( void *info );

  //******************************************
  // concrete help functions for parser rules

  void check_id_operator( void *info, std::string identifier );

  //************************************************************************
  // concrete help functions for parser rules that handle the afd structure

  void include_declaration( void *info, const std::string &file );
  void include_header( void *info, const std::string &file );
  bool avoid_recursion( void *info, const std::string &unique_id );
  void priority_list_defined( void *info );
  void priority_label_add( void *info, std::string label );
  void declare_base_type( void *info, const std::string &name, 
			  const std::string &type );
  void declare_base_type_structure( void *info, const std::string &name );
  void base_type_structure_element( void *info, const std::string &type, 
				    const std::string &name );

  void start_provider_type_declaration( void *info, bool serial, 
					const std::string &name );
  void add_provided_result_type( void *info, const std::string &ret, 
				 const std::string &par );

  void start_node_declaration( void *info, bool abstract,
			       const std::string &name );
  void finish_node_declaration( void *infoptr );
  void node_extends( void *info, const std::string &node );
  void node_provides( void *info, const std::string &type );

  void node_start_property_type( void *info, const std::string &type );
  void node_declare_property( void *info, const std::string &name );
  void node_declare_property( void *info, const std::string &type,
			      const std::string &name );

  void node_declare_alias( void *info, const std::string &alias, 
			   const std::string &property );

  void start_operand_type( void *info, const std::string &type );
  void declare_operand( void *info, const std::string &name ); 
  void declare_operand( void *info, const std::string &type,
			const std::string &name );
  void declare_container( void *info, bool serial,
			  const std::string &type,
			  const std::string &name );
  void declare_container( void *info, 
			  const std::string &type,
			  const std::string &name );

  void node_start_common_declaration( void *info );
  void node_start_first_declaration( void *info, const std::string &type="" ); 
  void node_start_last_declaration( void *info, const std::string &type="" ); 

  void node_solve_constraint( void *info, const Expression *exp );

  void node_start_solver( void *info, const std::string &solver );
  void node_solver_identifier( void *info, const std::string &name );  
  void node_add_solver_parameter( void *info ); // from reference
  void node_add_solver_const_parameter( void *info, bool b ); 
  void node_add_solver_const_parameter( void *info, double s ); 
  void node_add_solver_const_parameter( void *info, const std::string &s );
  void node_add_solver_const_parameter( void *info, 
					const std::string &function,
					const std::string &parameter); 
  void node_finish_solver( void *info ); 
  void node_solve_expression( void *info, const std::string &property,
			      Expression *exp );

  //void node_action_ref_param( info, int num ); // store reference parameter
  //void node_default_action( info, double, Expression *val );
  //void node_push_action( info, double );
  //void node_action_value( double );			// scalar value
  //void node_action_value( double, double, double  );	// vector value
  //void node_action_value( std::string );		// string value
  void node_start_action( void *info, const std::string &name, 
			  std::string priority_label );
  void node_add_action_parameter_ref( void *info );
  void node_add_action_parameter_str( void *info, std::string str );
  void node_add_action_parameter_exp( void *info, Expression *exp );
  void node_finish_action( void *info );

  void node_contains( void *info, bool max1, bool min1, 
		      const std::string &type );

  void node_start_provide( void *info, const std::string &type );

  void node_start_result_code( void *info, const std::string &ret_type, 
			       const std::string &par_type, 
			       const std::string &par_name );

  void node_result_essential_prop( void *info, const std::string &property );
  void node_result_essential_child_result( void *info, 
					   const std::string &provider, 
					   const std::string &ret="", 
					   const std::string &par="" );
  void node_result_essential_result( void *info, const std::string &provider, 
				     const std::string &ret="", 
				     const std::string &par="" );
  void node_result_essential_solver_function( void *info, 
					      const std::string &solver,
					      const std::string &function );

  void start_operator_declaration( void *info, const std::string &type,
				   const std::string &name );
  void start_operator_function_declaration( void *info, 
					    const std::string &name );
  void start_operator_function_code( void *info );
  void finish_operator_function_code( void *info );
  void add_operator_function_parameter( void *info, const std::string &name );
  void start_operator_version( void *info, const std::string &ret_type,
			     const std::string &name );
  void add_operator_version_parameter( void *info, const std::string &name );
  void finish_operator_version( void *infoptr );

  void start_event_solver_parameters( void *info, const std::string &name );
  void event_solver_parameter_operand( void *info, const std::string &type, 
				       const std::string &name );
  void event_solver_parameter_container( void *info, bool serial,
					 const std::string &type, 
					 const std::string &name );
  void start_event_solver_declaration( void *info );
  void finish_event_solver_declaration( void *info );

  void start_event_solver_init_solvers_block( void *info );
  void finish_event_solver_init_solvers_block( void *info );
  void start_event_solver_init_constraints_block( void *info );
  void finish_event_solver_init_constraints_block( void *info );
  void start_event_solver_function( void *info, const std::string &ret_type,
				      const std::string &name);
  void finish_event_solver_function( void *info );
  void start_event_solver_event_group( void *info, const std::string &name );
  void finish_event_solver_event_group( void *info );
  void start_event_solver_event(void *info, const std::string &name);

  void event_condition( void *info, const std::string &operand );
  void event_condition_container( void *info, 
				  const std::string &name );
  void event_condition_container_function( void *info, 
					   const std::string &name,
					   const std::string &return_type,
					   const std::string &parameter_type );
  void start_event_solver_init_code( void *info );
  void finish_event_solver_init_code( void *info );
  void start_event_group_reset_code( void *info );
  void finish_event_group_reset_code( void *info );
  void start_event_final_code( void *info );
  void finish_event_final_code( void *info );
  void start_event_reset_code( void *info );
  void finish_event_reset_code( void *info );
  void start_event_test_run_code( void *info );
  void finish_event_test_run_code( void *info );

  void event_solver_require_operand( void *info, const std::string &op );
  void event_solver_require_function( void *info, const std::string &func );
  void event_solver_require_solver_func( void *info, 
					   const std::string &solver,
					   const std::string &function );
  void event_solver_require_event( void *info, const std::string &event );
  void event_solver_require_event_group( void *info, const std::string &group);
  void event_solver_require_container( void *info, const std::string &name );
  void event_solver_require_container_function( void *info, 
						const std::string &name, 
						const std::string &return_type,
						const std::string 
						&parameter_type );
  void event_solver_code_set( void *info, std::string operand, 
			      std::string expression);
  void event_solver_code_try( void *info, std::string operand, 
			      std::string expression);
  void event_solver_code_try_reject( void *info );
  void event_solver_code_is_solved_in_try( void *info, std::string operand );
  void event_solver_code_is_just_solved( void *info, std::string operand );
  void user_code_prop_op( void *info, std::string );
  void user_code_prop_op_try( void *info, std::string );
  void user_code_solver_function( void *info, std::string solver, 
					  std::string function, 
					  std::string parameter, 
					  std::string opt_fail_bool_var );
  void user_code_return_prop( void *info, std::string operand );
  void user_code_return( void *info, std::string expression );
  void user_code_return_solver_function( void *info, 
						 std::string solver, 
						 std::string function, 
						 std::string parameter );
  void user_code_return_fail( void *info );
  void user_code_return_if_fail( void *info );

  void function_parameter( void *info, const std::string &type, 
			   const std::string &name );
  void start_variable_declaration( void *info, const std::string &type );
  void variable_declaration_name( void *info, const std::string &name );

  void write_code( void *info, double scalar );
  void write_code( void *info, std::string id );
  void res_ref_property( void *info, std::string prop );
  void res_ref_child( void *info, std::string provider, 
		      std::string result_type, std::string parameter_type, 
		      std::string parameter, std::string fail_bool_var );
  void res_ref_this( void *info, std::string provider, 
		     std::string result_type, std::string parameter_type, 
		     std::string parameter, std::string fail_bool_var );
  void res_ref_ret_child( void *info, std::string provider, 
			  std::string result_type, std::string parameter_type, 
			  std::string parameter );
  void res_ref_ret_this( void *info, std::string provider, 
			 std::string result_type, std::string parameter_type, 
			 std::string parameter );
  void res_ref_start_return_res( void *info );
  void res_ref_finish_return_res( void *info );
  void res_ref_start_return_prop( void *info );
  void res_ref_finish_return_prop( void *info );
  void res_ref_start_return( void *info );
  void res_ref_finish_return( void *info );
  void res_ref_return_fail( void *info );
  void res_ref_return_if_fail( void *info );

  Expression *bool_expr( Expression *exp1, const std::string &op, 
			 Expression *exp2 );
  Expression *bool_expr( const std::string &op, Expression *exp );
  Expression *bool_expr( Expression *exp );
  Expression *expr_from_ref( void *info );
  Expression *expr_bool( void *info, bool flag );
  Expression *expr_scalar( void *info, double val );
  Expression *expr_string( void *info, std::string str );
  Expression *expr( Expression *exp1, const std::string &op,Expression *exp2 );
  Expression *expr( const std::string &op, Expression *exp );
  Expression *expr( Expression *exp );
  Expression *expr_function(const std::string &name);
  Expression *expr_function(const std::string &name, 
			    Expression *par );
  Expression *expr_array( Expression *exp1, Expression *exp2 );

  void ref_prop_or_op( void *info, const std::string &name );
  void ref_node_prop( void *info, const std::string &prop);
  void ref_start_param( void *info );
  void ref_end_param( void *info );
  void ref_provider_type( void *info, const std::string &provider_type, 
			  const std::string &ret_type, 
			  const std::string &par_type );
  void ref_node_local_prev( void *info, std::string provider_type );
  void ref_node_local_next( void *info, std::string provider_type );
  void ref_node_local_child_first( void *info, const std::string &type );
  void ref_node_local_child_last( void *info, const std::string &type );
  void ref_node_local_child( void *info, const std::string &type, double n );
  void ref_node_prev( void *info );
  void ref_node_next( void *info );
  void ref_node_parent( void *info );
  void ref_node_first_child( void *info );
  void ref_node_last_child( void *info );
  void ref_node_child( void *info, double n );
  
  void common_add_identifier( void *info, std::string id );
}

#include "parser_functions_inline.cpp"

#endif
