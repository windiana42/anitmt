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

#include "object.hpp"

#include "val.hpp"
#include "solver.hpp"

namespace anitmt{


  //******************************************************************
  // Ani_Object: Animatable Object node that returns the object state 
  //******************************************************************

  // type name identifier as string
  const std::string Ani_Object::type_name = "object";

  std::string Ani_Object::get_type_name()
  {
    return type_name;
  }

  Ani_Object::Ani_Object( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      pos(true,false),
      dir(true,false),
      up(true,false) 
  {
    add_property( "center", &c );
  }

  Ani_Object::Optional_Return_Type Ani_Object::get_return_value
  ( values::Scalar t, Object_State ) throw( EX_user_error )
  {
    values::Matrix ret;

    std::pair<bool, values::Vector> p = pos.get_return_value( t );
    std::pair<bool, values::Vector> u = up.get_return_value( t );
    std::pair<bool, values::Vector> d = dir.get_return_value( t );

    if( !p.first || !u.first || !d.first ) 
      return Optional_Return_Type( false, Object_State() );

    values::Vector position = p.second;
    values::Vector direction = d.second;
    values::Vector up_vector = u.second;

    // !!! 
    //ret *= values::Matrix::Mtranslate( -c() );
    //ret *= values::Matrix::Mtranslate( position );
    //ret *= values::Matrix::Mrotate_pair_pair
    //   ( values::Vector(1,0,0), values::Vector(0,1,0), direction , up_vector );

    return Ani_Object::Optional_Return_Type( true, ret );
  }

  bool Ani_Object::try_add_child( Prop_Tree_Node *node ){
    Return<Position>  *p = dynamic_cast< Return<Position>*  >( node );
    Return<Direction> *d = dynamic_cast< Return<Direction>* >( node );
    Return<Up_Vector> *u = dynamic_cast< Return<Up_Vector>* >( node );

    bool res = false;
    if( p ) res = res || pos.try_add_child( p );
    if( d ) res = res || dir.try_add_child( d );
    if( u ) res = res || up.try_add_child( u );
    
    return res;
  }


  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Ani_Object::final_init()
  {
    pos.hierarchy_final_init();
    dir.hierarchy_final_init();
    up.hierarchy_final_init();
  }
}

