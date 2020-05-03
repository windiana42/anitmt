/*****************************************************************************/
/**   This file offers a tree structure for groups of properties	    **/
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

#include "proptree.hpp"

#include <message/message.hpp>
#include <utl/stdextend.hpp>

#include <string>
#include <assert.h>

namespace proptree
{
  //************************************************************
  // tree_info: stores information needed by any tree node
  //************************************************************

  std::string tree_info::get_unique_id()
  {
    return std::to_string(int(++id_counter));
  }
  
  tree_info::tree_info( solve::Priority_System *sys, Semi_Global *glob ) 
    : id_counter(0), priority_system(sys), GLOB(glob) {}

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  //**********
  // constants

  //**********
  // functions

  std::string Prop_Tree_Node::get_name(){
    return name;
  }

  std::string Prop_Tree_Node::get_type(){
    return type_name;
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

  Prop_Tree_Node* Prop_Tree_Node::get_child( int n )
  {
    Prop_Tree_Node *i;
    int z;
    for( i = first_child, z=0; i != 0; i = i->next, ++z )
    {
      if( z == n )
	return i;
    }
    return 0;
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
      name = "__" + type + info->get_unique_id(); 
    }

    Prop_Tree_Node *node = try_add_child( type, name );

    // link node in hierarchy on prop_tree_node level
    if( node )			
    {
      node->parent = this;
      if( first_child == 0 ) 
      {
	assert( last_child == 0 );

	first_child = node;
	last_child = node;
      }
      else
      {
	assert( last_child != 0 );
	// connect with last child
	node->prev = last_child;
	last_child->next = node;
	// replace last child
	last_child = node;
      }
    }

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
						       char separator )
  {
    std::string whole_ref = ref;
    Prop_Tree_Node *cur = this;
    std::string part;

    bool first = true;
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
	first = false;
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
	first = false;
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
	first = false;
	continue;
      }
      // part is no keyword -> assume it being child name
      Prop_Tree_Node *test = 0;
      do{
	test = cur->get_child( part );
	if( test != 0 ) break;
	if( !first )		// allow upwards child search only for first
	  break;		// part
	cur = cur->parent;	// search recursively upwards
      }while( cur != 0 );

      if( test != 0 )		// valid child name found?
      {
	cur = test;		// child is new current
	first = false;
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
						     char separator )
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

  //! function that is called after hierarchy was set up for each node
  void Prop_Tree_Node::hierarchy_final_init()
  {
    common_init();

    Prop_Tree_Node *n;
    for( n = first_child; n != 0; n = n->next )
    {
      n->hierarchy_final_init();
    }    
    custom_hierarchy_final_init();
  }


  //**************************
  // constructors / destructor

  Prop_Tree_Node::Prop_Tree_Node( std::string t, std::string n, tree_info *i,
				  message::Message_Consultant *msg_consultant) 
    : message::Message_Reporter( msg_consultant ),
      parent(0), prev(0), next(0), first_child(0), last_child(0),
      type_name(t), name(n), pos(message::GLOB::no_position), info(i) {}

  Prop_Tree_Node::~Prop_Tree_Node() 
  {
    if(pos!=message::GLOB::no_position) delete pos;
    // bidirectional delete of delete hierarchy
    if( first_child ) delete first_child;
    if( next ) delete next;
  }

  //**********************************************
  // Child_Manager: provides hierarchy traversal. 
  //**********************************************
  
  void Child_Manager::set_root_node( Prop_Tree_Node *node )
  {
    last_child.push(node);		// node is parent...
    last_child.push(no_child);	// ... so add a virtual child
    initialized = true;
  }

      Prop_Tree_Node *Child_Manager::get_child()
  {
    assert( initialized );	// there must be at least a no_child

    Prop_Tree_Node *last = last_child.top(); // get last child
    last_child.pop();		// remove old child
    Prop_Tree_Node *new_child;
    if( last != no_child )
    {
      new_child = last->get_next(); assert( new_child != 0 );
    }
    else
    {
      Prop_Tree_Node *parent = last_child.top();
      new_child = parent->get_first_child();
    }
    last_child.push(new_child);
    last_child.push(no_child);
    return new_child;
  }

  void Child_Manager::child_finished()
  {
    assert( initialized );	// there must be at least a no_child

    last_child.pop();
    assert( !last_child.empty() ); // it's not allowed to finish root node
  }
  
  Prop_Tree_Node *Child_Manager::no_child = 0; // static initialization

  //***************************************************************************
  // tree create test
  //***************************************************************************
  /*
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
  */
}

  
