// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#ifndef __functionality_scalar_base__
#define __functionality_scalar_base__

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

  class _pt_scalar_component : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_scalar_component *_tc_prev_scalar_component;
    _pt_scalar_component *_tc_next_scalar_component;
  public:
    // ** type specific node connection **
    _pt_scalar_component *get_prev_scalar_component();
    _pt_scalar_component *get_next_scalar_component();
    void set_prev_scalar_component( _pt_scalar_component* );
    void set_next_scalar_component( _pt_scalar_component* );

    // ** result functions **
    virtual std::pair<bool,value> _rf_scalar_component_value_time( time ) = 0;
    // ** is result availible **
    solve::Operand<bool> _av_scalar_component_value_time_is_avail;
    // ** init functions **
    virtual void _rf_scalar_component_first_init(){/*optional*/}
    virtual void _rf_scalar_component_last_init(){/*optional*/}
    // ** constructor **
    _pt_scalar_component( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_scalar_component() {}
  };

  class _pt_scalar_vsa : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_scalar_vsa *_tc_prev_scalar_vsa;
    _pt_scalar_vsa *_tc_next_scalar_vsa;
  public:
    // ** type specific node connection **
    _pt_scalar_vsa *get_prev_scalar_vsa();
    _pt_scalar_vsa *get_next_scalar_vsa();
    void set_prev_scalar_vsa( _pt_scalar_vsa* );
    void set_next_scalar_vsa( _pt_scalar_vsa* );

    // ** result functions **
    virtual std::pair<bool,acceleration> _rf_scalar_vsa_acceleration_time( time ) = 0;
    solve::Operand<time> _pr_scalar_vsa_acceleration_time_start_param;
    solve::Operand<time> _pr_scalar_vsa_acceleration_time_end_param;
    virtual std::pair<bool,slope> _rf_scalar_vsa_slope_time( time ) = 0;
    solve::Operand<time> _pr_scalar_vsa_slope_time_start_param;
    solve::Operand<time> _pr_scalar_vsa_slope_time_end_param;
    virtual std::pair<bool,value> _rf_scalar_vsa_value_time( time ) = 0;
    solve::Operand<time> _pr_scalar_vsa_value_time_start_param;
    solve::Operand<time> _pr_scalar_vsa_value_time_end_param;
    // ** is result availible **
    solve::Operand<bool> _av_scalar_vsa_acceleration_time_is_avail;
    solve::Operand<bool> _av_scalar_vsa_slope_time_is_avail;
    solve::Operand<bool> _av_scalar_vsa_value_time_is_avail;
    // ** init functions **
    virtual void _rf_scalar_vsa_first_init(){/*optional*/}
    virtual void _rf_scalar_vsa_last_init(){/*optional*/}
    // ** constructor **
    _pt_scalar_vsa( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_scalar_vsa() {}
  };

  // *****************
  // container classes

  class _container_scalar_component
  {
  public:
    typedef std::list<_pt_scalar_component*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    typedef std::map<std::string, proptree::Basic_Node_Factory<_pt_scalar_component>*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_scalar_component>* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _container_scalar_component(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_container_scalar_component() {}
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };

  class _serial_container_scalar_vsa
  {
  public:
    typedef std::list<_pt_scalar_vsa*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    solve::Multi_And_Operator *avail_operator_acceleration_time;
    solve::Multi_And_Operator *avail_operator_slope_time;
    solve::Multi_And_Operator *avail_operator_value_time;
    typedef std::map<std::string, proptree::Basic_Node_Factory<_pt_scalar_vsa>*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_scalar_vsa>* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    virtual std::pair<bool,acceleration> _rf_scalar_vsa_acceleration_time( time );
    virtual std::pair<bool,slope> _rf_scalar_vsa_slope_time( time );
    virtual std::pair<bool,value> _rf_scalar_vsa_value_time( time );
    // ** is result availible **
    solve::Operand<bool> _av_scalar_vsa_acceleration_time_is_avail;
    solve::Operand<bool> _av_scalar_vsa_slope_time_is_avail;
    solve::Operand<bool> _av_scalar_vsa_value_time_is_avail;
    // ** access functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _serial_container_scalar_vsa(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_serial_container_scalar_vsa() {}
    // ** Don't call the following functions! ** 
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };


  // ***************************
  // tree node type declarations
  // ***************************

  // *****************************
  // node type scalar

  class node_scalar
    : virtual public proptree::Prop_Tree_Node, 
      public _pt_scalar_component, 
      public _pt_scalar_vsa
  {
  protected:
    // ** properties **
    proptree::Scalar_Property _op_difference;
    proptree::Scalar_Property _op_duration;
    proptree::Scalar_Property _op_end_slope;
    proptree::Scalar_Property _op_end_time;
    proptree::Scalar_Property _op_end_value;
    proptree::Scalar_Property _op_slope_difference;
    proptree::Scalar_Property _op_start_slope;
    proptree::Scalar_Property _op_start_time;
    proptree::Scalar_Property _op_start_value;

    // ** operands **

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
    // ** child container **
    _serial_container_scalar_vsa _cn_scalar_vsa; // min1

    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common) for each node
    virtual void common_init();
    virtual void _rf_scalar_vsa_first_init();

    // ** result functions **
    virtual std::pair<bool,value> _rf_scalar_component_value_time( time );
    virtual std::pair<bool,acceleration> _rf_scalar_vsa_acceleration_time( time );
    virtual std::pair<bool,slope> _rf_scalar_vsa_slope_time( time );
    virtual std::pair<bool,value> _rf_scalar_vsa_value_time( time );

    // ** infrastructure functions **
    static void make_availible();
    // ** constructor **
    node_scalar( std::string name, proptree::tree_info *info, 
      message::Message_Consultant *msg_consultant );
  };

  // *****************************
  // make nodes availible 

  void make_scalar_base_nodes_availible();
}
#endif
