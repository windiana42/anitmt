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
  
  code_gen_info::code_gen_info( std::string name, 
				message::Message_Consultant *c ) 
    : msg(c),base_name(name), id_name(name)
  {}

}
