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
    Prop_Tree_Node *first_child;// first child node
    Prop_Tree_Node *next;	// next node
    Prop_Tree_Node *prev;	// previous node

    std::string type;		// type of this node
    std::string name;		// name for reference

    // properties
    typedef std::map<std::string, Property*> properties_type;
    properties_type properties;

    // priority levels for actions like a transmission of properties to 
    // neighbour nodes or like setting default values
    typedef std::vector< Priority_Action* > priority_levels_type;
    priority_levels_type priority_level;

  protected:
    // Access functions for hiding data structure
    inline void add_property( std::string name, Property *prop ){
      properties[ name ] = prop;
    }

    inline void add_action( int level, Priority_Action *act ){
      priority_level[ level ] = act;
    }

  public:
    std::string	get_name();	// return name
    std::string get_type();	// return type
    Property *get_property( std::string name );	
				// return property
    std::list<std::string> get_properties(); 
				// returns all property names

    Prop_Tree_Node *get_child( std::string name );
				// return child with name
    std::list<Prop_Tree_Node*> get_all_childs();
				// return all childs
    virtual bool is_child_type_valid( std::string type ) = 0;
				// check if type of child is valid
    virtual Prop_Tree_Node *add_child( std::string type, 
				       std::string name ) = 0;
				// add child of type with name
    virtual Prop_Tree_Node *get_referenced_node( std::string ref );
				// returns node by interpreting the hierachical
 				// reference string. 

    Prop_Tree_Node( std::string type, std::string name );
    virtual ~Prop_Tree_Node();

    //-- Templates

    class exception_unknown_property {}; // should be better integrated 

    // set property
    template< class T >  
    void set_property( std::string name, T val ) 
      throw( exception_unknown_property ) {

      if( get_property( name ) != 0 )
	properties[ name ] = val;
      else
	throw exception_unknown_property();
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

