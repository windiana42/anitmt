/*  
Copyright (c) 1999  Roberto Javier Peon


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU  General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Maintainer: Roberto Javier Peon
Contact: fenix@acm.org
*/
   
#include "vector3.hh"
#include <math.h>
//#define DIVZEROSAFE  // tell the geomprm lib to watch for divZero.

#ifndef DBL_MAX
#define DBL_MAX 100000000000000000000000000000000000000000000000000000000f
#endif

  //////////////////////////////////////////////////
  //////////////////////////////////////////////////

vector3::vector3(){}

  //////////////////////////////////////////////////

vector3::vector3(double x,double y, double z){
  double *t=coord;
  *t=x;
  t++;
  *t=y;
  t++;
  *t=z;
}
  //////////////////////////////////////////////////
vector3::vector3(double *arr){
  double *t=coord;
  *t=(*arr);
  t++;
  arr++;
  *t=(*arr);
  t++;
  arr++;
  *t=(*arr);
}
  //////////////////////////////////////////////////
 vector3::~vector3(){}

  //////////////////////////////////////////////////

ostream& operator<<(ostream& s, const vector3& v) {
  double * t= (double*)v.coord;
  s<< '[' << *t << ',';
  t++;
  s<< *t  << ',';
  t++;
  s<< *t  << ']';
  return s;
}
  //////////////////////////////////////////////////
vector3& vector3::operator=(const vector3& v){
  double * t2= (double*)v.coord,* t=coord;
  *t=(*t2);
  t++;
  t2++;
  *t=*t2;
  t++;
  t2++;
  *t=*t2;
  return *this;
}
  //////////////////////////////////////////////////

istream& operator>>(istream& s, vector3& v){
  double *t= v.coord;
  cin >> *t;
  t++;
  cin >> *t;
  t++;
  cin >> *t;
  return s;
}

  //////////////////////////////////////////////////
