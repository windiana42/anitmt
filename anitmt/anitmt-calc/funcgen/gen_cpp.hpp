/*****************************************************************************/
/**   This file offers functions to generate C++ Code from AFD data tree   **/
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
    std::string prefix_base_type; //! prefix for base type names
    std::string prefix_provider_type; //! prefix for provider type names
    std::string prefix_node_type; //! prefix for node types
    std::string prefix_prop_op; //! prefix for properties and operands
    std::string prefix_res_fun; //! prefix for result functions
    std::string prefix_param_range; //! prefix for start_/end_param
    std::string prefix_container_type; //! prefix for container types
    std::string prefix_serial_container_type; //! prefix for serial containers
    std::string prefix_container_name; //! prefix for container names
  public:
    virtual std::string open_block();
    virtual std::string close_block();
    virtual std::string base_type( std::string name );
    virtual std::string provider_type( std::string name );
    virtual std::string node_type( std::string name );
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
    virtual std::string start_param( std::string provider_type,
				     std::string ret_type,
				     std::string par_type );
    virtual std::string end_param( std::string provider_type,
				   std::string ret_type,
				   std::string par_type );
    virtual std::string prop_op( std::string name );
    virtual std::string prop_op_value( std::string name );
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
    virtual std::string first_init( std::string provider_type );
    virtual std::string last_init( std::string provider_type );

    Cpp_Code_Translator();
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
    void generate_base_types();
    void generate_types();
    void generate_nodes();
  public:
    Cpp_Code_Translator *get_translator();
    void generate_code( AFD_Root *afd, code_gen_info *info );
  };
}

#endif
