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

  Ani_Scene::Ani_Scene( std::string name ) 
    : Prop_Tree_Node( type_name, name ),
      scalar(false,false), object(false,false) {

    add_property( "filename", &filename );
    add_property( "scene_type", &scene_type );
  }

  bool Ani_Scene::try_add_child( Prop_Tree_Node *node ){

    Return<Scalar_State>  *scal = 
      dynamic_cast< Return<Scalar_State>*  >( node );
    Return<Object_State>  *obj = 
      dynamic_cast< Return<Object_State>*  >( node );

    bool res = false;
    if( scal ) res = res || scalar.try_add_child( scal );
    if( obj  ) res = res || object.try_add_child( obj );
    
    return res;
  }

}

