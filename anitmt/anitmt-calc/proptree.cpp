/*****************************************************************************/
/**   This file offers a tree structure for groups of properties	    **/
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

#include "proptree.hpp"

namespace anitmt{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  //**********
  // constants

  Prop_Tree_Node::child_factory_type Prop_Tree_Node::child_factory;

  //**********
  // functions

  std::string Prop_Tree_Node::get_name(){
    return name;
  }

  std::string Prop_Tree_Node::get_type(){
    return type;
  }

  Property *Prop_Tree_Node::get_property( std::string name ){
    properties_type::iterator i = properties.find( name );
    if( i != properties.end() )
      return i->second;
    else
      return 0;
  }

  std::list<std::string> Prop_Tree_Node::get_properties_names(){
    std::list<std::string> ret;
      
    properties_type::iterator i;
    for( i = properties.begin(); i != properties.end(); i++ )
      {
	ret.push_back( i->first );
      }

    return ret;
  }

  Prop_Tree_Node *Prop_Tree_Node::get_child( std::string name ){
    Prop_Tree_Node *n;

    for( n = first_child; n != 0; n = n->next )
      {
	if( n->get_name() == name )
	  return n;
      }
    return 0;
  }

  std::list<Prop_Tree_Node*> Prop_Tree_Node::get_all_childs(){
    std::list<Prop_Tree_Node*> ret;

    Prop_Tree_Node *n;

    for( n = first_child; n != 0; n = n->next )
      {
	ret.push_back( n );
      }

    return ret;
  }

  // returns next part before a dot or the end of string
  // !!! reference argument will be reduced by part that is returned
  inline std::string get_next_part( std::string &str ){
    std::string::size_type i = str.find('.');
    if( i == std::string::npos ) // dot not found -> i = eos
      {
	std::string ret = str;  // return whole string
	str = "";		// empty
	return ret;
      }
    else
      {
	std::string ret = str.substr( 0, i ); 
				// return string before dot
	str = str.substr( i+1 );// erase part to return 
	return ret;
      }    
  }

  // add child of type with name
  Prop_Tree_Node *Prop_Tree_Node::add_child( std::string type, 
					     std::string name ) 
    throw( EX_child_type_unkown, EX_child_type_rejected ) {

    child_factory_type::iterator i = child_factory.find( type );
    // if type not found
    if( i == child_factory.end() )
      throw EX_child_type_unkown();

    // create node with factory found
    Prop_Tree_Node *node = i->second->create( name, ani );

    if( first_child == 0 ) 
      {
	assert( last_child == 0 );

	first_child = node;
	last_child = node;
      }
    else
      {
	// connect with last child
	node->prev = last_child;
	last_child->next = node;
	// replace last child
	last_child = node;
      }

    if( !try_add_child( node ) )
      throw EX_child_type_rejected();

    return node;
  }

  // find node according to referencing string
  Prop_Tree_Node *Prop_Tree_Node::get_referenced_node( std::string ref ){

    Prop_Tree_Node *cur = this;
    std::string part;
    while( (part = get_next_part(ref)) != "" ){
      if( part == "parent" )
	{
	  cur = cur->parent;
	  if( cur == 0 )
	    throw EX_invalid_reference();
	  continue;
	}
      if( part == "next" )
	{
	  cur = cur->next;
	  if( cur == 0 )
	    throw EX_invalid_reference();
	  continue;
	}
      if( part == "prev" )
	{
	  cur = cur->prev;
	  if( cur == 0 )
	    throw EX_invalid_reference();
	  continue;
	}

      Prop_Tree_Node *test = get_child( part );
      if( test != 0 )
	{
	  cur = test;
	  continue;
	}

      throw EX_invalid_reference();
    }
  }

  // adds a factory object for class generation
  void Prop_Tree_Node::add_child_factory( std::string type_name, 
					  Child_Factory* fac )
    throw( EX_child_type_already_defined ) {

    child_factory_type::iterator i = child_factory.find( type_name );
    // if type not found
    if( i != child_factory.end() )
      throw EX_child_type_already_defined();
    
    child_factory[ type_name ] = fac;
  }

  //**************************
  // constructors / destructor

  Prop_Tree_Node::Prop_Tree_Node( std::string t, std::string n, Animation *a ) 
    : parent(0), prev(0), next(0), first_child(0), last_child(0), 
      type(t), name(n), ani(a) {}

  Prop_Tree_Node::~Prop_Tree_Node() {}

}

