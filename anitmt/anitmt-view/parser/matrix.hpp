/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#ifndef __vect_matrix__
#define __vect_matrix__

#include "vector.hpp"

namespace vect{
  class matrix4{
  public:
    // a 4x4 matrix is used to manipulate 3D vectors easily
    // the last row has to be always 0,0,0,1
    double val[4][4];
    matrix4();
    matrix4(double,double,double,double,
	    double,double,double,double,
	    double,double,double,double,
	    double,double,double,double);

    matrix4& operator= (const matrix4&);
    matrix4& operator*=(const matrix4&);
  };

  // this is a multiplication with a virtual 4D Vecter <x,y,z,1>
  // the forth coordinate of the result is cut afterwards
  vector3 operator*(const matrix4&, const vector3&);
  matrix4 operator*(const matrix4&, const matrix4&);

  // get the inverse matrix M^-1 ( M * M^-1 = E )
  matrix4 invert( const matrix4& m );

  /**************************/
  /* create rotate matrices */
  /**************************/

  // rotates around the x, y and z axes according to the coordinates of v
  matrix4 rotate( const vector3 v );

  // rotates a specified angle around v 
  matrix4 rotate_around( const vector3 v, double angle );

  // rotates a vector to another
  matrix4 rotate_vect_vect(const vector3 vect1, const vector3 vect2 );

  // rotates a vector to another by using a sperical rotation with the
  // horizontal plane defined by the normal vector "up"
  matrix4 rotate_vect_vect_up(const vector3 vect1, const vector3 vect2,
			      const vector3 up);

  // rotates a vector pair to another
  // the first vectors of each pair will mach exactly afterwards but the second
  // may differ in the angle to the first one. They will be in the same plane
  // then
  matrix4 rotate_pair_pair(const vector3 vect1f, const vector3 vect1u,
			   const vector3 vect2f, const vector3 vect2u);

  // spherical rotation with the horizontal plane defined through
  // the normal vector up and the front vector 
  // the x-coordinate of angles is used for the rotation around front
  // the y-coordinate of angles is used for the rotation around up
  // and the z-coordiante specifies the angle to go up from the plane
  matrix4 rotate_spherical_pair( const vector3 front, const vector3 up, 
				 const vector3 angles );

  /*****************/
  /* get rotations */
  /*****************/

  double get_angle( const vector3 v1, const vector3 v2 );

  // get the rotation from v1 to v2 around axis
  double get_rotation_around( const vector3 v1,const vector3 v2,
			      const vector3 axis );
}

#endif
