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
      scenes(true,false) {
  }

  bool Animation::try_add_child( Prop_Tree_Node *node ){
    bool res = false;

    res |= scenes.try_add_child( node );

    return res;
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Animation::final_init() 
  {
    scenes.hierarchy_final_init();
  }
}

