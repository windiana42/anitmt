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

#ifndef __AniTMT_Input_Interface__
#define __AniTMT_Input_Interface__

#include "animation.hpp"

namespace anitmt
{
  class Input_Interface 
  {
  protected:
    Animation *ani;
  public:
    inline void set_animation( Animation *a ) { ani = a; }

    //! create animation tree structure
    virtual void create_structure() = 0;
    //! create explicite references 
    virtual void insert_expl_ref() = 0; 
    //! insert concrete values for properties
    virtual void insert_values() = 0; 
    
    virtual ~Input_Interface() {}
  };
}

#endif

