/*
 * numerics/function.h
 * 
 * Numerics library function definition. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _WW_NUMERICS_FUNCTION_H_
#define _WW_NUMERICS_FUNCTION_H_

#include <numerics/num_math.h>

namespace NUM  // numerics
{

// Base class for function R^1 -> R^1
class Function_R_R
{
	protected:
		// Must use a derived class, hence protected: 
		Function_R_R()  { ncalls=0; }
		// To be overridden for function call: 
		virtual double function(double x);
	public:  _CPP_OPERATORS
		virtual ~Function_R_R(){}
		
		// Function call counter: Random access granted. 
		unsigned int ncalls;
		
		// Use that to evaluate the function: 
		// (Increases ncalls each time used.)
		inline double operator()(double x)
			{  ++ncalls;  return(function(x));  }
};


// Base class for R->R^n functions (i.e. curves): 
class Function_R_Rn
{
	protected:
		// Destination space dimension (i.e. the "n" in R^n): 
		int ddim;
		
		// Must use a derived class, hence protected: 
		Function_R_Rn(int _ddim)  {  ncalls=0;  ddim=_ddim; }
		// To be overridden for function call: 
		virtual void function(double x,double *result);
	public:  _CPP_OPERATORS
		virtual ~Function_R_Rn(){}
		
		// Function call counter: Random access granted. 
		unsigned int ncalls;
		
		// Get destination space dimension: 
		int DDim() const
			{  return(ddim);  }
		
		// Set the destination space dimension: 
		void SetDDim(int _ddim)
			{  ddim=_ddim;  }
		
		// Use that to evaluate the function: 
		// (Increases ncalls each time used.)
		// Result is stored in result which is an array 
		// of size ddim. 
		inline void operator()(double x,double *result)
			{  ++ncalls;  function(x,result);  }
};


template<typename T>class SMatrix;

// Base class for function (R,R^n) -> R^n for differential equations. 
class Function_ODE
{
	protected:
		// Source/destination space dimension (i.e. the "n" in R^n): 
		int dim;
		
		// Must use a derived class, hence protected: 
		Function_ODE(int _dim)  {  ncalls=0;  dim=_dim; }
		// To be overridden for function call: 
		virtual void function(double x,const double *y,double *result);
		// To be overridden for Jacobi Matrix eval if needed: 
		// df_dy[i][j] = d f[i] / d y[j]
		virtual void jacobian(double x,const double *y,
			SMatrix<double> &df_dy,double *df_dx);
		// To be overridden if used; calcuate the scale factors for 
		// error estimation: Stores result in yscale[]. 
		virtual void calcyscale(double xn,const double *yn,
			const double *dy_dx,double h,double *yscale);
	public:  _CPP_OPERATORS
		virtual ~Function_ODE(){}
		
		// Function call counter: Random access granted. 
		unsigned int ncalls;
		
		// Get source/destination space dimension: 
		int Dim() const
			{  return(dim);  }
		
		// Use that to evaluate the function: 
		// (Increases ncalls each time used.)
		// Pass parameters: "time value" x and values in array y[] 
		// of size dim. 
		// Result is stored in result which is an array of size dim. 
		inline void operator()(double x,const double *y,double *result)
			{  ++ncalls;  function(x,y,result);  }
		
		// Used to evaluate the Jacobi matrix of the function. 
		// Need only be provided if the used algorithm will make 
		// use of it. 
		inline void Jacobian(double x,const double *y,
			SMatrix<double> &df_dy,double *df_dx)
			{  jacobian(x,y,df_dy,df_dx);  }
		
		// Scale factor calculation function for error estimation. 
		// Stores result in yscale[]. 
		virtual void CalcYScale(double xn,const double *yn,
			const double *dy_dx,double h,double *yscale)
				{  return(calcyscale(xn,yn,dy_dx,h,yscale));  }
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_FUNCTION_H_ */
