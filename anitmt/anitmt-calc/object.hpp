/*****************************************************************************/
/**   This file offers the general object tree node   			    **/
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

#ifndef __AniTMT_Object__
#define __AniTMT_Object__

namespace anitmt {
  class Ani_Obj;
}

#include "val.hpp"
#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"

namespace anitmt{

  //******************************************************************
  // Ani_Object: Animatable Object node that returns the object state 
  //******************************************************************
  class Ani_Object: public Prop_Tree_Node, 
		    public Return< Object_State >{

    static const std::string type_name;

    Contain_Return<Position>	   pos;
    Contain_Return<Direction>      dir;
    Contain_Return<Up_Vector>      up;

    Vector_Property c;		// rotation center (?piveau? point)

    bool try_add_child( Prop_Tree_Node *node );

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  public:
    static std::string get_type_name();

    Ani_Object( std::string name, Animation *ani );

    Optional_Return_Type get_return_value( values::Scalar t, 
					   Object_State &m = type_id )
      throw( EX_user_error );
  };
  
  //**********************************************************
  // Object_Interface: reduced access interface to Ani_Object
  //**********************************************************

  //! reduced access interface to Ani_Object
  class Object_Interface {
    Ani_Object *object;
  public:
    //! get object state at time t
    inline Object_State get_object_state( values::Scalar t )
    { 
      return object->get_return_value( t ).second; 
    }

    Object_Interface( Ani_Object *obj ) : object(obj) {}
  };


}
#endif

