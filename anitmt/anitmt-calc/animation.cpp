/*****************************************************************************/
/**   This file offers the general animation tree node   			    **/
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

#include "animation.hpp"

namespace anitmt{

  //********************************
  // Animation: Animation root node
  //********************************

  // type name identifier as string
  const std::string Animation::type_name = "animation";

  std::string Animation::get_type_name(){
    return type_name;
  }

  Animation::Animation( std::string name ) 
    : Prop_Tree_Node( type_name, name, this /*this is THE animation*/ ),
      scene(true,false) {
  }

  bool Animation::try_add_child( Prop_Tree_Node *node ){

    Return<Scene_State>  *s = 
      dynamic_cast< Return<Scene_State>*  >( node );

    bool res = false;
    if( s ) res = res || scene.try_add_child( s );
    
    return res;
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Animation::final_init() 
  {
    scene.hierarchy_final_init();
  }
}

