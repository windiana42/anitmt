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

#include "adlparser.hpp"

#include <string>


namespace anitmt
{
  //***************************************
  //! Input Interface for reading ADL files
  //***************************************

  //! create animation tree structure
  void ADL_Input::create_structure()
  {
    adlparser::parse_adl( &ani->ani_root, consultant, filename, 
			  adlparser::pass1 );
  }
  //! create explicite references 
  void ADL_Input::insert_expl_ref()
  {
    // they are inserted together with values
  }
  //! insert concrete values for properties
  void ADL_Input::insert_values()
  {
    parse_adl( &ani->ani_root, consultant, filename, adlparser::pass2 );
  }
    
  //! Create a new ADL input interface
  /*
    \param filename - Name of .adl-file
    \param ani - Animation structure to be filled
    \param c - Message consultant that get's all errors etc. */
  ADL_Input::ADL_Input( std::string f, Animation *a, 
			message::Message_Consultant *c )
    : filename(f), ani(a), consultant(c)  {}
    
}

