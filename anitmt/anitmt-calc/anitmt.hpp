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

#ifndef __AniTMT_main_class__
#define __AniTMT_main_class__

#include "animation.hpp"

#include "input/input.hpp"
#include "output/output.hpp"

#include <param/param.hpp>
#include <message/message.hpp>

namespace anitmt
{
  enum Message_IDs{ MID_AniTMT, MID_Core, MID_Output, MID_Input, MID_Parameter,
		    MID_POV, MID_RAW, 
		    MID_ADL };

  class AniTMT 
  {
    message::Message_Consultant default_msg_consultant;
    Animation ani;
    
    bool has_errors();
    void report_error_state();
  public:
    //! handles input formats 
    Input_Manager      input;	
    //! handles output format
    Output_Manager     output;	
    //! central message manager
    message::Message_Manager    msg_manager; 

    /*! Involks all function calls to process the animation 
      (returns number of errors) */
    int process();

    AniTMT( param::Parameter_Manager *param, 
	    message::Message_Handler *msg_handler );

    //*****************
    // Semi global data 
    param::Parameter_Manager &param;
  };
}

#endif
