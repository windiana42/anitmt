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

#include <iostream>
#include <functional>
#include <algorithm>
#include <fstream>

#include "matrix.hpp"

#include "math.h"

namespace vect{
  matrix4::matrix4(){
    val[0][0] = 1;
    val[1][0] = 0;
    val[2][0] = 0;
    val[3][0] = 0;

    val[0][1] = 0;
    val[1][1] = 1;
    val[2][1] = 0;
    val[3][1] = 0;

    val[0][2] = 0;
    val[1][2] = 0;
    val[2][2] = 1;
    val[3][2] = 0;

    val[0][3] = 0;
    val[1][3] = 0;
    val[2][3] = 0;
    val[3][3] = 1;
  }

  matrix4::matrix4(double a1, double a2, double a3, double a4,
		   double b1, double b2, double b3, double b4,
		   double c1, double c2, double c3, double c4,
		   double d1, double d2, double d3, double d4){
    val[0][0] = a1;
    val[1][0] = a2;
    val[2][0] = a3;
    val[3][0] = a4;

    val[0][1] = b1;
    val[1][1] = b2;
    val[2][1] = b3;
    val[3][1] = b4;

    val[0][2] = c1;
    val[1][2] = c2;
    val[2][2] = c3;
    val[3][2] = c4;

    val[0][3] = d1;
    val[1][3] = d2;
    val[2][3] = d3;
    val[3][3] = d4;
  }

  matrix4& matrix4::operator= (const matrix4& m ){
    val[0][0] = m.val[0][0];
    val[1][0] = m.val[1][0];
    val[2][0] = m.val[2][0];
    val[3][0] = m.val[3][0];

    val[0][1] = m.val[0][1];
    val[1][1] = m.val[1][1];
    val[2][1] = m.val[2][1];
    val[3][1] = m.val[3][1];

    val[0][2] = m.val[0][2];
    val[1][2] = m.val[1][2];
    val[2][2] = m.val[2][2];
    val[3][2] = m.val[3][2];

    val[0][3] = m.val[0][3];
    val[1][3] = m.val[1][3];
    val[2][3] = m.val[2][3];
    val[3][3] = m.val[3][3];
  }
  
  matrix4& matrix4::operator*=(const matrix4& m ){
    matrix4 res;

    for( int i=0; i<4; i++ )
      for( int j=0; j<4; j++ )
	{
	  res.val[i][j] = 0;
	  for( int k=0; k<4; k++ )
	    {
	      res.val[i][j] += val[i][k] * m.val[k][j];
	    }
	}

    for( int l=0; l<4; l++ )
      for( int m=0; m<4; m++ )
	val[l][m] = res.val[l][m];

    return *this;
  }
  
  // this is a multiplication with a virtual 4D Vecter <x,y,z,1>
  // the forth coordinate of the result is cut afterwards
  vector3 operator*(const matrix4& m, const vector3& v){
    return vector3( v.coord[0]*m.val[0][0] + 
		    v.coord[1]*m.val[1][0] + 
		    v.coord[2]*m.val[2][0] +
		    m.val[3][0],

		    v.coord[0]*m.val[0][1] + 
		    v.coord[1]*m.val[1][1] + 
		    v.coord[2]*m.val[2][1] +
		    m.val[3][1],

		    v.coord[0]*m.val[0][2] + 
		    v.coord[1]*m.val[1][2] + 
		    v.coord[2]*m.val[2][2] +
		    m.val[3][2] );
  }

