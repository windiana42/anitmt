/*****************************************************************************/
/**   This file offers datatypes designed for AniTMT                        **/
/*****************************************************************************/
/**                                                                         **/
/** Authors: Wolfgang Wieser   (wwieser@gmx.de)                             **/
/**          Martin Trautmann  (martintrautmann@gmx.de)                     **/
/**                                                                         **/
/** Report bugs and suggestions to wwieser@gmx.de .                         **/
/**                                                                         **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**                                                                         **/
/*****************************************************************************/

#ifndef __values_h__
#define __values_h__

#include <string>

#include "vect.hpp"

namespace values
{
  class Scalar;
  class Vector;
  class Matrix;
  class String;
  class Flag;

  extern const double epsilon;  // Max. difference for comparisons. 
  
  // Inline function which computes x*x; often useful if you want to 
  // calculate the square of some non-trivial expression. 
  inline double sqr(double x)  {  return(x*x);  }
  
  class Valtype{
  public:
    enum Types { scalar, vector, matrix, string, flag };
  private:
    Types type;
  public:
    Valtype(Types t) : type(t) {}
    Types get_type() const {  return(type);  }
  };

/******************************************************************************/
/*   FLAG                                                                     */
/******************************************************************************/

  class Flag : public Valtype
  {
    bool x;
  public:
    Flag(const Flag &f) : Valtype(Valtype::flag),x(f.x) {}
    Flag(bool f) : Valtype(Valtype::flag),x(f) {}
    Flag() : Valtype(Valtype::flag) {}

    operator bool() const  {  return(x);  }

  };


/******************************************************************************/
/*   SCALAR                                                                   */
/******************************************************************************/

  class Scalar : public Valtype{
    double x;
  public:
    Scalar(double i) :        Valtype(Valtype::scalar),x(i)   {}
    Scalar() :                Valtype(Valtype::scalar),x(0.0) {}
    Scalar(const Scalar &s) : Valtype(Valtype::scalar),x(s.x) {}

    operator double() const  {  return(x);  }

    // Operators comparing scalars (using epsilon): 
    friend bool operator==(const Scalar &,const Scalar &);
    friend bool operator!=(const Scalar &,const Scalar &);
    friend bool operator==(const Scalar &,double);
    friend bool operator!=(const Scalar &,double);
    friend bool operator==(double,const Scalar &);
    friend bool operator!=(double,const Scalar &);

    // Returns 1, if this scalar is 0 (exactly: if |this->x| <= epsilon )
    bool operator!()  {  return(fabs(x)<=epsilon);  }

    // addition/subtraction operators: 
    Scalar &operator-=(double a)  {  x-=a;  return(*this);  }
    Scalar &operator+=(double a)  {  x+=a;  return(*this);  }

    // multiplication/division operators: 
    Scalar &operator*=(double a)  {  x*=a;  return(*this);  }
    Scalar &operator/=(double a)  {  x/=a;  return(*this);  }
  };
  
  // Operators comparing scalars are using epsilon: 
  // If the difference between two scalars is larger than epsilon, 
  // they are considered different, else equal. 
  inline bool operator==(const Scalar &a,const Scalar &b)
    {  return(fabs(a.x-b.x)<=epsilon);  }
  inline bool operator!=(const Scalar &a,const Scalar &b)
    {  return(fabs(a.x-b.x)>epsilon);  }
  inline bool operator==(double a,const Scalar &b)
    {  return(fabs(a-b.x)<=epsilon);  }
  inline bool operator!=(double a,const Scalar &b)
    {  return(fabs(a-b.x)>epsilon);  }
  inline bool operator==(const Scalar &a,double b)
    {  return(fabs(a.x-b)<=epsilon);  }
  inline bool operator!=(const Scalar &a,double b)
    {  return(fabs(a.x-b)>epsilon);  }
  
  
/******************************************************************************/
/*   VECTOR                                                                   */
/******************************************************************************/

  // This currently implements a 3-dimensional vector; changing this 
  // is easy, though. 
  class Vector : public Valtype{
    vect::vector<3> x;
    enum NoInit { noinit };
    // This constructor is fast as it does no initialisation: 
    Vector(NoInit) : Valtype(Valtype::vector),x() {}
  public:
    // Copy constructors: 
    Vector(const Vector &v) : Valtype(Valtype::vector),x(v.x)  {}
    Vector(const vect::vector<3> &v) : Valtype(Valtype::vector),x(v)  {}
    // This generates a vector initialized to 0. 
    Vector() : Valtype(Valtype::vector),x(0)  {}
    // This generates a 3d-vector and stores the specified values in it. 
    Vector(double _x,double _y,double _z) : Valtype(Valtype::vector),x(/*no init*/)
      {  x(0,_x);  x(1,_y);  x(2,_z);  }

