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

#include "spline.hpp"

#include <val/val.hpp>

#include <fstream>

namespace functionality
{

  /**********/
  /* Spline */
  /**********/

  Spline::Spline( std::list<values::Vector> points_list,
		  double max_a, double max_l, double min_l, int anz )
      : Space_Curve(max_a,max_l,min_l,anz),
	control_points(points_list.size()+4)	 
  {
    if( points_list.size() )
    {
      // add first point 3 times
      control_points[0] = points_list.front();
      control_points[1] = points_list.front();
      int idx;
      std::list<values::Vector>::iterator point;
      for( point=points_list.begin(), idx = 2; 
	   point != points_list.end(); ++point, ++idx )
      {
	control_points[idx] = *point;
      }
      // add last point 3 times
      control_points[idx] = points_list.back(); ++idx;
      control_points[idx] = points_list.back(); ++idx;

      point_count = idx;
    }
    else
    {
      point_count = 0;
    }
  }

  values::Vector Spline::get_point_pos( double t )
  {
    int parts = point_count - 3; // number of parts spline is devided in

    if( parts >= 2 )		// at least two parts (one real control point)
    {
      // following code needs equal sized intervals of t range per part
      // that is why t=1 has to be handled differently
      if( t == 1 )		
	return control_points.back();

      int part = int(t * parts); // 0 <= part <= parts - 1
      assert( (0 <= part) && (part < parts) );

      values::Vector &p1 = control_points[part];
      values::Vector &p2 = control_points[part+1];
      values::Vector &p3 = control_points[part+2];
      values::Vector &p4 = control_points[part+3];
      
      double local_t = t * parts - part; // parameter within spline part
      assert( (0 <= local_t) && (local_t <= 1) );
      return get_local_point_pos( local_t, p1, p2, p3, p4 );
    }
    else
    {
      return values::Vector(0,0,0); // no control points, no chance
    }
  }

  /***********/
  /* BSpline */
  /***********/

  BSpline::BSpline( std::list<values::Vector> control_points,
		    double max_a, double max_l, double min_l, int anz )
      : Spline(control_points,max_a,max_l,min_l,anz) 
  {
    init();
  }

  values::Vector BSpline::get_local_point_pos( double t, 
					       values::Vector p1, 
					       values::Vector p2,
					       values::Vector p3, 
					       values::Vector p4 )
  {
    // Qubic interpolation:
    // v = (1-t)^3 * p1 + 3*(1-t)^2*t * p2 + 3*(1-t)*t^2 * p3 + t^3 * p4
    return 
      (1-t)*(1-t)*(1-t)*p1 + 3*(1-t)*(1-t)*t*p2 + 3*(1-t)*t*t*p3 + t*t*t*p4;
  }
}
