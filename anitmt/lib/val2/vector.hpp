#ifndef _NS_vect_vector_HPP_
#define _NS_vect_vector_HPP_ 1

#include "internals.hpp"
#include "scalar.hpp"

namespace vect
{

template<int N> Vector<N> cross(const Vector<N> &a,const Vector<N> &b);

template<int N>class Vector
{
	private:
		internal_vect::vector<N> x;
		enum NoInit { noinit };
		// This constructor is fast as it does no initialisation: 
		Vector(NoInit) : x() {}
	public:
		// Copy constructors: 
		Vector(const Vector<N> &v) : x(v.x)  {}
		Vector(const internal_vect::vector<3> &v) : x(v)  {}
		// This generates a vector initialized to 0. 
		Vector() : x(0)  {}
		// If you use these constructors with the wrong number of args you 
		// will get a linker error. 
		Vector(double u,double v);   // 2d only
		Vector(double x,double y,double z);  // 3d only
		Vector(double x,double y,double z,double a);  // 4d only
		Vector(double x,double y,double z,double a,double b);  // 5d only
		
		// Assignment operator 
		Vector<N> &operator=(const Vector<N> &v)  {  x=v.x;  return(*this);  }
		
		//operator internal_vect::vector<N>() const  {  return(x);  }
		
		// This returns the i-th row value of the vector. 
		// For a 3d-vector, i must be in range 0...2. 
		// FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON i. 
		double operator[](int i)  const  {  return(x[i]);  }
		
		// This sets the i-th row value of the vector. 
		// FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON i. 
		// Return value is *this. 
		Vector<N> &operator()(int i,double a)  {  x(i,a);  return(*this);  }
		
		// (These versions aviod unecessray initialisations/copying.) 
		// Addition/Subtraction of two vectors: 
		template<int n>friend Vector<n> operator+(const Vector<n> &a,const Vector<n> &b);
		template<int n>friend Vector<n> operator-(const Vector<n> &a,const Vector<n> &b);
		
		// Multiplication/Division of a vector by a Scalar: 
		template<int n>friend Vector<n> operator*(const Vector<n> &a,Scalar b);
		template<int n>friend Vector<n> operator*(Scalar a,const Vector<n> &b);
		template<int n>friend Vector<n> operator/(const Vector<n> &a,Scalar b);
		
		Vector<N> &operator+=(const Vector<N> &b)  {  x.add(b.x);  return(*this);  }
		Vector<N> &operator-=(const Vector<N> &b)  {  x.sub(b.x);  return(*this);  }
		Vector<N> &operator*=(Scalar b) 		   {  x.mul(b);    return(*this);  }
		Vector<N> &operator/=(Scalar b) 		   {  x.div(b);    return(*this);  }
		
		// SCALAR multiplication:
		template<int n>friend Scalar operator*(const Vector<n> &a,const Vector<n> &b);
		template<int n>friend Scalar dot(const Vector<n> &a,const Vector<n> &b);
		
		// VECTOR multiplication: (3d vectors only, linker error for others)
		friend Vector<N> cross(const Vector<N> &a,const Vector<N> &b);
		// (Member function not faster than non-member friend.)
		Vector<N> &cross(const Vector<N> &b);
		
		// Multiplication of a vector with a matrix (resulting in a vector). 
		// This returns void for speed increase. 
		template<int n>friend void operator*=(Vector<n> &v,const Matrix<n,n> &b);
		// Special version: 
		friend void operator*=(Vector<3> &v,const Matrix<4,4> &b);
		
		// Multiplication of matrix and vector: 
		template<int C,int R>friend Vector<R> operator*(const Matrix<C,R> &a,const Vector<C> &b);
		template<int C,int R>friend Vector<R> operator*(const Vector<C> &a,const Matrix<C,R> &b);
		// Special versions: 
		friend Vector<3> operator*(const Matrix<4,4> &a,const Vector<3> &b);
		friend Vector<3> operator*(const Vector<3> &a,const Matrix<4,4> &b);
		
		// Unary operators: 
		Vector<N> operator+() const  {  return(*this);  }
		Vector<N> operator-() const  {  Vector<N> r(noinit);  r.x.neg(x);  return(r);  }
		
		// Operators comparing vectors (are using epsilon): 
		template<int n>friend bool operator==(const Vector<n> &,const Vector<n> &);
		template<int n>friend bool operator!=(const Vector<n> &,const Vector<n> &);
		
		// Returns 1, if this vector is the null-vector (or if no component 
		// is larger than epsilon). 
		bool operator!() const {  return(x.is_null(epsilon));  }
		bool is_null() const {  return(x.is_null(epsilon));  }
		
		// Return vector length and its square (the latter is faster): 
		// (Use abs(Vector) if you want a Scalar as return value.) 
		double abs()   const  {  return(x.abs());   }
		double abs2()  const  {  return(x.abs2());  }
		
		// Stretches vector to length 1: 
		Vector<N> &normalize()  {  x.normalize();  return(*this);  }
		template<int n>friend Vector<n> normalize(const Vector<n> &v);
		
		// Computes the angle between the two passed vectors; the returned 
		// value is in range 0...PI. 
		template<int n>friend Scalar angle(const Vector<n> &a,const Vector<n> &b);
		inline Scalar angle(const Vector<N> &b) const  {  return(vect::angle(*this,b));  }
		
		// Member rotation functions; result overwrites *this. 
		// Faster than the non-member functions. 
		// (ONLY AVAILABLE FOR 3d VECTORS.)
		Vector<N> &rotateX(double theta);
		Vector<N> &rotateY(double theta);
		Vector<N> &rotateZ(double theta);
		// There are also non-member rotation functions defined below. 
		
		// Translation functions: 
		// int xyz: x=0, y=1, z=2, ... no range check. 
		// Member (modifying *this) and non-member version: 
		Vector<N> &trans(double d,int xyz)  {  x.trans(d,xyz);  return(*this);  }
		template<int n>friend Vector<n> trans(const Vector<n> &v,double delta,int xyz);
		
		// Scalation functions: 
		// int xyz: x=0, y=1, z=2, ... no range check. 
		// Member (modifying *this) and non-member version: 
		Vector<N> &scale(double f,int xyz)  {  x.scale(f,xyz);  return(*this);  }
		template<int n>friend Vector<n> scale(const Vector<n> &v,double factor,int xyz);
		
		// Mirror functions: 
		// int xyz: x=0, y=1, z=2, ... no range check. 
		// Member (modifying *this) and non-member version: 
		Vector<N> &mirror(int xyz)  {  x.mirror(xyz);  return(*this);  }
		template<int n>friend Vector<n> mirror(const Vector<n> &v,int xyz);
		// Apply mirror to all components (works like unary operator-): 
		Vector<N> &mirror()         {  x.neg();   return(*this);  }
		template<int n>friend Vector<n> mirror(const Vector<n> &v);
		
		// Conversion: spherical <-> rectangular coordinates: 
		//           r,phi,theta <-> x,y,z  (in this order)
		// Result overwrites *this; there are also non-member functions 
		// available (below) which are slower. 
		// (ONLY AVAILABLE FOR 3d VECTORS.)
		Vector<N> &to_spherical();
		Vector<N> &to_rectangular();
		
		// Print vector to stream: 
		template<int n>friend ostream& operator<<(ostream& s,const Vector<n> &v);
};

template<int N>inline Vector<N> operator+(const Vector<N> &a,const Vector<N> &b)
	{  Vector<N> r(Vector<N>::noinit);  r.x.add(a.x,b.x);   return(r);  }
template<int N>inline Vector<N> operator-(const Vector<N> &a,const Vector<N> &b)
	{  Vector<N> r(Vector<N>::noinit);  r.x.sub(a.x,b.x);   return(r);  }

// Multiplication/Division of a vector by a Scalar: 
template<int N>inline Vector<N> operator*(const Vector<N> &a,Scalar b)
	{  Vector<N> r(Vector<N>::noinit);  r.x.mul(a.x,b);   return(r);  }
template<int N>inline Vector<N> operator*(Scalar a,const Vector<N> &b)
	{  Vector<N> r(Vector<N>::noinit);  r.x.mul(b.x,a);   return(r);  }
template<int N>inline Vector<N> operator/(const Vector<N> &a,Scalar b)
	{  Vector<N> r(Vector<N>::noinit);  r.x.div(a.x,b);   return(r);  }

// SCALAR multiplication:
template<int N>inline Scalar operator*(const Vector<N> &a,const Vector<N> &b)
	{  return(Scalar(internal_vect::scalar_mul(a.x,b.x)));  }
template<int N>inline Scalar dot(const Vector<N> &a,const Vector<N> &b)
	{  return(Scalar(internal_vect::scalar_mul(a.x,b.x)));  }

// Computes the square of the length of the specified vector: 
template<int N>inline Scalar abs2(const Vector<N> &v)  {  return(Scalar(v.abs2()));  }
// Computes length of vector: 
template<int N>inline Scalar abs(const Vector<N> &v)  {  return(Scalar(v.abs()));  }

// (using epsilon)
template<int N>inline bool operator==(const Vector<N> &a,const Vector<N> &b)
	{  return(a.x.compare_to(b.x,epsilon));  }
template<int N>inline bool operator!=(const Vector<N> &a,const Vector<N> &b)
	{  return(!a.x.compare_to(b.x,epsilon));  }
template<int N>inline bool operator<(const Vector<N> &a,const Vector<N> &b)
	{  return(abs2(a) < abs2(b));  }
template<int N>inline bool operator>(const Vector<N> &a,const Vector<N> &b)
	{  return(abs2(a) > abs2(b));  }
template<int N>inline bool operator<=(const Vector<N> &a,const Vector<N> &b)
	{  return(abs2(a) <= abs2(b));  }
template<int N>inline bool operator>=(const Vector<N> &a,const Vector<N> &b)
	{  return(abs2(a) >= abs2(b));  }

// Computes the angle between the two passed vectors; the returned 
// value is in range 0...PI. 
template<int N>inline Scalar angle(const Vector<N> &a,const Vector<N> &b)
	{  return(internal_vect::angle(a.x,b.x));  }

template<int N>inline Vector<N> normalize(const Vector<N> &v)
	{  Vector<N> r(Vector<N>::noinit);  r.x.normalize(v.x);  return(r);  }

// Non-member translation functions: 
template<int N>inline Vector<N> trans(const Vector<N> &v,double delta,int xyz)
	{  Vector<N> r(Vector<N>::noinit);  r.x.trans(v.x,delta,xyz);  return(r);  }

// Non-member scalation functions: 
template<int N>inline Vector<N> scale(const Vector<N> &v,double factor,int xyz)
	{  Vector<N> r(Vector<N>::noinit);  r.x.scale(v.x,factor,xyz);  return(r);  }

// Mirror functions: 
// x=0, y=1, z=2, no range check. 
template<int N>inline Vector<N> mirror(const Vector<N> &v,int xyz)
	{  Vector<N> r(Vector<N>::noinit);  r.x.mirror(v.x,xyz);  return(r);  }
// apply mirror to all components
template<int N>inline Vector<N> mirror(const Vector<N> &v)
	{  Vector<N> r(Vector<N>::noinit);  r.x.neg(v.x);  return(r);  }

template<int N>inline ostream& operator<<(ostream& s,const Vector<N> &v)
{  return(internal_vect::operator<<(s,v.x));  }

// VECTOR CONSTRUCTION: 
inline Vector<2>::Vector(double _0,double _1) : x(/*no init*/)
	{  x(0,_0);  x(1,_1);  }
inline Vector<3>::Vector(double _0,double _1,double _2) : x(/*no init*/)
	{  x(0,_0);  x(1,_1);  x(2,_2);  }
inline Vector<4>::Vector(double _0,double _1,double _2,double _3) : x(/*no init*/)
	{  x(0,_0);  x(1,_1);  x(2,_2);  x(3,_3);  }
inline Vector<5>::Vector(double _0,double _1,double _2,double _3,double _4) : x(/*no init*/)
	{  x(0,_0);  x(1,_1);  x(2,_2);  x(3,_3);  x(4,_4);  }

/** FUNCTIONS FOR 3d VECTORS: **/

inline Vector<3> cross(const Vector<3> &a,const Vector<3> &b)
	{  Vector<3> r(Vector<3>::noinit);  r.x.vector_mul(a.x,b.x);  return(r);  }
inline Vector<3> &Vector<3>::cross(const Vector<3> &b)
	{  Vector<3> tmp(vect::cross(*this,b));  this->operator=(tmp);  return(*this);  }

// Rotation functions. 
extern Vector<3> rotateX(const Vector<3> &v,double theta);
extern Vector<3> rotateY(const Vector<3> &v,double theta);
extern Vector<3> rotateZ(const Vector<3> &v,double theta);

// Conversion functions
extern Vector<3> to_spherical(const Vector<3> &v);
extern Vector<3> to_rectangular(const Vector<3> &v);

}  // namespace end 

#endif  /* _NS_vect_vector_HPP_ */