    // Assignment operator 
    Vector &operator=(const Vector &v)  {  x=v.x;  return(*this);  }

    //operator vect::vector<3>() const  {  return(x);  }

    // This returns the i-th row value of the vector. 
    // For a 3d-vector, i must be in range 0...2. 
    // FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON i. 
    double operator[](int i)  const  {  return(x[i]);  }

    // This sets the i-th row value of the vector. 
    // FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON i. 
    // Return value is *this. 
    Vector &operator()(int i,double a)  {  x(i,a);  return(*this);  }

    // (These versions aviod unecessray initialisations/copying.) 
    // Addition/Subtraction of two vectors: 
    friend Vector operator+(const Vector &a,const Vector &b);
    friend Vector operator-(const Vector &a,const Vector &b);

    // Multiplication/Division of a vector by a Scalar: 
    friend Vector operator*(const Vector &a,Scalar b);
    friend Vector operator*(Scalar a,const Vector &b);
    friend Vector operator/(const Vector &a,Scalar b);

    Vector &operator+=(const Vector &b)  {  x.add(b.x);  return(*this);  }
    Vector &operator-=(const Vector &b)  {  x.sub(b.x);  return(*this);  }
    Vector &operator*=(Scalar b)         {  x.mul(b);    return(*this);  }
    Vector &operator/=(Scalar b)         {  x.div(b);    return(*this);  }

    // SCALAR multiplication:
    friend Scalar operator*(const Vector &a,const Vector &b);
    friend Scalar dot(const Vector &a,const Vector &b);

    // VECTOR multiplication:
    friend Vector cross(const Vector &a,const Vector &b);
    // (Member function not faster than non-member friend.)
    Vector &cross(const Vector &b)
      {  Vector tmp(values::cross(*this,b));  this->operator=(tmp);  return(*this);  }

    // Multiplication of a vector with a matrix (resulting in a vector). 
    inline Vector &operator*=(const Matrix &b);
    friend Vector operator*(const Matrix &a,const Vector &b);
    friend Vector operator*(const Vector &a,const Matrix &b);

    // Unary operators: 
    Vector  operator+() const  {  return(*this);  }
    Vector  operator-() const {  Vector r(noinit);  r.x.neg(x);   return(r);  }

    // Operators comparing vectors (are using epsilon): 
    friend bool operator==(const Vector &,const Vector &);
    friend bool operator!=(const Vector &,const Vector &);

    // Returns 1, if this vector is the null-vector (or if no component 
    // is larger than epsilon). 
    bool operator!()  {  return(x.is_null(epsilon));  }
    bool is_null()    {  return(x.is_null(epsilon));  }

    // Return vector length and its square (the latter is faster): 
    // (Use abs(Vector) if you want a Scalar as return value.) 
    double abs()   const  {  return(x.abs());   }
    double abs2()  const  {  return(x.abs2());  }

    // Stretches vector to length 1: 
    Vector &normalize()  {  x.normalize();  return(*this);  }
    friend Vector normalize(const Vector &v);

    // Computes the angle between the two passed vectors; the returned 
    // value is in range 0...PI. 
    friend Scalar angle(const Vector &a,const Vector &b);
    inline Scalar angle(const Vector &b)  {  return(values::angle(*this,b));  }

    // Translation functions: 
    // int xyz: x=0, y=1, z=2, no range check. 
    // Member translation functions: 
    // (v.transX(d) is the same as v+Vector(d,0,0); transY() and tranzZ() 
    // work accordingly.)
    Vector &trans(double d,int xyz)  {  x.trans(d,xyz);  return(*this);  }
    Vector &transX(double delta)  {  return(trans(delta,0));  }
    Vector &transY(double delta)  {  return(trans(delta,1));  }
    Vector &transZ(double delta)  {  return(trans(delta,2));  }
    // Non-member translation functions: 
    friend Vector trans(const Vector &v,double delta,int xyz);
    friend Vector transX(const Vector &v,double delta);
    friend Vector transY(const Vector &v,double delta);
    friend Vector transZ(const Vector &v,double delta);

