/*****************************************************************************/
/**   This file offers an input interface                       	    **/
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

// must remain here, as anitmt.hpp needs the declaration of Input_Manager
#include "anitmt.hpp"

#ifndef __AniTMT_Input_Interface__
#define __AniTMT_Input_Interface__

namespace anitmt
{
  class Input_Interface;
  class Input_Manager;
};

#include <param/param.hpp>
#include <message/message.hpp>

#include "animation.hpp"

namespace anitmt
{
  class Input_Interface 
  {
  public:
    //! create animation tree structure
    virtual void create_structure() = 0;
    //! create explicite references 
    virtual void insert_expl_ref() = 0; 
    //! insert concrete values for properties
    virtual void insert_values() = 0; 

    virtual ~Input_Interface() {}
  };

  class Input_Manager {
    typedef std::list<Input_Interface*> formats_type;
    formats_type formats;
    param::Parameter_Manager *param;
    message::Message_Consultant *consultant;

    friend class AniTMT;
    void init();
    void read_structure();
    void read_values();
  public:
    void add_input_format( Input_Interface *format );
    Input_Manager( param::Parameter_Manager *par, 
		   message::Message_Consultant *consultant,
		   Animation *ani );
    ~Input_Manager();
  };
}

#endif

