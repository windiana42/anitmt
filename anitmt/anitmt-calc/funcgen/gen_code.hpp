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

#ifndef __AFD_generate_code__
#define __AFD_generate_code__

#include <string>

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
    std::string base_name;	// base filename for the generated file
    std::string id_name;	// base filename for the generated file

    code_gen_info( std::string name );
  };

  //! translates code peaces
  class Code_Translator 
  {
  public:
    virtual std::string open_block() = 0;
    virtual std::string close_block() = 0;
    virtual std::string base_type( std::string name ) = 0;
    virtual std::string provider_type( std::string name ) = 0;
    virtual std::string node_type( std::string name ) = 0;
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
    virtual std::string start_param( std::string provider_type,
				     std::string ret_type,
				     std::string par_type ) = 0;
    virtual std::string end_param( std::string provider_type,
				   std::string ret_type,
				   std::string par_type ) = 0;
    virtual std::string prop_op( std::string name ) = 0;
    virtual std::string prop_op_value( std::string name ) = 0;
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
    virtual std::string first_init( std::string provider_type ) = 0;
    virtual std::string last_init( std::string provider_type ) = 0;

    virtual ~Code_Translator(){}
  };

  //! generates code
  class Code_Generator
  {
  public:
    virtual void generate_code( AFD_Root *afd, code_gen_info *info ) = 0;
    virtual ~Code_Generator(){}
  };
}
#endif
