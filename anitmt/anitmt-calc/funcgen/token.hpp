/*****************************************************************************/
/**   This file offers a complex token class for storing all types          **/
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

#ifndef __anitmt_input_afd_token__
#define __anitmt_input_afd_token__

#include <string>
#include <message/message.hpp>
#include <afdbase.hpp>

namespace funcgen
{
  struct Token
  {
    std::string string;
    union{
      double scalar;
      bool boolean;
      Expression *exp;
    }u;
  };
}

#endif
