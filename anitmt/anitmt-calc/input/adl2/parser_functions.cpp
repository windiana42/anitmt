/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#include "parser_functions.hpp"

namespace anitmt
{
  namespace adlparser
  {
    //******************************
    // functions used by the parser
    //******************************

    // creates new tree node and makes it the current one
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name="" )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);

      Prop_Tree_Node *node = 
	info->get_current_tree_node()->add_child( type, name );

      if( node == 0 )
      {
	yyerr(vptr_info) << "couldn't add tree node";
      }
      else
      {
	info->set_new_tree_node( node );
      }
    }
  }
}
