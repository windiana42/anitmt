/*****************************************************************************/
/**   Main AniTMT class	                                        	    **/
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

#include "anitmt.hpp"
#include "save_filled.hpp"

namespace anitmt
{
  bool AniTMT::has_errors()
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
    // get the parameters
    param.read_parameters();
    if( has_errors() ){ report_error_state();return -1; }

    // init animation tree
    ani.init();
    if( has_errors() ){ report_error_state();return -1; }

    // initialize filters
    input.init();
    if( has_errors() ){ report_error_state();return -1; }

    // read input data 
    input.read_structure();
    if( has_errors() ){ report_error_state();return -1; }

    // finish structure initialization
    ani.hierarchy_final_init();	
    if( has_errors() ){ report_error_state();return -1; }

#warning !!! fixed filename for filled ADL output !!!
    save_filled("unfilled.out", &ani );

    // insert values
    input.read_values();
    if( has_errors() ){ report_error_state();return -1; }

#warning !!! fixed filename for filled ADL output !!!
    save_filled("expl_filled.out", &ani );

    ani.finish_calculations();
    if( has_errors() ){ report_error_state();return -1; }

#warning !!! fixed filename for filled ADL output !!!
    save_filled("filled.out", &ani );

    output.init();
    if( has_errors() ){ report_error_state();return -1; }

    output.check_components();
    if( has_errors() ){ report_error_state();return -1; }

    output.process_results();
    if( has_errors() ){ report_error_state();return -1; }

    report_error_state();
    return 0;
  }

  AniTMT::AniTMT( param::Parameter_Manager *par, 
		  message::Message_Handler *msg_handler )
    : default_msg_consultant(&msg_manager, MID_AniTMT ), 
      ani("noname", par, &msg_manager),
      input (par, &default_msg_consultant, &ani), 
      output(par, &default_msg_consultant, &ani),
      msg_manager(msg_handler), param(*par)
    // it should be ok to pass &msg_manager earlier...
  {
  }
}

