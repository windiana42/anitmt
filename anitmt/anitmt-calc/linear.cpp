/*****************************************************************************/
/**   This file offers a linear scalar interpolating function		    **/
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

#include "linear.hpp"

namespace anitmt{

  //**********************************************************
  // Scal_Linear: moves Objects on a staight flight path
  //**********************************************************

  const std::string Scal_Linear::type_name = "linear";

  std::string Scal_Linear::get_type_name(){
    return type_name;
  }

  Scal_Linear::Scal_Linear( std::string name, Animation *ani ) 
    : Prop_Tree_Node( type_name, name, ani ) {

    //*********
    // Solvers
    
    new Diff_Solver( &d, &v0, &ve );
    new Diff_Solver( &t, &t0, &te );
    new Diff_Solver( &t_f, &t0_f, &te_f );
    new Relation_Solver( &s, &d, &t );
    //new Relation_Solver( ?FPS?, &t_f, &t );
    //new Relation_Solver( ?FPS?, &t0_f, &t0 );
    //new Relation_Solver( ?FPS?, &te_f, &te );

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
    add_property( "slope",      &s );

    //*****************
    // Default Values

    // Default value 0 for slope on level 10 
    establish_Default_Value( &ani->pri_sys, 50, &s, 0 );

    /*
    // only first element
    if( !get_prev() )
      {
	// Default value 0 for starttime on level 15 
	establish_Default_Value( &ani->pri_sys, 55, &t0, 0 );
      }

    // only last element
    if( !get_next() )
      {
	// Default value <anim end> for endtime on level 16 
	//establish_Default_Value( &ani->pri_sys, 16, &te, ?GLOB?endtime? );
      }

    // only first element
    if( !get_prev() )
      {
	// Default value 0 for startvalue on level 20
	establish_Default_Value( &ani->pri_sys, 60, &v0, 0 );
      }
    */
    
    // Default value 0 for differance on level 30 
    establish_Default_Value( &ani->pri_sys, 70, &d, 0 );
  }

  //********
  // Pushes

  // initializes the connection to next/previous node
  void Scal_Linear::init_next( Return<values::Scalar> *next ) {
    Prop_Tree_Node *next_node = dynamic_cast< Prop_Tree_Node* >( next );

    // push endtime to next starttime on level 1
    establish_Push_Connection( &ani->pri_sys, 
			       1, &te, next_node, "starttime" );
    // push endvalue to next startvalue on level 3
    establish_Push_Connection( &ani->pri_sys, 
			       3, &ve, next_node, "startvalue" );
  }

  void Scal_Linear::init_prev( Return<values::Scalar> *prev ) {
    Prop_Tree_Node *prev_node = dynamic_cast< Prop_Tree_Node* >( prev );

    // push starttime to previous endtime on level 2
    establish_Push_Connection( &ani->pri_sys, 
			       4, &t0, prev_node, "endtime" );
    // push startvalue to previous endvalue on level 4
    establish_Push_Connection( &ani->pri_sys, 
			       4, &v0, prev_node, "endvalue" );
  }
    
  values::Scalar Scal_Linear::get_return_value( values::Scalar t,
						values::Scalar ) {
    return v0 + s * t;
  }

  bool Scal_Linear::try_add_child( Prop_Tree_Node *node ){
    return false;		// straight has no childs
  }
}

