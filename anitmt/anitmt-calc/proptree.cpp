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

#include "nodes.hpp"
#include "animation.hpp"
#include "save_filled.hpp"

#include <message/message.hpp>

#include <utl/stdextend.hpp>

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

  std::list<Prop_Tree_Node*> Prop_Tree_Node::get_all_children(){
    std::list<Prop_Tree_Node*> ret;

    Prop_Tree_Node *n;

    for( n = first_child; n != 0; n = n->next )
    {
      ret.push_back( n );
    }

    return ret;
  }

  //! add child of type with name
  //! if name is an empty string a default name is created
  Prop_Tree_Node *Prop_Tree_Node::add_child( std::string type, 
					     std::string name ) 
    throw() 
  {
    if( name == "" )
    {
      // create default name
      name = type + (ani->unique_name_counter++); 
    }

    child_factory_type::iterator i = child_factory.find( type );
    // if type not found
    if( i == child_factory.end() )
    {
      //err = AC_child_type_unknown; // return error type
      return 0;
    }

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
      return 0;

    return node;
  }

  // returns next part before a given separator or the end of string
  // !! reference argument will be reduced by part that is returned
  inline std::string get_next_part( std::string &str, char separator ){
    std::string::size_type i = str.find( separator );
    if( i == std::string::npos ) // separator not found -> i = eos
    {
      std::string ret = str;  // return whole string
      str = "";		// empty
      return ret;
    }
    else
    {
      std::string ret = str.substr( 0, i ); 
      // return string before separator
      str = str.substr( i+1 );// erase part to return 
      return ret;
    }    
  }


  // returns next part before a given separator or the end of string
  // !! reference argument will be reduced by part that is returned
  inline std::string get_last_part( std::string &str, char separator ){
    std::string::size_type i = str.rfind( separator );
    if( i == std::string::npos ) // separator not found -> i = eos
    {
      std::string ret = str;  // return whole string
      str = "";		// empty
      return ret;
    }
    else
    {
      std::string ret = str.substr( i+1 ); 
      // return string after separator
      str = str.substr( 0, i );// erase part to return 
      return ret;
    }    
  }


  // find node according to referencing string
  Prop_Tree_Node *Prop_Tree_Node::get_referenced_node( std::string ref,
						       char separator='.' )
  {
    std::string whole_ref = ref;
    Prop_Tree_Node *cur = this;
    std::string part;
    // ref is always reduced by part
    while( (part = get_next_part(ref,separator)) != "" )
    {
      // interprete part
      if( part == "parent" )
      {
	cur = cur->parent;
	if( cur == 0 )
	{
  #warning should output better error messages
	  std::cerr << "invalid reference \"" << whole_ref << "\"" 
		    << "in last node on this level"
		    << std::endl;
	  return 0;
	}
	continue;
      }
      if( part == "next" )
      {
	cur = cur->next;
	if( cur == 0 )
	{
	  std::cerr << "invalid reference \"" << whole_ref << "\"" 
		    << std::endl;
	  return 0;
	}
	continue;
      }
      if( part == "prev" )
      {
	cur = cur->prev;
	if( cur == 0 )
	{
	  std::cerr << "invalid reference \"" << whole_ref << "\"" 
		    << std::endl;
	  return 0;
	}
	continue;
      }
      // part is no keyword -> assume it being child name
      Prop_Tree_Node *test = cur->get_child( part );
      if( test != 0 )
      {
	cur = test;
	continue;
      }
      else			// automatic search
      {
	// if no match of part -> try to go on with parent level
	cur = cur->parent;
	if( cur == 0 )
	{
	  std::cerr << "invalid reference \"" << whole_ref << "\"" 
		    << std::endl;
	  return 0;
	}
	continue;
      }

      std::cerr << "invalid reference \"" << whole_ref << "\"" 
		<< std::endl;
      return 0;
    }
    return cur;
  }


  // find node according to referencing string
  Property *Prop_Tree_Node::get_referenced_property( std::string ref,
						     char separator='.' )
  {
    std::string whole_ref = ref;
    // reduce ref by last part and save it as the name of the property
    std::string property_name = get_last_part( ref, separator );
    Prop_Tree_Node *node;
    if( ref == "" ) // no remaining node?
      node = this;
    else
      node = get_referenced_node( ref, separator );
    if( node == 0 ) return 0;	// error should have been already reported
    Property *prop = node->get_property( property_name );

    if( prop == 0 )
    {
      std::cerr << "invalid reference \"" << whole_ref << "\"" << std::endl;
      return 0;
    }

    return prop;
  }

  // adds a factory object for class generation
  char Prop_Tree_Node::add_child_factory( std::string type_name, 
					  Child_Factory* fac )
    throw() 
  {

    child_factory_type::iterator i = child_factory.find( type_name );
    // if type not found
    if( i != child_factory.end() )
      return -1;
    
    child_factory[ type_name ] = fac;
    return 0;
  }

  //! function that is called after hierarchy was set up for each node
  void Prop_Tree_Node::hierarchy_final_init()
  {
    final_init();

    Prop_Tree_Node *n;
    for( n = first_child; n != 0; n = n->next )
    {
      n->hierarchy_final_init();
    }    
  }


  //**************************
  // constructors / destructor

  Prop_Tree_Node::Prop_Tree_Node( std::string t, std::string n, Animation *a ) 
    : message::Message_Reporter( a->msg.get_consultant() ),
      parent(0), prev(0), next(0), first_child(0), last_child(0),
      type(t), name(n), pos(message::GLOB::no_position), ani(a) {}

  Prop_Tree_Node::~Prop_Tree_Node() {}

  //***************************************************************************
  // tree create test
  //***************************************************************************

  int property_tree_test()
  {
    int errors = 0;

    message::Stream_Message_Handler msg_handler(std::cerr,std::cout,std::cout);
    message::Message_Manager manager(&msg_handler);

    std::cout << std::endl;
    std::cout << "-----------------" << std::endl;
    std::cout << "Tree Node Test..." << std::endl;
    std::cout << "-----------------" << std::endl;

    std::cout << " Node name initialization..." << std::endl;
    make_all_nodes_available();
    
    std::cout << " Building data hierarchy..." << std::endl;
    std::cout << "  ani dummy_name" << std::endl;
    Animation *ani = new Animation("dummy_name", &manager);
    std::cout << "    scene testscene" << std::endl;
    Prop_Tree_Node *tscene = ani->add_child( "scene", "testscene" );
    std::cout << "      scalar testval" << std::endl;
    Prop_Tree_Node *tscalar = tscene ->add_child( "scalar", "testval" );
    std::cout << "        linear testlinear1" << std::endl;
    Prop_Tree_Node *tlinear1 = tscalar->add_child( "linear", "testlinear1" );
    std::cout << "        linear testlinear2" << std::endl;
    Prop_Tree_Node *tlinear2 = tscalar->add_child( "linear", "testlinear2" );
    
    std::cout << " Setting values..." << std::endl;
    std::cout << "  scene.filename = \"test.scene\"" << std::endl;
    tscene->set_property( "filename", values::String("test.scene") );
    
    std::cout << "  testlinear1.starttime  = 0" << std::endl;
    tlinear1->set_property( "starttime",  values::Scalar(0) );
    std::cout << "  testlinear1.endtime    = 3" << std::endl;
    tlinear1->set_property( "endtime",    values::Scalar(3) );
    std::cout << "  testlinear1.endvalue   = 2" << std::endl;
    tlinear1->set_property( "endvalue",   values::Scalar(2) );
    std::cout << "  testlinear1.difference = 1" << std::endl;
    tlinear1->set_property( "difference", values::Scalar(1) );
    std::cout << "  testlinear2.endtime    = 10" << std::endl;
    tlinear2->set_property( "endtime",    values::Scalar(10) );
    std::cout << "  testlinear2.endvalue   = 1" << std::endl;
    tlinear2->set_property( "endvalue",   values::Scalar(1) );
    
    std::cout << " Save pre results..." << std::endl;
    save_filled( "test_pre.out", ani );

    std::cout << " Run actions..." << std::endl;
    ani->pri_sys.invoke_all_Actions(); // invoke actions

    std::cout << " Save final results..." << std::endl;
    save_filled( "test.out", ani );

    return errors;
  }

}

  
