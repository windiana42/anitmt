/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#ifndef __parsinfo__
#define __parsinfo__

#include "parser.hpp"

namespace parser{
  class Constants : public Parser_Info{
  public:
    virtual values::Valtype get( Parser &s, std::string word ) 
      const throw(unknown, Parser_Error);
  };

  class Functions : public Parser_Info{
  public:
    virtual values::Valtype get( Parser &s, std::string word ) 
      const throw(unknown, Parser_Error);
  };
}

#endif