vector3& vector3::operator+=(const double c){
  double *t= coord;
  *t+=c;
  t++;
  *t+=c;
  t++;
  *t+=c;
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::operator-=(const double c){
  double *t= coord;
  *t-=c;
  t++;
  *t-=c;
  t++;
  *t-=c;
  return *this;
}

  //////////////////////////////////////////////////
vector3& vector3::operator*=(const double c){
  double *t= coord;
  *t*=c;
  t++;
  *t*=c;
  t++;
  *t*=c;
  return *this;
}

  //////////////////////////////////////////////////
vector3& vector3::operator/=(const double c){
  double *t= coord;
#ifdef DIVZEROSAFE
  if(c==0){
    *t=DBL_MAX;
    t++;
    *t=DBL_MAX;
    t++;
    *t=DBL_MAX;
  }else {
#else
  *t/=c;
  t++;
  *t/=c;
  t++;
  *t/=c;
#endif
#ifdef DIVZEROSAFE
  }
#endif
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::operator+=(const vector3& c){
  double *t1=coord,*t2=(double*)c.coord;
  *t1+=*t2;
  t1++;
  t2++;
  *t1+=*t2;
  t1++;
  t2++;
  *t1+=*t2;
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::operator-=(const vector3& c){
  double *t1=coord,*t2=(double*)c.coord;
  *t1-=*t2;
  t1++;
  t2++;
  *t1-=*t2;
  t1++;
  t2++;
  *t1-=*t2;
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::operator*=(const vector3& c){
  double *t1=coord,*t2=(double*)c.coord;
  *t1*=*t2;
  t1++;
  t2++;
  *t1*=*t2;
  t1++;
  t2++;
  *t1*=*t2;
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::operator/=(const vector3& c){
  double *t1=coord,*t2=(double*)c.coord;
#ifdef DIVZEROSAFE
  if(*t2==0) *t1=DBL_MAX;
  else *t1/=*t2;
  t1++;
  t2++;
  if(*t2==0) *t1=DBL_MAX;
  else *t1/=*t2;
  t1++;
  t2++;
  if(*t2==0) *t1=DBL_MAX;
  else *t1/=*t2;
#else  
  *t1/=*t2;
  t1++;
  t2++;
  *t1/=*t2;
  t1++;
  t2++;
  *t1/=*t2;
#endif
  return *this;
}
  //////////////////////////////////////////////////
double vector3::dot(const vector3& c) const {
  double *t1=(double*)coord,*t2=(double*)c.coord,result;
  result= *t1**t2;
  t1++;
  t2++;
  result+= *t1**t2;
  t1++;
  t2++;
  result+= *t1**t2;
  return result;
}
  //////////////////////////////////////////////////
vector3& vector3::cross(const vector3& c){
  double *a=coord+1,*b=(double*)c.coord+2,x,y,z;
  x= *a * *b; // x = a[1]*b[2]
  a++;        // a is a[2]
  b--;        // b is b[1]
  x-=*a * *b; // x = x - a[2]*b[1]
              // x = a[1]*b[2] - a[2]*b[1]
  b--;        // b is b[0]
  y= *a * *b; // y = a[2]*b[0]
  a-=2;       // a is a[0]
  b+=2;       // b is b[2]
  y-= *a * *b;// y = y - a[0]*b[2]
              // y = a[2]*b[0] - a[0]*b[2]
  b--;        // b is b[1]
  z=*a * *b;  // z = a[0]*b[1]
  a++;        // a is a[1]
  b--;        // b is b[0]
  z-= *a * *b;// z = z - a[1]*b[0]
              // z = a[0]*b[1] - a[1]*b[0]
  *a=y;     // assign temps to perms.
  a--;
  *a=x;
  a+=2;
  *a=z;
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::normalize(){
  double length,*t=coord;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  length = sqrt(length);
#ifdef DIVZEROSAFE
  if(length==0) {
    *t=DBL_MAX;
    *(--t)=DBL_MAX;
    *(--t)=DBL_MAX;
    return *this;
  }
#endif
  *t /= length;
  t--;
  *t /= length;
  t--;
  *t /= length;
  return *this;
}
  //////////////////////////////////////////////////
double vector3::length() const {
  double length,*t=(double*)coord;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  return sqrt(length);
}

  //////////////////////////////////////////////////

double vector3::length2() const {
  double length,*t=(double*)coord;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  return length;
}

  //////////////////////////////////////////////////
vector3 vector3::operator+(const double c) const {
  double* t=(double*)coord,x,y,z;
  x=*t+c;
  t++;
  y=*t+c;
  t++;
  z=*t+c;
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3 vector3::operator-(const double c) const {
  double* t=(double*)coord,x,y,z;
  x=*t-c;
  t++;
  y=*t-c;
  t++;
  z=*t-c;
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3 vector3::operator*(const double c) const {
  double* t=(double*)coord,x,y,z;
  x=*t*c;
  t++;
  y=*t*c;
  t++;
  z=*t*c;
  return vector3(x,y,z);
}
//////////////////////////////////////////////////
vector3 vector3::operator/(const double c) const {
  double* t=(double*)coord;
#ifdef DIVZEROSAFE
  if(c==0) return vector3(DBL_MAX,DBL_MAX,DBL_MAX);
#endif
  double x,y,z;
  x=*t/c;
  t++;
  y=*t/c;
  t++;
  z=*t/c;
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3  vector3::operator+(const vector3& rhs) const {
  double* t=(double*)coord,*t2=(double*)rhs.coord,x,y,z;
  x=*t + *t2;
  t++;
  t2++;
  y=*t + *t2;
  t++;
  t2++;
  z=*t + *t2;
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3  vector3::operator-(const vector3& rhs) const {
  double* t=(double*)coord,*t2=(double*)rhs.coord,x,y,z;
  x=*t - *t2;
  t++;
  t2++;
  y=*t - *t2;
  t++;
  t2++;
  z=*t - *t2;
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3  vector3::operator*(const vector3& rhs) const {
  double* t=(double*)coord,*t2=(double*)rhs.coord,x,y,z;
  x=*t * *t2;
  t++;
  t2++;
  y=*t * *t2;
  t++;
  t2++;
  z=*t * *t2;
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3  vector3::operator/(const vector3& rhs) const {
  double* t=(double*)coord,*t2=(double*)rhs.coord,x,y,z;
#ifdef DIVZEROSAFE
  if(*t2==0) *t=DBL_MAX; else x=*t/ *t2;
  t++;
  t2++;
  if(*t2==0) *t=DBL_MAX; else y=*t/ *t2;
  t++;
  t2++;
  if(*t2==0) *t=DBL_MAX; else z=*t/ *t2;
  return vector3(x,y,z);
#else
  x=*t / *t2;
  t++;
  t2++;
  y=*t / *t2;
  t++;
  t2++;
  z=*t / *t2;
  return vector3(x,y,z);
#endif
}
  //////////////////////////////////////////////////
double dot(const vector3& lhs,const vector3& rhs)  {
  double *t1=(double*)lhs.coord,*t2=(double*)rhs.coord,result;
  result= (*t1 * *t2);
  t1++;
  t2++;
  result+= (*t1 * *t2);
  t1++;
  t2++;
  result+= (*t1 * *t2);
  return result;
}
  //////////////////////////////////////////////////
vector3 cross(const vector3& lhs,const  vector3& rhs)  {
  double *a=(double*)lhs.coord+1,*b=(double*)rhs.coord+2,x,y,z;
  x= *a * *b; // x = a[1]*b[2]
  a++;        // a is a[2]
  b--;        // b is b[1]
  x-=*a * *b; // x = x - a[2]*b[1]
              // x = a[1]*b[2] - a[2]*b[1]
  b--;        // b is b[0]
  y= *a * *b; // y = a[2]*b[0]
  a-=2;       // a is a[0]
  b+=2;       // b is b[2]
  y-= *a * *b;// y = y - a[0]*b[2]
              // y = a[2]*b[0] - a[0]*b[2]
  b--;        // b is b[1]
  z=*a * *b;  // z = a[0]*b[1]
  a++;        // a is a[1]
  b--;        // b is b[0]
  z-= *a * *b;// z = z - a[1]*b[0]
              // z = a[0]*b[1] - a[1]*b[0]
  return vector3(x,y,z);
}
//////////////////////////////////////////////////
vector3 normalize(const vector3& rhs)  {
  double length,*t=(double*)rhs.coord;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  length = sqrt(length);
#ifdef DIVZEROSAFE
  if(length==0)  return vector3(DBL_MAX,DBL_MAX,DBL_MAX);
#endif
  double x,y,z;
  z=*t/length;
  t--;
  y=*t/length;
  t--;
  x=*t/length;
  return vector3(x,y,z);
}
//////////////////////////////////////////////////
double length(const vector3& rhs)  {
  double length,*t=(double*)rhs.coord;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  return sqrt(length);
}
//////////////////////////////////////////////////
double length2(const vector3& rhs)  {
  double length,*t=(double*)rhs.coord;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  return length;
}
//////////////////////////////////////////////////
int vector3::operator&&(const vector3& rhs) const{
  double *t=(double*)coord,*t2=(double*)rhs.coord;
  int r,r2;
  r=  (*t!=0);
  r2= (*t2!=0);
  t++;
  t2++;
  r&=  (*t!=0);
  r2&= (*t2!=0);
  t++;
  t2++;
  r&=  (*t!=0);
  r2&= (*t2!=0);
  return ((r!=0)&&(r2!=0));
}
//////////////////////////////////////////////////
int vector3::operator||(const vector3& rhs) const{
  double *t=(double*)coord,*t2=(double*)rhs.coord;
  int r,r2;
  r=  (*t!=0);
  r2= (*t2!=0);
  t++;
  t2++;
  r|=  (*t!=0);
  r2|= (*t2!=0);
  t++;
  t2++;
  r|=  (*t!=0);
  r2|= (*t2!=0);
  return ((r>0)||(r2>0));
}//////////////////////////////////////////////////
int vector3::operator==(const vector3& rhs) const{
  double *t=(double*)coord,*t2=(double*)rhs.coord;
  if(*t!=*t2) return 0;
  t++;
  t2++;
  if(*t!=*t2)return 0;
  t++;
  t2++;
  if(*t!=*t2) return 0;
  return 1;
}
//////////////////////////////////////////////////
int vector3::operator!=(const vector3& rhs) const{
  double *t=(double*)coord,*t2=(double*)rhs.coord;
  if(*t!=*t2) return 1;
  t++;
  t2++;
  if(*t!=*t2)return 1;
  t++;
  t2++;
  if(*t!=*t2) return 1;
  return 0;
}
  //////////////////////////////////////////////////
vector3& vector3::rotateX(const double a){
  double y,*t=coord+1,z;
  double sina=sin(a),cosa=cos(a);
  y=*t*cosa;   // y1 = y0*cos(a)
  z=*t*sina;   // z1 = y0*sin(a)
  t++;         // t is z0
  y-= *t*sina; // y1 = y1-z0*sin(a)
  z+= *t*cosa; // z1 = z1+z0*cos(a)
  *t=z;        // z0=z1
  t--;         // t is y0
  *t=y;        // y0 = y1

 return *this;
}
  //////////////////////////////////////////////////

// inverted by Martin Trautmann
vector3& vector3::rotateY(const double a){
  double x,*t=coord,z;
  double sina=sin(a),cosa=cos(a);
  x=*t*cosa;   // x1 = x0*cos(a)
  z=-(*t*sina);// z1 = -x0*sin(a)
  t+=2;        // t is z0
  x+= *t*sina; // x1 = x1+z0*sin(a)
  z+= *t*cosa; // z1 = z1+z0*cos(a)
  *t=z;        // z0=z1
  t-=2;        // t is x0
  *t=x;        // x0 = x1
  return *this;
}
  //////////////////////////////////////////////////

vector3& vector3:: rotateZ(const double a){
  double x,*t=coord,y;
  double sina=sin(a),cosa=cos(a);
  x=*t*cosa;   // x1 = x0*cos(a)
  y=*t*sina;   // y1 = x0*sin(a)
  t++;         // t is y0
  x-= *t*sina; // x1 = x1-y0*sin(a)
  y+= *t*cosa; // y1 = y1+y0*cos(a)
  *t=y;        // y0=y1
  t--;         // t is x0
  *t=x;        // x0 = x1
  return *this;
}
  //////////////////////////////////////////////////



vector3& vector3::rotateXYZ(const vector3& angle){
  double tx,ty,tz,*t=coord+1,*t2=(double*)angle.coord;
  double sina,cosa;
  sina=sin(*t2);             //sina = sin(angle.coord[0])
  cosa=cos(*t2);             //cosa = cos(angle.coord[0])
  ty = cosa**t ;             // ty  = cosa * y
  tz = sina**t ;             // tz  = sina * y
  t++;                       // t is z
  ty-=sina**t;               // ty = ty - sina * z
  tz+=cosa**t;               // tz = tz + cosa * z
  t2++;                      // t2 is angle.coord[1]
  sina = sin(*t2);           // sina = sin(angle.coord[1])
  cosa = cos(*t2);           // cosa = cos(angle.coord[1])
  t-=2;                      // t is x;
  tx = cosa**t  - sina*tz;   // tx = cosa * x - sina * tz
  tz = sina**t  + cosa*tz;   // tz = sina * x - cosa * tz
  t2++;                      // t2 is angle.coord[2]
  sina = sin(*t2);           // sina = sin(angle.coord[2])
  cosa = cos(*t2);           // cosa = cos(angle.coord[2])
  *t  = cosa*tx - sina*ty;   // x = cosa * tx - sina * ty
  t++;                       // t is y
  *t  = sina*tx + cosa*ty;   // y = sina * tx + cosa * ty
  t++;                       // t is z
  *t=tz;                     // z = tz
  return *this;
}
  //////////////////////////////////////////////////

/* Will be removed:
   rotate a vector about an arbitrary center of rotation.
   It should be rotation about an axis
*/
vector3& vector3::rotateXYZ(const vector3& angle,const vector3& about){
  double *t=coord,*t2=(double*)about.coord;
  double tx,ty,tz;
  double sina,cosa;
  *t-=*t2;                   // x -= about.coord[0]
  t++;                       // t is y
  t2++;                      // t2 is about.coord[1]
  *t-=*t2;                   // y -= about.coord[1]
  t++;                       // t is z
  t2++;                      // t2 is about.coord[2]
  *t-=*t2;                   // z -= about.coord[2]
  t--;                       // t is y
  t2=(double*)angle.coord;   // t2  = angle.coord[0]
  sina=sin(*t2);             //sina = sin(angle.coord[0])
  cosa=cos(*t2);             //cosa = cos(angle.coord[0])
  ty = cosa**t ;             // ty  = cosa * y
  tz = sina**t ;             // tz  = sina * y
  t++;                       // t is z
  ty-=sina**t;               // ty = ty - sina * z
  tz+=cosa**t;               // tz = tz + cosa * z
  t2++;                      // t2 is angle.coord[1]
  sina = sin(*t2);           // sina = sin(angle.coord[1])
  cosa = cos(*t2);           // cosa = cos(angle.coord[1])
  t-=2;                      // t is x;
  tx = cosa**t  - sina*tz;   // tx = cosa * x - sina * tz
  tz = sina**t  + cosa*tz;   // tz = sina * x - cosa * tz
  t2++;                      // t2 is angle.coord[2]
  sina = sin(*t2);           // sina = sin(angle.coord[2])
  cosa = cos(*t2);           // cosa = cos(angle.coord[2])
  t2 =(double*)about.coord;  // t2 is about.coord[0]
  *t = cosa*tx - sina*ty+*t2;// x = cosa * tx - sina * ty +about.coord[0]
  t++;                       // t is y
  t2++;                      // t2 is about.coord[1]
  *t = sina*tx + cosa*ty+*t2;// y = sina * tx + cosa * ty +about.coord[1]
  t++;                       // t is z
  t2++;                      // t2 is about.coord[2]
  *t=tz+*t2;                 // z = tz + about.coord[2]
  return *this;
}

  //////////////////////////////////////////////////
vector3 rotateX(const vector3& point,const double a){
  double y,*t=((double*)point.coord)+1,z;
  double sina=sin(a),cosa=cos(a);
  y=*t*cosa;   // y1 = y0*cos(a)
  z=*t*sina;   // z1 = y0*sin(a)
  t++;         // t is z0
  y-= *t*sina; // y1 = y1-z0*sin(a)
  z+= *t*cosa; // z1 = z1+z0*cos(a)
  t-=2;       // t is x0
 return vector3(*t,y,z);
}
  //////////////////////////////////////////////////

vector3 rotateY(const vector3& point,const double a){
  double x,*t=((double*)point.coord),z;
  double sina=sin(a),cosa=cos(a);
  x=*t*cosa;   // x1 = x0*cos(a)
  z=-(*t*sina);// z1 = -x0*sin(a)
  t+=2;        // t is z0
  x+= *t*sina; // x1 = x1+z0*sin(a)
  z+= *t*cosa; // z1 = z1+z0*cos(a)
  t--;         // t is y0
  return vector3(x,*t,z);
}
  //////////////////////////////////////////////////

vector3 rotateZ(const vector3& point,const double a){
  double x,*t=((double*)point.coord),y;
  double sina=sin(a),cosa=cos(a);
  x=*t*cosa;   // x1 = x0*cos(a)
  y=*t*sina;   // y1 = x0*sin(a)
  t++;         // t is y0
  x-= *t*sina; // x1 = x1-y0*sin(a)
  y+= *t*cosa; // y1 = y1+y0*cos(a)
  t++;         // t is z0
  return vector3(x,y,*t);
}
  //////////////////////////////////////////////////

// !!! might have wrong y-rotation
vector3 rotateXYZ(const vector3& point,const vector3& angle){
  double tx,ty,tz;
  double *t=((double*)point.coord)+1;
  double *t2=(double*)angle.coord;
  double sina,cosa;
  sina=sin(*t2);             //sina = sin(angle.coord[0])
  cosa=cos(*t2);             //cosa = cos(angle.coord[0])
  ty = cosa**t ;             // ty  = cosa * y
  tz = sina**t ;             // tz  = sina * y
  t++;                       // t is z
  ty-=sina**t;               // ty = ty - sina * z
  tz+=cosa**t;               // tz = tz + cosa * z
  t2++;                      // t2 is angle.coord[1]
  sina = sin(*t2);           // sina = sin(angle.coord[1])
  cosa = cos(*t2);           // cosa = cos(angle.coord[1])
  t-=2;                      // t is x;
  tx = cosa**t  - sina*tz;   // tx = cosa * x - sina * tz
  tz = sina**t  + cosa*tz;   // tz = sina * x - cosa * tz
  t2++;                      // t2 is angle.coord[2]
  sina = sin(*t2);           // sina = sin(angle.coord[2])
  cosa = cos(*t2);           // cosa = cos(angle.coord[2])
  return vector3(cosa*tx - sina*ty ,sina*tx + cosa*ty ,tz);
}
  //////////////////////////////////////////////////

// !!! might have wrong y-rotation
vector3 rotateXYZ(const vector3& point,
		  const vector3& angle,
		  const vector3& about){
  double *t=((double*)point.coord),*t2=(double*)about.coord;
  double tx,ty,tz;
  double sina,cosa;
  *t-=*t2;                   // x -= about.coord[0]
  t++;                       // t is y
  t2++;                      // t2 is about.coord[1]
  *t-=*t2;                   // y -= about.coord[1]
  t++;                       // t is z
  t2++;                      // t2 is about.coord[2]
  *t-=*t2;                   // z -= about.coord[2]
  t--;                       // t is y
  t2=(double*)angle.coord;   // t2  = angle.coord[0]
  sina=sin(*t2);             //sina = sin(angle.coord[0])
  cosa=cos(*t2);             //cosa = cos(angle.coord[0])
  ty = cosa**t ;             // ty  = cosa * y
  tz = sina**t ;             // tz  = sina * y
  t++;                       // t is z
  ty-=sina**t;               // ty = ty - sina * z
  tz+=cosa**t;               // tz = tz + cosa * z
  t2++;                      // t2 is angle.coord[1]
  sina = sin(*t2);           // sina = sin(angle.coord[1])
  cosa = cos(*t2);           // cosa = cos(angle.coord[1])
  t-=2;                      // t is x;
  tx = cosa**t  - sina*tz;   // tx = cosa * x - sina * tz
  tz = sina**t  + cosa*tz;   // tz = sina * x - cosa * tz
  t2++;                      // t2 is angle.coord[2]
  sina = sin(*t2);           // sina = sin(angle.coord[2])
  cosa = cos(*t2);           // cosa = cos(angle.coord[2])
  t2 = (double*)about.coord; // t2 is about.coord[0]
  return vector3(cosa*tx - sina*ty+*t2,sina*tx + cosa*ty+*(t2+1),tz+*(t2+2));
}
  //////////////////////////////////////////////////

vector3  operator+(const double c, const vector3& v){
  return vector3(c + *v.coord,c + *(v.coord+1),c + *(v.coord+2));
}
  //////////////////////////////////////////////////
vector3  operator-(const double c, const vector3& v){
  return vector3(c - (*v.coord),c - *(v.coord+1),c - *(v.coord+2));
}

  //////////////////////////////////////////////////
vector3  operator*(const double c, const vector3& v){
  return vector3(c * *v.coord,c * *(v.coord+1),c * *(v.coord+2));
}
  //////////////////////////////////////////////////
vector3  operator/(const double c, const vector3& v){
#ifdef DIVZEROSAFE
  double* t=(double*)v.coord;
  double x= *t;
  t++;
  double y=*t;
  t++;
  double z=*t;
  if(x==0) x=DBL_MAX;
  else x=c/x;
  if(y==0) y=DBL_MAX;
  else y=c/y;
  if(z==0) z=DBL_MAX;
  else z=c/z;
  return vector3(x,y,z);
#else
  return vector3(c / *v.coord,c / *(v.coord+1),c / *(v.coord+2));
#endif
}
  //////////////////////////////////////////////////
// spherical  (ro, theta)
vector3& vector3::toSpherical(){
  double length,*t=(double*)coord;
#ifdef DIVZEROSAFE
  double q;
#endif
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
  length= sqrt(length);
#ifdef DIVZEROSAFE
  if(length==0) *t=M_PI_2;  // *t=acos(0);
#endif
  *t=acos(*t/length);       // coord[2]=acos(coord[2]/length)
  t--;                      // t is coord[1]
  *t=*coord;                // coord[1] = coord[0]
#ifdef DIVZEROSAFE
  if(length==0) *t=DBL_MAX;
  else if((q=sin(*coord))==0) *t=DBL_MAX;
  else *t/=(length*q);
#else
  *t/=(length*sin(*coord)); // coord[1] = coord[1]/(length*sin(coord[0]))
#endif
  t--;                      // t is coord[0]
  *t=length;                // coord[0] = length
  return *this;
}
  //////////////////////////////////////////////////
//rectangular (x y z)
vector3& vector3::toRectangular(){
  double *t=coord;
  double x=*t,y=*t,z=*t,q;
  t+=2;        // t is coord[2]
  q=sin(*t);   // q=sin(coord[2])
  x*= q;       // x=x*sin(coord[2])
  y*= q;       // y=y*sin(coord[2])
  z*= cos(*t); // z=z*cos(coord[2])
  t--;         // t is coord[1]
  x*=cos(*t);  // x=x*cos(coord[1])
  *t*=sin(*t); // coord[1]=sin(coord[1])  
  t--;         // t is coord[0]
  *t=x;        // coord[0]=x
  t+=2;        // t is coord[2]
  *t=z;        // coord[2]=z
  return *this;
}
  //////////////////////////////////////////////////
vector3& vector3::proj(const vector3& v){
  double *t=(double*)coord,*t2=(double*)v.coord,q,r;
  q= *t * *t;
  r= *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
  t-=2;
#ifdef DIVZEROSAFE
  if(q==0) r=DBL_MAX;
#endif
  r/=q;
  *t*=r;
  t++;
  *t*=r;
  t++;
  *t*=r;
  return *this;
}
  //////////////////////////////////////////////////
double  vector3::component(const vector3& v) const {
  double *t=(double*)coord,*t2=(double*)v.coord,q,r;
  q= *t * *t;
  r= *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
#ifdef DIVZEROSAFE
  if(q==0) return DBL_MAX;
#endif
  return r/sqrt(q);
}
  //////////////////////////////////////////////////
vector3 toSpherical(const vector3& v){
  double length,*t=(double*)v.coord,y,z;
  length= *t * *t;
  t++;
  length+= *t * *t;
  t++;
  length += *t * *t;
#ifdef DIVZEROSAFE
  if(length==0){
    y=DBL_MAX;
    z=acos(DBL_MAX);
    return vector3(sqrt(length),y,z);
  }
  length= sqrt(length);
  if(0==(y=sin(z=acos(*t/length)))) y=DBL_MAX;
  else y=*v.coord/(length*y);
  return vector3(length,y,z);
#else
  length= sqrt(length);
  z=acos(*t/length);        // z=acos(coord[2]/length)
  //y=coord[0]/(length*sin(z))
  y=*v.coord/(length*sin(z)); 
  return vector3(length,y,z);
#endif
}
  //////////////////////////////////////////////////
vector3 toRectangular(const vector3& v){
  double *t= (double*)v.coord;
  double x=*t,y=*t,z=*t,q;
  t+=2;        // t is coord[2]
  q=sin(*t);   // q=sin(coord[2])
  x*= q;       // x=x*sin(coord[2])
  y*= q;       // y=y*sin(coord[2])
  z*= cos(*t); // z=z*cos(coord[2])
  t--;         // t is coord[1]
  x*=cos(*t);  // x=x*cos(coord[1])
  y*=sin(*t); // coord[1]=sin(coord[1])  
  return vector3(x,y,z);
}
  //////////////////////////////////////////////////
vector3 proj(const vector3& u, const vector3& v){
  double *t=(double*)u.coord,*t2=(double*)v.coord,q,r,y;
  q= *t * *t;
  r= *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
  t-=2;
#ifdef DIVZEROSAFE
  if(q==0) r=DBL_MAX,q=1;
#endif
  r/=q;
  q= r * *t;
  t++;
  y = r * *t;
  t++;
  return vector3(q,y, r * *t);

}
  //////////////////////////////////////////////////
double  component(const vector3& u, const vector3& v)  {
  double *t=(double*)u.coord,*t2=(double*)v.coord,q,r;
  q= *t * *t;
  r= *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
  t++;
  t2++;
  q += *t * *t;
  r += *t * *t2;
#ifdef DIVZEROSAFE
  if(q==0) return DBL_MAX;
#endif
  return r/sqrt(q);
}
  //////////////////////////////////////////////////
  //////////////////////////////////////////////////

#ifdef TEST
void main(){
  //*************
  cout << "beginning test\n";
  cout.flush();
  //*************
  cout << "********** INITIALIZERS **********\n";
  cout << "vector3 v2;\n";
  vector3 v2;
  cout << "vector3 v(1,0,0);\n";
  vector3 v(1,0,0);
  cout << "vector3 v3(1,2,3);\n";
  vector3 v3(1,2,3);
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR<< **********\n";
  cout << "cout << v  << \"\\n\";\n";
  cout << v  << "\n";
  cout << "cout << v2 << \"\\n\";\n";
  cout << v2 << "\n";
  cout << "cout << v3 << \"\\n\";\n";
  cout << v3 << "\n";
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR= **********\n";
  cout << "v=v2;\n"; 
  v=v2;
  cout << "cout << v << \'\\n\';\n";
  cout << v << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR>> **********\n";
  cout << "\t\t*Please type 3 numbers...*\n";
  cout << "cin >> v;\n";
  cin >> v;
  cout <<"v == "<< v << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR+=(double) **********\n";
  cout << "v3+=2.0;\n";
  v3+=2.0;
  cout <<"v3 = " <<v3 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR-=(double) **********\n";
  cout << "v3-=2.0;\n";
  v3-=2.0;
  cout <<"v3 == "<< v3 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR*=(double) **********\n";
  cout << "v3*=2.0;\n";
  v3*=2.0;
  cout <<"v3 == "<< v3 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR/=(double) **********\n";
  cout << "v3/=2.0;\n";
  v3/=2.0;
  cout <<"v3 == " << v3 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR+=(vector3&) **********\n";
  cout << "v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;\n";
  v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;
  cout << "v2 == " << v2 << "\n";
  cout << "v3+=v2;\n";
  v3+=v2;
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR-=(vector3&) **********\n";
  cout << "v3-=v2;\n";
  v3-=v2;
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR*=(vector3&) **********\n";
  cout << "v3*=v2\n";
  v3*=v2;
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR/=(vector3&) **********\n";
  cout << "v3/=v2;\n";
  v3/=v2;
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** dot(vector3&) **********\n";
  cout << "cout << v2.dot(v3);<< \'\\n\'\n";
  cout << v2.dot(v3) << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** length() **********\n";
  cout << "cout << v2.length();<< \'\\n\'\n";
  cout << v2.length() << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "v = v3;\n";
  v = v3;
  cout << "********** normalize() **********\n";
  cout << "cout << v.normalize();<< \'\\n\'\n";
  cout << v.normalize() << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "v.coord[0]=0; v.coord[1]=0; v.coord[2]=1;\n";
  v.coord[0]=0; v.coord[1]=0; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "********** cross(vector3&) **********\n";
  cout << "cout << v.cross(v2);<< \'\\n\'\n";
  cout << v.cross(v2) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "v3.coord[0]=1; v3.coord[1]=2; v3.coord[2]=3;\n";
  v2.coord[0]=1; v3.coord[1]=2; v3.coord[2]=3;
  cout << "********** OPERATOR+(double) **********\n";
  cout << "cout << v3+2.0 << \'\\n\';\n";
  cout << v3+2.0 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR-(double) **********\n";
  cout << "cout << v3-2.0 << \'\\n\';\n";
  cout << v3-2.0 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR*(double) **********\n";
  cout << "cout << v3*2.0 << \'\\n\';\n";
  cout << v3*2.0 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR/(double) **********\n";
  cout << "cout << v3/2.0 << \'\\n\';\n";
  cout << v3/2.0 << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR+(vector3&) **********\n";
  cout << "v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;\n";
  v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;
  cout << "v2 == " << v2 << "\n";
  cout << "cout << v3+v2 << \'\\n\';\n";
  cout << v3+v2 << '\n';
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR-(vector3&) **********\n";
  cout << "v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;\n";
  v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;
  cout << "v2 == " << v2 << "\n";
  cout << "cout << v3-v2 << \'\\n\';\n";
  cout << v3-v2 << '\n';
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR*(vector3&) **********\n";
  cout << "v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;\n";
  v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;
  cout << "v2 == " << v2 << "\n";
  cout << "cout << v3*v2 << \'\\n\';\n";
  cout << v3*v2 << '\n';
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** OPERATOR/(vector3&) **********\n";
  cout << "v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;\n";
  v2.coord[0]=1; v2.coord[1]=3; v2.coord[2]=2;
  cout << "v2 == " << v2 << "\n";
  cout << "cout << v3/v2 << \'\\n\';\n";
  cout << v3/v2 << '\n';
  cout <<"v2 == " << v2 <<" v3 == "  << v3 << '\n' ;
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** dot(vector3&,vector3&) **********\n";
  cout << "cout << dot(v2,v3);<< \'\\n\'\n";
  cout << dot(v2,v3) << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** length(vector3&) **********\n";
  cout << "cout << length(v2);<< \'\\n\'\n";
  cout << length(v2) << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "v = v3;\n";
  v = v3;
  cout << "********** normalize(vector3&) **********\n";
  cout << "cout << normalize(v);<< \'\\n\'\n";
  cout << normalize(v) << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "v.coord[0]=0; v.coord[1]=0; v.coord[2]=1;\n";
  v.coord[0]=0; v.coord[1]=0; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "********** cross(vector3&,vector3&) **********\n";
  cout << "cout << cross(v,v2);<< \'\\n\'\n";
  cout << cross(v,v2) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** OPERATOR==(vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=3; v.coord[2]=2;\n";
  v.coord[0]=1; v.coord[1]=3; v.coord[2]=2;
  cout << "v2.coord[0]=0; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=0; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=1; v3.coord[1]=3; v3.coord[2]=2;\n";
  v3.coord[0]=1; v3.coord[1]=3; v3.coord[2]=2;
  cout << "cout << v==v2 << \' \'<< v2==v3 << \' \'<< v3==v<<\'\\n\';\n";
  cout << (v==v2) << " " << (v2==v3) << " " <<(v3==v)<<'\n';
  cout << "cout << v==v <<' '<<v2==v2<<' '<<v3==v3<<\'\\n\';\n";
  cout << (v==v) <<' '<<(v2==v2)<<' '<<(v3==v3)<<'\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** OPERATOR!=(vector3&) **********\n";
  cout << "cout << v!=v2 << \' \'<< v2!=v3 << \' \'<< v3!=v<<\'\\n\';\n";
  cout << (v!=v2) << ' ' <<(v2!=v3) << ' ' <<(v3!=v)<<'\n';
  cout << "cout << v!=v <<' '<<v2!=v2<<' '<<v3!=v3<<\'\\n\';\n";
  cout << (v!=v) <<' '<<(v2!=v2)<<' '<<(v3!=v3)<<'\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** OPERATOR&&(vector3&) **********\n";
  cout << "cout << v&&v2 << \' \'<< v2&&v3 << \' \'<< v3&&v<<\'\\n\';\n";
  cout << (v&&v2) << ' ' <<(v2&&v3) << ' ' <<(v3&&v)<<'\n';
  cout << "cout << v&&v <<' '<<v2&&v2<<' '<<v3&&v3<<\'\\n\';\n";
  cout << (v&&v) <<' '<<(v2&&v2)<<' '<<(v3&&v3)<<'\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** OPERATOR||(vector3&) **********\n";
  cout << "cout << v||v2 << \' \'<< v2||v3 << \' \'<< v3||v<<\'\\n\';\n";
  cout << (v||v2) << ' ' << (v2||v3) << ' ' <<(v3||v)<<'\n';
  cout << "cout << v||v <<' '<<v2||v2<<' '<<v3||v3<<\'\\n\';\n";
  cout << (v||v) <<' '<<(v2||v2)<<' '<<(v3||v3)<<'\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateX(double) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;
  cout << " cout << v.rotateX(M_PI/2.0) << '\\n';\n";
  cout << v.rotateX(M_PI/2.0) << '\n';
  cout << " cout << v2.rotateX(M_PI/2.0) << '\\n';\n";
  cout << v2.rotateX(M_PI/2.0) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateY(double) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;
  cout << " cout << v.rotateY(M_PI/2.0) << '\\n';\n";
  cout << v.rotateY(M_PI/2.0) << '\n';
  cout << " cout << v2.rotateY(M_PI/2.0) << '\\n';\n";
  cout << v2.rotateY(M_PI/2.0) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateZ(double) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;
  cout << " cout << v.rotateZ(M_PI/2.0) << '\\n';\n";
  cout << v.rotateZ(M_PI/2.0) << '\n';
  cout << " cout << v2.rotateZ(M_PI/2.0) << '\\n';\n";
  cout << v2.rotateZ(M_PI/2.0) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateX(vector3&,double) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;
  cout << " cout << rotateX(v,M_PI/2.0) << '\\n';\n";
  cout << rotateX(v,M_PI/2.0) << '\n';
  cout << " cout << rotateX(v2,M_PI/2.0) << '\\n';\n";
  cout << rotateX(v2,M_PI/2.0) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateY(vector3&,double) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;
  cout << " cout << rotateY(v,M_PI/2.0) << '\\n';\n";
  cout << rotateY(v,M_PI/2.0) << '\n';
  cout << " cout << rotateY(v2,M_PI/2.0) << '\\n';\n";
  cout << rotateY(v2,M_PI/2.0) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateZ(vector3&,double) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;
  cout << " cout << rotateZ(v,M_PI/2.0) << '\\n';\n";
  cout << rotateZ(v,M_PI/2.0) << '\n';
  cout << " cout << rotateZ(v2,M_PI/2.0) << '\\n';\n";
  cout << rotateZ(v2,M_PI/2.0) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateXYZ(const vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=M_PI/2.0; v3.coord[1]=M_PI/4.0; v3.coord[2]=M_PI/8.0;
  cout << " cout << v.rotateXYZ(v3) << '\\n';\n";
  cout << v.rotateXYZ(v3) << '\n';
  cout << " cout << v2.rotateXYZ(v3) << '\\n';\n";
  cout << v2.rotateXYZ(v3) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** rotateXYZ(const vector3& const vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=0;\n";
  v3.coord[0]=M_PI/2.0; v3.coord[1]=M_PI/4.0; v3.coord[2]=M_PI/8.0;
  cout << " cout << rotateXYZ(v,v3) << '\\n';\n";
  cout << rotateXYZ(v,v3) << '\n';
  cout << " cout << rotateXYZ(v2,v3) << '\\n';\n";
  cout << rotateXYZ(v2,v3) << '\n';
  cout << '\n';
  cout.flush();

  //*************
  cout << "********** operator+(const double, const vector3& ) **********\n"; 
  cout << "v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;\n";
  v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;
  cout << "cout << 2+v << '\\n'\n";
  cout << 2+v << '\n';
  cout.flush();
  //*************
  cout << "********** operator+(const double, const vector3& ) **********\n"; 
  cout << "v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;\n";
  v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;
  cout << "cout << 2-v << '\\n'\n";
  cout << 2-v << '\n';
  cout.flush();
  //*************
  cout << "********** operator*(const double, const vector3& ) **********\n"; 
  cout << "v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;\n";
  v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;
  cout << "cout << 2*v << '\\n'\n";
  cout << 2*v << '\n';
  cout.flush();
  //*************
  cout << "********** operator/(const double, const vector3& ) **********\n"; 
  cout << "v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;\n";
  v.coord[0]=1; v.coord[1]=2; v.coord[2]=3;
  cout << "cout << 2/v << '\\n'\n";
  cout << 2/v << '\n';
  cout.flush();
  //*************
  cout << "********** toSpherical() **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << " cout << v.toSpherical() << '\\n';\n";
  cout << v.toSpherical() << '\n';
  cout << " cout << v2.toSpherical() << '\\n';\n";
  cout << v2.toSpherical() << '\n';
  cout << '\n';
  cout.flush();
  //*************

  cout << "********** toRectangular() **********\n";
  cout << "cout << v.toRectangular() << '\\n';\n";
  cout << v.toRectangular() << '\n';
  cout << "cout << v2.toRectangular() << '\\n';\n";
  cout << v2.toRectangular() << '\n';
  cout.flush();
  //*************
  cout << "********** proj(const vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;
  cout << "cout << v.proj(v2)<< v2.proj(v3) << \' \'<< v3.proj(v)<<\'\\n\';\n";
  cout << (v.proj(v2)) << ' ' <<(v2.proj(v3)) << ' ' <<(v3.proj(v))<<'\n';
  cout << "cout << v2.proj(v) << \' \'<< v3.proj(v2) << \' \'<< v.proj(v3)<<\'\\n\';\n";
  cout << (v2.proj(v)) << ' ' <<(v3.proj(v2)) << ' ' <<(v.proj(v3))<<'\n';
  cout << "cout << v.proj(v) <<' '<<v2.proj(v2)<<' '<<v3.proj(v3)<<\'\\n\';\n";
  cout << (v.proj(v)) <<' '<<(v2.proj(v2))<<' '<<(v3.proj(v3))<<'\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** component() **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;
  cout << "cout << v.component(v2) << \' \'<< v2.component(v3) << \' \'<< v3.component(v)<<\'\\n\';\n";
  cout << (v.component(v2)) << ' ' <<(v2.component(v3)) << ' ' <<(v3.component(v))<<'\n';
  cout << "cout << v2.component(v) << \' \'<< v3.component(v2) << \' \'<< v.component(v3)<<\'\\n\';\n";
  cout << (v2.component(v)) << ' ' <<(v3.component(v2)) << ' ' <<(v.component(v3))<<'\n';
  cout << "cout << v.component(v) <<' '<<v2.component(v2)<<' '<<v3.component(v3)<<\'\\n\';\n";
  cout << (v.component(v)) <<' '<<(v2.component(v2))<<' '<<(v3.component(v3))<<'\n';
  cout << '\n';
  cout.flush();
  //*************


  cout << "********** toSpherical(const vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << " cout << v.toSpherical() << '\\n';\n";
  cout << v.toSpherical() << '\n';
  cout << " cout << v2.toSpherical() << '\\n';\n";
  cout << v2.toSpherical() << '\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** toRectangular(const vector3&) **********\n";
  cout << "cout << toRectangular(toSpherical(v)) << '\\n';\n";
  cout << toRectangular(toSpherical(v)) << '\n';
  cout << "cout << toRectangular(toSpherical(v2)) << '\\n';\n";
  cout << toRectangular(toSpherical(v2)) << '\n';
  cout.flush();
  //*************
  cout << "********** proj(const vector3&, const vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;
  cout << "cout << proj(v,v2) << \' \'<< proj(v2,v3) << \' \'<< proj(v3,v)<<\'\\n\';\n";
  cout << (proj(v,v2)) << ' ' <<(proj(v2,v3)) << ' ' <<(proj(v3,v))<<'\n';
  cout << "cout << proj(v2,v) << \' \'<< proj(v3,v2) << \' \'<< proj(v,v3)<<\'\\n\';\n";
  cout << (proj(v2,v)) << ' ' <<(proj(v3,v2)) << ' ' <<(proj(v,v3))<<'\n';
  cout << "cout << proj(v,v) <<' '<<proj(v2,v2)<<' '<<proj(v3,v3)<<\'\\n\';\n";
  cout << (proj(v,v)) <<' '<<(proj(v2,v2))<<' '<<(proj(v3,v3))<<'\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** component(const vector3&, const vector3&) **********\n";
  cout << "v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;\n";
  v.coord[0]=1; v.coord[1]=1; v.coord[2]=1;
  cout << "v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;\n";
  v2.coord[0]=1; v2.coord[1]=0; v2.coord[2]=0;
  cout << "v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;\n";
  v3.coord[0]=0; v3.coord[1]=0; v3.coord[2]=1;
  cout << "cout << component(v,v2) << \' \'<< component(v2,v3) << \' \'<< component(v3,v)<<\'\\n\';\n";
  cout << (component(v,v2)) << ' ' <<(component(v2,v3)) << ' ' <<(component(v3,v))<<'\n';
  cout << "cout << component(v2,v) << \' \'<< component(v3,v2) << \' \'<< component(v,v3)<<\'\\n\';\n";
  cout << (component(v2,v)) << ' ' <<(component(v3,v2)) << ' ' <<(component(v,v3))<<'\n';
  cout << "cout << component(v,v) <<' '<<component(v2,v2)<<' '<<component(v3,v3)<<\'\\n\';\n";
  cout << (component(v,v)) <<' '<<(component(v2,v2))<<' '<<(component(v3,v3))<<'\n';
  cout << '\n';
  cout.flush();
  //*************
  cout << "********** **********\n";
  cout << "end test\n";
  cout.flush();
  //*************

}
#endif

