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

#ifndef __AFD_generate_cpp_code__
#define __AFD_generate_cpp_code__

#include <string>
#include <fstream>

#include "afdbase.hpp"
#include "gen_code.hpp"

namespace funcgen
{
  //! translates code peaces to C++ code
  class Cpp_Code_Translator : public Code_Translator
  {
    std::string prefix_priority_label; //! prefix for priority labels
    std::string prefix_base_type; //! prefix for base type names
    std::string prefix_provider_type; //! prefix for provider type names
    std::string prefix_node_type; //! prefix for node types
    std::string prefix_prop_op; //! prefix for properties and operands
    std::string prefix_res_fun; //! prefix for result functions
    std::string prefix_is_avail; //! prefix for is_avail operand
    std::string prefix_param_range; //! prefix for start_/end_param
    std::string prefix_container_type; //! prefix for container types
    std::string prefix_serial_container_type; //! prefix for serial containers
    std::string prefix_container_name; //! prefix for container names
    std::string prefix_operator_class_name; //! prefix for operator classes
  public:
    virtual std::string open_block();
    virtual std::string close_block();
    virtual std::string priority_label( std::string name );
    virtual std::string base_type( std::string name );
    virtual std::string provider_type( std::string name );
    virtual std::string node_type( std::string name );
    virtual std::string node_base_type();
    virtual std::string open_action( std::string action, 
				     std::string priority_label );
    virtual std::string parameter_add( std::string param );
    virtual std::string close_function();
    virtual std::string result_function_decl( std::string provider_type,
					      std::string ret_type, 
					      std::string par_type );
    virtual std::string result_function_impl( std::string provider_type,
					      std::string ret_type,
					      std::string par_type,
					      std::string par);
    virtual std::string result_function_call( std::string provider_type,
					      std::string ret_type,
					      std::string par_type,
					      std::string par );
    virtual std::string start_return_res( std::string return_type );
    virtual std::string finish_return_res( std::string return_type );
    virtual std::string start_return_prop( std::string return_type );
    virtual std::string finish_return_prop( std::string return_type );
    virtual std::string start_return( std::string return_type );
    virtual std::string finish_return( std::string return_type );
    virtual std::string return_fail();
    virtual std::string return_if_fail();
    virtual std::string start_param( std::string provider_type,
				     std::string ret_type,
				     std::string par_type );
    virtual std::string end_param( std::string provider_type,
				   std::string ret_type,
				   std::string par_type );
    virtual std::string prop_op( std::string name );
    virtual std::string prop_op_value( std::string name );
    virtual std::string node_prop( std::string name );
    virtual std::string property_type( std::string name );
    virtual std::string operand_type( std::string name );
    virtual std::string container( std::string provider_type );
    virtual std::string serial_container( std::string provider_type );
    virtual std::string container_name( std::string provider_type );
    virtual std::string child_result
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par );
    virtual std::string provided_result
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par );
    virtual std::string child_result_with_status
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par, std::string fail_bool_var  );
    virtual std::string provided_result_with_status
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par, std::string fail_bool_var );
    virtual std::string first_init( std::string provider_type );
    virtual std::string last_init( std::string provider_type );
    virtual std::string is_avail( std::string provider_type,
				  std::string ret_type,
				  std::string par_type);
    virtual std::string prev();
    virtual std::string next();
    virtual std::string prev( std::string provider_type );
    virtual std::string next( std::string provider_type );
    virtual std::string parent();
    virtual std::string first_child();
    virtual std::string last_child();
    virtual std::string get_child( int n );
    virtual std::string first_child( std::string child_type );
    virtual std::string last_child( std::string child_type );
    virtual std::string get_child( std::string child_type, int n );
    virtual std::string reference_concat_string();
    virtual std::string operator_class_name( std::string name );

    Cpp_Code_Translator(code_gen_info *);
  };

  //! generates C++ code
  class Cpp_Code_Generator : public Code_Generator
  {
    std::ostream *decl;		// stream to output declaration code
    std::ostream *impl;		// stream to output implementation code
    AFD_Root *afd;
    code_gen_info *info;

    Cpp_Code_Translator translator;	// cpp code translator

    void generate_header();
    void generate_footer();
    void generate_priority_list();
    void generate_base_types();
    void generate_types();
    void generate_operators();
    void generate_nodes();
  public:
    Cpp_Code_Translator *get_translator();
    void generate_code( AFD_Root *afd );
    Cpp_Code_Generator( code_gen_info *info );
  };
}

#endif
