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

#include "gen_code.hpp"

namespace funcgen
{

  // ****************************************************************
  // code_gen_info:  additional information for the code generation
  // ****************************************************************
  
  void code_gen_info::add_include_path( std::string path )
  {
    include_paths.push_back( path );
  }

  void code_gen_info::set_path_separator( char sep )
  {
    path_separator = sep;
  }

  code_gen_info::code_gen_info( std::string ns, std::string name, 
				message::Message_Consultant *c ) 
    : msg(c),base_name(name), id_name(name), namespace_name(ns),
      path_separator('/')
  {}

  // ****************************************
  // Code_Translator: translates code peaces
  // ****************************************

  Code_Translator::Code_Translator( code_gen_info *i )
    : info(i)
  {}

}
