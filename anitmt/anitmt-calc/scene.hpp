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

#ifndef __AniTMT_Scene__
#define __AniTMT_Scene__

namespace anitmt {
  class Ani_Scene;
}

#include "val.hpp"
#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"
#include "scalar.hpp"
#include "object.hpp"

namespace anitmt
{
  //******************************************************************
  // Ani_Scene: general Scene node 
  //******************************************************************
  class Ani_Scene: public Prop_Tree_Node 
  {
    static const std::string type_name;

    String_Property filename;
    String_Property scene_type;

    //! all scalar components in the scene 
    Contain< Ani_Scalar > scalars;
    //! all scalar components in the scene 
    Contain< Ani_Object > objects;

    bool try_add_child( Prop_Tree_Node *node );

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  public:
    static std::string get_type_name();

    inline values::String get_filename()  {  return(filename());  }
    inline values::String get_scene_type()  {  return(scene_type());  }

    std::map< values::String, Object_Interface > get_all_objects();
    std::map< values::String, Scalar_Interface > get_all_scalars();

    // these functions should be used only of really nessessary
    inline const Contain< Ani_Scalar >& get_scalars() 
    { return scalars; }
    inline const Contain< Ani_Object >& get_objects() 
    { return objects; }

    Ani_Scene( std::string name, Animation *ani );
  };
  
}
#endif

