/*****************************************************************************/
/**   This file offers a parser for the Animation Description Language     **/
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


#ifndef __anitmt_input_adlparser__
#define __anitmt_input_adlparser__

#include <message/message.hpp>

#include <input/input.hpp>
#include "animation.hpp"

class adlparser_FlexLexer;	// declare prototype for lexer

namespace anitmt
{
  //***************************************
  //! Input Interface for reading ADL files
  //***************************************
  
  class ADL_Input : public Input_Interface {
    std::string filename;
    Animation *ani;
    message::Message_Consultant *consultant;
  public:
    //! create animation tree structure
    virtual void create_structure();
    //! create explicite references 
    virtual void insert_expl_ref(); 
    //! insert concrete values for properties
    virtual void insert_values(); 
    
    //! Create a new ADL input interface
    /*
      \param filename - Name of .adl-file
      \param ani - Animation structure to be filled
      \param c - Message consultant that get's all errors etc. */
    ADL_Input( std::string filename, Animation *ani, 
	       message::Message_Consultant *c);
  };

  namespace adlparser
  {
    enum pass_type{pass1,pass2};

    //! parse an adl file (defined in parser.yy)
    int parse_adl( proptree::Prop_Tree_Node *node, 
		   message::Message_Consultant *c, 
		   std::string filename, pass_type pass );

  }
}

//include inline implementation
#include "adlparser_inline.cpp"

#endif
