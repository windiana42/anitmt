/*
 * function.cpp
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define _GNU_SOURCE  // \___ Must be included as very first 
#include <math.h>    // /    files or things break. 
#include "function.hpp"

#include <stdio.h>
#include <assert.h>

namespace exparse
{

namespace  // `static' namespace
{

// Arg check functions: 
// Return 0 if okay, 2 if too many, 1 otherwise. 
// (`too many' needed by FunctionCaller to stop user from 
//  adding more and more args.) 
int ca_exactly1(int n)  {  return((n==1) ? 0 : ((n>1) ? 2 : 1));  }
int ca_exactly2(int n)  {  return((n==2) ? 0 : ((n>2) ? 2 : 1));  }
int ca_exactly3(int n)  {  return((n==3) ? 0 : ((n>3) ? 2 : 1));  }
int ca_atleast1(int n)  {  return(n<1);  }
int ca_atleast2(int n)  {  return(n<2);  }
int ca_1or2    (int n)  {  return((n>2) ? 2 : ((n<1) ? 1 : 0));  }

// These are extremely ugly but save extremely much writing. 
#define FuncArgConv(vtype,arg,idx) \
	const Value *arg=fa->args[idx]; \
	Value conv##idx; \
	if(arg->type()!=VT##vtype) \
	{ \
		if(!arg->can_convert(VT##vtype)) \
		{  return(FSIllegalArg);  } \
		conv##idx =arg; \
		conv##idx .convert(VT##vtype); \
		arg=&conv##idx; \
	}

// FuncScalarConv(arg,0) 
// Ensures that arg points to the first arg converted to a 
// Scalar or returns FSIllegalArg. 
#define FuncFlagConv(arg,idx)   FuncArgConv(Flag,arg,idx)
#define FuncScalarConv(arg,idx) FuncArgConv(Scalar,arg,idx)
#define FuncVectorConv(arg,idx) FuncArgConv(Vector,arg,idx)


//! The functions themselves: 
//! The functions are guaranteed that 
//!  - fa.result is non-NULL. 
//!  - fa.nargs is a value supported by them (according to 
//!             check_nargs call). 
//!  - fa.fdesc is the corresponding function description. 
FunctionState pf_abs(FunctionArgs *fa)
{
	const Value *arg=fa->args[0];
	switch(arg->type())
	{
		case VTScalar:  *fa->result=Scalar(fabs(arg->rscalar()));  break;
		case VTVector:  *fa->result=Scalar(arg->pvector()->abs());  break;
		case VTFlag:    *fa->result=Flag(arg->rflag());  break;
		default:
			// See if we can convert the arg to a scalar: 
			if(!arg->can_convert(VTScalar))
			{  return(FSIllegalArg);  }
			
			Value tmp(*arg);
			tmp.convert(VTScalar);
			assert(tmp.type()==VTScalar);
			*fa->result=Scalar(fabs(tmp.rscalar()));
			break;
	}
	return(FSSuccess);
}

FunctionState pf_acos(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(acos(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_acosh(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(acosh(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_asin(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(asin(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_asinh(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(asinh(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_atan(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(atan(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_atan2(FunctionArgs *fa)
{
	FuncScalarConv(argy,0)
	FuncScalarConv(argx,1)
	*fa->result=Scalar(atan2(argy->rscalar(),argx->rscalar()));
	return(FSSuccess);
}

FunctionState pf_atanh(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(atanh(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_cbrt(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(cbrt(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_ceil(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(ceil(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_cos(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(cos(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_cosh(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(cosh(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_deg2rad(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(deg2rad(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_exp(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(exp(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_exp2(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(pow(2.0,arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_floor(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(floor(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_hypot(FunctionArgs *fa)
{
	FuncScalarConv(argx,0)
	FuncScalarConv(argy,1)
	*fa->result=Scalar(hypot(argx->rscalar(),argy->rscalar()));
	return(FSSuccess);
}

FunctionState pf_isinf(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Flag(isinf(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_isnan(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Flag(isnan(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_ln(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(log(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_log(FunctionArgs *fa)
{
	FuncScalarConv(argbase,0)
	FuncScalarConv(argx,1)
	*fa->result=Scalar(log(argx->rscalar())/log(argbase->rscalar()));
	return(FSSuccess);
}

FunctionState pf_log10(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(log10(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_log2(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(log(arg->rscalar())/M_LN2);
	return(FSSuccess);
}

FunctionState pf_max(FunctionArgs *fa)
{
	{
		FuncScalarConv(arg,0);
		fa->result->assign(arg);
	}
	for(int i=1; i<fa->nargs; i++)
	{
		FuncScalarConv(arg,i);
		if(arg->rscalar()>fa->result->rscalar())
		{  fa->result->assign(arg);  }
	}
	return(FSSuccess);
}

FunctionState pf_min(FunctionArgs *fa)
{
	{
		FuncScalarConv(arg,0);
		fa->result->assign(arg);
	}
	for(int i=1; i<fa->nargs; i++)
	{
		FuncScalarConv(arg,i);
		if(arg->rscalar()<fa->result->rscalar())
		{  fa->result->assign(arg);  }
	}
	return(FSSuccess);
}

FunctionState pf_nop(FunctionArgs *fa)
{
	// No Operation function. Simply returns arg. 
	fa->result->assign(fa->args[0]);
	return(FSSuccess);
}

FunctionState pf_normalize(FunctionArgs *fa)
{
	FuncVectorConv(arg,0);
	*fa->result=Vector(arg->cvector().normalize());
	return(FSSuccess);
}

FunctionState pf_pow(FunctionArgs *fa)
{
	FuncScalarConv(arga,0)
	FuncScalarConv(argb,1)
	*fa->result=Scalar(pow(arga->rscalar(),argb->rscalar()));
	return(FSSuccess);
}

FunctionState pf_rad2deg(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(rad2deg(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_rotate(FunctionArgs *fa)
{
	return(FSIllegalArg);
}

FunctionState pf_rotatex(FunctionArgs *fa)
{
	return(FSIllegalArg);
}

FunctionState pf_rotatey(FunctionArgs *fa)
{
	return(FSIllegalArg);
}

FunctionState pf_rotatez(FunctionArgs *fa)
{
	return(FSIllegalArg);
}

FunctionState pf_round(FunctionArgs *fa)
{
	return(FSIllegalArg);
}

FunctionState pf_sgn(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	// I take double here, NOT Scalar because of the comparison. 
	double a=arg->rscalar();
	     if(a>0.0)  {  *fa->result=Scalar(+1.0);  }
	else if(a<0.0)  {  *fa->result=Scalar(-1.0);  }
	else            {  *fa->result=Scalar( 0.0);  }
	return(FSSuccess);
}

FunctionState pf_sin(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(sin(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_sinh(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(sinh(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_sqrt(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(sqrt(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_tan(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(tan(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_tanh(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(tanh(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_trunc(FunctionArgs *fa)
{
	FuncScalarConv(arg,0);
	*fa->result=Scalar(trunc(arg->rscalar()));
	return(FSSuccess);
}

FunctionState pf_vcross(FunctionArgs *fa)
{
	FuncVectorConv(a,0);
	FuncVectorConv(b,1);
	*fa->result=Vector(cross(a->rvector(),b->rvector()));
	return(FSSuccess);
}

FunctionState pf_vdot(FunctionArgs *fa)
{
	FuncVectorConv(a,0);
	FuncVectorConv(b,1);
	*fa->result=Scalar(dot(a->rvector(),b->rvector()));
	return(FSSuccess);
}
FunctionState pf_vlength(FunctionArgs *fa)
{
	FuncVectorConv(arg,0);
	*fa->result=Scalar(arg->pvector()->abs());
	return(FSSuccess);
}

/******** INTERNAL FUNCTIONS ********/

