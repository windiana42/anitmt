/*****************************************************************************/
/**   This file offers operand/operator tree objects			    **/
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

#ifndef __AniTMT_Operand_Classes__
#define __AniTMT_Operand_Classes__

#include "val.hpp"
#include "property_classes.hpp"

namespace anitmt{
  // classes

  class Basic_Operand;
  template<class T> class Operand;
  template<class T> class Constant;
  template<class T_Result=values::Scalar, class T_Operand = T_Result> 
  class Not_Operator;
  template<class T_Result=values::Scalar, 
	   class T_Op1 = T_Result, class T_Op2 = T_Op1> 
  class Add_Operator;

  /*
  template<class T_Result, 
	   class T_Operand = T_Result> class Basic_Operator_for_1_param;
  template<class T_Result, class T_Op1 = T_Result, 
	   class T_Op2 = T_Op1> class Basic_Operator_for_2_params;
  */
}
#endif

