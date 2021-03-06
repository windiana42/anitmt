/*****************************************************************************/
/**   This file offers the general animation tree node   		    **/
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

#include <config.h>

#include <functionality/scene_base.hpp>
#include <functionality/make_avail.hpp>

#include "animation.hpp"

#include "anitmt.hpp"

#include <assert.h>

namespace proptree
{
    Semi_Global::Semi_Global( param::Parameter_Manager *parameter_manager, 
			      message::Message_Consultant* c )
      : param(parameter_manager->old_sys), msg(c), final_run_info(&handler) {}
#warning only to wrap the old parameter system
}

namespace anitmt{

#if YYDEBUG
  namespace adlparser {
    extern int yydebug;
  }
#endif

  // ********************************
  // Animation: Animation root node
  // ********************************

  Animation::Animation( std::string name, 
			param::Parameter_Manager *parameter_manager,
			message::Message_Manager *manager ) 
    : GLOB( parameter_manager, 
	    new message::Message_Consultant(manager,MID_Core) ),
      tree_info( new solve::Priority_System(), &GLOB )
  { 
    ani_root_original = 
      new functionality::node_root( name, &tree_info, 
				    GLOB.msg.get_consultant() );
    ani_root = ani_root_original;
  }

  Animation::~Animation()
  {
    delete ani_root_original;
  }

  std::string Animation::get_name()
  {
    return ani_root_original->get_name();
  }

  void Animation::init()
  {
    // register animation tree nodes
    functionality::make_nodes_availible();

#if YYDEBUG
    // activate DEBUG function of parser at verbose level 5
    if( GLOB.param.verbose() >= 4 )
    {
      adlparser::yydebug = 1;
    }
#endif
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Animation::hierarchy_final_init()
  {
    ani_root_original->hierarchy_final_init();
  }
  void Animation::finish_calculations()
  {
    tree_info.priority_system->invoke_all_Actions();
  }

  // ***************************************
  // Interfaces to the animation data tree
  // ***************************************

  // ***************************
  // Scalar_Component_Interface 
  
  Scalar_Component_Interface Scalar_Component_Interface::get_next()
  {
    return ++scalar_component;
  }

  std::string Scalar_Component_Interface::get_name()
  {
    return (*scalar_component)->get_name();
  }

  std::pair<bool,values::Scalar> 
  Scalar_Component_Interface::get_value( values::Scalar t )
  {
    solve::Solve_Run_Info *info = 
      &(*scalar_component)->get_tree_info()->GLOB->final_run_info;

    return (*scalar_component)->_rf_scalar_component_value_time( t, info );
  }

  Scalar_Component_Interface::Scalar_Component_Interface
  ( functionality::_container_scalar_component::elements_type::iterator 
    scalar ) : scalar_component(scalar) {}
    
  // ***************************
  // Object_Component_Interface 
  
  Object_Component_Interface Scene_Interface::get_first_object()
  {
    functionality::node_scene *scene_node = 
      dynamic_cast<functionality::node_scene *> (*scene);
    assert( scene_node != 0 );

    return scene_node->_cn_object_component.elements_begin();
  }

  Object_Component_Interface Scene_Interface::get_object_end()
  {
    functionality::node_scene *scene_node = 
      dynamic_cast<functionality::node_scene *> (*scene);
    assert( scene_node != 0 );

    return scene_node->_cn_object_component.elements_end();
  }


  Object_Component_Interface Object_Component_Interface::get_next()
  {
    return ++object_component;
  }

  std::string Object_Component_Interface::get_name()
  {
    return (*object_component)->get_name();
  }

  std::pair<bool,Object_State> 
  Object_Component_Interface::get_state( values::Scalar t )
  {
    solve::Solve_Run_Info *info = 
      &(*object_component)->get_tree_info()->GLOB->final_run_info;

    std::pair<bool,Object_State> ret; ret.first = false;

    std::pair<bool,functionality::translation> trans = 
      (*object_component)->_rf_object_component_translation_time( t, info );
    std::pair<bool,functionality::rotation> rot = 
      (*object_component)->_rf_object_component_rotation_time( t, info );
    std::pair<bool,functionality::position> pos = 
      (*object_component)->_rf_object_component_position_time( t, info );
    std::pair<bool,functionality::front> front = 
      (*object_component)->_rf_object_component_front_time( t, info );
    std::pair<bool,functionality::up_vector> up = 
      (*object_component)->_rf_object_component_up_vector_time( t, info );

    if( !trans.first ) return ret;
    if( !rot.first   ) return ret;
    if( !pos.first   ) return ret;
    if( !front.first ) return ret;
    if( !up.first    ) return ret;

    ret.second.translate = trans.second;
    ret.second.rotate    = rot.second;
    ret.second.position  = pos.second;
    ret.second.front     = front.second;
    ret.second.up_vector = up.second;
    ret.first = true;

    return ret;
  }

  Object_Component_Interface::Object_Component_Interface
  ( functionality::_container_object_component::elements_type::iterator 
    object ) : object_component(object) {}

  // ***************************
  // Scene_Interface 
  
  Scene_Interface Scene_Interface::get_next()
  {
    return ++scene;
  }

  std::string Scene_Interface::get_name()
  {
    return (*scene)->get_name();
  }

  std::string Scene_Interface::get_filename()
  {
    proptree::String_Property *fn
      = dynamic_cast<proptree::String_Property*>
      ((*scene)->get_property("filename"));
    if( !fn ) return "<undefined>";
    if( !fn->is_solved() ) return "<unsolved>";

    return fn->get_value();
  }

  std::string Scene_Interface::get_scene_type()
  {
    proptree::String_Property *fn
      = dynamic_cast<proptree::String_Property*>
      ((*scene)->get_property("scene_type"));
    if( !fn ) return "<undefined>";
    if( !fn->is_solved() ) return "<unsolved>";

    return fn->get_value();
  }

  Scalar_Component_Interface Scene_Interface::get_first_scalar()
  {
    functionality::node_scene *scene_node = 
      dynamic_cast<functionality::node_scene *> (*scene);

    assert( scene_node != 0 );

    return scene_node->_cn_scalar_component.elements_begin();
  }

  Scalar_Component_Interface Scene_Interface::get_scalar_end()
  {
    functionality::node_scene *scene_node = 
      dynamic_cast<functionality::node_scene *> (*scene);
    assert( scene_node != 0 );

    return scene_node->_cn_scalar_component.elements_end();
  }

  Scene_Interface::Scene_Interface
  ( functionality::_container_scene_type::elements_type::iterator s )
    : scene(s) {}

  Scene_Interface Prop_Tree_Interface::get_first_scene()
  {
    return ani_root->_cn_scene_type.elements_begin();
  }

  Scene_Interface Prop_Tree_Interface::get_scene_end()
  {
    return ani_root->_cn_scene_type.elements_end();
  }

  Prop_Tree_Interface::Prop_Tree_Interface( functionality::node_root *root )
    : ani_root(root) {}
  Prop_Tree_Interface::Prop_Tree_Interface( Animation *ani )
    : ani_root(ani->ani_root_original) {}
}

