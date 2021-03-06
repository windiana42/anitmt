/*****************************************************************************/
/**   This file offers functions to generate C++ Code from AFD data tree   **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __AFD_generate_code__
#define __AFD_generate_code__

#include <string>
#include <list>

#include <message/message.hpp>

namespace funcgen
{
  //! additional information for the code generation
  class code_gen_info;
  //! translates code peaces
  class Code_Translator;
  //! generates code
  class Code_Generator;
}

#include "afdbase.hpp"

namespace funcgen
{

  //! additional information for the code generation
  class code_gen_info
  {
  public:
    message::Message_Reporter msg; // for reporting errors
    std::string base_name;	// base filename for the generated file
    std::string id_name;	// identifier for the package
    std::string namespace_name; // name for the namespace
    std::list<std::string> include_paths; // pathes to search for include files
    char path_separator;	// path separator

    void add_include_path( std::string path );
    void set_path_separator( char );

    code_gen_info( std::string namespace_name, std::string base_name, 
		   message::Message_Consultant *c );
  };

  //! translates code peaces
  class Code_Translator 
  {
    code_gen_info *info;
  public:
    inline const code_gen_info *get_info() { return info; }

    virtual std::string open_block() = 0;
    virtual std::string close_block() = 0;
    virtual std::string priority_label( std::string name ) = 0;
    virtual std::string base_type( std::string name ) = 0;
    virtual std::string provider_type( std::string name ) = 0;
    virtual std::string node_type( std::string name ) = 0;
    virtual std::string node_base_type() = 0;
    virtual std::string open_action( std::string action, 
				     std::string priority_label ) = 0;
    virtual std::string parameter_add( std::string param ) = 0;
    virtual std::string close_function() = 0;
    virtual std::string solver_identifier( std::string name ) = 0;
    virtual std::string result_function_decl( std::string provider_type,
					      std::string ret_type, 
					      std::string par_type ) = 0;
    virtual std::string result_function_impl( std::string provider_type,
					      std::string ret_type,
					      std::string par_type,
					      std::string par) = 0;
    virtual std::string result_function_call( std::string provider_type,
					      std::string ret_type,
					      std::string par_type,
					      std::string par ) = 0;
    virtual std::string start_return_result( std::string return_type ) = 0;
    virtual std::string finish_return_result( std::string return_type ) = 0;
    virtual std::string start_return_value( std::string return_type ) = 0;
    virtual std::string finish_return_value( std::string return_type ) = 0;
    virtual std::string return_fail() = 0;
    virtual std::string return_if_fail() = 0;
    virtual std::string start_param( std::string provider_type,
				     std::string ret_type,
				     std::string par_type ) = 0;
    virtual std::string end_param( std::string provider_type,
				   std::string ret_type,
				   std::string par_type ) = 0;
    virtual std::string prop_op( std::string name ) = 0;
    virtual std::string prop_op_value( std::string name ) = 0;
    virtual std::string prop_op_value_try( std::string name ) = 0;
    virtual std::string node_prop( std::string name ) = 0;
    virtual std::string property_type( std::string name ) = 0;
    virtual std::string operand_type( std::string name ) = 0;
    virtual std::string container( std::string provider_type ) = 0;
    virtual std::string serial_container( std::string provider_type ) = 0;
    virtual std::string container_name( std::string provider_type ) = 0;
    virtual std::string child_result
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par ) = 0;
    virtual std::string provided_result
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par ) = 0;
    virtual std::string child_result_with_status
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par, std::string fail_bool_var  ) = 0;
    virtual std::string provided_result_with_status
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par, std::string fail_bool_var ) = 0;
    virtual std::string first_init( std::string provider_type ) = 0;
    virtual std::string last_init( std::string provider_type ) = 0;
    virtual std::string is_avail( std::string provider_type,
				  std::string ret_type="" ,
				  std::string par_type="" ) = 0;
    virtual std::string prev() = 0;
    virtual std::string next() = 0;
    virtual std::string prev( std::string provider_type ) = 0;
    virtual std::string next( std::string provider_type ) = 0;
    virtual std::string parent() = 0;
    virtual std::string first_child() = 0;
    virtual std::string last_child() = 0;
    virtual std::string get_child( int n ) = 0;
    virtual std::string first_child( std::string child_type ) = 0;
    virtual std::string last_child( std::string child_type ) = 0;
    virtual std::string get_child( std::string child_type, int n ) = 0;
    virtual std::string reference_concat_string() = 0;
    virtual std::string operator_class_name( std::string name ) = 0;
    virtual std::string solver_class_name( std::string name ) = 0;
    virtual std::string operand_from_bool( bool flag ) = 0;
    virtual std::string operand_from_scalar( double val ) = 0;
    virtual std::string operand_from_string( std::string ) = 0;
    virtual std::string operand_from_function( std::string name, 
					       std::string parameters ) = 0;
    /*
    virtual std::string function( std::string function_name,
				  std::string parameters ) = 0;
    */
    virtual std::string event_code_set( std::string operand, 
					std::string expression ) = 0;
    virtual std::string event_code_try( std::string operand, 
					std::string expression ) = 0;
    virtual std::string event_code_try_reject( const std::list<std::string> 
					       &bad_ops ) = 0;
    virtual std::string event_code_is_solved_in_try( std::string operand ) = 0;
    virtual std::string event_code_is_just_solved( std::string operand ) = 0;
    virtual std::string solver_function_value( std::string solver, 
					       std::string function, 
					       std::string parameter, 
					       std::string opt_fail_bool_var )
      = 0;
    virtual std::string solver_function_result( std::string solver, 
						std::string function, 
						std::string parameter ) = 0;
    virtual std::string container_function_value
    ( std::string container, std::string provider_type, 
      std::string return_type, std::string parameter_type, 
      std::string parameter, std::string opt_fail_bool_var ) = 0;
    virtual std::string container_for_each_element
    ( std::string element, std::string container, std::string provider_type )
      = 0;
    virtual std::string element_function_value
    ( std::string element, std::string provider_type, std::string return_type,
      std::string parameter_type, std::string parameter,
      std::string opt_fail_bool_var ) = 0;
    virtual std::string container_first_index( std::string container ) = 0;
    virtual std::string container_last_index( std::string container ) = 0;
    virtual std::string container_element_function_value
    ( std::string container, int index, std::string provider_type, 
      std::string return_type, std::string parameter_type, 
      std::string parameter, std::string opt_fail_bool_var ) = 0;

    Code_Translator( code_gen_info * );

    virtual ~Code_Translator(){}
  };

  //! generates code
  class Code_Generator
  {
  public:
    virtual void generate_code( AFD_Root *afd ) = 0;
    virtual ~Code_Generator(){}
  };
}
#endif
