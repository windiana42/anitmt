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

#include "scalar.hpp"

#include "solver.hpp"

namespace anitmt{

  //******************************************************************
  // Ani_Scalar: Animatable Scalar node that returns the scalar state 
  //******************************************************************

  // type name identifier as string
  const std::string Ani_Scalar::type_name = "scalar";

  std::string Ani_Scalar::get_type_name()
  {
    return type_name;
  }

  Ani_Scalar::Ani_Scalar( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      s(false,false) 
  {
    
  }

  Scalar_State Ani_Scalar::get_return_value( values::Scalar t, 
					     Scalar_State )
  {
    return s.get_return_value(t);
  }

  bool Ani_Scalar::try_add_child( Prop_Tree_Node *node )
  {
    Return<values::Scalar>  *scal = 
      dynamic_cast< Return<values::Scalar>*  >( node );

    bool res = false;
    if( scal ) res = res || s.try_add_child( scal );
    
    return res;
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Ani_Scalar::final_init()
  {
    s.hierarchy_final_init();
  }
}

