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

  Scal_Linear::Scal_Linear( std::string name ) 
    : Prop_Tree_Node( type_name, name ) {
    new Diff_Solver( &d, &v0, &ve );
    new Diff_Solver( &t, &t0, &te );
    new Diff_Solver( &t_f, &t0_f, &te_f );
    new Relation_Solver( &s, &d, &t );
    // ...
    
    add_property( "startvalue", &v0 );
    add_property( "endvalue",   &ve );
    add_property( "difference", &d );
    add_property( "duration",   &t );
    add_property( "starttime", &t0 );
    add_property( "endtime",   &te );
    add_property( "slope",      &s );
 
    //add_action( 5, new Default_Value<values::Scalar>( &s, 0, this ) );
    // add_default_value( &s, values::Scalar(0), 5 );
  }
    
  values::Scalar Scal_Linear::get_return_value( values::Scalar t,
						values::Scalar ) {
    return v0 + s * t;
  }

  bool Scal_Linear::try_add_child( Prop_Tree_Node *node ){
    return false;		// straight has no childs
  }
}

