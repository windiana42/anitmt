// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#ifndef __functionality_object_base__
#define __functionality_object_base__

#include <list>
#include <string>
#include <map>
#include <math.h>

#include <message/message.hpp>
#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/solver.hpp>
#include <proptree/property.hpp>
#include <proptree/proptree.hpp>

#include "base_func.hpp"
#include "solver.hpp"
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

  class _pt_object_component : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_object_component *_tc_prev_object_component;
    _pt_object_component *_tc_next_object_component;
  public:
    // ** type specific node connection **
    _pt_object_component *get_prev_object_component();
    _pt_object_component *get_next_object_component();
    void set_prev_object_component( _pt_object_component* );
    void set_next_object_component( _pt_object_component* );

    // ** result functions **
    virtual std::pair< bool,front > _rf_object_component_front_time( time ) = 0;
    virtual std::pair< bool,position > _rf_object_component_position_time( time ) = 0;
    virtual std::pair< bool,rotation > _rf_object_component_rotation_time( time ) = 0;
    virtual std::pair< bool,speed > _rf_object_component_speed_time( time ) = 0;
    virtual std::pair< bool,translation > _rf_object_component_translation_time( time ) = 0;
    virtual std::pair< bool,up_vector > _rf_object_component_up_vector_time( time ) = 0;
    // ** is result availible **
    solve::Operand<bool> _av_object_component_front_time_is_avail;
    solve::Operand<bool> _av_object_component_position_time_is_avail;
    solve::Operand<bool> _av_object_component_rotation_time_is_avail;
    solve::Operand<bool> _av_object_component_speed_time_is_avail;
    solve::Operand<bool> _av_object_component_translation_time_is_avail;
    solve::Operand<bool> _av_object_component_up_vector_time_is_avail;
    // ** init functions **
    virtual void _rf_object_component_first_init(){/*optional*/}
    virtual void _rf_object_component_last_init(){/*optional*/}
    // ** constructor **
    _pt_object_component( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_object_component() {}
  };

  class _pt_object_state : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_object_state *_tc_prev_object_state;
    _pt_object_state *_tc_next_object_state;
  public:
    // ** type specific node connection **
    _pt_object_state *get_prev_object_state();
    _pt_object_state *get_next_object_state();
    void set_prev_object_state( _pt_object_state* );
    void set_next_object_state( _pt_object_state* );

    // ** result functions **
    virtual std::pair< bool,acceleration > _rf_object_state_acceleration_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_object_state_acceleration_stretch_start_param;
    solve::Operand< stretch > _pr_object_state_acceleration_stretch_end_param;
    virtual std::pair< bool,acceleration > _rf_object_state_acceleration_time( time ) = 0;
    solve::Operand< time > _pr_object_state_acceleration_time_start_param;
    solve::Operand< time > _pr_object_state_acceleration_time_end_param;
    virtual std::pair< bool,direction > _rf_object_state_direction_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_object_state_direction_stretch_start_param;
    solve::Operand< stretch > _pr_object_state_direction_stretch_end_param;
    virtual std::pair< bool,direction > _rf_object_state_direction_time( time ) = 0;
    solve::Operand< time > _pr_object_state_direction_time_start_param;
    solve::Operand< time > _pr_object_state_direction_time_end_param;
    virtual std::pair< bool,front > _rf_object_state_front_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_object_state_front_stretch_start_param;
    solve::Operand< stretch > _pr_object_state_front_stretch_end_param;
    virtual std::pair< bool,front > _rf_object_state_front_time( time ) = 0;
    solve::Operand< time > _pr_object_state_front_time_start_param;
    solve::Operand< time > _pr_object_state_front_time_end_param;
    virtual std::pair< bool,position > _rf_object_state_position_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_object_state_position_stretch_start_param;
    solve::Operand< stretch > _pr_object_state_position_stretch_end_param;
    virtual std::pair< bool,position > _rf_object_state_position_time( time ) = 0;
    solve::Operand< time > _pr_object_state_position_time_start_param;
    solve::Operand< time > _pr_object_state_position_time_end_param;
    virtual std::pair< bool,speed > _rf_object_state_speed_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_object_state_speed_stretch_start_param;
    solve::Operand< stretch > _pr_object_state_speed_stretch_end_param;
    virtual std::pair< bool,speed > _rf_object_state_speed_time( time ) = 0;
    solve::Operand< time > _pr_object_state_speed_time_start_param;
    solve::Operand< time > _pr_object_state_speed_time_end_param;
    virtual std::pair< bool,stretch > _rf_object_state_stretch_time( time ) = 0;
    solve::Operand< time > _pr_object_state_stretch_time_start_param;
    solve::Operand< time > _pr_object_state_stretch_time_end_param;
    virtual std::pair< bool,up_vector > _rf_object_state_up_vector_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_object_state_up_vector_stretch_start_param;
    solve::Operand< stretch > _pr_object_state_up_vector_stretch_end_param;
    virtual std::pair< bool,up_vector > _rf_object_state_up_vector_time( time ) = 0;
    solve::Operand< time > _pr_object_state_up_vector_time_start_param;
    solve::Operand< time > _pr_object_state_up_vector_time_end_param;
    // ** is result availible **
    solve::Operand<bool> _av_object_state_acceleration_stretch_is_avail;
    solve::Operand<bool> _av_object_state_acceleration_time_is_avail;
    solve::Operand<bool> _av_object_state_direction_stretch_is_avail;
    solve::Operand<bool> _av_object_state_direction_time_is_avail;
    solve::Operand<bool> _av_object_state_front_stretch_is_avail;
    solve::Operand<bool> _av_object_state_front_time_is_avail;
    solve::Operand<bool> _av_object_state_position_stretch_is_avail;
    solve::Operand<bool> _av_object_state_position_time_is_avail;
    solve::Operand<bool> _av_object_state_speed_stretch_is_avail;
    solve::Operand<bool> _av_object_state_speed_time_is_avail;
    solve::Operand<bool> _av_object_state_stretch_time_is_avail;
    solve::Operand<bool> _av_object_state_up_vector_stretch_is_avail;
    solve::Operand<bool> _av_object_state_up_vector_time_is_avail;
    // ** init functions **
    virtual void _rf_object_state_first_init(){/*optional*/}
    virtual void _rf_object_state_last_init(){/*optional*/}
    // ** constructor **
    _pt_object_state( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_object_state() {}
  };

  class _pt_timing : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_timing *_tc_prev_timing;
    _pt_timing *_tc_next_timing;
  public:
    // ** type specific node connection **
    _pt_timing *get_prev_timing();
    _pt_timing *get_next_timing();
    void set_prev_timing( _pt_timing* );
    void set_next_timing( _pt_timing* );

    // ** result functions **
    virtual std::pair< bool,acceleration > _rf_timing_acceleration_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_timing_acceleration_stretch_start_param;
    solve::Operand< stretch > _pr_timing_acceleration_stretch_end_param;
    virtual std::pair< bool,acceleration > _rf_timing_acceleration_time( time ) = 0;
    solve::Operand< time > _pr_timing_acceleration_time_start_param;
    solve::Operand< time > _pr_timing_acceleration_time_end_param;
    virtual std::pair< bool,speed > _rf_timing_speed_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_timing_speed_stretch_start_param;
    solve::Operand< stretch > _pr_timing_speed_stretch_end_param;
    virtual std::pair< bool,speed > _rf_timing_speed_time( time ) = 0;
    solve::Operand< time > _pr_timing_speed_time_start_param;
    solve::Operand< time > _pr_timing_speed_time_end_param;
    virtual std::pair< bool,stretch > _rf_timing_stretch_time( time ) = 0;
    solve::Operand< time > _pr_timing_stretch_time_start_param;
    solve::Operand< time > _pr_timing_stretch_time_end_param;
    // ** is result availible **
    solve::Operand<bool> _av_timing_acceleration_stretch_is_avail;
    solve::Operand<bool> _av_timing_acceleration_time_is_avail;
    solve::Operand<bool> _av_timing_speed_stretch_is_avail;
    solve::Operand<bool> _av_timing_speed_time_is_avail;
    solve::Operand<bool> _av_timing_stretch_time_is_avail;
    // ** init functions **
    virtual void _rf_timing_first_init(){/*optional*/}
    virtual void _rf_timing_last_init(){/*optional*/}
    // ** constructor **
    _pt_timing( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_timing() {}
  };

  class _pt_track : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_track *_tc_prev_track;
    _pt_track *_tc_next_track;
  public:
    // ** type specific node connection **
    _pt_track *get_prev_track();
    _pt_track *get_next_track();
    void set_prev_track( _pt_track* );
    void set_next_track( _pt_track* );

    // ** result functions **
    virtual std::pair< bool,direction > _rf_track_direction_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_track_direction_stretch_start_param;
    solve::Operand< stretch > _pr_track_direction_stretch_end_param;
    virtual std::pair< bool,position > _rf_track_position_stretch( stretch ) = 0;
    solve::Operand< stretch > _pr_track_position_stretch_start_param;
    solve::Operand< stretch > _pr_track_position_stretch_end_param;
    // ** is result availible **
    solve::Operand<bool> _av_track_direction_stretch_is_avail;
    solve::Operand<bool> _av_track_position_stretch_is_avail;
    // ** init functions **
    virtual void _rf_track_first_init(){/*optional*/}
    virtual void _rf_track_last_init(){/*optional*/}
    // ** constructor **
    _pt_track( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_track() {}
  };

  // *****************
  // container classes

  class _container_object_component
  {
  public:
    typedef std::list<_pt_object_component*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    typedef std::map<std::string, proptree::Basic_Node_Factory< _pt_object_component >*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory< _pt_object_component >* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _container_object_component(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_container_object_component() {}
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };

  class _serial_container_object_state
  {
  public:
    typedef std::list<_pt_object_state*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    solve::Multi_And_Operator *avail_operator_acceleration_stretch;
    solve::Multi_And_Operator *avail_operator_acceleration_time;
    solve::Multi_And_Operator *avail_operator_direction_stretch;
    solve::Multi_And_Operator *avail_operator_direction_time;
    solve::Multi_And_Operator *avail_operator_front_stretch;
    solve::Multi_And_Operator *avail_operator_front_time;
    solve::Multi_And_Operator *avail_operator_position_stretch;
    solve::Multi_And_Operator *avail_operator_position_time;
    solve::Multi_And_Operator *avail_operator_speed_stretch;
    solve::Multi_And_Operator *avail_operator_speed_time;
    solve::Multi_And_Operator *avail_operator_stretch_time;
    solve::Multi_And_Operator *avail_operator_up_vector_stretch;
    solve::Multi_And_Operator *avail_operator_up_vector_time;
    typedef std::map<std::string, proptree::Basic_Node_Factory< _pt_object_state >*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory< _pt_object_state >* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    virtual std::pair< bool,acceleration > _rf_object_state_acceleration_stretch( stretch );
    virtual std::pair< bool,acceleration > _rf_object_state_acceleration_time( time );
    virtual std::pair< bool,direction > _rf_object_state_direction_stretch( stretch );
    virtual std::pair< bool,direction > _rf_object_state_direction_time( time );
    virtual std::pair< bool,front > _rf_object_state_front_stretch( stretch );
    virtual std::pair< bool,front > _rf_object_state_front_time( time );
    virtual std::pair< bool,position > _rf_object_state_position_stretch( stretch );
    virtual std::pair< bool,position > _rf_object_state_position_time( time );
    virtual std::pair< bool,speed > _rf_object_state_speed_stretch( stretch );
    virtual std::pair< bool,speed > _rf_object_state_speed_time( time );
    virtual std::pair< bool,stretch > _rf_object_state_stretch_time( time );
    virtual std::pair< bool,up_vector > _rf_object_state_up_vector_stretch( stretch );
    virtual std::pair< bool,up_vector > _rf_object_state_up_vector_time( time );
    // ** is result availible **
    solve::Operand<bool> _av_object_state_acceleration_stretch_is_avail;
    solve::Operand<bool> _av_object_state_acceleration_time_is_avail;
    solve::Operand<bool> _av_object_state_direction_stretch_is_avail;
    solve::Operand<bool> _av_object_state_direction_time_is_avail;
    solve::Operand<bool> _av_object_state_front_stretch_is_avail;
    solve::Operand<bool> _av_object_state_front_time_is_avail;
    solve::Operand<bool> _av_object_state_position_stretch_is_avail;
    solve::Operand<bool> _av_object_state_position_time_is_avail;
    solve::Operand<bool> _av_object_state_speed_stretch_is_avail;
    solve::Operand<bool> _av_object_state_speed_time_is_avail;
    solve::Operand<bool> _av_object_state_stretch_time_is_avail;
    solve::Operand<bool> _av_object_state_up_vector_stretch_is_avail;
    solve::Operand<bool> _av_object_state_up_vector_time_is_avail;
    // ** access functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _serial_container_object_state(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_serial_container_object_state() {}
    // ** Don't call the following functions! ** 
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };

  class _serial_container_timing
  {
  public:
    typedef std::list<_pt_timing*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    solve::Multi_And_Operator *avail_operator_acceleration_stretch;
    solve::Multi_And_Operator *avail_operator_acceleration_time;
    solve::Multi_And_Operator *avail_operator_speed_stretch;
    solve::Multi_And_Operator *avail_operator_speed_time;
    solve::Multi_And_Operator *avail_operator_stretch_time;
    typedef std::map<std::string, proptree::Basic_Node_Factory< _pt_timing >*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory< _pt_timing >* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    virtual std::pair< bool,acceleration > _rf_timing_acceleration_stretch( stretch );
    virtual std::pair< bool,acceleration > _rf_timing_acceleration_time( time );
    virtual std::pair< bool,speed > _rf_timing_speed_stretch( stretch );
    virtual std::pair< bool,speed > _rf_timing_speed_time( time );
    virtual std::pair< bool,stretch > _rf_timing_stretch_time( time );
    // ** is result availible **
    solve::Operand<bool> _av_timing_acceleration_stretch_is_avail;
    solve::Operand<bool> _av_timing_acceleration_time_is_avail;
    solve::Operand<bool> _av_timing_speed_stretch_is_avail;
    solve::Operand<bool> _av_timing_speed_time_is_avail;
    solve::Operand<bool> _av_timing_stretch_time_is_avail;
    // ** access functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _serial_container_timing(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_serial_container_timing() {}
    // ** Don't call the following functions! ** 
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };

  class _serial_container_track
  {
  public:
    typedef std::list<_pt_track*>elements_type;
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    elements_type elements;
    solve::Multi_And_Operator *avail_operator_direction_stretch;
    solve::Multi_And_Operator *avail_operator_position_stretch;
    typedef std::map<std::string, proptree::Basic_Node_Factory< _pt_track >*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory< _pt_track >* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    virtual std::pair< bool,direction > _rf_track_direction_stretch( stretch );
    virtual std::pair< bool,position > _rf_track_position_stretch( stretch );
    // ** is result availible **
    solve::Operand<bool> _av_track_direction_stretch_is_avail;
    solve::Operand<bool> _av_track_position_stretch_is_avail;
    // ** access functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _serial_container_track(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_serial_container_track() {}
    // ** Don't call the following functions! ** 
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };


  // ********************
  // ********************
  // operator declartions
  // ********************
  // ********************

}
namespace solve
{

}
namespace functionality
{

