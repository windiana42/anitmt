/*****************************************************************************/
/**   Main AniTMT class	                                        	    **/
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

#include "anitmt.hpp"

namespace anitmt
{
  bool AniTMT::is_errors()
  {
    return default_msg_consultant.get_num_messages(message::MT_Error) > 0;
  }
  void AniTMT::report_error_state()
  {
    message::Message_Reporter msg(&default_msg_consultant);
    msg.verbose() << "  Errors: " << msg.get_num_errors()
		  << "  Warnings: " << msg.get_num_warnings();
  }

  /*! Involks all function calls to process the animation 
    (returns number of errors) */
  int AniTMT::process()
  {
    // init animation tree
    ani.init();

    // get the parameters
    param.read_parameters();
    if( is_errors() ){ report_error_state();return -1; }

    // initialize filters
    input.init();
    if( is_errors() ){ report_error_state();return -1; }
    output.init();
    if( is_errors() ){ report_error_state();return -1; }

    // read input data 
    input.read_structure();
    if( is_errors() ){ report_error_state();return -1; }

    // finish structure initialization
    ani.hierarchy_final_init();	
    if( is_errors() ){ report_error_state();return -1; }

    // insert values
    input.read_values();
    if( is_errors() ){ report_error_state();return -1; }

    // may already check, whether the components match to output 
    output.check_components();
    if( is_errors() ){ report_error_state();return -1; }

    ani.finish_calculations();
    if( is_errors() ){ report_error_state();return -1; }

    output.process_results();
    if( is_errors() ){ report_error_state();return -1; }

    report_error_state();
    return 0;
  }

  AniTMT::AniTMT( param::Parameter_Manager *par, 
		  message::Message_Handler *msg_handler )
    : default_msg_consultant(&msg_manager, MID_AniTMT ),
      ani("noname", &msg_manager),
      input (par, &default_msg_consultant, &ani), 
      output(par, &default_msg_consultant, &ani),
      msg_manager(msg_handler), 
      param(*par)
  {
  }
}

