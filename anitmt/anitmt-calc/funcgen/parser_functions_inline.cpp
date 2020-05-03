/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#ifndef __inlineimplementation_afd_input_parser_functions__
#define __inlineimplementation_afd_input_parser_functions__

#include "parser_functions.hpp"

#ifdef EXTREME_INLINE
#define _INLINE_ inline
#else
#define _INLINE_
#endif

namespace funcgen
{
  // **********************
  // interfaces to lexer
  // **********************

  _INLINE_ int yylex( Token *lvalp, void *mode, afd_info *info )
  {
    return info->lexer->yylex(lvalp, mode, info);
  }

  //******************************
  // functions used by the parser
  //******************************

  //! sets the position of a Property in the adl source
  _INLINE_ void initialize_lexer( void *vptr_info )
  {
    afd_info *info = static_cast<afd_info*>(vptr_info);
    info->lexer->goto_initial_state();
  }
}

#undef _INLINE_

#endif