FunctionState pfi_dot(FunctionArgs *fa)
{
	// Implementation of the member select code. 
	const Value *argA=fa->args[0];
	if(argA->type()==VTVector)
	{
		const Value *argB=fa->args[1];
		int v_idx=-1;
		int vect_dims=3;   // (of argA) #### needs fixing for 2d,4d,5d vectors *FIXME*
		if(argB->type()==VTScalar)
		{
			Scalar a=argB->rscalar();
			// First, I do a larger range check (purpose: prevent 
			// integer overflow). 
			if(a<-2.0 || a>10.0)  // index value out of range
			{  return(FSOutOfRange);  }
			v_idx=int(a);
		}
		else if(argB->type()==VTVector)
		{
			// All elements must be 0 and one must be 1. 
			Vector v=argB->rvector();
			for(int i=0; i<3; i++)
			{
				if(v[i]==1.0)  // compared using epsilon
				{  v_idx=i;  break;  }
			}
			for(int i=0; i<3; i++)
			{
				if(v_idx!=i && v[i]!=0.0)  // compared using epsilon
				{  return(FSOutOfRange);  }
			}
		}
		else
		{  return(FSIllegalArg);  }
		
		if(v_idx<0 || v_idx>=vect_dims)
		{  return(FSOutOfRange);  }
		
		*fa->result=Scalar(argA->rvector()[v_idx]);
		return(FSSuccess);
	}
	return(FSIllegalArg);
}

