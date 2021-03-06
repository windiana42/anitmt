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

#include "sp_curve.hpp"

#include <val/val.hpp>

#include <fstream>
#include <iomanip>
#include <assert.h>

namespace functionality{

  /***************/
  /* Space_Curve */
  /***************/

  // debug function
  void Space_Curve::print_points(){
    std::ofstream os("points.out");

    double last_s = 0;
    values::Vector last_front;

    os << std::setw(10) << "t" << "\t" << std::setw(10) << "stretch" << "\t" 
       << std::setw(40) << "pos" << "\t" << std::setw(40) << "front" << "\t" 
       << std::setw(40) << "up" <<"\t" << std::setw(10) << "s/t" << "\t" 
       << std::setw(10) << "ds" << "\t" << std::setw(10) << "angle" 
       << std::endl;
    for( pointtype::iterator it=points.begin(); it != points.end(); ++it )
      {
	//Workaround for VC++
	#ifndef WIN32
	os << std::setw(10) << it->t << "\t" << std::setw(10) << it->stretch 
	   << "\t" << std::setw(40) << it->pos << "\t" 
	   << std::setw(40) << it->front << "\t" << std::setw(40) << it->up 
	   << "\t" << std::setw(10) << it->stretch / it->t 
	   << "\t" << std::setw(10) << it->stretch - last_s << "\t" 
	   << std::setw(10) << vec_angle(it->front, last_front) 
	   << std::endl;	  
	#endif
	last_s = it->stretch;
	last_front = it->front;
      }
  }

  int Space_Curve::get_point_anz()
  {
    return points.size();
  }

  void Space_Curve::init()
  {
    if( !initialized )
    {
      // initialize with some points
      for( int i=0; i < init_point_anz; i++ )
      {
	double t = double(i) / (init_point_anz-1); // t ranges from 0 to 1
	points.push_back( Sp_Curve_Point( t, get_point_pos(t) ) );
      }

      points.front().stretch = 0;

      // initialize sample points with a max angle and a max length between
      // each triple of them
      pointtype::iterator it0 = points.begin();
      pointtype::iterator it1;
      pointtype::iterator it2;
      for(;;){			// forever
	it1 = it0; it1++;	// it1 is the following point of it0
	it2 = it1; it2++;	// it2 is the following point of it1


	values::Vector &p0 =  it0->pos;
	values::Vector &p1 =  it1->pos;

	double len = abs(p1-p0);

	if( len > max_len )
	{
	  // insert one more point with t as arithmetical average
	  // between it and it+1
	  double t = 0.5*(it0->t + it1->t);
	  points.insert( it1, Sp_Curve_Point( t, get_point_pos(t) ) );
	  continue;
	}

	// calculate stretch
	it1->stretch = it0->stretch + len;

	// check if there is no more point triple
	if( it2 == points.end() )
	{
	  length = it1->stretch; // set total length
	  break;		// finish calculation of points
	}

	values::Vector &p2 = it2->pos;

	// check angle between point triple
	double angle = vec_angle( p1 - p0, p2 - p1 );

	// is angle to large?
	if( angle > max_angle )
	{
	  double t;
	  double len;
	  // insert 2 more points with t as arithmetical average
	  // if they points aren't already too close

	  bool nothing_done = true;

	  len = abs(p2-p1);
	  if( len > min_len )
	  {
	    t = 0.5*( it1->t + it2->t);
	    points.insert( it2, Sp_Curve_Point( t, get_point_pos(t) ) );
	    nothing_done = false;
	  }

	  len = abs(p1-p0);
	  if( len > min_len )
	  {
	    t = 0.5*( it0->t + it1->t);
	    points.insert( it1, Sp_Curve_Point( t, get_point_pos(t) ) );
	    nothing_done = false;
	  }

	  if( nothing_done )
	    ++it0;

	  continue;
	}
	else
	{
	  // continue with next point
	  ++it0;
	  continue;
	}
      }

      // calculate fronts
      it0 = it1 = points.begin(); // it1 will be an increased it0
      do
      {
	++it1; 
      }while( (it1 != points.end()) && (it0->pos == it1->pos) );
	
      while( it1 != points.end() )
      {
	values::Vector front = it1->pos - it0->pos;
	while( it0 != it1 )
	{
	  it0->front = front;
	  ++it0;
	  assert( it0 != points.end() );
	}
	assert( it0 == it1 );
	do
	{
	  ++it1;
	}while( (it1 != points.end()) && (it0->pos == it1->pos) );
      }
      values::Vector front = points.back().pos - it0->pos;

      while( (front == values::Vector(0,0,0)) && (it0 != points.begin()) )
	front = points.back().pos - (--it0)->pos;

      while( it0 != points.end() )
      {
	it0->front = front; 
	++it0;
      }

      initialized = true;	// only initialize once
    }
  }

