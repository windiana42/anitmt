// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#ifndef __functionality_base_func__
#define __functionality_base_func__

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

namespace functionality
{
  // **********************
  // base type declarations
  // **********************

  typedef values::Vector vector;
  typedef values::Scalar scalar;
  typedef scalar time;
  typedef scalar value;
  typedef scalar slope;
  typedef scalar acceleration;
  typedef scalar stretch;
  typedef vector position;
  typedef vector direction;
  typedef vector up_vector;
  struct orientation
  {
    direction dir;
    up_vector up;
  };

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
    virtual std::pair<bool,orientation> _rf_object_component_orientation_time( time ) = 0;
    virtual std::pair<bool,position> _rf_object_component_position_time( time ) = 0;
    // ** is result availible **
    solve::Operand<bool> _av_object_component_orientation_time_is_avail;
    solve::Operand<bool> _av_object_component_position_time_is_avail;
    // ** init functions **
    virtual void _rf_object_component_first_init(){/*optional*/}
    virtual void _rf_object_component_last_init(){/*optional*/}
    // ** constructor **
    _pt_object_component( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_object_component() {}
  };

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

  class _pt_space_state : virtual public proptree::Prop_Tree_Node
  {
  private:
    // ** type specific node connection **
    _pt_space_state *_tc_prev_space_state;
    _pt_space_state *_tc_next_space_state;
  public:
    // ** type specific node connection **
    _pt_space_state *get_prev_space_state();
    _pt_space_state *get_next_space_state();
    void set_prev_space_state( _pt_space_state* );
    void set_next_space_state( _pt_space_state* );

    // ** result functions **
    virtual std::pair<bool,orientation> _rf_space_state_orientation_stretch( stretch ) = 0;
    solve::Operand<stretch> _pr_space_state_orientation_stretch_start_param;
    solve::Operand<stretch> _pr_space_state_orientation_stretch_end_param;
    virtual std::pair<bool,orientation> _rf_space_state_orientation_time( time ) = 0;
    solve::Operand<time> _pr_space_state_orientation_time_start_param;
    solve::Operand<time> _pr_space_state_orientation_time_end_param;
    virtual std::pair<bool,position> _rf_space_state_position_stretch( stretch ) = 0;
    solve::Operand<stretch> _pr_space_state_position_stretch_start_param;
    solve::Operand<stretch> _pr_space_state_position_stretch_end_param;
    virtual std::pair<bool,position> _rf_space_state_position_time( time ) = 0;
    solve::Operand<time> _pr_space_state_position_time_start_param;
    solve::Operand<time> _pr_space_state_position_time_end_param;
    // ** is result availible **
    solve::Operand<bool> _av_space_state_orientation_stretch_is_avail;
    solve::Operand<bool> _av_space_state_orientation_time_is_avail;
    solve::Operand<bool> _av_space_state_position_stretch_is_avail;
    solve::Operand<bool> _av_space_state_position_time_is_avail;
    // ** init functions **
    virtual void _rf_space_state_first_init(){/*optional*/}
    virtual void _rf_space_state_last_init(){/*optional*/}
    // ** constructor **
    _pt_space_state( message::Message_Consultant *consultant );
    // ** virtual destructor **
    virtual ~_pt_space_state() {}
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
    virtual std::pair<bool,position> _rf_track_position_stretch( stretch ) = 0;
    solve::Operand<stretch> _pr_track_position_stretch_start_param;
    solve::Operand<stretch> _pr_track_position_stretch_end_param;
    // ** is result availible **
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
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    typedef std::list<_pt_object_component*>elements_type;
    elements_type elements;
    typedef std::map<std::string, proptree::Basic_Node_Factory<_pt_object_component>*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_object_component>* );
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

  class _container_scalar_component
  {
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    typedef std::list<_pt_scalar_component*>elements_type;
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
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    typedef std::list<_pt_scalar_vsa*>elements_type;
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

  class _container_scene_type
  {
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    typedef std::list<_pt_scene_type*>elements_type;
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

