/*****************************************************************************/
/**   This file offers the general scalar tree node   			    **/
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

#ifndef __AniTMT_Scalar__
#define __AniTMT_Scalar__

namespace anitmt {
  class Ani_Scalar;
}

#include "val.hpp"
#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"

namespace anitmt{

  //******************************************************************
  // Ani_Scalar: Animatable Scalar node 
  //******************************************************************
  class Ani_Scalar: public Prop_Tree_Node, 
		    public Return<Scalar_State>{

    static const std::string type_name;

    Contain_Return<values::Scalar> s;

    bool try_add_child( Prop_Tree_Node *node );
  public:
    static std::string get_type_name();

    Ani_Scalar( std::string name, Animation *ani );

    Scalar_State get_return_value( values::Scalar t, 
				   Scalar_State m = Scalar_State() );
  };
  

}
#endif

