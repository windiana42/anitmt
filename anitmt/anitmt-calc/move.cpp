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

#include "animation.hpp"

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
  std::string Obj_Move::get_type_name()
  {
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
      up(true,false) 
  {
    add_property( "center", &c );
  }

  std::pair<bool, Position> Obj_Move::get_return_value( values::Scalar t, 
							Position ) 
    throw( EX_user_error )
  {
    return pos.get_return_value( t );
  }
  std::pair<bool, Direction> Obj_Move::get_return_value( values::Scalar t, 
							 Direction ) 
    throw( EX_user_error )
  {
    return dir.get_return_value( t );
  }
  std::pair<bool, Up_Vector> Obj_Move::get_return_value( values::Scalar t, 
							 Up_Vector ) 
    throw( EX_user_error )
  {
    return up.get_return_value( t );
  }

  bool Obj_Move::try_add_child( Prop_Tree_Node *node )
  {
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
  void Obj_Move::final_init()
  {
    pos.hierarchy_final_init();
    dir.hierarchy_final_init();
    up.hierarchy_final_init();
  }

  //**********************************************************
  // Obj_Move_Straight: moves Objects on a staight flight path
  //**********************************************************

  const std::string Obj_Move_Straight::type_name = "straight";

  std::string Obj_Move_Straight::get_type_name()
  {
    return type_name;
  }

  Obj_Move_Straight::Obj_Move_Straight( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ) 
  {
    Operand<values::Scalar> &fps = 
      const_op( values::Scalar(ani->param.fps()) );

    establish_accel_solver( s, t, a, v0, ve );
    establish_sum_solver( te, t, t0 );
    establish_sum_solver( te_f, t_f, t0_f );
    establish_product_solver( t_f,  t,  fps ); // t_f = t * fps 
    establish_product_solver( t0_f, t0, fps ); // t0_f = t0 * fps 
    establish_product_solver( te_f, te, fps ); // te_f = te * fps 
    
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
    
  std::pair<bool,Position> Obj_Move_Straight::get_return_value
  ( values::Scalar t, Position ) throw( EX_user_error )
  {
    // if not active
    if( (t > te()) || (t < t0()) ) 
      return std::pair<bool,Position>( false, Position() );

    values::Scalar s_t = v0() * t + 0.5 * a() * t*t;
    values::Scalar rel_pos = s() / s_t;

    return std::pair<bool,Position>(true, (1-rel_pos)*s0() + rel_pos*se());
  }

  std::pair<bool,Direction> Obj_Move_Straight::get_return_value
  ( values::Scalar t, Direction ) throw( EX_user_error )
  {
    // if not active
    if( (t > te()) || (t < t0()) ) 
      return std::pair<bool,Direction>( false, Direction() );

    return std::pair<bool,Direction>(true, d0() * (1/abs(d0()))); 
    // !!! rotations not reguarded
  }

  std::pair<bool,Up_Vector> Obj_Move_Straight::get_return_value
  ( values::Scalar t, Up_Vector ) throw( EX_user_error )
  {
    // if not active
    if( (t > te()) || (t < t0()) ) 
      return std::pair<bool,Up_Vector>( false, Up_Vector() );

    return std::pair<bool,Up_Vector>(true, u0() * (1/abs(u0()))); 
    // !!! object rotation not reguarded
  }

  bool Obj_Move_Straight::try_add_child( Prop_Tree_Node *node )
  {
    return false;		// straight has no children
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Obj_Move_Straight::final_init() {}

}

