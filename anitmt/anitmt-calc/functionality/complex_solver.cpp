/*****************************************************************************/
/**   This file offers solver for properties          			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include <assert.h>
#include <algorithm>

#include "complex_solver.hpp"
#include <solve/solver.hpp>	// basic solvers
#include "solver.hpp"		// generated solvers and operators

namespace solve{
  //*********************************************************
  // Accel_Solver: Solver for a constantly accelerated system
  //*********************************************************

  void accel_solver( Operand<values::Scalar> &s, 
		     Operand<values::Scalar> &t, 
		     Operand<values::Scalar> &a, 
		     Operand<values::Scalar> &v0,
		     Operand<values::Scalar> &ve )
  {
    message::Message_Consultant *msg = s.get_consultant();
    //ve = v0 + a*t
    Operand<values::Scalar> &at = *new Operand<values::Scalar>(msg);
    product_solver( at, a, t );	// x = a*t
    sum_solver( ve, v0, at );	// ve = v0 + x

    //v0 = s/t - 0.5*a*t
    v0 = s/t - 0.5*a*t;		// this is the only equation without ve

    //s  = 0.5 * (v0+ve) * t
    Operand<values::Scalar> &v0ve = *new Operand<values::Scalar>(msg);
    sum_solver( v0ve, v0, ve );	// v0ve = v0 + ve
    Operand<values::Scalar> &vt = *new Operand<values::Scalar>(msg);
    product_solver( vt, v0ve, t ); // vt = v0ve * t
    //  s = 0.5 * prod
    product_solver( s, const_op(values::Scalar(0.5),msg), vt ); 
    
    //ve^2 = v0^2 + 2*a*s  
    Operand<values::Scalar> &ve2 = *new Operand<values::Scalar>(msg);
    Operand<values::Scalar> &v02 = *new Operand<values::Scalar>(msg);
    Operand<values::Scalar> &prod = *new Operand<values::Scalar>(msg);
    Operand<values::Scalar> &as = *new Operand<values::Scalar>(msg);
    product_solver( as, a, s );	// as = a * s
    product_solver( prod, const_op(values::Scalar(2),msg), as );
    square_solver( ve2, ve );	// ve2 = ve^2
    square_solver( v02, v0 );	// v02 = v0^2
    sum_solver( ve2, v02, prod ); // ve2 = ve0 + prod
  }
}  