    // Scalation functions: 
    // int xyz: x=0, y=1, z=2, no range check. 
    // Member functions: 
    // (v.scaleX(f) is the same as v*Vector(f,1,1); scaleY() and scaleZ() 
    // work accordingly.)
    Vector &scale(double f,int xyz)  {  x.scale(f,xyz);  return(*this);  }
    Vector &scaleX(double factor)  {  return(scale(factor,0));  }
    Vector &scaleY(double factor)  {  return(scale(factor,1));  }
    Vector &scaleZ(double factor)  {  return(scale(factor,2));  }
    // Non-member functions: 
    friend Vector scale(const Vector &v,double factor,int xyz);
    friend Vector scaleX(const Vector &v,double factor);
    friend Vector scaleY(const Vector &v,double factor);
    friend Vector scaleZ(const Vector &v,double factor);

    // Member rotation functions; result overwrites *this. 
    // Faster than the non-member functions. 
    Vector &rotateX(double theta);
    Vector &rotateY(double theta);
    Vector &rotateZ(double theta);
    // There are also non-member rotation functions defined below. 

    // Mirror functions: 
    // int xyz: x=0, y=1, z=2, no range check. 
    // Member mirror functions (modifying *this): 
    Vector &mirror(int xyz)  {  x.mirror(xyz);  return(*this);  }
    Vector &mirrorX()  {  return(mirror(0));  }
    Vector &mirrorY()  {  return(mirror(1));  }
    Vector &mirrorZ()  {  return(mirror(2));  }
    // Apply mirror to all components (works like unary operator-): 
    Vector &mirror()         {  x.neg();        return(*this);  }
    // Non-member mirror functions: 
    friend Vector mirror(const Vector &v,int xyz);
    friend Vector mirrorX(const Vector &v);
    friend Vector mirrorY(const Vector &v);
    friend Vector mirrorZ(const Vector &v);
    friend Vector mirror(const Vector &v);   // apply mirror to all components 

    // Conversion: spherical <-> rectangular coordinates: 
    //           r,phi,theta <-> x,y,z  (in this order)
    // Result overwrites *this; there are also non-member functions 
    // available (below) which are slower. 
    Vector &to_spherical();
    Vector &to_rectangular();

    // Print vector to stream: 
    friend ostream& operator<<(ostream& s,const Vector &v);
  };

  inline Vector operator+(const Vector &a,const Vector &b)
    {  Vector r(Vector::noinit);  r.x.add(a.x,b.x);   return(r);  }
  inline Vector operator-(const Vector &a,const Vector &b)
    {  Vector r(Vector::noinit);  r.x.sub(a.x,b.x);   return(r);  }

  // Multiplication/Division of a vector by a Scalar: 
  inline Vector operator*(const Vector &a,Scalar b)
    {  Vector r(Vector::noinit);  r.x.mul(a.x,b);   return(r);  }
  inline Vector operator*(Scalar a,const Vector &b)
    {  Vector r(Vector::noinit);  r.x.mul(b.x,a);   return(r);  }
  inline Vector operator/(const Vector &a,Scalar b)
    {  Vector r(Vector::noinit);  r.x.div(a.x,b);   return(r);  }

  // SCALAR multiplication:
  inline Scalar operator*(const Vector &a,const Vector &b)
    {  return(Scalar(vect::scalar_mul(a.x,b.x)));  }
  inline Scalar dot(const Vector &a,const Vector &b)
    {  return(Scalar(vect::scalar_mul(a.x,b.x)));  }

  // VECTOR multiplication:
  inline Vector cross(const Vector &a,const Vector &b)
    {  Vector r(Vector::noinit);  r.x.vector_mul(a.x,b.x);   return(r);  }

  // (using epsilon)
  inline bool operator==(const Vector &a,const Vector &b)
    {  return(a.x.compare_to(b.x,epsilon));  }
  inline bool operator!=(const Vector &a,const Vector &b)
    {  return(!a.x.compare_to(b.x,epsilon));  }

  // Computes the square of the length of the specified vector: 
  inline Scalar abs2(Vector v)  {  return(Scalar(v.abs2()));  }
  // Computes length of vector: 
  inline Scalar abs(Vector v)  {  return(Scalar(v.abs()));  }

  // Computes the angle between the two passed vectors; the returned 
  // value is in range 0...PI. 
  inline Scalar angle(const Vector &a,const Vector &b)
    {  return(vect::angle(a.x,b.x));  }

