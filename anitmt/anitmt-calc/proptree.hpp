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

#ifndef __AniTMT_Prop_Tree__
#define __AniTMT_Prop_Tree__

#include <list>
#include <map>
#include <vector>

namespace anitmt{
  class Prop_Tree_Node;
  class Priority_Action;
}

#include "val.hpp"
#include "property.hpp"

namespace anitmt{

  //************************************************************
  // Prop_Tree_Node: provides tree structure for property groups
  //************************************************************

  // type for a list of involved nodes
  typedef std::list< Prop_Tree_Node* > involve_list_type; 

  class Prop_Tree_Node{
    // navigation in tree
    Prop_Tree_Node *parent;	// parent node
    Prop_Tree_Node *prev;	// previous node
    Prop_Tree_Node *next;	// next node
    Prop_Tree_Node *first_child;// first child node
    Prop_Tree_Node *last_child; // last child node

    std::string type;		// type of this node
    std::string name;		// name for reference

    //************
    // properties
    typedef std::map< std::string, Property* > properties_type;
    properties_type properties;

    // priority levels for actions like a transmission of properties to 
    // neighbour nodes or like setting default values
    typedef std::map< int, Priority_Action* > priority_levels_type;
    priority_levels_type priority_level;

    //********
    // childs
    virtual bool try_add_child( Prop_Tree_Node *node ) = 0;

    // abstract child factory:
    class Child_Factory{
    public:
      virtual Prop_Tree_Node *create( std::string name ) = 0;
    };

    // map of child fatories associated to a name as string 
    typedef std::map< std::string, Child_Factory* > child_factory_type;
    static child_factory_type child_factory;

  protected:
    // Access functions for hiding data structure
    inline void add_property( std::string name, Property *prop ){
      properties[ name ] = prop;
    }

    inline void add_action( int level, Priority_Action *act ){
      priority_level[ level ] = act;
    }

  public:
    // known exceptions
    class exception_child_type_unkown {};
    class exception_child_type_rejected {};
    class exception_child_type_already_defined {};
    class exception_unknown_property {};
    class exception_wrong_property_type {};
    class exception_property_rejected {};

    std::string	get_name();	// return name
    std::string get_type();	// return type
    Property *get_property( std::string name );	
				// return property
    std::list<std::string> get_properties_names(); 
				// returns all property names

    Prop_Tree_Node *get_child( std::string name );
				// return child with name
    std::list<Prop_Tree_Node*> get_all_childs();
				// return all childs
    Prop_Tree_Node *add_child( std::string type, std::string name )
      throw( exception_child_type_unkown, exception_child_type_rejected );
				// add child of type with name
    virtual Prop_Tree_Node *get_referenced_node( std::string ref );
				// returns node by interpreting the hierachical
 				// reference string. 

    static void add_child_factory( std::string name, Child_Factory* fac )
      throw( exception_child_type_already_defined );

    Prop_Tree_Node( std::string type, std::string name );
    virtual ~Prop_Tree_Node();

    //-- Templates

    // set property
    template< class T >  
    void set_property( std::string name, T val ) 
      throw( exception_unknown_property,exception_wrong_property_type ) {

      if( get_property( name ) != 0 )
	{
	  Type_Property<T> *p = 
	    dynamic_cast< Type_Property<T>* >( properties[ name ] );

	  if( !p )
	    throw exception_wrong_property_type();

	  bool accepted = p->set_if_ok( val );
	  if( !accepted )
	    throw exception_property_rejected();
	}
      else
	throw exception_unknown_property();
    }
  };

  // child factory template
  template< class NT>
  class Node_Factory : public Prop_Tree_Node::Child_Factory{
  public:
    virtual Prop_Tree_Node *create( std::string name ){
      return new NT( name );
    }
  };
  
  class Priority_Action{
  public:
    virtual involve_list_type do_it() = 0;
    
    virtual ~Priority_Action() {}
  };

  template<class T>
  class Default_Value : public Priority_Action{
    Type_Property<T> *prop;
    T val;
    Prop_Tree_Node *node;
  public:
    virtual involve_list_type do_it(){ 
      //node->clear_involve_list();
      prop->set_if_ok( val ); 
      //return node->get_involve_list();
    }

    Default_Value( Type_Property<T> *p, T v, Prop_Tree_Node *n ) 
      : prop(p), val(v), node(n){}
  };
}
#endif

