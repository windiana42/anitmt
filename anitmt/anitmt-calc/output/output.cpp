/*****************************************************************************/
/**   This file offers an output interface                       	    **/
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

#include "output.hpp"

#include "oformats.hpp"

namespace anitmt
{
  void Output_Manager::init()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      (*i)->init();
    }
  }
  void Output_Manager::check_components()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      (*i)->check_components();
    }
  }
  void Output_Manager::process_results()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      (*i)->process_results();
    }
  }

  //! adds a output format
  //!   /param format is deleted automatically by destructor of Output_Manager
  void Output_Manager::add_output_format( Output_Interface *format )
  {
    formats.push_back( format );
  }

  Output_Manager::Output_Manager( param::Parameter_Manager *par, 
				message::Message_Consultant *msg,
				Animation *ani )
    : param(par), message_consultant(msg)
  {
    add_output_format( new Raw_Output(ani) );
    add_output_format( new Pov_Output(ani) );
  }

  Output_Manager::~Output_Manager()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      delete (*i);
    }
  }
}


