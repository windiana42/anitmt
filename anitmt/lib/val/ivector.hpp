/*
 * ivector.hpp
 * 
 * Header file containing an internally used vector template. 
 * Have a look at vector.hpp as this will probably be what you are 
 * looking for. 
 * 
 * Copyright (c) 2000--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_internal_vect_ivector_HPP_
#define _NS_internal_vect_ivector_HPP_

// NOTE: You should gain speed if you apply loop unrolling 
//       here (gcc -funroll-loops). 

// Ugly macros increase readability and reduce amount of source. 
#define _vFOR(_i_)  for(int _i_=0; _i_<N; _i_++)
#define _vFORP(_p_) for(double *_p_=x; _p_<&x[N]; _p_++)
#define _vFORCP(_p_) for(const double *_p_=x; _p_<&x[N]; _p_++)

namespace internal_vect
{

// Internal: 
namespace internal
{
	// Suffix 1 for one-dim array (vector). 
	extern ostream& stream_write_array1(ostream& s,const double *x,int n);
}

template<int N> class vector
{
	private:
		double x[N];
	public:
		// Constructor which generates an uninitialized vector: 
		vector()     { }
		// Constructor for a null-vector: 
		vector(int)  {  _vFORP(p)  *p=0.0;  }
		// Copy-constructor: 
		vector(const vector<N> &v)  {  _vFOR(i) x[i]=v.x[i];  }
		
		// Assignment operator: 
		vector<N> &operator=(const vector<N> &v)
			{  _vFOR(i) x[i]=v.x[i];  return(*this);  }
		
		// Set null-vector: 
		void set_null()  {  _vFORP(p)  *p=0.0;  }
		
		// This returns the i-th row value of the vector. 
		// For a 3d-vector, i must be in range 0...2. 
		// _vFOR SPEED INCREASE, NO RANGE CHECK IS PER_vFORMED ON i. 
		// Returns reference allowing you to set values using []. 
		double &operator[](int i)         {  return(x[i]);  }   // returning ref
		double  operator[](int i)  const  {  return(x[i]);  }   // returning copy
		
		// Functions that return the length of the vector (abs()) and 
		// the square of the length (abs2(); faster as no sqrt() is needed). 
		double abs2()  const  {  double s=0.0;  _vFORCP(p) s+=(*p)*(*p);  return(s);  }
		double abs()   const  {  return(sqrt(abs2()));  }
		
		/****************************************************************/
		/* Functions which can be applied to non-initialized vectors as */
		/* they overwrite the content of *this:                         */
		
		// Basic operations: 
		vector<N> &add(const vector<N> &a,const vector<N> &b)
			{  _vFOR(i)  x[i]=a.x[i]+b.x[i];  return(*this);  }
		vector<N> &sub(const vector<N> &a,const vector<N> &b)
			{  _vFOR(i)  x[i]=a.x[i]-b.x[i];  return(*this);  }
		vector<N> &mul(const vector<N> &a,double b)
			{  _vFOR(i)  x[i]=a.x[i]*b;  return(*this);  }
		vector<N> &div(const vector<N> &a,double b)
			{  b=1.0/b;  _vFOR(i)  x[i]=a.x[i]*b;  return(*this);  }
		
		vector<N> &vector_mul(const vector<N> &a,const vector<N> &b);
		
		// (Stretches vector v so that it gets the length 1; result stored 
		// in *this and returned.)
		vector<N> &normalize(const vector<N> &v)
			{  return(div(v,v.abs()));  }
		
		// Changes the sign of every element:
		vector<N> &neg(const vector<N> &a)
			{  _vFOR(i)  x[i]=-a.x[i];  return(*this);  }
		
		// Translation (copies vector, adds value delta to component 
		// with index n). 
		vector<N> &trans(const vector<N> &v,double delta,int n)
			{  _vFOR(i)  x[i]=v.x[i];  x[n]+=delta;  return(*this);  }
		
		// Scale vector (copies vector, then multiplies value f to 
		// component with index n). 
		vector<N> &scale(const vector<N> &v,double f,int n)
			{  _vFOR(i)  x[i]=v.x[i];  x[n]*=f;  return(*this);  }
		
		// Mirror functions; just swaps the sign of the n-th component. 
		// NO RANGE CHECK IS PER_vFORMED ON n. 
		vector<N> &mirror(const vector<N> &v,int n)
			{  _vFOR(i)  x[i]=v.x[i];  x[n]=-v.x[n];  return(*this);  }
		
		/****************************************************************/
		/* Functions taking *this as argument a and overwriting *this   */
		/* with the result:                                             */
		vector<N> &add(const vector<N> &b)
			{  _vFOR(i)  x[i]+=b.x[i];  return(*this);  }
		vector<N> &sub(const vector<N> &b)
			{  _vFOR(i)  x[i]-=b.x[i];  return(*this);  }
		vector<N> &mul(double b)
			{  _vFORP(p)  (*p)*=b;  return(*this);  }
		vector<N> &div(double b)
			{  b=1.0/b;  _vFORP(p)  (*p)*=b;  return(*this);  }
		vector<N> &normalize()
			{  return(div(abs()));  }
		
		// Changes the sign of every element:
		vector<N> &neg()
			{  _vFORP(p)  *p=-(*p);  return(*this);  }
		
		// Translation (adds value delta to component with index n). 
		vector<N> &trans(double delta,int n)
			{  x[n]+=delta;  return(*this);  }
		
		// Scale vector (multiplies value f to component with index n). 
		vector<N> &scale(double f,int n)
			{  x[n]*=f;  return(*this);  }
		
		// Mirror functions; just swaps the sign of the n-th component. 
		// NO RANGE CHECK IS PER_vFORMED ON n. 
		vector<N> &mirror(int n)
			{  x[n]=-x[n];  return(*this);  }
		
		/****************************************************************/
		
		template<int n>friend ostream& operator<<(ostream &s,const vector<n> &v);
		
		template<int r,int c>friend void mult(vector<r> &r,const matrix<r,c> &m,const vector<c> &v);
		friend void mult(vector<3> &r,const matrix<4,4> &m,const vector<3> &v);
		
		// Returns 1, if a is equal to *this (or each component pair does not 
		// differ more than epsilon). 
		int compare_to(const vector<N> &a,double epsilon) const 
			{  _vFOR(i)  if(fabs(x[i]-a.x[i])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is a null-vector (no component > epsilon). 
		int is_null(double epsilon) const 
			{  _vFORCP(p)  if(fabs(*p)>epsilon) return(0);  return(1);  }
};

// Scalar multiplication of two vectors. 
template<int N> inline double scalar_mul(const vector<N> &a,const vector<N> &b)
{  double r=0.0;  _vFOR(i) r+=a[i]*b[i];  return(r);  }

// Calculates the angle between the two specified vectors. 
// The returned value is in range 0...PI. 
template<int N> inline double angle(const vector<N> &a,const vector<N> &b)
{  return(acos(scalar_mul(a,b)/sqrt(a.abs2()*b.abs2())));  }

// Vector multiplication is currently only implemented for 3d-vectors. 
inline vector<3> &vector<3>::vector_mul(const vector<3> &a,const vector<3> &b)
{
	x[0] = a.x[1]*b.x[2] - a.x[2]*b.x[1];
	x[1] = a.x[2]*b.x[0] - a.x[0]*b.x[2];
	x[2] = a.x[0]*b.x[1] - a.x[1]*b.x[0];
	return(*this);
}
		
template<int N> inline ostream& operator<<(ostream &s,const vector<N> &v)
{  return(internal::stream_write_array1(s,v.x,N));  }

}  /* end of namespace */

#undef _vFOR
#undef _vFORP

#endif  /* _NS_internal_vect_ivector_HPP_ */
