/*****************************************************************************/
/**   Parameter handling system                                   	    **/
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

#ifndef __lib_Param_inline__
#define __lib_Param_inline__

#include "param.hpp"

namespace param
{
  void Parmeter_Interface::int_parameter( solve::Operand<int>* param, std::string name, 
					  std::string description, int default_value,
					  bool needed )
  {
  }
}

#endif
