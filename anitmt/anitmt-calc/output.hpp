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

#ifndef __AniTMT_Output_Interface__
#define __AniTMT_Output_Interface__

#include "animation.hpp"

namespace anitmt
{
  class Output_Interface 
  {
  protected:
    Animation *ani;
  public:
    //! init interface (ex: check if scene file exists)
    virtual void init() throw( EX ) = 0;
    //! check the components (verify them and copy files)
    virtual void check_components() throw( EX ) = 0;
    //! process the resulting animation (ex: integrate it in scene description)
    virtual void process_results() throw( EX ) = 0; 
    
    Output_Interface( Animation *a ) : ani(a) {}
    virtual ~Output_Interface() {}
  };
}

#endif

