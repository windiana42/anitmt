/*****************************************************************************/
/**   This file offers solver for operands          			    **/
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

#ifndef __Complex_Solver__
#define __Complex_Solver__

#include <val/val.hpp>
#include <solve/operand.hpp>

namespace solve
{
  //*********************************************************
  // Accel_Solver: Solver for a constantly accelerated system
  //*********************************************************

  void accel_solver( Operand<values::Scalar> &s, 
		     Operand<values::Scalar> &t, 
		     Operand<values::Scalar> &a, 
		     Operand<values::Scalar> &v0, 
		     Operand<values::Scalar> &ve,
		     message::Message_Consultant *msgc );

}

#endif
