/*****************************************************************************/
/**   This file offers a class where the parser stores information         **/
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

#ifndef __anitmt_afd_parsinfo_inline_implementation__
#define __anitmt_afd_parsinfo_inline_implementation__

#include "parsinfo.hpp"

namespace funcgen
{
  //! store position for later access
  inline void afd_info::store_pos()
  {
    while( old_positions.size() >= max_old_positions )
      {
	delete old_positions.back();
	old_positions.pop_back();
      }
    old_positions.push_front( get_pos() );
  }
  //! get current position (must be deleted!)
  inline message::Abstract_Position *afd_info::get_pos()
  {
    return file_pos.duplicate();
  }
  //! get stored position n (n=0: last) (must be deleted!)
  inline message::Abstract_Position *afd_info::get_old_pos( unsigned n)
  {
    // too few elements availible?
    if( old_positions.size() <= n ) return 0;

    return old_positions[n];
  }
  //! set maximum number of stored positions
  inline void afd_info::set_max_old_positions( unsigned n )
  {
    max_old_positions = n;
    while( old_positions.size() > max_old_positions )
      {
	delete old_positions.back();
	old_positions.pop_back();
      }
  }
}
#endif
