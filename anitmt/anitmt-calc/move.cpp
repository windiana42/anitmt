/*****************************************************************************/
/**   This file offers a move function                			    **/
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

#include "move.hpp"

namespace anitmt{


  //**********************************************************
  // Obj_Move: moves Objects on a flight path
  //**********************************************************

  const std::string Obj_Move::type_name = "move";

  //
  // Function Obj_Move::get_type_name ()
  //
  //    returns type name as string
  //
  //
  std::string Obj_Move::get_type_name(){
    return type_name;
  }

  //
  // Function Obj_Move::Obj_Move (name)
  //
  //    constuctor of Obj_Move
  //
  //
  Obj_Move::Obj_Move( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      pos(true,false),
      dir(true,false),
      up(true,false) {

    add_property( "center", &c );
  }

  //
  // Function Obj_Move::get_return_value (t, Matrix)
  //
  //    returns a Matrix as the object state at time t 
  //
  //
  values::Matrix Obj_Move::get_return_value( values::Scalar t, 
					     values::Matrix ) {

    values::Matrix ret;

    //ret *= values::Matrix::translate( -c );
    //ret *= values::Matrix::translate( pos.get_return_value(t) );
    //ret *= values::Matrix::rotate_pair_pair
    //         ( x, y, dir.get_return_value(t), up.get_return_value(t) );

    return ret;
  }

  Position Obj_Move::get_return_value( values::Scalar t, 
				       Position ) {
    return pos.get_return_value( t );
  }
  Direction Obj_Move::get_return_value( values::Scalar t, 
					Direction ) {
    return dir.get_return_value( t );
  }
  Up_Vector Obj_Move::get_return_value( values::Scalar t, 
					Up_Vector ) {
    return up.get_return_value( t );
  }

  bool Obj_Move::try_add_child( Prop_Tree_Node *node ){
    Return<Position>  *p = dynamic_cast< Return<Position>*  >( node );
    Return<Direction> *d = dynamic_cast< Return<Direction>* >( node );
    Return<Up_Vector> *u = dynamic_cast< Return<Up_Vector>* >( node );

    bool res = false;
    if( p ) res = res || pos.try_add_child( p );
    if( d ) res = res || dir.try_add_child( d );
    if( u ) res = res || up.try_add_child( u );
    
    return res;
  }

  //**********************************************************
  // Obj_Move_Straight: moves Objects on a staight flight path
  //**********************************************************

  const std::string Obj_Move_Straight::type_name = "straight";

  std::string Obj_Move_Straight::get_type_name(){
    return type_name;
  }

  Obj_Move_Straight::Obj_Move_Straight( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ) {
    establish_accel_solver( s, t, a, v0, ve );
    establish_sum_solver( te, t, t0 );
    establish_sum_solver( te_f, t_f, t0_f );
    
    add_property( "startpos", &s0 );
    add_property( "startpos", &s0 );
    add_property( "endpos", &se );
    add_property( "length", &s );
    add_property( "duration", &t );
    add_property( "acceleration", &a );
    add_property( "startspeed", &v0 );
    add_property( "endspeed", &ve );
 
    //add_action( 3, new Default_Value<values::Scalar>( &a, 0, this ) );
    // add_default_value( &a, values::Scalar(0), 3 );
  }
    
  Position Obj_Move_Straight::get_return_value( values::Scalar t,
						Position ) {
    values::Scalar s_t = v0 * t + 0.5 * a * t*t;
    values::Scalar rel_pos = s / s_t;

    return (1-rel_pos)*s0 + rel_pos*se;
  }

  Direction Obj_Move_Straight::get_return_value( values::Scalar t,
						 Direction ) {
    return d0 * (1/abs(d0));	// !!! rotations not reguarded
  }

  Up_Vector Obj_Move_Straight::get_return_value( values::Scalar t, 
						 Up_Vector ) {
    return u0 * (1/abs(u0));	// !!! object rotation not reguarded
  }

  bool Obj_Move_Straight::try_add_child( Prop_Tree_Node *node ){
    return false;		// straight has no childs
  }
}