  matrix4 operator*(const matrix4& m1, const matrix4& m2){
    matrix4 ret;

    for( int i=0; i<4; i++ )
      for( int j=0; j<4; j++ )
	{
	  ret.val[i][j] = 0;
	  for( int k=0; k<4; k++ )
	    {
	      ret.val[i][j] += m1.val[k][j] * m2.val[i][k];
	    }
	}

    return ret;
  }

  
  matrix4 invert( const matrix4& m ){

    matrix4 res = matrix4( // first row 
			   +m.val[1][1]*m.val[2][2]*m.val[3][3]  
			   +m.val[2][1]*m.val[3][2]*m.val[1][3]  
			   +m.val[3][1]*m.val[1][2]*m.val[2][3]
			   -m.val[1][3]*m.val[2][2]*m.val[3][1]  
			   -m.val[2][3]*m.val[3][2]*m.val[1][1]  
			   -m.val[3][3]*m.val[1][2]*m.val[2][1],

			   +m.val[1][0]*m.val[2][2]*m.val[3][3]  
			   +m.val[2][0]*m.val[3][2]*m.val[1][3]  
			   +m.val[3][0]*m.val[1][2]*m.val[2][3]
			   -m.val[1][3]*m.val[2][2]*m.val[3][0]  
			   -m.val[2][3]*m.val[3][2]*m.val[1][0]  
			   -m.val[3][3]*m.val[1][2]*m.val[2][0],

			   +m.val[1][0]*m.val[2][1]*m.val[3][3]  
			   +m.val[2][0]*m.val[3][1]*m.val[1][3]  
			   +m.val[3][0]*m.val[1][1]*m.val[2][3]
			   -m.val[1][3]*m.val[2][1]*m.val[3][0]  
			   -m.val[2][3]*m.val[3][1]*m.val[1][0]  
			   -m.val[3][3]*m.val[1][1]*m.val[2][0],

			   +m.val[1][0]*m.val[2][1]*m.val[3][2]  
			   +m.val[2][0]*m.val[3][1]*m.val[1][2]  
			   +m.val[3][0]*m.val[1][1]*m.val[2][2]
			   -m.val[1][2]*m.val[2][1]*m.val[3][0]  
			   -m.val[2][2]*m.val[3][1]*m.val[1][0]  
			   -m.val[3][2]*m.val[1][1]*m.val[2][0],

			   // second row 
			   +m.val[0][1]*m.val[2][2]*m.val[3][3]  
			   +m.val[2][1]*m.val[3][2]*m.val[0][3]  
			   +m.val[3][1]*m.val[0][2]*m.val[2][3]
			   -m.val[0][3]*m.val[2][2]*m.val[3][1]  
			   -m.val[2][3]*m.val[3][2]*m.val[0][1]  
			   -m.val[3][3]*m.val[0][2]*m.val[2][1],

			   +m.val[0][0]*m.val[2][2]*m.val[3][3]  
			   +m.val[2][0]*m.val[3][2]*m.val[0][3]  
			   +m.val[3][0]*m.val[0][2]*m.val[2][3]
			   -m.val[0][3]*m.val[2][2]*m.val[3][0]  
			   -m.val[2][3]*m.val[3][2]*m.val[0][0]  
			   -m.val[3][3]*m.val[0][2]*m.val[2][0],

			   +m.val[0][0]*m.val[2][1]*m.val[3][3]  
			   +m.val[2][0]*m.val[3][1]*m.val[0][3]  
			   +m.val[3][0]*m.val[0][1]*m.val[2][3]
			   -m.val[0][3]*m.val[2][1]*m.val[3][0]  
			   -m.val[2][3]*m.val[3][1]*m.val[0][0]  
			   -m.val[3][3]*m.val[0][1]*m.val[2][0],

			   +m.val[0][0]*m.val[2][1]*m.val[3][2]  
			   +m.val[2][0]*m.val[3][1]*m.val[0][2]  
			   +m.val[3][0]*m.val[0][1]*m.val[2][2]
			   -m.val[0][2]*m.val[2][1]*m.val[3][0]  
			   -m.val[2][2]*m.val[3][1]*m.val[0][0]  
			   -m.val[3][2]*m.val[0][1]*m.val[2][0],

			   // third row 
			   +m.val[0][1]*m.val[1][2]*m.val[3][3]  
			   +m.val[1][1]*m.val[3][2]*m.val[0][3]  
			   +m.val[3][1]*m.val[0][2]*m.val[1][3]
			   -m.val[0][3]*m.val[1][2]*m.val[3][1]  
			   -m.val[1][3]*m.val[3][2]*m.val[0][1]  
			   -m.val[3][3]*m.val[0][2]*m.val[1][1],

			   +m.val[0][0]*m.val[1][2]*m.val[3][3]  
			   +m.val[1][0]*m.val[3][2]*m.val[0][3]  
			   +m.val[3][0]*m.val[0][2]*m.val[1][3]
			   -m.val[0][3]*m.val[1][2]*m.val[3][0]  
			   -m.val[1][3]*m.val[3][2]*m.val[0][0]  
			   -m.val[3][3]*m.val[0][2]*m.val[1][0],

			   +m.val[0][0]*m.val[1][1]*m.val[3][3]  
			   +m.val[1][0]*m.val[3][1]*m.val[0][3]  
			   +m.val[3][0]*m.val[0][1]*m.val[1][3]
			   -m.val[0][3]*m.val[1][1]*m.val[3][0]  
			   -m.val[1][3]*m.val[3][1]*m.val[0][0]  
			   -m.val[3][3]*m.val[0][1]*m.val[1][0],

			   +m.val[0][0]*m.val[1][1]*m.val[3][2]  
			   +m.val[1][0]*m.val[3][1]*m.val[0][2]  
			   +m.val[3][0]*m.val[0][1]*m.val[1][2]
			   -m.val[0][2]*m.val[1][1]*m.val[3][0]  
			   -m.val[1][2]*m.val[3][1]*m.val[0][0]  
			   -m.val[3][2]*m.val[0][1]*m.val[1][0],

			   // fourth row 
			   +m.val[0][1]*m.val[1][2]*m.val[2][3]  
			   +m.val[1][1]*m.val[2][2]*m.val[0][3]  
			   +m.val[2][1]*m.val[0][2]*m.val[1][3]
			   -m.val[0][3]*m.val[1][2]*m.val[2][1]  
			   -m.val[1][3]*m.val[2][2]*m.val[0][1]  
			   -m.val[2][3]*m.val[0][2]*m.val[1][1],

			   +m.val[0][0]*m.val[1][2]*m.val[2][3]  
			   +m.val[1][0]*m.val[2][2]*m.val[0][3]  
			   +m.val[2][0]*m.val[0][2]*m.val[1][3]
			   -m.val[0][3]*m.val[1][2]*m.val[2][0]  
			   -m.val[1][3]*m.val[2][2]*m.val[0][0]  
			   -m.val[2][3]*m.val[0][2]*m.val[1][0],

			   +m.val[0][0]*m.val[1][1]*m.val[2][3]  
			   +m.val[1][0]*m.val[2][1]*m.val[0][3]  
			   +m.val[2][0]*m.val[0][1]*m.val[1][3]
			   -m.val[0][3]*m.val[1][1]*m.val[2][0]  
			   -m.val[1][3]*m.val[2][1]*m.val[0][0]  
			   -m.val[2][3]*m.val[0][1]*m.val[1][0],

			   +m.val[0][0]*m.val[1][1]*m.val[2][2]  
			   +m.val[1][0]*m.val[2][1]*m.val[0][2]  
			   +m.val[2][0]*m.val[0][1]*m.val[1][2]
			   -m.val[0][2]*m.val[1][1]*m.val[2][0]  
			   -m.val[1][2]*m.val[2][1]*m.val[0][0]  
			   -m.val[2][2]*m.val[0][1]*m.val[1][0] );

    double det = +m.val[0][0]*m.val[1][1]*m.val[2][2]  
                 +m.val[1][0]*m.val[2][1]*m.val[0][2]  
		 +m.val[2][0]*m.val[0][1]*m.val[1][2]
		 -m.val[0][2]*m.val[1][1]*m.val[2][0]  
		 -m.val[1][2]*m.val[2][1]*m.val[0][0]  
		 -m.val[2][2]*m.val[0][1]*m.val[1][0];

    double mul = 1/det;

    for( int i=0; i<4; i++ )
      {
	for( int j=0; j<4; j++ )
	  {
	    res.val[i][j] *= mul;
	    mul *= -1;
	  }
	mul *= -1;
      }

    return res;
  }
  

