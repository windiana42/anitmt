/*****************************************************************************/
/**   This file offers a parser for the Animation Description Language     **/
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

#include "adlparser.hpp"

#include <string>

//!!! old parameter interface
#include <par/params.hpp>

namespace anitmt
{
  // ***************************************
  // Input Interface for reading ADL files
  // ***************************************

  //! init interface
  void ADL_Input::init()
  {
    // dirty interface that allows only adl input files
    stringlist adlfiles = ani->GLOB.param.adl();
    if( adlfiles.empty() )
    {
      error() << "no animation descriptions specified";
    }
  }

  //! create animation tree structure
  void ADL_Input::create_structure()
  {
    // dirty interface that allows only adl input files
    stringlist adlfiles = ani->GLOB.param.adl();
    for(stringlist::iterator i=adlfiles.begin(); i!=adlfiles.end(); ++i)
    {
      const std::string &filename = *i;
      adlparser::parse_adl( &ani->ani_root, consultant, filename, 
			    adlparser::pass1 );
    }
  }
  //! create explicite references 
  void ADL_Input::insert_expl_ref()
  {
    // they are inserted together with values
  }
  //! insert concrete values for properties
  void ADL_Input::insert_values()
  {
    // dirty interface that allows only adl input files
    stringlist adlfiles = ani->GLOB.param.adl();
    for(stringlist::iterator i=adlfiles.begin(); i!=adlfiles.end(); ++i)
    {
      const std::string &filename = *i;
      parse_adl( &ani->ani_root, consultant, filename, adlparser::pass2 );
    }
  }
    
  //! Create a new ADL input interface
  /*
    \param filename - Name of .adl-file
    \param ani - Animation structure to be filled
    \param c - Message consultant that get's all errors etc. */
  ADL_Input::ADL_Input( Animation *a, message::Message_Consultant *c )
    : message::Message_Reporter(c), ani(a), consultant(c)  {}
    
}

