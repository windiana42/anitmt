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

#ifndef __anitmt_input_parser_functions_inlineimplementation
#define __anitmt_input_parser_functions_inlineimplementation

#include "parser_functions.hpp"

namespace anitmt
{
  namespace adlparser
  {
    //**********************
    // interfaces to lexer
    //**********************

    inline int yylex( Token *lvalp, void *vinfo )
    {
      adlparser_info *info = static_cast<adlparser_info*> (vinfo);
      info->lexer->yylval = lvalp; // lvalue variable to return token value
      return info->lexer->yylex();
    }

    //*************************
    // interfaces to messages
    //*************************

    inline message::Message_Stream yyerr( void* vinfo )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);
      
      message::Message_Stream msg(message::noinit);
      info->msg.error( &info->file_pos ).copy_to(msg);
      return msg;
    }

    inline message::Message_Stream yywarn( void* vinfo )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);

      message::Message_Stream msg(message::noinit);
      info->msg.warn( &info->file_pos ).copy_to(msg);
      return msg;
    }

    inline message::Message_Stream yyverbose( void* vinfo, 
					      bool with_position, 
					      int vlevel, int detail )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vinfo);

      message::Message_Stream msg(message::noinit);
      if( with_position )
	info->msg.verbose( vlevel, &info->file_pos, detail ).copy_to(msg);
      else
	info->msg.verbose( vlevel, message::GLOB::no_position, detail ).
	  copy_to(msg);
      
      return msg;
    }

    //******************************
    // functions used by the parser
    //******************************

    // changes back to the parent tree node
    inline void change_to_parent( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->tree_node_done();
    }

    // tells the lexer to resolve identifiers as properties
    inline void resolve_properties( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_property;
    }

    // tells the lexer to resolve identifiers as property references
    inline void resolve_references( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_reference;
    }
  }
}

#endif