  // calculate up-vectors in forward direction
  void Space_Curve::calc_up_forward( values::Vector start_up )
  {
    if( !init_up )
    {
      points.begin()->up = start_up;

      pointtype::iterator it0, it1;

      it0 = it1 = points.begin(); 
      for( it1++; it1 != points.end(); ++it0, ++it1 )
      {
	values::Vector v1,v2;

	v1 = vec_normalize(it0->front); // normalize to make bigger...
	v2 = vec_normalize(it1->front);

	if( v1 != v2 )
	{
	  // get the rotation of the front vectors
	  values::Vector axis = cross(v1, v2);
	  double angle = get_rotation_around( v1, v2, axis );
	  
	  // do the same rotation with the up vectors
	  it1->up = mat_rotate_around( axis, angle ) * it0->up;
	}
	else
	  it1->up = it0->up;
      }

      init_up = true;
    }
  }

  // calculate up-vectors in backward direction
  void Space_Curve::calc_up_backward( values::Vector end_up ){
    if( !init_up )
      {
	points.rbegin()->up = end_up;

	pointtype::reverse_iterator it0, it1;

	it0 = it1 = points.rbegin(); 
	for( it1++; it1 != points.rend(); ++it0, ++it1 )
	  {
	    values::Vector v1,v2;

	    v1 = vec_normalize(it0->front); // normalize to make bigger...
	    v2 = vec_normalize(it1->front);

	    if( v1 != v2 )
	    {
	      // get the rotation of the front vectors
	      values::Vector axis = cross(v1, v2);
	      double angle = get_rotation_around( v1, v2, axis );
	      
	      // do the same rotation with the up vectors
	      it1->up = mat_rotate_around( axis, angle ) * it0->up;
	    }
	    else
	      it1->up = it0->up;
	  }
	init_up = true;
      }
  }

  // get position at a certain stretch
  values::Vector Space_Curve::get_pos    ( double s ){
    // get the point with stretch greater than s
    pointtype::iterator it;

    for( it = points.begin(); it != points.end(); ++it )
      {
	if( it->stretch > s )
	  break;
      }

    if( it == points.end() ) --it; // if no point found: use last point 

    double s1 = it->stretch;
    double t1 = it->t;
    --it;
    double s0 = it->stretch;
    double t0 = it->t;

    // get koefficient for interpolation
    double q = ( s - s0 ) / (s1 - s0);

    // return position of interpolated t
    return get_point_pos( (1-q) * t0 + q * t1 );
  }

  // get front-vector at a certain stretch
  values::Vector Space_Curve::get_front  ( double s ){
    // get the point with stretch greater than s
    pointtype::iterator it;

    for( it = points.begin(); it != points.end(); ++it )
      {
	if( it->stretch > s )
	  break;
      }

    if( it == points.end() ) --it; // if no point found: use last point 

    double        s1 = it->stretch;
    values::Vector f1 = it->front;
    if( it != points.begin() )
      --it;
    double        s0 = it->stretch;
    values::Vector f0 = it->front;

    // get coefficient for interpolation
    if( s0 == s1 )
      return vec_normalize(f0);

    double q = ( s - s0 ) / (s1 - s0);

    // return direction
    return vec_normalize((1-q) * f0 + q * f1);
  }

  // get up-vector at a certain stretch
  values::Vector Space_Curve::get_up     ( double s ){
    // get the point with stretch greater than s
    pointtype::iterator it;

    for( it = points.begin(); it != points.end(); ++it )
      {
	if( it->stretch > s )
	  break;
      }

    if( it == points.end() ) --it; // if no point found: use last point 

    double        s1 = it->stretch;
    values::Vector u1 = it->up;
    --it;
    double        s0 = it->stretch;
    values::Vector u0 = it->up;

    // get koefficient for interpolation
    double q = ( s - s0 ) / (s1 - s0);

    // return direction
    return vec_normalize((1-q) * u0 + q * u1);
  }

  // get first up-vector
  values::Vector Space_Curve::get_beg_up (){
    return vec_normalize(points.front().up);
  }

  // get last up-vector
  values::Vector Space_Curve::get_end_up (){
    //return points.last().up;   // last() isn't availible in my gcc version
    return vec_normalize(points.back().up);
  }

  Space_Curve::~Space_Curve()
  {
#ifdef __debug__
    print_points();
#endif
  } 

  /****************/
  /* Bezier_Curve */
  /****************/
  
  values::Vector Bezier_Curve::get_point_pos( double t )
  {
    double mt = 1 - t;

    return mt*mt*mt*p0 + 3*mt*mt*t*p1 + 3*mt*t*t*p2 + t*t*t*p3;
  }

}
