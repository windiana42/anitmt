// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#ifndef __functionality_scene_base__
#define __functionality_scene_base__

#include <list>
#include <string>
#include <map>

#include <message/message.hpp>
#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/solver.hpp>
#include <proptree/property.hpp>
#include <proptree/proptree.hpp>

#include "scalar_base.hpp"
#include "base_func.hpp"
#include "object_base.hpp"
#include "base_func.hpp"
namespace functionality
{
  // **********************
  // base type declarations
  // **********************


  // **************************
  // provider type declarations
  // **************************

  // ****************
  // provider classes

  class _pt_scene_type : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_scene_type *_tc_prev_scene_type;
    _pt_scene_type *_tc_next_scene_type;
  public:
    // ** type specific node connection **
    _pt_scene_type *get_prev_scene_type();
    _pt_scene_type *get_next_scene_type();
    void set_prev_scene_type( _pt_scene_type* );
    void set_next_scene_type( _pt_scene_type* );

    // ** result functions **
    // ** is result availible **
    // ** init functions **
    virtual void _rf_scene_type_first_init(){/*optional*/}
    virtual void _rf_scene_type_last_init(){/*optional*/}
    // ** constructor **
    _pt_scene_type( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_scene_type() {}
  };

  // *****************
  // container classes

  class _container_scene_type
  {
  public:
    typedef std::list<_pt_scene_type*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    typedef std::map<std::string, proptree::Basic_Node_Factory<_pt_scene_type>*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_scene_type>* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _container_scene_type(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_container_scene_type() {}
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };


  // ***************************
  // tree node type declarations
  // ***************************

  // *****************************
  // node type root

  class node_root
    : virtual public proptree::Prop_Tree_Node
  {
  protected:
    // ** properties **

    // ** operands **

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
    // ** child container **
    _container_scene_type _cn_scene_type; // 

    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common) for each node
    virtual void common_init();

    // ** result functions **

    // ** infrastructure functions **
    static void make_availible();
    // ** constructor **
    node_root( std::string name, proptree::tree_info *info, 
      message::Message_Consultant *msg_consultant );
  };

  // *****************************
  // node type scene

  class node_scene
    : virtual public proptree::Prop_Tree_Node, 
      public _pt_scene_type
  {
  protected:
    // ** properties **
    proptree::String_Property _op_filename;
    proptree::String_Property _op_scene_type;

    // ** operands **

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
    // ** child container **
    _container_object_component _cn_object_component; // 
    _container_scalar_component _cn_scalar_component; // 

    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common) for each node
    virtual void common_init();

    // ** result functions **

    // ** infrastructure functions **
    static void make_availible();
    // ** constructor **
    node_scene( std::string name, proptree::tree_info *info, 
      message::Message_Consultant *msg_consultant );
  };

  // *****************************
  // make nodes availible 

  void make_scene_base_nodes_availible();
}
#endif
