/*
 * ivector.hpp
 *
 * Header file containing an internally used vector template. 
 * Have a look at vector.hpp as this will probably be what you are 
 * looking for. 
 *
 * Copyright (c) 2000--2001 by Wolfgang Wieser
 * Bugs, suggestions to wwieser@gmx.de. 
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 * 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * See the GNU General Public License for details.
 * If you have not received a copy of the GNU General Public License,
 * write to the 
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * Revision History:
 *   Dec 2000   started writing
 *
 */

#ifndef _NS_internal_vect_ivector_HPP_
#define _NS_internal_vect_ivector_HPP_

// NOTE: You should gain speed if you apply loop unrolling 
//       here (gcc -funroll-loops). 

// Ugly macros increase readability and reduce amount of source. 
#define FOR(_i_)  for(int _i_=0; _i_<N; _i_++)
#define FORP(_p_) for(double *_p_=x; _p_<&x[N]; _p_++)
#define FORCP(_p_) for(const double *_p_=x; _p_<&x[N]; _p_++)

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
		// Damn function which returns a pointer to the first element of x. 
		// It is needed as long as I do not succeed in making the functions 
		// which use it friends of matrix. 
		// Either I am too stupid for this or the compiler too pedantic or 
		// C++ does not allow me to do this. 
		double *_get_ptr()  const  {  return((double*)x);  }
		
		// Constructor which generates an uninitialized vector: 
		vector()     { }
		// Constructor for a null-vector: 
		vector(int)  {  FORP(p)  *p=0.0;  }
		// Copy-constructor: 
		vector(const vector<N> &v)  {  FOR(i) x[i]=v.x[i];  }
		
		// Assignment operator: 
		vector<N> &operator=(const vector<N> &v)
			{  FOR(i) x[i]=v.x[i];  return(*this);  }
		
		// This returns the i-th row value of the vector. 
		// For a 3d-vector, i must be in range 0...2. 
		// FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON i. 
		double operator[](int i)  const  {  return(x[i]);  }
		
		// This sets the i-th row value of the vector. 
		// FOR SPEED INCREASE, NO RANGE CHECK IS PERFORMED ON i. 
		// Return value is *this. 
		vector<N> &operator()(int i,double a)  {  x[i]=a;  return(*this);  }
		
		// Functions that return the length of the vector (abs()) and 
		// the square of the length (abs2(); faster as no sqrt() is needed). 
		double abs2()  const  {  double s=0.0;  FORCP(p) s+=(*p)*(*p);  return(s);  }
		double abs()   const  {  return(sqrt(abs2()));  }
		
		/****************************************************************/
		/* Functions which can be applied to non-initialized vectors as */
		/* they overwrite the content of *this:                         */
		
		// Basic operations: 
		vector<N> &add(const vector<N> &a,const vector<N> &b)
			{  FOR(i)  x[i]=a.x[i]+b.x[i];  return(*this);  }
		vector<N> &sub(const vector<N> &a,const vector<N> &b)
			{  FOR(i)  x[i]=a.x[i]-b.x[i];  return(*this);  }
		vector<N> &mul(const vector<N> &a,double b)
			{  FOR(i)  x[i]=a.x[i]*b;  return(*this);  }
		vector<N> &div(const vector<N> &a,double b)
			{  b=1.0/b;  FOR(i)  x[i]=a.x[i]*b;  return(*this);  }
		
		vector<N> &vector_mul(const vector<N> &a,const vector<N> &b);
		
		// (Stretches vector v so that it gets the length 1; result stored 
		// in *this and returned.)
		vector<N> &normalize(const vector<N> &v)
			{  return(div(v,v.abs()));  }
		
		// Changes the sign of every element:
		vector<N> &neg(const vector<N> &a)
			{  FOR(i)  x[i]=-a.x[i];  return(*this);  }
		
		// Translation (copies vector, adds value delta to component 
		// with index n). 
		vector<N> &trans(const vector<N> &v,double delta,int n)
			{  FOR(i)  x[i]=v.x[i];  x[n]+=delta;  return(*this);  }
		
		// Scale vector (copies vector, then multiplies value f to 
		// component with index n). 
		vector<N> &scale(const vector<N> &v,double f,int n)
			{  FOR(i)  x[i]=v.x[i];  x[n]*=f;  return(*this);  }
		
		// Mirror functions; just swaps the sign of the n-th component. 
		// NO RANGE CHECK IS PERFORMED ON n. 
		vector<N> &mirror(const vector<N> &v,int n)
			{  FOR(i)  x[i]=v.x[i];  x[n]=-v.x[n];  return(*this);  }
		
		/****************************************************************/
		/* Functions taking *this as argument a and overwriting *this   */
		/* with the result:                                             */
		vector<N> &add(const vector<N> &b)
			{  FOR(i)  x[i]+=b.x[i];  return(*this);  }
		vector<N> &sub(const vector<N> &b)
			{  FOR(i)  x[i]-=b.x[i];  return(*this);  }
		vector<N> &mul(double b)
			{  FORP(p)  (*p)*=b;  return(*this);  }
		vector<N> &div(double b)
			{  b=1.0/b;  FORP(p)  (*p)*=b;  return(*this);  }
		vector<N> &normalize()
			{  return(div(abs()));  }
		
		// Changes the sign of every element:
		vector<N> &neg()
			{  FORP(p)  *p=-(*p);  return(*this);  }
		
		// Translation (adds value delta to component with index n). 
		vector<N> &trans(double delta,int n)
			{  x[n]+=delta;  return(*this);  }
		
		// Scale vector (multiplies value f to component with index n). 
		vector<N> &scale(double f,int n)
			{  x[n]*=f;  return(*this);  }
		
		// Mirror functions; just swaps the sign of the n-th component. 
		// NO RANGE CHECK IS PERFORMED ON n. 
		vector<N> &mirror(int n)
			{  x[n]=-x[n];  return(*this);  }
		
		/****************************************************************/
		
		friend ostream& operator<< <>(ostream &s,const vector<N> &v);
		
		// Returns 1, if a is equal to *this (or each component pair does not 
		// differ more than epsilon). 
		int compare_to(const vector<N> &a,double epsilon) const 
			{  FOR(i)  if(fabs(x[i]-a.x[i])>epsilon) return(0);  return(1);  }
		
		// Returns 1, if this is a null-vector (no component > epsilon). 
		int is_null(double epsilon) const 
			{  FORCP(p)  if(fabs(*p)>epsilon) return(0);  return(1);  }
};

// Scalar multiplication of two vectors. 
template<int N> inline double scalar_mul(const vector<N> &a,const vector<N> &b)
{  double r=0.0;  FOR(i) r+=a[i]*b[i];  return(r);  }

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

#undef FOR
#undef FORP

#endif  /* _NS_internal_vect_ivector_HPP_ */
