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

#include <val/val.hpp>
#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"

namespace anitmt
{
  //******************************************************************
  // Ani_Scalar: Animatable Scalar node 
  //******************************************************************
  class Ani_Scalar: public Prop_Tree_Node, 
		    public Return<Scalar_State>{

    static const std::string type_name;

    Contain_Return<values::Scalar> s;

    bool try_add_child( Prop_Tree_Node *node );

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  public:
    static std::string get_type_name();

    Ani_Scalar( std::string name, Animation *ani );

    Optional_Return_Type get_return_value
    ( values::Scalar t, Scalar_State &m = type_id )
      throw( EX_user_error );
  };
  
  //**********************************************************
  // Scalar_Interface: reduced access interface to Ani_Scalar
  //**********************************************************

  //! reduced access interface to Ani_Scalar
  class Scalar_Interface {
    Ani_Scalar *scalar;
  public:
    //! get scalar state at time t
    inline Scalar_State get_scalar_state( values::Scalar t )
    { 
      return scalar->get_return_value( t ).second; 
    }

    Scalar_Interface( Ani_Scalar *obj ) : scalar(obj) {}
  };


}
#endif

