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

#include <stack>

#include "parser_functions.hpp"

#include <utl/stdextend.hpp>
#include <proptree/proptree.hpp>

namespace anitmt
{
  namespace adlparser
  {
    //******************************
    // functions used by the parser
    //******************************

    //***************
    // Child Manager

    // keeps track of the tree node hierarchy in pass2
    class Child_Manager{
      std::stack<proptree::Prop_Tree_Node*> last_child; 
      static proptree::Prop_Tree_Node *no_child;
      bool initialized;
    public:
      inline bool is_initialized() { return initialized; }

      void set_root_node( proptree::Prop_Tree_Node *node )
      {
	last_child.push(node);		// node is parent...
	last_child.push(no_child);	// ... so add a virtual child
	initialized = true;
      }

      proptree::Prop_Tree_Node *get_child()
      {
	assert( initialized );	// there must be at least a no_child

	proptree::Prop_Tree_Node *last = last_child.top(); // get last child
	last_child.pop();		// remove old child
	proptree::Prop_Tree_Node *new_child;
	if( last != no_child )
	{
	  new_child = last->get_next(); assert( new_child != 0 );
	}
	else
	{
	  proptree::Prop_Tree_Node *parent = last_child.top();
	  new_child = parent->get_first_child();
	}
	last_child.push(new_child);
	last_child.push(no_child);
	return new_child;
      }

      void child_finished()
      {
	assert( initialized );	// there must be at least a no_child

	last_child.pop();
	assert( !last_child.empty() ); // it's not allowed to finish root node
      }
      Child_Manager() : initialized(false) {}
    };
    proptree::Prop_Tree_Node *Child_Manager::no_child = 0; // static initialization

    Child_Manager child_manager;

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
