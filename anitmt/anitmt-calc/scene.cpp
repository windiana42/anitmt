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

  #warning *******************************************
  #warning ** MAAAARRTIIINNN!!!!!!! PLEASE FIX THAT **
  #warning *******************************************
  #if 0
  std::map<values::String,Object_Interface> Ani_Scene::get_objects()
  {
    std::map<values::String,Object_Interface> ret;

    const Contain< Ani_Object >::content_type &all_objects = 
      objects.get_content();
    Contain< Ani_Object >::content_type::const_iterator object;
    
    for( object = all_objects.begin(); object != all_objects.end(); ++object )
      {
	ret[ (*object)->get_name() ] = object;
      }

    return ret;
  }

  std::map< values::String, Scalar_Interface > Ani_Scene::get_scalars()
  {
    std::map<values::String,Scalar_Interface> ret;

    const Contain<Ani_Scalar>::content_type &all_scalars = 
      scalars.get_content();
    Contain<Ani_Scalar>::content_type::const_iterator scalar;
    
    for( scalar = all_scalars.begin(); scalar != all_scalars.end(); ++scalar )
      {
	ret[ (*scalar)->get_name() ] = scalar;
      }

    return ret;
  }
  #endif

  Ani_Scene::Ani_Scene( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      scalars(false,false), objects(false,false) {

    add_property( "filename", &filename );
    add_property( "scene_type", &scene_type );
  }

  bool Ani_Scene::try_add_child( Prop_Tree_Node *node )
  {
    bool res = false;
    res |= scalars.try_add_child( node );
    res |= objects.try_add_child( node );
    
    return res;
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Ani_Scene::final_init()
  {
    scalars.hierarchy_final_init();
    objects.hierarchy_final_init();
  }
  
}

