/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#include "parser_functions.hpp"

#include <proptree/proptree.hpp>
#include <utl/stdextend.hpp>

#include <stack>
#include <assert.h>

namespace anitmt
{
  namespace adlparser
  {
    //******************************
    // functions used by the parser
    //******************************

    proptree::Child_Manager child_manager;

    //**************************
    // hierarchy move functions

    // creates new tree node and makes it the current one
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);

      proptree::Prop_Tree_Node *node = 0;
      switch( info->pass )
      {
      case pass1:
	if( name != "" )	// if there is a name given -> search child
	  node = info->get_current_tree_node()->get_child( name );
	if( node != 0 )		// if already found -> check type
	{
	  if( node->get_type() != type )
	    yyerr(vptr_info) << "child " << name 
			    << " already exists with different type " 
			    << node->get_type();
	}
	else			// else -> add new child
	  node = info->get_current_tree_node()->add_child( type, name );
	
	if( node == 0 )		// was it not possible to add child?
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
	// initialized child manager
	if( !child_manager.is_initialized() ) 
	  child_manager.set_root_node( info->get_current_tree_node() );

	info->set_new_tree_node( child_manager.get_child() );
	break;
      default:
	assert(0);
      }
      // also set the declaration position for the new tree node
      set_node_pos(vptr_info);
    }

    // changes back to the parent tree node
    void change_to_parent( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      switch( info->pass )
      {
      case pass1:
	break;
      case pass2:
	child_manager.child_finished();
	break;
      }
      info->tree_node_done();
    }

  }
}
