/*****************************************************************************/
/** AniTMT -- A program for creating foto-realistic animations		    **/
/**   This file belongs to the component tga2avi    			    **/
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

#ifndef __myvektor__
#define __myvektor__

class vektor;

#include <math.h>

#define maxvektlen 100       /* Maximale Vektorlänge als Strings */

class vektor{
 public:
  double x,y,z;
  
  virtual char *getAsString();

  vektor *cpy();

  vektor *smul( double factor );
  vektor *sdiv( double factor );
  vektor *add( vektor *op );
  vektor *sub( vektor *op );
  double abs();
  vektor *norm();
  double sprod( vektor *op ); // scalar product

  vektor(double sx,double sy,double sz){
    x=sx;y=sy;z=sz;
  }
  vektor(){}
  virtual ~vektor(){}
};

char *vektor::getAsString(){
  char *str=(char *)malloc(maxvektlen);
  sprintf(str,"<%g,%g,%g>",x,y,z);
  return str;
}

vektor *vektor::cpy()
{
  return new vektor( x, y, z );
}

vektor *vektor::smul( double factor )
{
  return new vektor( x * factor, y * factor, z * factor );
}

vektor *vektor::sdiv( double factor )
{
  return new vektor( x / factor, y / factor, z / factor );
}

vektor *vektor::add( vektor *op )
{
  return new vektor( x + op->x, y + op->y, z + op->z );
}

vektor *vektor::sub( vektor *op )
{
  return new vektor( x - op->x, y - op->y, z - op->z );
}

double vektor::abs()
{
  return sqrt( x*x + y*y + z*z ); // Pytagoras
}

vektor *vektor::norm() // normalize vector
{
  return sdiv( abs() );
}

double vektor::sprod( vektor *op )
{
  double ret = x*op->x + y*op->y + z*op->z;
  return ret;
}

#endif









