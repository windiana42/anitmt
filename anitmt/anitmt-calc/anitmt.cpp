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
  /*! Involks all function calls to process the animation 
    (returns number of errors) */
  int AniTMT::process()
  {
    // init animation tree
    ani.init();

    // get the parameters
    param.read_parameters();

    // initialize filters
    input.init();
    output.init();

    // read input data 
    input.read_structure();
    // finish structure initialization
    ani.hierarchy_final_init();	
    // insert values
    input.read_values();

    // may already check, whether the components match to output 
    output.check_components();

    ani.finish_calculations();

    output.process_results();
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