  /**************************/
  /* create rotate matrices */
  /**************************/

  matrix4 rotateX( double angle ){
    return vect::matrix4( 1,         0,          0,0,
			  0,cos(angle),-sin(angle),0,
			  0,sin(angle), cos(angle),0,
			  0,         0,          0,1 );
  }

  matrix4 rotateY( double angle ){
    return vect::matrix4( cos(angle), 0, sin(angle),0,
			  0,          1,          0,0,
			  -sin(angle),0, cos(angle),0,
			  0,          0,          0,1 );
  }

  matrix4 rotateZ( double angle ){
    return vect::matrix4( cos(angle),-sin(angle),0,0,
			  sin(angle), cos(angle),0,0,
			  0,       0,            1,0,
			  0,       0,            0,1 );
  }

  // rotates around the x, y and z axes according to the coordinates of v
  matrix4 rotate( const vector3 v ){
    matrix4 res;
    // Modifications.push_back( new Mod_Rotate( v ) );
    double angle;

    // rotate around x-axis
    angle = v.coord[0];
    res *= rotateX(angle);

    // rotate around y-axis
    angle = v.coord[1];
    res *= rotateY(angle);

    // rotate around z-axis
    angle = v.coord[2];
    res *= rotateZ(angle);

    return res;
  }

