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

namespace anitmt{

  //******************************************************************
  // Ani_Scene: general Scene node 
  //******************************************************************
  class Ani_Scene: public Prop_Tree_Node, 
		   public Return< Scene_State > {

    static const std::string type_name;

    String_Property filename;
    String_Property scene_type;

    //! all scalar components in the scene 
    Contain_Return< Scalar_State > scalars;
    //! all scalar components in the scene 
    Contain_Return< Object_State > objects;

    bool try_add_child( Prop_Tree_Node *node );

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  public:
    inline const Contain_Return<Scalar_State>& get_scalars() 
    { return scalars; }
    inline const Contain_Return<Object_State>& get_objects() 
    { return objects; }

    static std::string get_type_name();

    Ani_Scene( std::string name, Animation *ani );

    Optional_Return_Type get_return_value( values::Scalar t, 
					   Scene_State s = Scene_State() ) 
      throw( EX_user_error );    
  };
  
}
#endif

