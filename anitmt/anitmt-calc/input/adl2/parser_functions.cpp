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

#include <utl/stdextend.hpp>

namespace anitmt
{
  namespace adlparser
  {
    //******************************
    // functions used by the parser
    //******************************

    long global_numbering = 1;

    // creates new tree node and makes it the current one
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name="" )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);

      Prop_Tree_Node *node;
      switch( info->pass )
      {
      case pass1:
	node = info->get_current_tree_node()->add_child( type, name );
	
	if( node == 0 )
	{
	  yyerr(vptr_info) << "couldn't add tree node " << name << " as type " 
			   << type << " isn't allowed here";
	}
	else
	{
	  info->set_new_tree_node( node );
	}
	break;
      case pass2:
	if( name == "" ) 
	{
	  name = type + global_numbering;
	  global_numbering++;
	}

	node = info->get_current_tree_node()->get_child( name );
	
	if( node == 0 )
	{
	  yyerr(vptr_info) << "internal error: couldn't refind " << type 
			   << " " << name;
	}
	else
	{
	  info->set_new_tree_node( node );
	}	
	break;
      default:
	assert(0);
      }
    }
  }
}