  // rotates a specified angle around v 
  matrix4 rotate_around( const vector3 v, double angle ){
    matrix4 res;
    
    // rotate v to z
    res *= rotate_vect_vect( v, vector3(0,0,1) );

    // z-rotation (around v)
    res *= rotateZ(angle);

    // rotate z back to v
    res *= rotate_vect_vect( vector3(0,0,1), v );

    return res;
  }

  // rotates a vector to another
  matrix4 rotate_vect_vect(const vector3 vect1, const vector3 vect2 ){
    matrix4 res;
    
    vector3 v1 = vect1, v2 = vect2;
    double angle;

    //********
    // rotation from v1 to <1,0,0> (save to matrix)

    // z-rotation 
    angle = -atan2( v1.coord[1], v1.coord[0] );
    v1.rotateZ(angle);

    res *= rotateZ(angle);

    // y-rotation 
    angle = atan2( v1.coord[2], v1.coord[0] );
    //v1.rotateY(angle);

    res *= rotateY(angle);

    //********
    // rotation from v2 to <1,0,0>

    // z-rotation 
    double z_angle = -atan2( v2.coord[1], v2.coord[0] );
    v2.rotateZ(z_angle);

    // y-rotation 
    double y_angle = atan2( v2.coord[2], v2.coord[0] );
    //v2.rotateY(y_angle);

    //********
    // save rotation from <1,0,0> to v2

    // y-rotation 
    res *= rotateY(-y_angle);

    // z-rotation 
    res *= rotateZ(-z_angle);

    return res;
  }

  // rotates a vector to another by using a sperical rotation with the
  // horizontal plane defined by the normal vector "up"
  matrix4 rotate_vect_vect_up(const vector3 vect1, const vector3 vect2,
			      const vector3 up){
    matrix4 res;

    //********
    // rotation from v1 to <1,0,0> 

    // rotate up to z and vect1 to the x-z plain
    matrix4 up_v1_2_z_x = rotate_pair_pair( up,            vect1,
					    vector3(0,0,1),vector3(1,0,0) );
    

    // rotate to map the up-rotation to a z-rotation
    res *= up_v1_2_z_x;
    
    vector3 v1 = up_v1_2_z_x * vect1;
    // y-rotation 
    double angle = atan2( v1.coord[2], v1.coord[0] );
    // rotation is made by other y-rotation
    
    vector3 v2 = up_v1_2_z_x * vect2;
    //********
    // rotation from v2 to <1,0,0> by a rotation around z (up) and y

    // z-rotation (up is z now)
    double z_angle = -atan2( v2.coord[1], v2.coord[0] );
    v2.rotateZ(z_angle);

    // y-rotation 
    double y_angle = atan2( v2.coord[2], v2.coord[0] );
    v2.rotateY(y_angle);

    // additional rotation around y-axis
    angle -= y_angle;
    res *= rotateY(angle);
     
    // rotate around z-axis
    angle = -z_angle;
    res *= rotateZ(angle);

    // rotate back
    res *= rotate_pair_pair( vector3(0,0,1),vector3(1,0,0),up,vect1 );

    return res;
  }

