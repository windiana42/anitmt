/*
 * numerics/misc.cc
 * 
 * Misc routines and defs for the numerics code. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "num_math.h"

#include <stdio.h>


namespace NUM
{

static volatile void _CME_add1(double x,volatile double *res);

static volatile double _ComputeMachEps1()
{
	volatile double e=1.0e-5,pe=e,e1;
	do {
		pe=e;
		e/=1.5;
		_CME_add1(e,&e1);
	} while(e1>1.0);
	//fprintf(stderr,"Computed macheps1=%g=pow(2,%.2f) (sizeof(double)=%u)\n",
	//	pe,log(pe)/log(2.0),sizeof(double));
	return(pe);
}

static volatile void _CME_add1(double x,volatile double *res)
{
	*res=1.0+x;
}

// This computes the precision macheps...
static double _ComputeMachEps1_Prec()
{
	register double e=1.0e-5,pe=e;
	while(1.0+e>1.0)
	{  pe=e;  e/=1.5;  }
	//fprintf(stderr,"Computed macheps1_prec=%g\n",pe);
	return(pe);
}

// Global: machine epsilon: smallest value with 1.0+macheps1>1.0. 
const double macheps1=_ComputeMachEps1();
// Square root of the above constant: 
const double sqrt_macheps1=sqrt(macheps1);

// Precision macheps1: smallest value with 1.0+macheps1_prec>1.0 
// when vars are in registers. 
const double macheps1_prec=_ComputeMachEps1_Prec();


void _AllocFailure(size_t size)
{
	fprintf(stderr,"Failed to allocate %u bytes.\n",size);
	abort();
	exit(1);
}

}  // end of namespace NUM
