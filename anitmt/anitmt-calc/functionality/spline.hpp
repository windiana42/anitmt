/*****************************************************************************/
/** AniTMT -- A program for creating foto-realistic animations		    **/
/**   This file belongs to the component anitmt-calc			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** Website: http://www.anitmt.org/      				    **/
/** EMail:   anitmt@theofel.de						    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#ifndef __functionality_splines__
#define __functionality_splines__

#include <list>
#include <vector>
#include <val/val.hpp>

#include "sp_curve.hpp"

namespace functionality
{
  // virtual class for splines
  class Spline : public Space_Curve
  {
  public:
    Spline::Spline( std::list<values::Vector> control_points,
		    double max_a, double max_l, double min_l, int anz );

  private:
    std::vector<values::Vector> control_points;
    int point_count;

    values::Vector get_point_pos( double t );
    virtual values::Vector get_local_point_pos( double t,
						values::Vector p0,
						values::Vector p1,
						values::Vector p2,
						values::Vector p3 ) = 0;
    //! derived concrete classes have to call init() !
  };

  // class for bezier curves
  class BSpline : public Spline
  {
  public:
    BSpline::BSpline( std::list<values::Vector> control_points,
		      double max_a, double max_l, double min_l, int anz );

  private:
    virtual values::Vector get_local_point_pos( double t,
						values::Vector p0,
						values::Vector p1,
						values::Vector p2,
						values::Vector p3 );
  };
}

#endif
