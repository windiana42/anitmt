/*****************************************************************************/
/**   This file offers a reference operator from operand to operand         **/
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

#include "reference.hpp"

#include "operator.hpp"
#include <val/val.hpp>

namespace solve
{
  //***************
  // test function
  //***************
  int reference_test()
  {
    int errors = 0;

    { // user override test
      Operand<values::Scalar> src;
      Operand<values::Scalar> dest;
      Operand<values::Scalar> expression = src * const_op(values::Scalar(3));
      explicite_reference( dest, expression );
      
      if( dest.set_value( 5 ) )
      {
	std::cout << "User may override explicite references: ok" << std::endl;
     }
      else
      {
	std::cout << "Error: User may NOT override explicite references" 
		  << std::endl;
	errors++;
      }
    }
    
    return errors;
  }
}