FunctionState pfi_not(FunctionArgs *fa)
{
	FuncFlagConv(arg,0);
	*fa->result=Flag(!arg->cflag());
	return(FSSuccess);
}

FunctionState pfi_pos(FunctionArgs *fa)
{
	const Value *arg=fa->args[0];
	switch(arg->type())
	{
		case VTScalar:  *fa->result=arg->cscalar();  break;
		case VTVector:  *fa->result=arg->cvector();  break;
		default:  return(FSIllegalArg);
	}
	return(FSSuccess);
}

FunctionState pfi_neg(FunctionArgs *fa)
{
	const Value *arg=fa->args[0];
	switch(arg->type())
	{
		case VTScalar:  *fa->result=Scalar(-(arg->cscalar()));  break;
		case VTVector:  *fa->result=Vector(-(arg->cvector()));  break;
		default:  return(FSIllegalArg);
	}
	return(FSSuccess);
}

FunctionState pfi_pow(FunctionArgs *fa)
{
	return(pf_pow(fa));
}

FunctionState pfi_mul(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Scalar(a->cscalar() * b->cscalar());  goto brk;  }
	if(a->type()==VTScalar && b->type()==VTVector)
	{  *fa->result=Vector(a->cscalar() * b->cvector());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTScalar)
	{  *fa->result=Vector(a->cvector() * b->cscalar());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTVector)
	{
		fprintf(stderr,"vector*vector: dot, cross, or pov-mul?\n");  
		return(FSIllegalArg);
	}
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_div(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Scalar(a->cscalar() / b->cscalar());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTScalar)
	{  *fa->result=Vector(a->cvector() / b->cscalar());  goto brk;  }
	if(a->type()==VTScalar && b->type()==VTVector)
	{
		fprintf(stderr,"cannot div scalar/vector\n");
		return(FSIllegalArg);
	}
	if(a->type()==VTVector && b->type()==VTVector)
	{
		fprintf(stderr,"vector*vector: dot, cross, or pov-mul?\n");  
		return(FSIllegalArg);
	}
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_add(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Scalar(a->cscalar() + b->cscalar());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTVector)
	{  *fa->result=Vector(a->cvector() + b->cvector());  goto brk;  }
	if(a->type()==VTString && b->type()==VTString)
	{  *fa->result=String(a->rstring() + b->rstring());  goto brk;  }
	if(a->type()==VTScalar && b->type()==VTVector || 
	   a->type()==VTVector && b->type()==VTScalar )
	{
		fprintf(stderr,"cannot add scalar to vector or vice versa.\n");  
		return(FSIllegalArg);
	}
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_sub(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Scalar(a->cscalar() - b->cscalar());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTVector)
	{  *fa->result=Vector(a->cvector() - b->cvector());  goto brk;  }
	if(a->type()==VTScalar && b->type()==VTVector || 
	   a->type()==VTVector && b->type()==VTScalar )
	{
		fprintf(stderr,"cannot sub scalar from vector or vice versa.\n");  
		return(FSIllegalArg);
	}
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_le(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Flag((a->rscalar()<=b->rscalar()));  goto brk;  }
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_ge(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Flag((a->rscalar()>=b->rscalar()));  goto brk;  }
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_l(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Flag((a->rscalar()<b->rscalar()));  goto brk;  }
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_g(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Flag((a->rscalar()>b->rscalar()));  goto brk;  }
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_eq(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Flag(a->rscalar()==b->rscalar());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTVector)
	{  *fa->result=Flag(a->rvector()==b->rvector());  goto brk;  }
	if(a->type()==VTFlag && b->type()==VTFlag)
	{  *fa->result=Flag(a->rflag()==b->rflag());  goto brk;  }
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_neq(FunctionArgs *fa)
{
	const Value *a=fa->args[0];
	const Value *b=fa->args[1];
	if(a->type()==VTScalar && b->type()==VTScalar)
	{  *fa->result=Flag(a->rscalar()!=b->rscalar());  goto brk;  }
	if(a->type()==VTVector && b->type()==VTVector)
	{  *fa->result=Flag(a->rvector()!=b->rvector());  goto brk;  }
	if(a->type()==VTFlag && b->type()==VTFlag)
	{  *fa->result=Flag(a->rflag()!=b->rflag());  goto brk;  }
	return(FSIllegalArg);
	brk: return(FSSuccess);
}

