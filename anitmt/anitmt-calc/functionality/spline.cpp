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
      int part = int(t * parts); // 0 <= part <= parts - 1

      if( part == parts ) return control_points.back();	// for t == 1

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

  /*****************/
  /* Matrix_Spline */
  /*****************/

  Matrix_Spline::Matrix_Spline( std::list<values::Vector> control_points,
		    double max_a, double max_l, double min_l, int anz )
    : Spline(control_points,max_a,max_l,min_l,anz)
  {}

  values::Vector Matrix_Spline::get_local_point_pos( double t, 
					       values::Vector p1, 
					       values::Vector p2,
					       values::Vector p3, 
					       values::Vector p4 )
  {
    double t_sq = t*t; double t_cub = t_sq * t;
    // matrix usage might be different than in some books (trasposed)
    values::Vector<4> prod = matrix * values::Vector<4>(t_cub, t_sq, t, 1);

    values::Vector<3> pos = 
      prod[0] * p1 + prod[1] * p2 + prod[2] * p3 + prod[3] * p4;

    return pos;
  }

  /***********/
  /* BSpline */
  /***********/

  BSpline::BSpline( std::list<values::Vector> control_points,
		    double max_a, double max_l, double min_l, int anz )
    : Matrix_Spline(control_points,max_a,max_l,min_l,anz)
  {
    matrix[0][0] = -1; matrix[0][1] =  3; matrix[0][2] = -3; matrix[0][3] =  1;
    matrix[1][0] =  3; matrix[1][1] = -6; matrix[1][2] =  0; matrix[1][3] =  4;
    matrix[2][0] = -3; matrix[2][1] =  3; matrix[2][2] =  3; matrix[2][3] =  1;
    matrix[3][0] =  1; matrix[3][1] =  0; matrix[3][2] =  0; matrix[3][3] =  0;

    matrix *= 1./6.;

    init();
  }

  /***********/
  /* CRSpline */
  /***********/

  CRSpline::CRSpline( std::list<values::Vector> control_points,
		    double max_a, double max_l, double min_l, int anz )
    : Matrix_Spline(control_points,max_a,max_l,min_l,anz)
  {
    matrix[0][0] = -1; matrix[0][1] =  2; matrix[0][2] = -1; matrix[0][3] =  0;
    matrix[1][0] =  3; matrix[1][1] = -5; matrix[1][2] =  0; matrix[1][3] =  2;
    matrix[2][0] = -3; matrix[2][1] =  4; matrix[2][2] =  1; matrix[2][3] =  0;
    matrix[3][0] =  1; matrix[3][1] = -1; matrix[3][2] =  0; matrix[3][3] =  0;

    matrix *= 1./2.;

    init();
  }
}
