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

    Contain_Return<values::Matrix> mat;
    Contain_Return<Position>	   pos;
    Contain_Return<Direction>      dir;
    Contain_Return<Up_Vector>      up;

    Vector_Property c;		// rotation center (?piveau? point)

    bool try_add_child( Prop_Tree_Node *node );
  public:
    static std::string get_type_name();

    Ani_Object( std::string name );

    Object_State get_return_value( values::Scalar t, 
				   Object_State m = Object_State() );
  };
  

}
#endif