  inline Vector normalize(const Vector &v)
    {  Vector r(Vector::noinit);  r.x.normalize(v.x);  return(r);  }

  // Non-member translation functions: 
  inline Vector trans(const Vector &v,double delta,int xyz)
    {  Vector r(Vector::noinit);  r.x.trans(v.x,delta,xyz);  return(r);  }
  inline Vector transX(const Vector &v,double d)  {  return(trans(v,d,0));  }
  inline Vector transY(const Vector &v,double d)  {  return(trans(v,d,1));  }
  inline Vector transZ(const Vector &v,double d)  {  return(trans(v,d,2));  }

  // Non-member scalation functions: 
  inline Vector scale(const Vector &v,double factor,int xyz)
    {  Vector r(Vector::noinit);  r.x.scale(v.x,factor,xyz);  return(r);  }
  inline Vector scaleX(const Vector &v,double f)  {  return(scale(v,f,0));  }
  inline Vector scaleY(const Vector &v,double f)  {  return(scale(v,f,1));  }
  inline Vector scaleZ(const Vector &v,double f)  {  return(scale(v,f,2));  }

  // Rotation functions. 
  extern Vector rotateX(const Vector &v,double theta);
  extern Vector rotateY(const Vector &v,double theta);
  extern Vector rotateZ(const Vector &v,double theta);

  // Mirror functions: 
  inline Vector mirror(const Vector &v,int xyz)  // x=0, y=1, z=2, no range check. 
    {  Vector r(Vector::noinit);  r.x.mirror(v.x,xyz);  return(r);  }
  inline Vector mirrorX(const Vector &v)  {  return(mirror(v,0));  }
  inline Vector mirrorY(const Vector &v)  {  return(mirror(v,1));  }
  inline Vector mirrorZ(const Vector &v)  {  return(mirror(v,2));  }
  inline Vector mirror(const Vector &v) // apply mirror to all components
    {  Vector r(Vector::noinit);  r.x.neg(v.x);  return(r);  }

  extern Vector to_spherical(const Vector &v);
  extern Vector to_rectangular(const Vector &v);

  inline ostream& operator<<(ostream& s,const Vector &v)
    {  return(vect::operator<<(s,v.x));  }


/******************************************************************************/
/*   MATRIX                                                                   */
/******************************************************************************/

  class Matrix : public Valtype{
    vect::matrix<4,4> x;
    enum NoInit { noinit };
    // This constructor is fast as it does no initialisation: 
    Matrix(NoInit) : Valtype(Valtype::matrix),x() {}
  public:
    enum NullMat { null };
    enum IdentMat { ident };
    enum MatRotX { matrotx };
    enum MatRotY { matroty };
    enum MatRotZ { matrotz };
    enum MatScale { matscale };
    enum MatTrans { mattrans };

    // Copy constructor: 
    Matrix(const Matrix &m) : Valtype(Valtype::matrix),x(m.x)  {}
    //Matrix(const vect::matrix<4,4> &m) : Valtype(Valtype::matrix),x(m)  {}
    // These generate an initialized identity matrix. 
    Matrix()         : Valtype(Valtype::matrix),x(0)  {}
    Matrix(IdentMat) : Valtype(Valtype::matrix),x(0)  {}
    // This generates an initialized null matrix. 
    Matrix(NullMat) : Valtype(Valtype::matrix),x()  {  x.set_null(); }

    // You may (of couse) use these functions but the non-member 
    // functions below (MrotateX(),...) are more convenient. 
    Matrix(enum MatRotX,double angle);
    Matrix(enum MatRotY,double angle);
    Matrix(enum MatRotZ,double angle);
    Matrix(enum MatScale,double fact,int idx);  // idx unchecked. 
    Matrix(enum MatScale,const Vector &v);
    Matrix(enum MatTrans,double delta,int idx);  // idx unchecked. 
    Matrix(enum MatTrans,const Vector &v);

    // Assignment operator 
    Matrix &operator=(const Matrix &m)  {  x=m.x;  return(*this);  }

    //operator vect::matrix<4,4>() const  {  return(x);  }

    /************************************/
    /* COLUMNS -> c -> ``X coordinate'' */
    /* ROWS    -> r -> ``Y coordinate'' */
    /* ORDER: ALWAYS c,r  ( -> x,y)     */
    /************************************/

    // This returns the i-th column of the vector; use a second index 
    // operator to get a single value. 
    // Using Matrix mat, you can access every element by calling 
    //   double val=mat[c][r];
    // FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON c, r. 
    vect::matrix_column<4> operator[](int c)  {  return(x[c]);  }

