/*****************************************************************************/
/**   This file offers the general animation tree node			    **/
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

#ifndef __AniTMT_Animation__
#define __AniTMT_Animation__

//#include <functionality/scene_base_prototypes.hpp> this file often changes 
namespace functionality
{
  // prototypes for needed classes (avoids to include generated files here)
  class _pt_scalar_component;
  class _pt_object_component;
  class _pt_scene_type;
  class node_root;
}

#include <param/param.hpp>
// !!! parameter system should be changed
#include <par/params.hpp>

#include <val/val.hpp>
#include <solve/priority.hpp>
#include <proptree/property.hpp>
#include <proptree/proptree.hpp>

#include <list>

namespace proptree 
{
  // *****************************************
  // semi global variables for the animation
  class Semi_Global
  {
  public:
    anitmt::Animation_Parameters &param;	// all options
    message::Message_Reporter msg; // to report messages to the user

    solve::User_Problem_Handler handler;
    solve::Solve_Run_Info final_run_info;
				// final test run info for collecting the data

    Semi_Global( param::Parameter_Manager *parameter_manager, 
		 message::Message_Consultant* );
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
    proptree::Prop_Tree_Node *ani_root;		 // root node of animation
    functionality::node_root *ani_root_original; // root node as original type

    std::string get_name();

    void init();
    void hierarchy_final_init();
    void finish_calculations();
    Animation( std::string name, param::Parameter_Manager *parameter_manager,
	       message::Message_Manager *manager );
    ~Animation();
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
    std::list<functionality::_pt_scalar_component*>::iterator scalar_component;
  public:
    Scalar_Component_Interface get_next();
    std::string get_name();
    std::pair<bool,values::Scalar> get_value( values::Scalar time );

    inline bool operator==( Scalar_Component_Interface scalar2 ) const
    { return scalar_component == scalar2.scalar_component; }
    inline bool operator!=( Scalar_Component_Interface scalar2 ) const
    { return scalar_component != scalar2.scalar_component; }

    Scalar_Component_Interface
    ( std::list<functionality::_pt_scalar_component*>::iterator scalar );
  };
  class Object_Component_Interface 
  {
    std::list<functionality::_pt_object_component*>::iterator object_component;
  public:
    Object_Component_Interface get_next();
    std::string get_name();
    std::pair<bool,Object_State> get_state( values::Scalar time );

    inline bool operator==( Object_Component_Interface object2 ) const
    { return object_component == object2.object_component; }
    inline bool operator!=( Object_Component_Interface object2 ) const
    { return object_component != object2.object_component; }

    Object_Component_Interface
    ( std::list<functionality::_pt_object_component*>::iterator object );
  };
  class Scene_Interface 
  {
    std::list<functionality::_pt_scene_type*>::iterator scene;
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
    ( std::list<functionality::_pt_scene_type*>::iterator scene );
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

