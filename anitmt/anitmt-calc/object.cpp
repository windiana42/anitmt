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

#include "solver.hpp"

namespace anitmt{


  //******************************************************************
  // Ani_Object: Animatable Object node that returns the object state 
  //******************************************************************

  // type name identifier as string
  const std::string Ani_Object::type_name = "object";

  std::string Ani_Object::get_type_name(){
    return type_name;
  }

  Ani_Object::Ani_Object( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      mat(false,false),
      pos(true,false),
      dir(true,false),
      up(true,false) {

    add_property( "center", &c );
  }

  Object_State Ani_Object::get_return_value( values::Scalar t, 
					     Object_State ){
    try {
      return mat.get_return_value(t);
    }
    catch( EX_no_active_child ) {}

    values::Matrix ret;

    //ret *= values::Matrix::translate( -c );
    //ret *= values::Matrix::translate( pos.get_return_value(t) );
    //ret *= values::Matrix::rotate_pair_pair
    //         ( x, y, dir.get_return_value(t), up.get_return_value(t) );

    return ret;
  }

  bool Ani_Object::try_add_child( Prop_Tree_Node *node ){
    Return<values::Matrix>  *m = 
      dynamic_cast< Return<values::Matrix>*  >( node );
    Return<Position>  *p = dynamic_cast< Return<Position>*  >( node );
    Return<Direction> *d = dynamic_cast< Return<Direction>* >( node );
    Return<Up_Vector> *u = dynamic_cast< Return<Up_Vector>* >( node );

    bool res = false;
    if( m ) res = res || mat.try_add_child( m );
    if( p ) res = res || pos.try_add_child( p );
    if( d ) res = res || dir.try_add_child( d );
    if( u ) res = res || up.try_add_child( u );
    
    return res;
  }


  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Ani_Object::final_init()
  {
    mat.hierarchy_final_init();
    pos.hierarchy_final_init();
    dir.hierarchy_final_init();
    up.hierarchy_final_init();
  }

}