  // ***************************
  // tree node type declarations
  // ***************************

  // *****************************
  // node type object

  class node_object
    : virtual public proptree::Prop_Tree_Node, 
      public _pt_object_component, 
      public _pt_object_state
  {
  protected:
    // ** properties **
    proptree::Vector_Property _op_center;
    proptree::Vector_Property _op_front;
    proptree::Vector_Property _op_up_vector;

    // ** operands **

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
    // ** child container **
    _serial_container_object_state _cn_object_state; // min1

    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common) for each node
    virtual void common_init();

    // ** result functions **
    virtual std::pair< bool,front > _rf_object_component_front_time( time );
    virtual std::pair< bool,position > _rf_object_component_position_time( time );
    virtual std::pair< bool,rotation > _rf_object_component_rotation_time( time );
    virtual std::pair< bool,speed > _rf_object_component_speed_time( time );
    virtual std::pair< bool,translation > _rf_object_component_translation_time( time );
    virtual std::pair< bool,up_vector > _rf_object_component_up_vector_time( time );
    virtual std::pair< bool,acceleration > _rf_object_state_acceleration_stretch( stretch );
    virtual std::pair< bool,acceleration > _rf_object_state_acceleration_time( time );
    virtual std::pair< bool,direction > _rf_object_state_direction_stretch( stretch );
    virtual std::pair< bool,direction > _rf_object_state_direction_time( time );
    virtual std::pair< bool,front > _rf_object_state_front_stretch( stretch );
    virtual std::pair< bool,front > _rf_object_state_front_time( time );
    virtual std::pair< bool,position > _rf_object_state_position_stretch( stretch );
    virtual std::pair< bool,position > _rf_object_state_position_time( time );
    virtual std::pair< bool,speed > _rf_object_state_speed_stretch( stretch );
    virtual std::pair< bool,speed > _rf_object_state_speed_time( time );
    virtual std::pair< bool,stretch > _rf_object_state_stretch_time( time );
    virtual std::pair< bool,up_vector > _rf_object_state_up_vector_stretch( stretch );
    virtual std::pair< bool,up_vector > _rf_object_state_up_vector_time( time );

    // ** infrastructure functions **
    static void make_availible();
    // ** constructor **
    node_object( std::string name, proptree::tree_info *info, 
      message::Message_Consultant *msg_consultant );
  };

  // *****************************
  // make nodes availible 

  void make_object_base_nodes_availible();
}
#endif
