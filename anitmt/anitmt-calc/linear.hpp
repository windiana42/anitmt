/*****************************************************************************/
/**   This file offers a linear scalar interpolating function		    **/
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

#ifndef __AniTMT_Scalar_Linear__
#define __AniTMT_Scalar_Linear__

namespace anitmt {
  class Scal_Linear;
}

#include "val.hpp"
#include "tmttype.hpp"
#include "property.hpp"
#include "solver.hpp"
#include "proptree.hpp"
#include "return.hpp"

namespace anitmt{

  //**********************************************************
  // Scal_Linear: moves Objects on a staight flight path
  //**********************************************************
  class Scal_Linear : public Prop_Tree_Node, 
		      public Return< values::Scalar > {
    static const std::string type_name;

    Scalar_Property v0;		// startvalue
    Scalar_Property ve;		// endvalue
    Scalar_Property d;		// difference
    Scalar_Property t;		// duration in s
    Scalar_Property t0;		// starttime in s
    Scalar_Property te;		// endtime in s
    Scalar_Property t_f;	// duration in frames
    Scalar_Property t0_f;	// startframe 
    Scalar_Property te_f;	// endframe
    Scalar_Property s;		// slope or changing speed

    //...
  protected:
  public:
    static std::string get_type_name();

    Scal_Linear( std::string name );
    
    values::Scalar get_return_value( values::Scalar t, 
				     values::Scalar = values::Scalar() );

    bool try_add_child( Prop_Tree_Node *node );
  };
}
#endif