FunctionState pfi_and(FunctionArgs *fa)
{
	FuncFlagConv(a,0);
	FuncFlagConv(b,1);
	*fa->result=Flag(a->rflag() && b->rflag());
	return(FSSuccess);
}

FunctionState pfi_or(FunctionArgs *fa)
{
	FuncFlagConv(a,0);
	FuncFlagConv(b,1);
	*fa->result=Flag(a->rflag() || b->rflag());
	return(FSSuccess);
}

FunctionState pfi_cond(FunctionArgs *fa)
{
	// NOTE: conditional expressions need not have same value types 
	//       in if-else branch: e.g. (flag ? <1,2,3> : 1.7)
	FuncFlagConv(cond,0);
	if(fa->args[1]->type()!=fa->args[2]->type())
	{  fprintf(stderr,"warning: different valtypes in conditional expression\n");  }
	if(cond->rflag()==true)
	{  fa->result->assign(fa->args[1]);  }
	else
	{  fa->result->assign(fa->args[2]);  }
	return(FSSuccess);
}

FunctionState pfi_gv(FunctionArgs *fa)
{
	FuncScalarConv(x,0);
	FuncScalarConv(y,1);
	FuncScalarConv(z,2);
	*fa->result=Vector(x->rscalar(),y->rscalar(),z->rscalar());
	return(FSSuccess);
}


const struct FunctionDesc funcs[]=
{ /* IMPORTANT: KEEP THE FUNCTIONS SORTED ALPHABETICALLY! */
	{ "abs",       &ca_exactly1, &pf_abs       },
	{ "acos",      &ca_exactly1, &pf_acos      },
	{ "acosh",     &ca_exactly1, &pf_acosh     },
	{ "asin",      &ca_exactly1, &pf_asin      },
	{ "asinh",     &ca_exactly1, &pf_asinh     },
	{ "atan",      &ca_exactly1, &pf_atan      },
	{ "atan2",     &ca_exactly2, &pf_atan2     },
	{ "atanh",     &ca_exactly1, &pf_atanh     },
	{ "cbrt",      &ca_exactly1, &pf_cbrt      },
	{ "ceil",      &ca_exactly1, &pf_ceil      },
	{ "cos",       &ca_exactly1, &pf_cos       },
	{ "cosh",      &ca_exactly1, &pf_cosh      },
	{ "deg2rad",   &ca_exactly1, &pf_deg2rad   },
	{ "exp",       &ca_exactly1, &pf_exp       },
	{ "exp2",      &ca_exactly1, &pf_exp2      },
	{ "floor",     &ca_exactly1, &pf_floor     },
	{ "hypot",     &ca_exactly2, &pf_hypot     },
	{ "isinf",     &ca_exactly1, &pf_isinf     },
	{ "isnan",     &ca_exactly1, &pf_isnan     },
	{ "ln",        &ca_exactly1, &pf_ln        },
	{ "log",       &ca_exactly1, &pf_log       },
	{ "log10",     &ca_exactly1, &pf_log10     },
	{ "log2",      &ca_exactly1, &pf_log2      },
	{ "max",       &ca_atleast1, &pf_max       },
	{ "min",       &ca_atleast1, &pf_min       },
	{ "nop",       &ca_exactly1, &pf_nop       },
	{ "normalize", &ca_exactly1, &pf_normalize },
	{ "pow",       &ca_exactly2, &pf_pow       },
	{ "rad2deg",   &ca_exactly1, &pf_rad2deg   },
	{ "rotate",    &ca_exactly3, &pf_rotate    },
	{ "rotatex",   &ca_exactly2, &pf_rotatex   },
	{ "rotatey",   &ca_exactly2, &pf_rotatey   },
	{ "rotatez",   &ca_exactly2, &pf_rotatez   },
	{ "round",     &ca_1or2,     &pf_round     },
	{ "sgn",       &ca_exactly1, &pf_sgn       },
	{ "sin",       &ca_exactly1, &pf_sin       },
	{ "sinh",      &ca_exactly1, &pf_sinh      },
	{ "sqrt",      &ca_exactly1, &pf_sqrt      },
	{ "tan",       &ca_exactly1, &pf_tan       },
	{ "tanh",      &ca_exactly1, &pf_tanh      },
	{ "trunc",     &ca_exactly1, &pf_trunc     },
	{ "vcross",    &ca_exactly2, &pf_vcross    },
	{ "vdot",      &ca_exactly2, &pf_vdot      },
	{ "vlength",   &ca_exactly1, &pf_vlength   },
	{ NULL, NULL, NULL }
};

