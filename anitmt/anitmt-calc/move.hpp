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

namespace anitmt {
  class Obj_Move;
  class Obj_Move_Straight;
}

#include <val/val.hpp>
#include <solve/solver.hpp>
#include "tmttype.hpp"
#include "property.hpp"
#include "proptree.hpp"
#include "return.hpp"

namespace anitmt{

  //**********************************************************
  // Obj_Move: moves Objects on a flight path
  //**********************************************************
  class Obj_Move : public Prop_Tree_Node, 
		   public Return<Position>,
		   public Return<Direction>,
		   public Return<Up_Vector> {

    static const std::string type_name;
    
    Contain_Return<Position>  pos;
    Contain_Return<Direction> dir;
    Contain_Return<Up_Vector> up;

    Vector_Property c;		// rotation center (?piveau? point)

    bool try_add_child( Prop_Tree_Node *node );

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  public:
    static std::string get_type_name();

    Obj_Move( std::string name, Animation *ani );

    std::pair<bool,Position>  get_return_value
    ( values::Scalar t, Position &m = Return<Position>::type_id ) 
      throw( EX_user_error );
    std::pair<bool,Direction> get_return_value
    ( values::Scalar t, Direction &m = Return<Direction>::type_id ) 
      throw( EX_user_error );
    std::pair<bool,Up_Vector> get_return_value
    ( values::Scalar t, Up_Vector &m = Return<Up_Vector>::type_id ) 
      throw( EX_user_error );
  };
  

  //**********************************************************
  // Obj_Move_Straight: moves Objects on a staight flight path
  //**********************************************************
  class Obj_Move_Straight : public Prop_Tree_Node, 
			    public Return<Position>, 
			    public Return<Direction>,
			    public Return<Up_Vector> {
    static const std::string type_name;

    Vector_Property s0;		// startpos
    Vector_Property se;		// endpos
    Vector_Property d0;		// startdir
    Vector_Property de;		// enddir
    Vector_Property u0;		// startup
    Vector_Property ue;		// endup
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

    //! individual final init after hierarchy is set up (Has to call the 
    //! function of the return type container
    virtual void final_init();
  protected:
  public:
    static std::string get_type_name();

    Obj_Move_Straight( std::string name, Animation *ani );
    
    std::pair<bool,Position> get_return_value
    ( values::Scalar t, Position& = Return<Position>::type_id )
      throw( EX_user_error );
    std::pair<bool,Direction> get_return_value
    ( values::Scalar t, Direction& = Return<Direction>::type_id )
      throw( EX_user_error );
    std::pair<bool,Up_Vector> get_return_value
    ( values::Scalar t, Up_Vector& = Return<Up_Vector>::type_id )
      throw( EX_user_error );

    bool try_add_child( Prop_Tree_Node *node );
  };
}
#endif

