/*****************************************************************************/
/** AniTMT -- A program for creating foto-realistic animations		    **/
/**   This file belongs to the component anitmt-view  			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** Website: http://www.anitmt.org/      				    **/
/** EMail:   anitmt@theofel.de						    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/*****************************************************************************/

#ifndef __osdep__
#define __osdep__

#include <string>

#include <unistd.h>

namespace osdep{
  void chdir( std::string dir ){
    ::chdir( dir.c_str() );
  }
}

#endif