  class _serial_container_space_state
  {
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    typedef std::list<_pt_space_state*>elements_type;
    elements_type elements;
    solve::Multi_And_Operator *avail_operator_orientation_stretch;
    solve::Multi_And_Operator *avail_operator_orientation_time;
    solve::Multi_And_Operator *avail_operator_position_stretch;
    solve::Multi_And_Operator *avail_operator_position_time;
    typedef std::map<std::string, proptree::Basic_Node_Factory<_pt_space_state>*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_space_state>* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    virtual std::pair<bool,orientation> _rf_space_state_orientation_stretch( stretch );
    virtual std::pair<bool,orientation> _rf_space_state_orientation_time( time );
    virtual std::pair<bool,position> _rf_space_state_position_stretch( stretch );
    virtual std::pair<bool,position> _rf_space_state_position_time( time );
    // ** is result availible **
    solve::Operand<bool> _av_space_state_orientation_stretch_is_avail;
    solve::Operand<bool> _av_space_state_orientation_time_is_avail;
    solve::Operand<bool> _av_space_state_position_stretch_is_avail;
    solve::Operand<bool> _av_space_state_position_time_is_avail;
    // ** access functions **
    elements_type::iterator elements_begin(); 
    elements_type::iterator elements_end(); 
    bool elements_empty(); 
    // ** constructor **
    _serial_container_space_state(bool max1, bool min1, message::Message_Consultant* );
    // ** virtual destructor **
    virtual ~_serial_container_space_state() {}
    // ** Don't call the following functions! ** 
    //! function that is called after hierarchy was set up for each node
    void hierarchy_final_init();
  };

  class _serial_container_track
  {
  private:
    bool max1; // maximal one element
    bool min1; // minimal one element
    typedef std::list<_pt_track*>elements_type;
    elements_type elements;
    solve::Multi_And_Operator *avail_operator_position_stretch;
    typedef std::map<std::string, proptree::Basic_Node_Factory<_pt_track>*> node_factories_type;
    static node_factories_type node_factories;
  public:
    static void add_node_factory( std::string name, proptree::Basic_Node_Factory<_pt_track>* );
    proptree::Prop_Tree_Node *add_child( std::string type, std::string name, proptree::tree_info *info, message::Message_Consultant *msg, proptree::Prop_Tree_Node *already_obj );
    // ** result functions **
    virtual std::pair<bool,position> _rf_track_position_stretch( stretch );
    // ** is result availible **
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

    // ** child container **
    _container_scene_type _cn_scene_type; // 

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
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

    // ** child container **
    _serial_container_scalar_vsa _cn_scalar_vsa; // min1

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
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
  // node type scalar_base

  class node_scalar_base
    : virtual public proptree::Prop_Tree_Node
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

    // ** child container **

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common) for each node
    virtual void common_init();

    // ** result functions **

    // ** infrastructure functions **
    static void make_availible();
    // ** constructor **
    node_scalar_base( std::string name, proptree::tree_info *info, 
      message::Message_Consultant *msg_consultant );
  };

  // *****************************
  // node type scalar_subfunction

  class node_scalar_subfunction
    : virtual public proptree::Prop_Tree_Node
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

    // ** child container **

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
    // ** constraint, solver and action establishing functions **
    //! establish general dependencies (common) for each node
    virtual void common_init();

    // ** result functions **

    // ** infrastructure functions **
    static void make_availible();
    // ** constructor **
    node_scalar_subfunction( std::string name, proptree::tree_info *info, 
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

    // ** operands **

    // ** child container **
    _container_object_component _cn_object_component; // 
    _container_scalar_component _cn_scalar_component; // 

    // ** virtual tree node functions **
    virtual proptree::Prop_Tree_Node *try_add_child( std::string type, std::string name ) throw();
    //! custom initialization after hierarchy was set up
    virtual void custom_hierarchy_final_init();
  public:
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

  void make_base_func_nodes_availible();
}
#endif
