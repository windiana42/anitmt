/*
 * numerics/function.cc
 * 
 * Numerics library function definition helpers. 
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

#include "function.h"
#include <assert.h>

namespace NUM  // numerics
{

double Function_R_R::function(double /*x*/)
{
	// Should be overridden. At least that's the purpose 
	// of the class...
	assert(0);
	return(0.0);
}


//------------------------------------------------------------------------------

void Function_R_Rn::function(double /*x*/,double *result)
{
	// Should be overridden. At least that's the purpose 
	// of the class...
	assert(0);
	for(int i=0; i<ddim; i++)
	{  result[ddim]=0.0;  }
}


//------------------------------------------------------------------------------

void Function_ODE::function(double /*x*/,const double * /*y*/,double *result)
{
	// Must be overridden when used: 
	assert(0);
	for(int i=0; i<dim; i++)
	{  result[i]=0.0;  }
}

void Function_ODE::jacobian(double /*x*/,const double * /*y*/,
	SMatrix<double> &/*df_dy*/,double * /*df_dx*/)
{
	// Must be overridden if used. 
	assert(0);
}

void Function_ODE::calcyscale(double /*xn*/,const double * /*yn*/,
	const double * /*dy_dx*/,double /*h*/,double *yscale)
{
	// Must be overridden if used. 
	for(int i=0; i<dim; i++)
		yscale[i]=1.0;
	assert(0);
}

}  // end of namespace NUM
