/*****************************************************************************/
/**   This file offers the output in a raw data format           	    **/
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

#ifndef __AniTMT_Raw_Output_Interface__
#define __AniTMT_Raw_Output_Interface__

#include <message/message.hpp>

#include "../output.hpp"

namespace anitmt
{
  //! raw data output format
  class Raw_Output : public Output_Interface, public message::Message_Reporter
  {
    Animation *ani;
  public:
    //! init interface (ex: check if scene file exists)
    virtual void init() throw();
    //! check the components (verify them and copy files)
    virtual void check_components() throw();
    //! process the resulting animation (ex: integrate it in scene description)
    virtual void process_results() throw(); 

    Raw_Output( Animation *a );
  };
}

#endif

