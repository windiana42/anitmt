/*****************************************************************************/
/**   This file offers a accelerated scalar interpolating function		    **/
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

#include "accelerated.hpp"

namespace anitmt{

  //**********************************************************
  // Scal_Accelerated: moves Objects on a staight flight path
  //**********************************************************

  const std::string Scal_Accelerated::type_name = "accelerated";

  Scal_Accelerated::Scal_Accelerated( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ),
      v0( "startvalue", this ),
      ve( "endvalue", this ),
      d( "difference", this ),
      t( "duration", this ),
      t0( "starttime", this ),
      te( "endtime", this ),
      t_f( "frames", this ),
      t0_f( "startframe", this ),
      te_f( "endframe", this ),
      s0( "startslope", this )
      se( "endslope", this )
      a( "acceleration", this )
  {

    //*********
    // Solvers

    solve::Operand<values::Scalar> &fps = 
      solve::const_op( values::Scalar(ani->param.fps()), get_consultant() );

    solve::sum_solver( ve, d, v0 ); // ve = d + v0
    solve::sum_solver( te, t, t0 );
    //solve::sum_solver( te_f, t_f, t0_f );

    solve::accel_solver( d, t, a, s0, s2 ); // s = 0.5*a*t*t; v=a*t

    solve::product_solver( t_f,  t,  fps ); // t_f = t * fps 
    solve::product_solver( t0_f, t0, fps ); // t0_f = t0 * fps 
    solve::product_solver( te_f, te, fps ); // te_f = te * fps 

    //*********************
    // Register Properties
    
    add_property( "startvalue", &v0 );
    add_property( "endvalue",   &ve );
    add_property( "difference", &d );
    add_property( "duration",   &t );
    add_property( "starttime",  &t0 );
    add_property( "endtime",    &te );
    add_property( "frames",     &t_f );
    add_property( "startframe", &t0_f );
    add_property( "endframe",   &te_f );
    add_property( "startslope", &s0 );
    add_property( "endslope",   &se );
    add_property( "acceleration",&a );

    //*****************
    // Default Values

    // Default value 0 for slope on level 150 
    solve::establish_Default_Value( &ani->pri_sys, 150, s, 0 );

    // Default value 0 for differance on level 170 
    solve::establish_Default_Value( &ani->pri_sys, 170, d, 0 );
  }

  // initializes the first node on a level
  void Scal_Accelerated::init_first( Return<values::Scalar>* ) 
  {
    // Default value 0 for starttime on level 130 
    solve::establish_Default_Value( &ani->pri_sys, 130, t0, 0 );

    // Default value 0 for startvalue on level 160
    solve::establish_Default_Value( &ani->pri_sys, 160, v0, 0 );
  }

  // initializes the last node on a level
  void Scal_Accelerated::init_last( Return<values::Scalar>* ) 
  {
    //Default value <anim end> for endtime on level 131 
    solve::establish_Default_Value( &ani->pri_sys, 131, te, 
				    ani->param.endtime() );
  }

  //********
  // Pushes

  // initializes the connection to next/previous node
  void Scal_Accelerated::init_next( Return<values::Scalar> *next ) 
  {
    Prop_Tree_Node *next_node = dynamic_cast< Prop_Tree_Node* >( next );

    // push endtime to next starttime on level 1
    solve::establish_Push_Connection( &ani->pri_sys, 
				      1, te, next_node, "starttime" );
    // push endvalue to next startvalue on level 5
    solve::establish_Push_Connection( &ani->pri_sys, 
				      5, ve, next_node, "startvalue" );
  }

  void Scal_Accelerated::init_prev( Return<values::Scalar> *prev ) 
  {
    Prop_Tree_Node *prev_node = dynamic_cast< Prop_Tree_Node* >( prev );

    // push starttime to previous endtime on level 2
    solve::establish_Push_Connection( &ani->pri_sys, 
				      2, t0, prev_node, "endtime" );
    // push startvalue to previous endvalue on level 6
    solve::establish_Push_Connection( &ani->pri_sys, 
				      6, v0, prev_node, "endvalue" );
  }
    
  Scal_Accelerated::Optional_Return_Type Scal_Accelerated::get_return_value
  ( values::Scalar t, values::Scalar& ) throw()
  {
    // if not active
    if( (t > te()) || (t < t0()) ) 
      return Optional_Return_Type( false, values::Scalar() );
    // else return result
    values::Scalar time = t-t0();
    return Optional_Return_Type( true, 
				 v0() 
				 + s0() * time
				 + 0.5 * a() * time * time );
  }

  bool Scal_Accelerated::try_add_child( Prop_Tree_Node *node )
  {
    return false;		// straight has no children
  }

  //! individual final init after hierarchy is set up (Has to call the 
  //! function of the return type container
  void Scal_Accelerated::final_init() {}
}

