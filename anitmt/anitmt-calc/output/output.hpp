/*****************************************************************************/
/**   This file offers an output interface                       	    **/
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

// must remain here, as anitmt.hpp needes the declaration of Output_Manager!!
#include "anitmt.hpp"

#ifndef __AniTMT_Output_Interface__
#define __AniTMT_Output_Interface__

namespace anitmt
{
  class Output_Interface;
  class Output_Manager;
}

#include "animation.hpp"

#include <param/param.hpp>
#include <proptree/proptree.hpp>
#include <message/message.hpp>

namespace anitmt
{
  class Output_Interface 
  {
  public:
    //! init interface (ex: check if scene file exists)
    virtual void init() throw() = 0;
    //! check the components (verify them and copy files)
    virtual void check_components() throw() = 0;
    //! process the resulting animation (ex: integrate it in scene description)
    virtual void process_results() throw() = 0; 
    
    virtual ~Output_Interface() {}
  };

  class Output_Manager {
    typedef std::list<Output_Interface*> formats_type;
    formats_type formats;
    param::Parameter_Manager *param;
    message::Message_Consultant *message_consultant;

    friend class AniTMT;
    void init();
    void check_components();
    void process_results();
  public:
    void add_output_format( Output_Interface *format );
    Output_Manager( param::Parameter_Manager *par, 
		    message::Message_Consultant *msg,
		    //!!! could be realized by a special interface
		    Animation *ani );
    ~Output_Manager();
  };
}

#endif