  // rotates a vector pair to another
  // the first vectors of each pair will mach exactly afterwards but the second
  // may differ in the angle to the first one. They will be in the same plane
  // then
  matrix4 rotate_pair_pair(const vector3 vect1f, const vector3 vect1u,
			   const vector3 vect2f, const vector3 vect2u){
    matrix4 res;
    vect::vector3 x,y;
    double angle;

    //******
    // get rotation from <1,0,0> and <0,1,0> to vector pair 1

    // get the vectors
    x = vect1f; 
    y = vect1u;
    // rotate x to <1,0,0> and y to <0,1,0> now
    
    // get z-rotation
    double z_angle1 = -atan2( x.coord[1], x.coord[0] );
    x.rotateZ(z_angle1);
    y.rotateZ(z_angle1);

    // get y-rotation
    double y_angle1 = atan2( x.coord[2], x.coord[0] );
    //x.rotateY(y_angle1);
    y.rotateY(y_angle1);

    // get x-rotation
    double x_angle1 = -atan2( y.coord[2], y.coord[1] );
    //x.rotateX(x_angle1);
    //y.rotateX(x_angle1);

    //******
    // get rotation from <1,0,0> and <0,1,0> to vector pair 2

    // get the vectors
    x = vect2f; 
    y = vect2u;
    // rotate x to <1,0,0> and y to <0,1,0> now
    
    // get z-rotation
    double z_angle2 = -atan2( x.coord[1], x.coord[0] );
    x.rotateZ(z_angle2);
    y.rotateZ(z_angle2);

    // get y-rotation
    double y_angle2 = atan2( x.coord[2], x.coord[0] );
    //x.rotateY(y_angle2);
    y.rotateY(y_angle2);

    // get x-rotation
    double x_angle2 = -atan2( y.coord[2], y.coord[1] );
    //x.rotateX(x_angle2);
    //y.rotateX(x_angle2);

    //******
    // save the rotation from pair 1 to origin 

    // rotate around z-axis
    angle = z_angle1;
    res *= rotateZ(angle);

    // rotate around y-axis
    angle = y_angle1;
    res *= rotateY(angle);

    // rotate around x-axis
    angle = x_angle1;
    res *= rotateX(angle);

    //******
    // save the rotation from origin to pair2 

    // rotate around x-axis
    angle = -x_angle2;
    res *= rotateX(angle);

    // rotate around y-axis
    angle = -y_angle2;
    res *= rotateY(angle);

    // rotate around z-axis
    angle = -z_angle2;
    res *= rotateZ(angle);

    return res;
  }

  // spherical rotation with the horizontal plane defined through
  // the normal vector up and the front vector 
  // the x-coordinate of angles is used for the rotation around front
  // the y-coordinate of angles is used for the rotation around up
  // and the z-coordiante specifies the angle to go up from the plane
  matrix4 rotate_spherical_pair( const vector3 front, const vector3 up, 
				 const vector3 angles ){
    // first rotate around front 
    matrix4 res = rotate_around( front, angles.coord[0] );

    vector3 u = res * up;	// get the new up vector

    // then around up
    matrix4 res2 = rotate_around( up, angles.coord[1] );
    res *= res2;

    vector3 f = res2 * front;	// get the new front vector
    
    // and last around u x f
    res *= rotate_around( f.cross(u), angles.coord[2] );

    return res;
  }

  /*****************/
  /* get rotations */
  /*****************/

  double get_angle( const vector3 v1, const vector3 v2 ){
    double len1 = v1.length();
    double len2 = v2.length();
    
    return acos( dot(v1,v2) / (len1*len2) );
  }

  // get the rotation from v1 to v2 around axis
  double get_rotation_around( const vector3 v1,const vector3 v2,
			      const vector3 axis ){
    // rotate both vectors so that axis maches z and v1 is in the x-z-plain
    matrix4 rot_easy = rotate_pair_pair( axis,           v1, 
					 vector3(0,0,1), vector3(1,0,0) );
    vector3 easy_v2 = rot_easy * v2;
    
    // get rotation from easy_v2 to x-z-plain around z
    double z_angle = atan2( easy_v2.coord[1], easy_v2.coord[0] );

    return z_angle;
  }

}
