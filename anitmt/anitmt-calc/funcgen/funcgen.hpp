/*****************************************************************************/
/**   functionality generation main header                          	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann, Wolfgang Wieser				    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de, wwieser@gmx.de			    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef _ANITMT_FUNCGEN_FUNCGEN_HPP_
#define _ANITMT_FUNCGEN_FUNCGEN_HPP_ 1

// Parameter system: 
#include <hlib/parbase.h>

class FuncgenParameters : par::ParameterConsumer_Overloaded
{
  public:
    FuncgenParameters(par::ParameterManager *manager);
    ~FuncgenParameters();
    
    // Overriding virtual: 
    int CheckParams();
    
    int yydebug;   // parser debugging info
    int stdebug;   // structure debugging info
    RefString in_file;
    RefString out_basename;
    RefString namesp;
    RefStrList include_path;
};

#endif  /* _ANITMT_FUNCGEN_FUNCGEN_HPP_ */
