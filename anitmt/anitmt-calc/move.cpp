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
							Position& ) 
    throw( EX_user_error )
  {
    return pos.get_return_value( t );
  }
  std::pair<bool, Direction> Obj_Move::get_return_value( values::Scalar t, 
							 Direction& ) 
    throw( EX_user_error )
  {
    return dir.get_return_value( t );
  }
  std::pair<bool, Up_Vector> Obj_Move::get_return_value( values::Scalar t, 
							 Up_Vector& ) 
    throw( EX_user_error )
  {
    return up.get_return_value( t );
  }

  bool Obj_Move::try_add_child( Prop_Tree_Node *node )
  {
    bool res = false;
    res |= pos.try_add_child( node );
    res |= dir.try_add_child( node );
    res |= up.try_add_child ( node );
    
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
    add_property( "length", &s );
    add_property( "startpos", &s0 );
    add_property( "endpos", &se );
    add_property( "startdir", &d0 );
    add_property( "enddir", &de );
    add_property( "startup", &u0 );
    add_property( "endup", &ue );
    add_property( "duration", &t );
    add_property( "starttime", &t0 );
    add_property( "endtime", &te );
    add_property( "frames", &t_f );
    add_property( "startframe", &t0_f );
    add_property( "endframe", &te_f );
    add_property( "acceleration", &a );
    add_property( "startspeed", &v0 );
    add_property( "endspeed", &ve );
 
    Operand<values::Scalar> &fps = 
      const_op( values::Scalar(ani->param.fps()) );

    accel_solver( s, t, a, v0, ve );
    equal_solver( d0, de );
    equal_solver( u0, ue ); //!!! rot_up not recognized
    sum_solver( te, t, t0 );
    sum_solver( te_f, t_f, t0_f );
    product_solver( t_f,  t,  fps ); // t_f = t * fps 
    product_solver( t0_f, t0, fps ); // t0_f = t0 * fps 
    product_solver( te_f, te, fps ); // te_f = te * fps 

    Operand<values::Vector> sdiff; // position difference
    sum_solver( se, sdiff, s0 );
    //product_solver( sdiff, d0,  s ); // sdiff = d0 * s 
    //!!! d0 has to be 1 in length !!!

    
    //add_action( 3, new Default_Value<values::Scalar>( &a, 0, this ) );
    // add_default_value( &a, values::Scalar(0), 3 );
  }
    
  std::pair<bool,Position> Obj_Move_Straight::get_return_value
  ( values::Scalar global_time, Position& ) throw( EX_user_error )
  {
    // if not active
    if( (t > te()) || (t < t0()) ) 
      return std::pair<bool,Position>( false, Position() );

    values::Scalar t = (global_time-t0());
    values::Scalar s_t = v0() * t + 0.5 * a() * t*t;
    values::Scalar rel_pos = s() / s_t;

    return std::pair<bool,Position>(true, (1-rel_pos)*s0() + rel_pos*se());
  }

  std::pair<bool,Direction> Obj_Move_Straight::get_return_value
  ( values::Scalar t, Direction& ) throw( EX_user_error )
  {
    // if not active
    if( (t > te()) || (t < t0()) ) 
      return std::pair<bool,Direction>( false, Direction() );

    return std::pair<bool,Direction>(true, d0() * (1/abs(d0()))); 
    // !!! rotations not reguarded
  }

  std::pair<bool,Up_Vector> Obj_Move_Straight::get_return_value
  ( values::Scalar t, Up_Vector& ) throw( EX_user_error )
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

