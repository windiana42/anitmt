/*****************************************************************************/
/**   Here are all availible tree nodes listed        			    **/
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

#include "proptree.hpp"

#include "scene.hpp"

#include "scalar.hpp"
#include "linear.hpp"

#include "object.hpp"
#include "move.hpp"

namespace anitmt{

  // makes all nodes availible       
  void make_all_nodes_availible(){
    Prop_Tree_Node::add_child_factory( Ani_Object::get_type_name(), new Node_Factory< Ani_Object > );
    Prop_Tree_Node::add_child_factory( Obj_Move::get_type_name(), new Node_Factory< Obj_Move > );
    Prop_Tree_Node::add_child_factory( Obj_Move_Straight::get_type_name(), new Node_Factory< Obj_Move_Straight > );
    Prop_Tree_Node::add_child_factory( Ani_Scalar::get_type_name(), new Node_Factory< Ani_Scalar > );
    Prop_Tree_Node::add_child_factory( Scal_Linear::get_type_name(), new Node_Factory< Scal_Linear > );
    Prop_Tree_Node::add_child_factory( Ani_Scene::get_type_name(), new Node_Factory< Ani_Scene > );
    //Prop_Tree_Node::add_child_factory( Xxx::get_type_name(), new Node_Factory< Xxx > );
  }

}

