/*****************************************************************************/
/**   This file offers a move function                			    **/
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

#ifndef __AniTMT_Object_Move__
#define __AniTMT_Object_Move__

#include <vector>
#include <list>
#include <map>

namespace anitmt{
  class Obj_Move_Straight;
}

#include "val.hpp"
#include "property.hpp"
#include "solver.hpp"
#include "proptree.hpp"

namespace anitmt{
  //**********************************************************
  // Obj_Move_Straight: moves Objects on a staight flight path
  //**********************************************************
  class Obj_Move_Straight : public Prop_Tree_Node{
    Vector_Property s0;		// startpos
    Vector_Property se;		// endpos
    Vector_Property d0;		// startdir
    Vector_Property de;		// enddir
    Scalar_Property s;		// length
    Scalar_Property t;		// duration in s
    Scalar_Property t0;		// starttime in s
    Scalar_Property te;		// endtime in s
    Scalar_Property t_f;	// duration in frames
    Scalar_Property t0_f;	// startframe 
    Scalar_Property te_f;	// endframe
    Scalar_Property a;		// acceleration
    Scalar_Property v0;		// startspeed
    Scalar_Property ve;		// endspeed

    //...
  protected:
  public:
    Obj_Move_Straight(){
      new Accel_Solver( &s, &t, &a, &v0, &ve );
      new Diff_Solver( &t, &t0, &te );
      new Diff_Solver( &t_f, &t0_f, &te_f );
      
      properties["startpos"] = &s0;
      properties["endpos"] = &se;
      properties["length"] = &s;
      properties["duration"] = &t;
      properties["acceleration"] = &a;
      properties["startspeed"] = &v0;
      properties["endspeed"] = &ve;
      //...
      
      /*
      start_give_props["time"] = &t0;
      start_give_props["pos"] = &s0;
      start_give_props["speed"] = &v0;
      //...

      end_give_props["time"] = &te;
      end_give_props["pos"] = &se;
      end_give_props["speed"] = &ve;
      //...

      priority_level[1] = new &t0, dir::back)  );
      priority_level[2].push_back( prop_dir_pair(&te, dir::front)  );
      //...
      */
 
      //add_start_give_prop(&t0, "time", 1 /*priority*/ );
      //add_end_give_prop  (&te, "time", 2 /*priority*/ );
 
      priority_level[3] = new Default_Value<values::Scalar>( &a, 0, this );
      //...
      /*
      add_default_value( &a, values::Scalar(0) );
      */
    }
  };

}
#endif

