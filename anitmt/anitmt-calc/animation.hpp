/*****************************************************************************/
/**   This file offers the general animation tree node			    **/
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

#ifndef __AniTMT_Animation__
#define __AniTMT_Animation__

#include <val/val.hpp>

#include <param/param.hpp>
// !!! parameter system should be changed
#include <par/params.hpp>

#include <solve/priority.hpp>
#include <proptree/property.hpp>
#include <proptree/proptree.hpp>

#include <functionality/base_func.hpp>

namespace proptree {
  // *****************************************
  // semi global variables for the animation
  class Semi_Global
  {
  public:
    anitmt::Animation_Parameters &param;	// all options
    message::Message_Reporter msg; // to report messages to the user

    Semi_Global( param::Parameter_Manager *parameter_manager, message::Message_Consultant* );
  };
}

namespace anitmt{

  // ********************************
  // Animation: Animation root node 
  // ********************************

  class Animation {
  public:
    proptree::Semi_Global GLOB;
    proptree::tree_info tree_info;
    functionality::node_root ani_root;		// root node of animation

    std::string get_name();

    void init();
    void hierarchy_final_init();
    void finish_calculations();
    Animation( std::string name, param::Parameter_Manager *parameter_manager,
	       message::Message_Manager *manager );
  };

  //***************************************
  // Interfaces to the animation data tree
  //***************************************

  struct Object_State 
  {
    values::Matrix matrix;	// transformation matix

    values::Vector translate;
    values::Vector rotate;

    values::Vector position;
    values::Vector front;	
    values::Vector up_vector;
  };

  class Scalar_Component_Interface 
  {
    functionality::_container_scalar_component::elements_type::iterator 
    scalar_component;
  public:
    Scalar_Component_Interface get_next();
    std::string get_name();
    std::pair<bool,values::Scalar> get_value( values::Scalar time );

    inline bool operator==( Scalar_Component_Interface scalar2 ) const
    { return scalar_component == scalar2.scalar_component; }
    inline bool operator!=( Scalar_Component_Interface scalar2 ) const
    { return scalar_component != scalar2.scalar_component; }

    Scalar_Component_Interface
    ( functionality::_container_scalar_component::elements_type::iterator 
      scalar );
  };
  class Object_Component_Interface 
  {
    functionality::_container_object_component::elements_type::iterator  
    object_component;
  public:
    Object_Component_Interface get_next();
    std::string get_name();
    std::pair<bool,Object_State> get_state( values::Scalar time );

    inline bool operator==( Object_Component_Interface object2 ) const
    { return object_component == object2.object_component; }
    inline bool operator!=( Object_Component_Interface object2 ) const
    { return object_component != object2.object_component; }

    Object_Component_Interface
    ( functionality::_container_object_component::elements_type::iterator 
      object );
  };
  class Scene_Interface 
  {
    functionality::_container_scene_type::elements_type::iterator scene;
  public:
    Scene_Interface get_next();
    std::string get_name();
    std::string get_filename();
    std::string get_scene_type();
    Scalar_Component_Interface get_first_scalar();
    Scalar_Component_Interface get_scalar_end();
    Object_Component_Interface get_first_object();
    Object_Component_Interface get_object_end();

    inline bool operator==( Scene_Interface scene2 ) const
    { return scene == scene2.scene; }
    inline bool operator!=( Scene_Interface scene2 ) const
    { return scene != scene2.scene; }

    Scene_Interface
    ( functionality::_container_scene_type::elements_type::iterator scene );
  };
  class Prop_Tree_Interface
  {
    functionality::node_root *ani_root;
  public:
    Scene_Interface get_first_scene();
    Scene_Interface get_scene_end();
    Prop_Tree_Interface( functionality::node_root *ani_root );
    Prop_Tree_Interface( Animation *ani );
  };
}
#endif

