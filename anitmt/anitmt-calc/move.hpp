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

#include "val.hpp"
#include "tmttype.hpp"
#include "property.hpp"
#include "solver.hpp"
#include "proptree.hpp"
#include "return.hpp"

namespace anitmt{

  //**********************************************************
  // Obj_Move: moves Objects on a flight path
  //**********************************************************
  class Obj_Move : public Prop_Tree_Node, 
		   public Return<values::Matrix>,
		   public Return<Position>,
		   public Return<Direction>,
		   public Return<Up_Vector> {

    static const std::string type_name;
    
    Contain_Return<Position>  pos;
    Contain_Return<Direction> dir;
    Contain_Return<Up_Vector> up;

    Vector_Property c;		// rotation center (?piveau? point)

    bool try_add_child( Prop_Tree_Node *node );
  public:
    static std::string get_type_name();

    Obj_Move( std::string name );

    values::Matrix get_return_value( values::Scalar t, 
				     values::Matrix m = values::Matrix() );
    Position get_return_value( values::Scalar t, 
			       Position m = Position() );
    Direction get_return_value( values::Scalar t, 
				Direction m = Direction() );
    Up_Vector get_return_value( values::Scalar t, 
				Up_Vector m = Up_Vector() );
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
  protected:
  public:
    static std::string get_type_name();

    Obj_Move_Straight( std::string name );
    
    Position get_return_value( values::Scalar t, Position = Position() );
    Direction get_return_value( values::Scalar t, Direction = Direction() );
    Up_Vector get_return_value( values::Scalar t, Up_Vector = Up_Vector() );

    bool try_add_child( Prop_Tree_Node *node );
  };
}
#endif