const struct FunctionDesc internal_funcs[]=
{  // These do not have to be sorted. 
	{ "!",  &ca_exactly1, &pfi_not  },  // OTNot
	{ " +", &ca_exactly1, &pfi_pos  },  // OTPos   // space in " +" is important
	{ " -", &ca_exactly1, &pfi_neg  },  // OTNeg
	{ "^",  &ca_exactly2, &pfi_pow  },  // OTPow
	{ "*",  &ca_exactly2, &pfi_mul  },  // OTMul
	{ "/",  &ca_exactly2, &pfi_div  },  // OTDiv
	{ "+",  &ca_exactly2, &pfi_add  },  // OTAdd
	{ "-",  &ca_exactly2, &pfi_sub  },  // OTSub
	{ "<=", &ca_exactly2, &pfi_le   },  // OTLE
	{ ">=", &ca_exactly2, &pfi_ge   },  // OTGE
	{ "<",  &ca_exactly2, &pfi_l    },  // OTL
	{ ">",  &ca_exactly2, &pfi_g    },  // OTG
	{ "==", &ca_exactly2, &pfi_eq   },  // OTEq
	{ "!=", &ca_exactly2, &pfi_neq  },  // OTNEq
	{ "&&", &ca_exactly2, &pfi_and  },  // OTAnd
	{ "||", &ca_exactly2, &pfi_or   },  // OTOr
	{ "?",  &ca_exactly3, &pfi_cond },  // OTCondQ,OTCondC 
	{ "gv", &ca_exactly3, &pfi_gv   },  // (generate vector; used by <>)
	{ ".",  &ca_exactly2, &pfi_dot  },  // OTDot
	// ":", ",", "(", ")" not needed as thrown away after parsing. 
	{ NULL, NULL, NULL }
};

// Non-const because of initialisation of the fdesc pointers: 
struct OperatorDesc _operators[_OTLast]=
{  // These operators are sorted by precedence. 
   // All operators with the same precedence must bind the 
   // same direction. 
	{ "[value]",   -1, 0,  0, NULL },  // OTValue [special precedence -1]
	{ "[function]",-1,-1,  0, NULL },  // OTFunction [special precedence -1]
	{ "(",         -1, 0,  0, NULL },  // OTBrOp [..]
	{ ")",         -1, 0,  0, NULL },  // OTBrCl [..]
	{ ".",         10, 2, +1, NULL },  // OTDot
	{ "!",          9, 1, -1, NULL },  // OTNot
	{ " +",         9, 1, -1, NULL },  // OTPos   // space in " +" is important for 
	{ " -",         9, 1, -1, NULL },  // OTNeg   // lookup in _InitOperators()
	{ "^",          8, 2, +1, NULL },  // OTPow
	{ "*",          7, 2, +1, NULL },  // OTMul
	{ "/",          7, 2, +1, NULL },  // OTDiv
	{ "+",          6, 2, +1, NULL },  // OTAdd
	{ "-",          6, 2, +1, NULL },  // OTSub
	{ "<=",         5, 2, +1, NULL },  // OTLE
	{ ">=",         5, 2, +1, NULL },  // OTGE
	{ "<",          5, 2, +1, NULL },  // OTL
	{ ">",          5, 2, +1, NULL },  // OTG
	{ "==",         4, 2, +1, NULL },  // OTEq
	{ "!=",         4, 2, +1, NULL },  // OTNEq
	{ "&&",         3, 2, +1, NULL },  // OTAnd
	{ "||",         2, 2, +1, NULL },  // OTOr
	{ "?",          1, 3,  0, NULL },  // OTCondQ  \ __ These need one precedence level 
	{ ":",          1, 2,  0, NULL },  // OTCondC  /    exclusively for them. 
	{ ",",          0, 0, +1, NULL }   // OTSep
};
const struct OperatorDesc *operators=_operators;

