/*****************************************************************************/
/**   This file offers the general scene tree node   			    **/
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

#include "scene.hpp"

namespace anitmt{

  //******************************************************************
  // Ani_Scene: Animatable Scene node that returns the scene state 
  //******************************************************************

  // type name identifier as string
  const std::string Ani_Scene::type_name = "scene";

  std::string Ani_Scene::get_type_name(){
    return type_name;
  }

  Ani_Scene::Ani_Scene( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      scalars(false,false), objects(false,false) {

    add_property( "filename", &filename );
    add_property( "scene_type", &scene_type );
  }

  bool Ani_Scene::try_add_child( Prop_Tree_Node *node ){

    Return<Scalar_State>  *scal = 
      dynamic_cast< Return<Scalar_State>*  >( node );
    Return<Object_State>  *obj = 
      dynamic_cast< Return<Object_State>*  >( node );

    bool res = false;
    if( scal ) res = res || scalars.try_add_child( scal );
    if( obj  ) res = res || objects.try_add_child( obj );
    
    return res;
  }

  Ani_Scene::Optional_Return_Type Ani_Scene::get_return_value
  ( values::Scalar t, Scene_State s = Scene_State() ) 
    throw( EX_user_error )
  {
    return Optional_Return_Type( false, Scene_State() ); // return nothing...
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Ani_Scene::final_init()
  {
    scalars.hierarchy_final_init();
    objects.hierarchy_final_init();
  }
  
}

