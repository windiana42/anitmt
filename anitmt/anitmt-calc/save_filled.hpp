/*****************************************************************************/
/**   This file offers a save function in the filled ADL format		    **/
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

#ifndef __AniTMT_Save_Filled__
#define __AniTMT_Save_Filled__

#include <string>
#include <proptree/proptree.hpp>
#include "animation.hpp"

namespace anitmt {
  static const std::string WN_INDENT_STRING = "  ";
  void save_filled( std::string filename, Animation *root );

  //****************
  // debug function
  //****************

  class Save_Filled_Action : public solve::Priority_Action {
    std::string filename;
    Animation *root;
  public:
    virtual void do_it();

    Save_Filled_Action( solve::Priority_System *sys, 
			solve::Priority_System::level_type level,
			std::string filename, Animation *root );
  };
  
  void save_filled_action( solve::Priority_System *sys, 
			   solve::Priority_System::level_type level,
			   std::string filename, Animation *root );

}

#endif

