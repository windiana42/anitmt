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

#include "animation_classes.hpp"

#include "val.hpp"
#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"

#include "options.hpp"

namespace anitmt{

  //********************************
  // Animation: Animation root node 
  //********************************

  class Animation: public Prop_Tree_Node {

    static const std::string type_name;

    Contain_Return< Scene_State > scene;

    bool try_add_child( Prop_Tree_Node *node );
  public:
    static std::string get_type_name();

    Animation( std::string name );

    //*****************************************
    // semi global variables for the animation

    Animation_Options opts;	// all options

  };
}
#endif

