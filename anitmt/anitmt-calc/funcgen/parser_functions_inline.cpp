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

namespace funcgen
{
  // **********************
  // interfaces to lexer
  // **********************

  inline int yylex( Token *lvalp, void *vinfo )
  {
    afd_info *info = static_cast<afd_info*> (vinfo);
    info->lexer->yylval = lvalp; // lvalue variable to return token value
    return info->lexer->yylex();
  }

  //*************************
  // interfaces to messages
  //*************************

  inline message::Message_Stream yyerr( void* vinfo, int back )
  {
    afd_info *info = static_cast<afd_info*>(vinfo);
      
    message::Abstract_Position *pos = 0;
    if( back >= 0 )		// shall I report an old position?
      pos = info->get_old_pos(back);
    if( !pos )		// still no position set
      pos = info->get_pos();	// use current position
    message::Message_Stream msg(message::noinit);
    info->msg.error( pos ).copy_to(msg);
    return msg;
  }

  inline message::Message_Stream yywarn( void* vinfo, int back )
  {
    afd_info *info = static_cast<afd_info*>(vinfo);

    message::Abstract_Position *pos = 0;
    if( back >= 0 )		// shall I report an old position?
      pos = info->get_old_pos(back);
    if( !pos )		// still no position set
      pos = info->get_pos();	// use current position

    message::Message_Stream msg(message::noinit);
    info->msg.warn( pos ).copy_to(msg);
    return msg;
  }

  inline message::Message_Stream yyverbose( void* vinfo, int back, 
					    bool with_position, 
					    int vlevel, int detail )
  {
    afd_info *info = static_cast<afd_info*>(vinfo);

    message::Message_Stream msg(message::noinit);
    if( with_position )
      {
	message::Abstract_Position *pos = 0;
	if( back >= 0 )		// shall I report an old position?
	  pos = info->get_old_pos(back);
	if( !pos )		// still no position set
	  pos = info->get_pos();	// use current position

	info->msg.verbose( vlevel, pos, detail ).copy_to(msg);
      }
    else
      info->msg.verbose( vlevel, message::GLOB::no_position, detail ).
	copy_to(msg);
      
    return msg;
  }

  //******************************
  // functions used by the parser
  //******************************

  //! sets the position of a Property in the adl source
  inline void initialize_lexer( void *vptr_info )
  {
    afd_info *info = static_cast<afd_info*>(vptr_info);
    info->lexer->goto_initial_state();
  }
}

#endif
