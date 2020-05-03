/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#ifndef __inlineimplementation_anitmt_input_parser_functions__
#define __inlineimplementation_anitmt_input_parser_functions__

#include "parser_functions.hpp"

#include <solve/reference.hpp>
#include <val/val.hpp>

#include <string>
#include <assert.h>

#ifdef EXTREME_INLINE
#define _INLINE_ inline
#else
#define _INLINE_
#endif

namespace anitmt
{
  namespace adlparser
  {
    //**********************
    // interfaces to lexer
    //**********************

    _INLINE_ int yylex( Token *lvalp, void *mode, adlparser_info *info )
    {
      return info->lexer->yylex(lvalp, mode, info);
    }

    //******************************
    // functions used by the parser
    //******************************

    //! sets the position of a Property in the adl source
    _INLINE_ void initialize_lexer( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->lexer->goto_initial_state();
    }

    //! sets the position of a Property in the adl source
    _INLINE_ void set_node_pos( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->get_current_tree_node()->set_position( info->file_pos.duplicate());
    }

    // tells the lexer to resolve identifiers as properties
    _INLINE_ void resolve_properties( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_property;
    }

    // tells the lexer to resolve identifiers as property references
    _INLINE_ void resolve_references( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_reference;
    }
  }
}
#undef _INLINE_

#endif