    // Set an element of the matrix: c is the column index, r the row index. 
    // NO RANGE CHECK IS PERFORMED ON c,r. 
    // Returns *this. 
    Matrix &operator()(int c,int r,double val)
      {  x(c,r,val);  return(*this); }

    // Multiplication/Division of a matrix and/by a scalar: 
    // (Divides/multiplies every element of the matrix.) 
    friend Matrix operator*(const Matrix &a,Scalar b);
    friend Matrix operator*(Scalar a,const Matrix &b);
    friend Matrix operator/(const Matrix &a,Scalar b);

    // Multiplication/Division of a matrix and/by a scalar changing *this 
    // and returning it. 
    Matrix &operator*=(Scalar b)  {  x.mul(b);  return(*this);  }
    Matrix &operator/=(Scalar b)  {  x.div(b);  return(*this);  }

    // Multiplication of a matrix and a vector: 
    // (Vector::operator*=(Matrix) is also available.)
    friend Vector operator*(const Matrix &a,const Vector &b);
    friend Vector operator*(const Vector &a,const Matrix &b);

    // Multiplication of a matrix and a matrix: 
    friend Matrix operator*(const Matrix &a,const Matrix &b);
    // Version changing *this and returning it: 
    Matrix &operator*=(const Matrix &b)  {  x.mul(b.x); return(*this);  }

    // Addition/Subtraction of two matrices (element-by-elemnt). 
    friend Matrix operator+(const Matrix &a,const Matrix &b);
    friend Matrix operator-(const Matrix &a,const Matrix &b);
    Matrix &operator+=(const Matrix &b)  {  x.add(b.x);  return(*this);  }
    Matrix &operator-=(const Matrix &b)  {  x.sub(b.x);  return(*this);  }

    // Unary operators: 
    Matrix operator+() const {  return(*this);  }
    Matrix operator-() const {  Matrix r(noinit);  r.x.neg(x);   return(r);  }

    // Operators comparing matrices (are using epsilon): 
    friend bool operator==(const Matrix &,const Matrix &);
    friend bool operator!=(const Matrix &,const Matrix &);

    // Returns 1, if this matrix is the identity-matrix (uses epsilon). 
    bool operator!()  {  return(x.is_ident(epsilon));  }
    bool is_ident()   {  return(x.is_ident(epsilon));  }

    // Returns 1, if this matrix is the null-matrix (uses epsilon). 
    bool is_null()   {  return(x.is_null(epsilon));  }

    // These functions calculate the inverse matrix. 
    Matrix &invert()  {  x.invert();  return(*this);  }
    friend Matrix invert(const Matrix &m);

    // Print matrix to stream: 
    friend ostream& operator<<(ostream& s,const Matrix &m);
  };

  inline Matrix operator*(const Matrix &a,Scalar b)
    {  Matrix r(Matrix::noinit);  r.x.mul(a.x,b);  return(r);  }
  inline Matrix operator*(Scalar a,const Matrix &b)
    {  Matrix r(Matrix::noinit);  r.x.mul(b.x,a);  return(r);  }
  inline Matrix operator/(const Matrix &a,Scalar b)
    {  Matrix r(Matrix::noinit);  r.x.div(a.x,b);  return(r);  }

  inline Vector operator*(const Matrix &m,const Vector &v)
    {  Vector r(Vector::noinit);  vect::mult(r.x,m.x,v.x);  return(r);  }
  inline Vector operator*(const Vector &v,const Matrix &m)
    {  Vector r(Vector::noinit);  vect::mult(r.x,m.x,v.x);  return(r);  }

  inline Matrix operator+(const Matrix &a,const Matrix &b)
    {  Matrix r(Matrix::noinit);  r.x.add(a.x,b.x);   return(r);  }
  inline Matrix operator-(const Matrix &a,const Matrix &b)
    {  Matrix r(Matrix::noinit);  r.x.sub(a.x,b.x);   return(r);  }

  // This one must use a temporary: 
  inline Vector &Vector::operator*=(const Matrix &m)
    {  return(this->operator=(values::operator*(m,*this)));  }

  inline Matrix operator*(const Matrix &a,const Matrix &b)
    {  Matrix r(Matrix::noinit);  vect::mult(r.x,a.x,b.x);  return(r);  }

