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

#include <val/val.hpp>

// !!! parameter system should be changed
#include <par/params.hpp>

#include <solve/priority.hpp>

#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"
#include "scene.hpp"

namespace anitmt{

  //********************************
  // Animation: Animation root node 
  //********************************

  class Animation: public Prop_Tree_Node {

    static const std::string type_name;

    Contain< Ani_Scene > scenes;

    bool try_add_child( Prop_Tree_Node *node );

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  public:
    static std::string get_type_name();

    inline const Contain< Ani_Scene >& get_scenes() 
    { return scenes; }
    
    void init();
    void finish_calculations();

    Animation( std::string name );

    //*****************************************
    // semi global variables for the animation

    Animation_Parameters param;	// all options
    solve::Priority_System   pri_sys;	// priority system
  };
}
#endif