int _CountFuncs()
{
	int nfuncs;
	for(nfuncs=0; funcs[nfuncs].name; nfuncs++);
	assert(nfuncs>1);
	return(nfuncs);
}

//! Calculates and returns the max. precedence value. 
//! Internal use to set up array sizes. 
int _MaxPrecedenceValue()
{
	int maxp=0;
	for(int i=0; i<_OTLast; i++)
	{
		int tmp=operators[i].precedence;
		if(maxp<tmp)
		{  maxp=tmp;  }
	}
	return(maxp);
}

// Assign fdesc field in OperatorDesc: 
void _InitOperators()
{
	for(int otype=0; otype<_OTLast; otype++)
	{
		OperatorDesc *od=&_operators[otype];
		od->fdesc=LookupInternalFunction(od->name);
		// fdesc may be NULL for OTValue and those operators which get 
		// discarted after parsing. That are the ones with special 
		// precedence -1 or with 0 operands. 
		assert((od->precedence==-1 || od->noperands==0) || 
		       od->fdesc || otype==OTCondC);
	}
}

void _InitBindDir(int *bdarray,int sz)
{
	for(int i=-1; i<sz; i++)
	{  bdarray[i]=0;  }
	for(int otype=0; otype<_OTLast; otype++)
	{
		OperatorDesc *od=&_operators[otype];
		assert(od->precedence<sz && od->precedence>=-1);
		if(od->binddir!=bdarray[od->precedence])
		{
			// Remember: all operators with same precedence must have 
			// the same bind direction. 
			assert(!bdarray[od->precedence]);
			bdarray[od->precedence]=od->binddir;
		}
	}
}

void _OopsOTypeRange(OperatorType otype)
{
	std::cerr << "Oops: otype (" << int(otype) << ") out of range." << std::endl;
}

}  // end of `static' namespace


//! Looks up function using binary search and returns NULL 
//! if not found. 
//! Note: Binary search is about twice as fast as stupid linear 
//!       search when I look up all 40 functions and 5 unknown 
//!       ones. You don't gain too much. 
const FunctionDesc *LookupFunction(const char *name)
{
	if(!name)  return(NULL);
	
	const FunctionDesc *funcsend=funcs+n_functions;
	
	const FunctionDesc *start=funcs;
	const FunctionDesc *end=funcsend-1;
	
	// strcmp(a,b) calculates a-b. 
	
	// First, test for first and last element to make sure 
	// that the list range is not left for too small or too 
	// large names: 
	int sv=strcmp(name,start->name);
	if(sv<=0)  return(sv ? NULL : start);
	sv=strcmp(name,end->name);
	if(sv>=0)  return(sv ? NULL : end);
	
	++start;
	--end;
	
	// Now, do the binary search: 
	int delta=end-start;
	while(delta>2)  // binary search has no sense for <=2 elems
	{
		const FunctionDesc *middle=start+delta/2;
		sv=strcmp(name,middle->name);
		if(!sv)  return(middle);
		else if(sv<0)  // first half
		{  end=middle-1;  }
		else if(sv>0)  // second half
		{  start=middle+1;  }
		delta=end-start+1;  // +1 as end points to last elem itself
	}
	
	if(delta>0)
	{
		if(!strcmp(name,start->name))
		{  return(start);  }
		if(delta>1)
		{
			if(!strcmp(name,end->name))
			{  return(end);  }
		}
	}
	
	return(NULL);
}