  inline bool operator==(const Matrix &a,const Matrix &b)
    {  return(a.x.compare_to(b.x,epsilon));  }
  inline bool operator!=(const Matrix &a,const Matrix &b)
    {  return(!a.x.compare_to(b.x,epsilon));  }

  inline Matrix invert(const Matrix &m)
    {  Matrix r(Matrix::noinit);  r.x.invert(m.x);  return(r);  }

  inline ostream& operator<<(ostream& s,const Matrix &m)
    {  return(vect::operator<<(s,m.x));  }

  // **** Constructing matrices: ****
  // Scalation (?) matrices: 
  inline Matrix MscaleX(double fact)  {  return(Matrix(Matrix::matscale,fact,0));  }
  inline Matrix MscaleY(double fact)  {  return(Matrix(Matrix::matscale,fact,1));  }
  inline Matrix MscaleZ(double fact)  {  return(Matrix(Matrix::matscale,fact,2));  }
  inline Matrix Mscale(const Vector &v)  {  return(Matrix(Matrix::matscale,v));  }
  // Translation matrices: 
  inline Matrix MtranslateX(double d)  {  return(Matrix(Matrix::mattrans,d,0));  }
  inline Matrix MtranslateY(double d)  {  return(Matrix(Matrix::mattrans,d,1));  }
  inline Matrix MtranslateZ(double d)  {  return(Matrix(Matrix::mattrans,d,2));  }
  inline Matrix Mtranslate(const Vector &v)  {  return(Matrix(Matrix::mattrans,v));  }
  // Rotation matrices: 
  inline Matrix MrotateX(double angle)  {  return(Matrix(Matrix::matrotx,angle));  }
  inline Matrix MrotateY(double angle)  {  return(Matrix(Matrix::matroty,angle));  }
  inline Matrix MrotateZ(double angle)  {  return(Matrix(Matrix::matrotz,angle));  }
  // Rotates around x,y and z in this order; angles stored in v: 
  inline Matrix Mrotate(const Vector &v)
    {  return(MrotateX(v[0])*MrotateY(v[1])*MrotateZ(v[2]));  }

  // rotates a specified angle around v 
  extern Matrix Mrotate_around(const Vector &v,double angle);
  // rotates a vector to another
  extern Matrix Mrotate_vect_vect(const Vector &from,const Vector &to);
  // rotates a vector to another by using a sperical rotation with the
  // horizontal plane defined by the normal vector "up"
  extern Matrix Mrotate_vect_vect_up(const Vector &from,const Vector &to,
    const Vector &up);
  // rotates a vector pair to another
  // the first vectors of each pair will mach exactly afterwards but the second
  // may differ in the angle to the first one. They will be in the same plane
  // then. 
  extern Matrix Mrotate_pair_pair(
    const Vector &vect1f,const Vector &vect1u,
    const Vector &vect2f,const Vector &vect2u);
  // spherical rotation with the horizontal plane defined through
  // the normal vector up and the front vector 
  // the x-coordinate of angles is used for the rotation around front
  // the y-coordinate of angles is used for the rotation around up
  // and the z-coordiante specifies the angle to go up from the plane
  extern Matrix Mrotate_spherical_pair(
    const Vector &front,const Vector &up,const Vector &angles);

  // get the rotation from v1 to v2 around axis 
  extern double get_rotation_around(
    const Vector &v1,const Vector &v2,const Vector &axis);

  /*! returns a vector which tells the resulting scalation of an x,y and z 
     vector when being multipied with Matrix m.
     To reproduce the effet of m, scale, then rotate and finally translate */
  extern Vector get_scale_component( const Matrix &mat );
  /*! returns a vector representing the rotation around the x, y and finally 
     the z axis that are done to a vector when being multiplied with the 
     Matrix m.
     To reproduce the effet of m, scale, then rotate and finally translate */
  extern Vector get_rotation_component( const Matrix &mat );
  /*! returns the position independant
     translation as vector, that is done to a vector when being multipied
     with the Matrix m
     To reproduce the effet of m, scale, then rotate and finally translate */
  extern Vector get_translation_component( const Matrix &mat );
/******************************************************************************/
/*   STRING                                                                   */
/******************************************************************************/

  class String : public Valtype, public std::string {
  public:
    String() : Valtype(Valtype::string),std::string() { }
    String(std::string s) : Valtype(Valtype::string),std::string(s) { }
    String(const String &s) : Valtype(Valtype::string),std::string(s) { }
  };
}
#endif  /* __values_h__ */
