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
  void copy_code_line( char *line, int len );
  void code_block_escape( afd_info *info );

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

  void declare_base_type( void *info, const std::string &name, 
			  const std::string &type );
  void declare_base_type_structure( void *info, const std::string &name );
  void base_type_structure_element( void *info, const std::string &type, 
				    const std::string &name );

  void start_provider_type_declaration( void *info, const std::string &name );
  void add_provided_result_type( void *info, const std::string &ret, 
				 const std::string &par );

  void start_node_declaration( void *info, const std::string &name );
  void node_extends( void *info, const std::string &node );
  void node_provides( void *info, const std::string &type, bool seriatim );

  void node_start_property_type( void *info, const std::string &type );
  void node_declare_property( void *info, const std::string &name );

  void node_declare_alias( void *info, const std::string &alias, 
			   const std::string &property );

  void node_start_operand_type( void *info, const std::string &type );
  void node_declare_operand( void *info, const std::string &name ); 

  void node_start_common_declaration( void *info );
  void node_start_first_declaration( void *info, const std::string &type="" ); 
  void node_start_last_declaration( void *info, const std::string &type="" ); 

  void node_solve_constraint( void *info );

  void node_start_solver( void *info, const std::string &solver );
  void node_add_solver_parameter( void *info ); // from reference
  void node_finish_solver( void *info ); 
  void node_solve_expression( void *info, const std::string &property );

  void node_start_action( void *info, const std::string &name, 
			  double priority );
  void node_add_action_parameter( void *info );
  void node_finish_action( void *info );

  void node_contains( void *info, bool max1, bool min1, 
		      const std::string &type, bool seriatim );

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

  std::string bool_expr( void *info, const std::string &exp1, 
			 const std::string &op, 
			 const std::string &exp2 );
  std::string expr_from_ref( void *info );
  std::string expr_scalar( void *info, double val );
  std::string expr( void *info, const std::string &exp1, 
		    const std::string &op, 
		    const std::string &exp2 );
  std::string expr_function(void *info, const std::string &name, 
			    const std::string &par );

  void ref_prop_or_op( void *info, const std::string &name );
  void ref_node_prop( void *info, const std::string &prop);
  void ref_start_param( void *info );
  void ref_end_param( void *info );
  void ref_child_res( void *info );
  void provider_type( void *info, const std::string &provider_type, 
		      const std::string &ret_type, 
		      const std::string &par_type );
  void provider_result( void *info, const std::string &provider_type, 
			const std::string &ret_type );

  void ref_node_local_prev( void *info );
  void ref_node_local_next( void *info );
  void ref_node_local_child_first( void *info, const std::string &type );
  void ref_node_local_child_last( void *info, const std::string &type );
  void ref_node_local_child( void *info, const std::string &type, double n );
  void ref_node_prev( void *info );
  void ref_node_next( void *info );
  void ref_node_parent( void *info );
  void ref_node_first_child( void *info );
  void ref_node_last_child( void *info );
  void ref_node_child( void *info, double n );

}

#include "parser_functions_inline.cpp"

#endif