//! Look up internal functions as used by operators. 
//! This is a linear search and only performed once. 
const FunctionDesc *LookupInternalFunction(const char *name)
{
	for(const FunctionDesc *fd=internal_funcs; fd->name; fd++)
	{
		if(!strcmp(name,fd->name))
		{  return(fd);  }
	}
	return(NULL);
}


//! Returns the operator description correspondig to the operator. 
//! This is a simple array lookup. 
const OperatorDesc *OperatorDescription(OperatorType otype)
{
	if(otype>=_OTLast || otype<0)
	{
		_OopsOTypeRange(otype);
		return(NULL);
	}
	return(&operators[otype]);
}


//! Actually call & compute a function pasing the arguments in 
//! FunctionArgs::args and storing the result under FunctionArgs::result 
//! if successful. 
//! YOU WILL NORMALLY use the interface class FunctionCaller (ilfcall.hpp). 
//! See FunctionState for possible return values. 
FunctionState CallFunction(FunctionArgs *fa)
{
	if(!fa->fdesc)  return(FSNoFunction);
	// Check if the function allows fa.nargs args. 
	{ int rv=(*fa->fdesc->check_nargs)(fa->nargs);
	if(rv)
	{  return((rv==2) ? FSTooManyArgs : FSWrongArgNo);  } }
	// Check if the result pointer is non-NULL
	if(!fa->result)  return(FSNoResult);
	// Really compute the function: 
	FunctionState rv=(*fa->fdesc->func)(fa);
	if(rv!=FSSuccess)
	{  fa->result->zap();  }  // make sure there's no result 
	return(rv);
}


// Number of functions. 
// This is actually const but I cannot make it const as it has 
// to be assigned one time. 
int n_functions=-1;
// Max. precedence value. 
// Actually const.. (see above)
int max_precedence_val=-1;
int cond_precedence_val=-1;
// Get bind direction from precedence value: 
int *binddir_of_precedence=NULL;

// Function description of the generate vector function as 
// used by <>. 
const FunctionDesc *gen_vec_fdesc=NULL;

//! This must be called upon startup. 
void InitData()
{
	if(binddir_of_precedence)  return;   // already initialized 
	
	fprintf(stderr,"Initializing exparse: ");
	_InitOperators();
	fprintf(stderr,"functions: ");
	n_functions=_CountFuncs();
	gen_vec_fdesc=LookupInternalFunction("gv");
	assert(gen_vec_fdesc);
	fprintf(stderr,"%d; precedence max: ",n_functions);
	max_precedence_val=_MaxPrecedenceValue();
	binddir_of_precedence=new int[max_precedence_val+2];
	++binddir_of_precedence;   // allow index [-1]
	_InitBindDir(binddir_of_precedence,max_precedence_val+1);
	fprintf(stderr,"%d; ",max_precedence_val);
	cond_precedence_val=OperatorDescription(OTCondC)->precedence;
	fprintf(stderr,"Done\n");
}

void CleanupData()
{
	if(binddir_of_precedence)
	{
		delete (binddir_of_precedence-1);
		binddir_of_precedence=NULL;
	}
}

}  // namespace end 



#if 0

namespace exparse
{

/* This is to see the speed relation, only: linear search */
const FunctionDesc *LinearLookupFunction(const char *name)
{
	for(const FunctionDesc *fd=funcs; fd->name; fd++)
	{
		if(!strcmp(name,fd->name))
		{  return(fd);  }
	}
	return(NULL);
}

}  // namespace end 

using namespace exparse;
int main(int argc,char **arg)
{
	for(int i=1; i<argc; i++)
	{
		const FunctionDesc *fd;
		for(int j=0; j<10000; j++)  // a little benchmark
			fd=LookupFunction(arg[i]);
		fprintf(stderr,"%s -> %s  (%d)\n",
			arg[i],fd ? "found" : "ERROR",nloop);
		if(fd && strcmp(arg[i],fd->name))
		{  fprintf(stderr,"OOPS!!\n");  }
	}
}

#endif
