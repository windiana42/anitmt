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

#ifndef __functionality_space_curves__
#define __functionality_space_curves__

#include <list>
#include <val/val.hpp>

namespace functionality
{
  // this class is used to store one point of the space curve
  class sp_curve_point{
  public:
    double		t;	// variable that ranges from 0 to 1
    values::Vector	pos;	// position of this point

    // resulting values
    double		stretch; // stretch to this point
    values::Vector	front;	// front direction at this point
    values::Vector	up;	// up direction at this point

    sp_curve_point( double it, values::Vector ipos ) : t(it), pos(ipos) {}
  };

  // virtual class for space curves
  class space_curve{
    typedef std::list<sp_curve_point> pointtype;
    pointtype points;

    virtual values::Vector get_point_pos( double t ) = 0;

    bool initialized;		// are points, streches and fronts initialized
    bool init_up;		// are up vectors initialized

    // these variables have an influence on the number of sample points on the
    // curve
    double max_angle;      // = 1 /180 *const_PI;// 1 degree in radians
    double max_len;        // = 1;	// maximal length between 2 points
    double min_len;        // = .01;    // minimal length 
    int    init_point_anz; // = 10; // initial number of points (>3)

  protected:
    // initializes points, streches and front-vectors
    // this should be called by derived classes after initializing their
    // get_point_pos() method
    void init();
  public:
    double length;		// length of whole curve

    // calculates up-vectors depending on one at the beginning ...
    void calc_up_forward ( values::Vector start_up );
    // ... or the end
    void calc_up_backward( values::Vector end_up );
    
    void print_points();
    int  get_point_anz();

    values::Vector get_pos    ( double s );
    values::Vector get_front  ( double s );
    values::Vector get_up     ( double s );
    values::Vector get_beg_up ();
    values::Vector get_end_up ();

    space_curve( double max_a, double max_l, double min_l, int anz) 
      : initialized(false), init_up(false), max_angle(max_a), 
	max_len(max_l), min_len(min_l), init_point_anz(anz) {}


    virtual ~space_curve(){
#ifdef __debug__
      print_points();
#endif
    } 

  };

  // class for bezier curves
  class bezier_curve : public space_curve{
    values::Vector p0,p1,p2,p3;

    values::Vector get_point_pos( double t );

  public:
    bezier_curve( values::Vector point0, values::Vector point1, 
		  values::Vector point2, values::Vector point3,
		  double max_a, double max_l, double min_l, int anz )
      : space_curve( max_a, max_l, min_l, anz ),
	p0(point0), p1(point1), p2(point2), p3(point3)	
    {
      init();
    }
  };
}

#endif
