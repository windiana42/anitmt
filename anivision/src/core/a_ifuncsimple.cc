/*
 * core/a_ifuncsimple.cc
 * 
 * Simple/basic internal functions (sin(),cos(),...). 
 * This is part of the AniVision project. 
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

#include "animation.h"

#include <ani-parser/treereg.h>
#include "execthread.h"

#warning "Seems to work... still fix all $$#'s."


using namespace ANI;

// These are the actual simple intenal "functions": 
// First the ones with exactly one arg: 

inline double deg2rag(double x)
	{  return(x*(M_PI/180.0));  }
inline double rad2deg(double x)
	{  return(x*(180.0/M_PI));  }

static void SIF_nop(ExprValue &retval,const ExprValue &arg0)
{
	retval.assign( arg0 );
}

// DOUBLE ARGUMENT, DOUBLE RETURN: 
#define TEMPLATE(name,cname) \
	static void SIF_ ## name(ExprValue &retval,const ExprValue &arg0) \
	{ \
		retval.assign( (double) cname(arg0.GetPODDouble()) ); \
	}
TEMPLATE(abs,    fabs)
TEMPLATE(acos,   acos)
TEMPLATE(acosh,  acosh)
TEMPLATE(asin,   asin)
TEMPLATE(asinh,  asinh)
TEMPLATE(atan,   atan)
TEMPLATE(atanh,  atanh)
TEMPLATE(cbrt,   cbrt)
TEMPLATE(ceil,   ceil)
TEMPLATE(cos,    cos)
TEMPLATE(cosh,   cosh)
TEMPLATE(deg2rad,deg2rag)
TEMPLATE(exp,    exp)
TEMPLATE(floor,  floor)
TEMPLATE(log,    log)
TEMPLATE(log10,  log10)
TEMPLATE(rad2deg,rad2deg)
TEMPLATE(round,  round)
TEMPLATE(sin,    sin)
TEMPLATE(sinh,   sinh)
TEMPLATE(sqrt,   sqrt)
TEMPLATE(tan,    tan)
TEMPLATE(tanh,   tanh)
TEMPLATE(trunc,  trunc)
#undef TEMPLATE

// DOUBLE ARGUMENT, INT RETURN: 
#define TEMPLATE(name,cname) \
	static void SIF_ ## name(ExprValue &retval,const ExprValue &arg0) \
	{ \
		retval.assign( (int) cname(arg0.GetPODDouble()) ); \
	}
TEMPLATE(isinf,  isinf)
TEMPLATE(isnan,  isnan)
TEMPLATE(finite, finite)
#undef TEMPLATE

static void SIF_abs_i(ExprValue &retval,const ExprValue &arg0)
{
	int val=arg0.GetPODInt();
	retval.assign(val<0 ? (-val) : val);
}

static void SIF_sgn(ExprValue &retval,const ExprValue &arg0)
{
	double val=arg0.GetPODDouble();
	retval.assign(val==0.0 ? 0 : (val<0.0 ? (-1) : 1));
}
static void SIF_sgn_i(ExprValue &retval,const ExprValue &arg0)
{
	int val=arg0.GetPODInt();
	retval.assign(val ? (val<0 ? (-1) : 1) : 0);
}

static void SIF_vlength(ExprValue &retval,const ExprValue &arg0)
{
	Value tmp;
	arg0.GetPOD(&tmp);
	const Vector *v=tmp.GetPtr<Vector>();
	retval.assign( (double) v->abs() );
}

static void SIF_vnormalize(ExprValue &retval,const ExprValue &arg0)
{
	Value tmp;
	arg0.GetPOD(&tmp);
	const Vector *v=tmp.GetPtr<Vector>();
	retval.assign( normalize(*v) );
}
static ExprValueType SIFRV_vnormalize(int nargs,const ExprValueType *arg_types)
{
	// nargs=-1: Special case for diagnostic messages/dumps. 
	if(nargs<0)
	{  return(ExprValueType::RVectorType(Vector::default_n));  }
	assert(nargs==1);
	const Value::CompleteType *podA=arg_types[0].PODType();
	if(!podA || podA->type!=Value::VTVector)
	{  return(ExprValueType::CNullRef);  }
	return(arg_types[0]);
}

static void SIF_puts(ExprValue &retval,const ExprValue &arg0)
{
	Value tmp;
	arg0.GetPOD(&tmp);
	FILE *out=stdout;
	fprintf(out,"\33[0;34m" "%s" "\33[00m",tmp.GetPtr<String>()->str());
	fflush(out);
	retval=ExprValue::CVoidType;
}
static void SIF_strof(ExprValue &retval,const ExprValue &arg0)
{
	// Heck... that's easy here :)
	retval.assign( arg0.ToString() );
}

//------------------------------------------------------------------------------

static void SIF_atan2(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	retval.assign( (double)atan2(
		args[0].GetPODDouble(),args[1].GetPODDouble()) );
}
static void SIF_hypot(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	retval.assign( (double)hypot(
		args[0].GetPODDouble(),args[1].GetPODDouble()) );
}
static void SIF_pow(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	retval.assign( (double)pow(
		args[0].GetPODDouble(),args[1].GetPODDouble()) );
}
static void SIF_pow_ii(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	int y=args[0].GetPODInt();
	int x=args[1].GetPODInt();
	int r = x>=0 ? 1 : 0;
	for(;x>0;x--)
	{  r*=y;  }
	retval.assign( r );
}
static void SIF_fmod(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	retval.assign( (double)fmod(
		args[0].GetPODDouble(),args[1].GetPODDouble()) );
}

static void SIF_vcross(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	
	Value tmpA,tmpB;
	args[0].GetPOD(&tmpA);
	const Vector *va=tmpA.GetPtr<Vector>();
	args[1].GetPOD(&tmpB);
	const Vector *vb=tmpB.GetPtr<Vector>();
	
	retval.assign( cross(*va,*vb) );
}

static void SIF_vdot(ExprValue &retval,int nargs,const ExprValue *args)
{
	assert(nargs==2);
	
	Value tmpA,tmpB;
	args[0].GetPOD(&tmpA);
	const Vector *va=tmpA.GetPtr<Vector>();
	args[1].GetPOD(&tmpB);
	const Vector *vb=tmpB.GetPtr<Vector>();
	
	retval.assign( (double)dot(*va,*vb) );
}
static ExprValueType SIFRV_vdot(int nargs,const ExprValueType *arg_types)
{
	// nargs=-1: Special case for diagnostic messages/dumps. 
	if(nargs>=0)
	{
		assert(nargs==2);
		const Value::CompleteType *podA=arg_types[0].PODType();
		const Value::CompleteType *podB=arg_types[1].PODType();
		if(!podA || !podB || 
		   podA->type!=Value::VTVector || podB->type!=Value::VTVector || 
		   podA->n!=podB->n)
		{  return(ExprValueType::CNullRef);  }
	}
	return(ExprValueType::RScalarType());
}

//------------------------------------------------------------------------------

static void SIF_rand01(ExprValue &retval)
{
	// Implement random number generator. 
	assert(0);
}

//==============================================================================

// "Register internal function, simple". 
// Version for no args: 
static void _RIF_S(AniAniSetObj *parent,
	const char *name,ExprValueType (*rettype_hdl)(int,const ExprValueType*),
	void (*ifptr0)(ExprValue &),
	int flags=0)
{
	RefString tmp;
	tmp.set(name);
	AniInternalFunc_Simple *ifs=new AniInternalFunc_Simple(
		tmp,parent,rettype_hdl,/*nargs=*/0,NULL);
	ifs->SetIFPtr(ifptr0,flags);
}
// Version for one arg: 
static void _RIF_S(AniAniSetObj *parent,
	const char *name,ExprValueType (*rettype_hdl)(int,const ExprValueType*),
	const char *argname0,const ExprValueType &argtype0,
	void (*ifptr1)(ExprValue &,const ExprValue &),
	int flags=0)
{
	RefString tmp;
	tmp.set(name);
	AniInternalFunc_Simple::AIFSFuncArgDesc fad(argname0,argtype0);
	AniInternalFunc_Simple *ifs=new AniInternalFunc_Simple(
		tmp,parent,rettype_hdl,/*nargs=*/1,&fad);
	ifs->SetIFPtr(ifptr1,flags);
}
// Version for two args: 
static void _RIF_S(AniAniSetObj *parent,
	const char *name,ExprValueType (*rettype_hdl)(int,const ExprValueType*),
	const char *argname0,const ExprValueType &argtype0,
	const char *argname1,const ExprValueType &argtype1,
	void (*ifptrN)(ANI::ExprValue &,int,const ANI::ExprValue *),
	int flags=0)
{
	RefString tmp;
	tmp.set(name);
	AniInternalFunc_Simple::AIFSFuncArgDesc fad[2];
	fad[0].aname=argname0;  fad[0].atype=argtype0;
	fad[1].aname=argname1;  fad[1].atype=argtype1;
	AniInternalFunc_Simple *ifs=new AniInternalFunc_Simple(
		tmp,parent,rettype_hdl,/*nargs=*/2,fad);
	ifs->SetIFPtr(ifptrN,flags);
}


// "Register internal function, special". 
// Version for no args: 
static void _RIF_SPC(AniAniSetObj *parent,
	const char *name,ExprValueType (*rettype_hdl)(int,const ExprValueType*),
	int flags)
{
	RefString tmp;
	tmp.set(name);
	AniInternalFunc_Simple *ifs=new AniInternalFunc_Simple(
		tmp,parent,rettype_hdl,/*nargs=*/0,NULL);
	ifs->_InternalSetFlags(flags);
}
// Version for one arg: 
static void _RIF_SPC(AniAniSetObj *parent,
	const char *name,ExprValueType (*rettype_hdl)(int,const ExprValueType*),
	const char *argname0,const ExprValueType &argtype0,
	int flags)
{
	RefString tmp;
	tmp.set(name);
	AniInternalFunc_Simple::AIFSFuncArgDesc fad(argname0,argtype0);
	AniInternalFunc_Simple *ifs=new AniInternalFunc_Simple(
		tmp,parent,rettype_hdl,/*nargs=*/1,&fad);
	ifs->_InternalSetFlags(flags);
}

static ExprValueType EVTof_Integer(int=-1,const ExprValueType* =NULL)
{
	return(ExprValueType::RIntegerType());
}
static ExprValueType EVTof_Scalar(int=-1,const ExprValueType* =NULL)
{
	return(ExprValueType::RScalarType());
}
static ExprValueType EVTof_Vector3(int=-1,const ExprValueType* =NULL)
{
	return(ExprValueType::RVectorType(3));
}
static ExprValueType EVTof_VectorAny()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTVector;
	tmp.n=-1;  // special
	return(ExprValueType(tmp));
}
static ExprValueType EVTof_String(int=-1,const ExprValueType* =NULL)
{
	return(ExprValueType::RStringType());
}
static ExprValueType EVTof_Void(int=-1,const ExprValueType* =NULL)
{
	return(ExprValueType::CVoidType);
}
static ExprValueType EVTof_Any()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTVector;  // yes, super-special...
	tmp.n=-99;  // special
	return(ExprValueType(tmp));
}

void AniInternalFunc_Simple::_RegisterInternalFunctions(AniAniSetObj *parent)
{
	if(parent->ASType()==AST_Animation)
	{
		//fprintf(stderr,"Registration of internal animation-level functions for %s\n",
		//  AniScopeTypeString(parent->ASType()));
		
		AniAniSetObj *p=parent;
		// "Register internal function, simple": 
		_RIF_S(p,"abs",&EVTof_Integer,"x",EVTof_Integer(),&SIF_abs_i);
		_RIF_S(p,"abs",&EVTof_Scalar, "x",EVTof_Scalar(), &SIF_abs);
		_RIF_S(p,"acos",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_acos);
		_RIF_S(p,"acosh",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_acosh);
		_RIF_S(p,"asin",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_asin);
		_RIF_S(p,"asinh",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_asinh);
		_RIF_S(p,"atan",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_atan);
		_RIF_S(p,"atanh",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_atanh);
		_RIF_S(p,"cbrt",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_cbrt);
		_RIF_S(p,"ceil",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_ceil);
		_RIF_S(p,"ceil",&EVTof_Integer,"x",EVTof_Integer(),&SIF_nop);
		_RIF_S(p,"cos",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_cos);
		_RIF_S(p,"cosh",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_cosh);
		_RIF_S(p,"deg2rad",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_deg2rad);
		_RIF_S(p,"exp",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_exp);
		_RIF_S(p,"finite",&EVTof_Integer,"x",EVTof_Scalar(),&SIF_finite);
		_RIF_S(p,"floor",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_floor);
		_RIF_S(p,"floor",&EVTof_Integer,"x",EVTof_Integer(),&SIF_nop);
		_RIF_S(p,"isinf",&EVTof_Integer,"x",EVTof_Scalar(),&SIF_isinf);
		_RIF_S(p,"isnan",&EVTof_Integer,"x",EVTof_Scalar(),&SIF_isnan);
		_RIF_S(p,"log",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_log);
		_RIF_S(p,"log10",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_log10);
		_RIF_S(p,"rad2deg",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_rad2deg);
		_RIF_S(p,"round",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_round);
		_RIF_S(p,"sgn",&EVTof_Integer,"x",EVTof_Scalar(),&SIF_sgn);
		_RIF_S(p,"sgn",&EVTof_Integer,"x",EVTof_Integer(),&SIF_sgn_i);
		_RIF_S(p,"sin",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_sin);
		_RIF_S(p,"sinh",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_sinh);
		_RIF_S(p,"sqrt",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_sqrt);
		_RIF_S(p,"tan",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_tan);
		_RIF_S(p,"tanh",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_tanh);
		_RIF_S(p,"trunc",&EVTof_Scalar,"x",EVTof_Scalar(),&SIF_trunc);
		
		_RIF_S(p,"puts",&EVTof_Void,"s",EVTof_String(),&SIF_puts,
			AniInternalFunc_Simple::IFF_MayNotTryIEval);
		_RIF_S(p,"strof",&EVTof_String,"x",EVTof_Any(),&SIF_strof);
		
		_RIF_S(p,"vlength",&EVTof_Scalar,"v",EVTof_VectorAny(),&SIF_vlength);
		
		_RIF_S(p,"atan2",&EVTof_Scalar,"y",EVTof_Scalar(),
			"x",EVTof_Scalar(),&SIF_atan2);
		_RIF_S(p,"hypot",&EVTof_Scalar,"x",EVTof_Scalar(),
			"y",EVTof_Scalar(),&SIF_hypot);
		_RIF_S(p,"pow",&EVTof_Scalar,"x",EVTof_Scalar(),
			"y",EVTof_Scalar(),&SIF_pow);
		_RIF_S(p,"pow",&EVTof_Integer,"x",EVTof_Integer(),
			"y",EVTof_Integer(),&SIF_pow_ii);
		_RIF_S(p,"fmod",&EVTof_Scalar,"x",EVTof_Scalar(),
			"y",EVTof_Scalar(),&SIF_fmod);
		
		_RIF_S(p,"vcross",&EVTof_Vector3,"x",EVTof_Vector3(),
			"y",EVTof_Vector3(),&SIF_vcross);
		
		_RIF_S(p,"vdot",&SIFRV_vdot,"a",EVTof_VectorAny(), // <-- ANY -> checked by SIFRV_vdot
			"b",EVTof_VectorAny(),&SIF_vdot);
		
		_RIF_S(p,"vnormalize",&SIFRV_vnormalize,"v",EVTof_VectorAny(), // <-- ANY -> checked by SIFRV_vdot
			&SIF_vnormalize);
		
		_RIF_S(p,"rand01",&EVTof_Scalar,&SIF_rand01,IFF_MayNotTryIEval);
		
		// Special functions: 
		_RIF_SPC(p,"clone",&EVTof_Integer,
			IFF_EvalFuncClone|IFF_MayNotTryIEval);
		_RIF_SPC(p,"yield",&EVTof_Integer,
			IFF_EvalFuncYield|IFF_MayNotTryIEval);
		_RIF_SPC(p,"delay",&EVTof_Integer,
			"dt",EVTof_Scalar(),
			IFF_EvalFuncDelay|IFF_MayNotTryIEval);
		
		_RIF_SPC(p,"kill",&EVTof_Void,IFF_EvalFuncKillS|IFF_MayNotTryIEval);
		
		_RIF_SPC(p,"abort",&EVTof_Void,IFF_EvalFuncAbort|IFF_MayNotTryIEval);
		
		//fprintf(stderr,"Registration done.\n");
	}
}


//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallSimpleInternal::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	assert(ifptrN);
	
	ExprValue val[function->nargs];
	int idx=0;
	TreeNode *i=tn->down.first()->next;
	TreeNode *stn=ess->popTN(i);
	for(; i!=stn; i=i->next,idx++)
	{  ess->popEV(&val[arg_perm[idx]]);  }
	ExprValue tmpval,*valp;
	for(; i; i=i->next,idx++)
	{
		valp = arg_assfunc[idx] ? &tmpval : &val[arg_perm[idx]];
		if(i->Eval(*valp,info))
		{
			ess->pushTN(i);  // <-- Do not move!
			for(--idx,i=i->prev; idx>=0; i=i->prev,idx--)
			{  ess->pushEV(val[arg_perm[idx]]);  }
			return(1);
		}
		if(!info && *valp==ExprValue::None)
		{  retval=ExprValue::None;  return(0);  }
		if(arg_assfunc[idx])
		{
			arg_assfunc[idx]->Assign(
				/*lvalue=*/val[arg_perm[idx]],
				/*rvalue=*/*valp);
		}
	}
	//assert(idx==function->nargs);  // can be left away
	
	// Actually call the function: 
	(*ifptrN)(retval,function->nargs,val);
	
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallSimpleInternal::~EvalFunc_FCallSimpleInternal()
{
	// Virtual destructor. 
	
	if(arg_assfunc)
	{
		for(ssize_t i=0; i<function->nargs; i++)
		{  DELETE(arg_assfunc[i]);  }
		arg_assfunc=(AssignmentFuncBase**)LFree(arg_assfunc);
	}
	
	arg_perm=(int*)LFree(arg_perm);
}


//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallSimpleInternal1A::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// This is simple. We have one arg, so context switches aren't 
	// a problem either. 
	ExprValue tmp;
	if(tn->down.first()->next->Eval(tmp,info))
	{  return(1);  }
	if(arg_assfunc)
	{
		ExprValue tmp2;
		arg_assfunc->Assign(/*lvalue=*/tmp2,/*rvalue=*/tmp);
		(*ifptr1)(retval,tmp2);
	}
	else
	{  (*ifptr1)(retval,tmp);  }
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallSimpleInternal1A::~EvalFunc_FCallSimpleInternal1A()
{
	// Virtual destructor. 
	DELETE(arg_assfunc);
}


//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallSimpleInternal0A::Eval(
	ExprValue &retval,TNOperatorFunction * /*tn*/,ExecThreadInfo * /*info*/)
{
	// This is very simple. 
	(*ifptr0)(retval);
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallSimpleInternal0A::~EvalFunc_FCallSimpleInternal0A()
{
	// Virtual destructor. 
}

//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallInternalClone::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Okay, so we have to clone. 
	// Tell the scheduler that we want to clone by setting the 
	// apropriate schedule_status and then do a "context switch" to 
	// "return to scheduler loop"  which will then clone all the 
	// data. When we come back, correctly treat the new schedule_status 
	// for parent and new (cloned) thread. 
	ExecThread *et=(ExecThread*)info;
	assert(et);
	switch(et->schedule_status)
	{
		case ExecThread::SSDisabled:
			Error(tn->GetLocationRange(),
				"cloning forbidden (no cloning outside animation context)\n");
			abort();
			return(0);
		case ExecThread::SSReady:
			et->schedule_status=ExecThread::SSRequest;
			return(1);  // <-- context switch
		case ExecThread::SSDoneClone:
			et->schedule_status=ExecThread::SSReady;
			retval.assign((int)0);
			return(0);
		default:
			int cth_id=et->schedule_status;  // clone thread_id
			assert(cth_id>0);
			et->schedule_status=ExecThread::SSReady;
			retval.assign((int)cth_id);
			return(0);
	}
	// Never reached. 
	assert(0);
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallInternalClone::~EvalFunc_FCallInternalClone()
{
	// Virtual destructor. 
}

//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallInternalYield::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Simple here; just trigger context switch. 
	
	if(info->ess->popS())
	{
		// Returning from context switch. 
		retval.assign((int)0);
		return(0);
	}
	
	ExecThread *et=(ExecThread*)info;
	assert(et);
	if(et->schedule_status==ExecThread::SSReady)
	{
		et->schedule_status=ExecThread::SSYield;
		info->ess->pushS(1);
		return(1);
	}
	else if(et->schedule_status==ExecThread::SSDisabled)
	{
		Warning(tn->GetLocationRange(),
			"ignoring yield() in single threaded (non-animation) context\n");
		retval.assign((int)0);
		return(0);
	}
	else assert(0);
	// Never reached. 
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallInternalYield::~EvalFunc_FCallInternalYield()
{
	// Virtual destructor. 
}

//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallInternalDelay::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	if(info->ess->popS())
	{
		// Returning from delay. 
		retval.assign((int)0);
		return(0);
	}
	
	// This is simple. We have one arg, so context switches aren't 
	// a problem either. 
	ExprValue tmp,delta_time;
	if(tn->down.first()->next->Eval(arg_assfunc ? tmp : delta_time,info))
	{  return(1);  }
	if(arg_assfunc)
	{  arg_assfunc->Assign(/*lvalue=*/delta_time,/*rvalue=*/tmp);  }
	
	ExecThread *et=(ExecThread*)info;
	assert(et);
	if(et->schedule_status==ExecThread::SSReady)
	{
		// Okay. see how long the delay should be: 
		double delay_len=delta_time.GetPODDouble();
		
		et->tdelay.StartDelay(delay_len);
		info->ess->pushS(1);
		return(1);
	}
	else if(et->schedule_status==ExecThread::SSDisabled)
	{
		Warning(tn->GetLocationRange(),
			"ignoring delay() in single threaded (non-animation) context\n");
		retval.assign((int)0);
		return(0);
	}
	else assert(0);
	// Never reached. 
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallInternalDelay::~EvalFunc_FCallInternalDelay()
{
	DELETE(arg_assfunc);
}

//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallInternalKillS::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	ExecThread *et=(ExecThread*)info;
	assert(et);
	switch(et->schedule_status)
	{
		case ExecThread::SSDisabled:
			Error(tn->GetLocationRange(),
				"self-kill()ing forbidden outside animation context\n");
			abort();
			return(0);
		case ExecThread::SSReady:
			// Kill outselves. 
			// As always, destruction is easy...
			et->schedule_status=ExecThread::SSKillMe;
			return(1);  // <-- context switch
		default: assert(0);
	}
	// Unreached. 
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallInternalKillS::~EvalFunc_FCallInternalKillS()
{
	// empty
}

//------------------------------------------------------------------------------

int AniInternalFunc_Simple::EvalFunc_FCallInternalAbort::Eval(
	ExprValue &retval,TNOperatorFunction *tn,ExecThreadInfo *info)
{
	Error(call_tn->down.first()->GetLocationRange(),
		"aborting due to user abort()\n");
	
	// Okay... then let's do it. 
	// This is useful for debugging and hence, I am not doing a 
	// exit() or _exit() here but a real abort(). 
	abort();
	
	// Unreached. 
	retval=ExprValue::CVoidType;
	return(0);
}

AniInternalFunc_Simple::EvalFunc_FCallInternalAbort::~EvalFunc_FCallInternalAbort()
{
	// empty
}

//------------------------------------------------------------------------------


void AniInternalFunc_Simple::SetIFPtr(
	void (*_ifptr0)(ANI::ExprValue &),
	int _flags)
{
	assert(!ifptr0);
	assert(nargs==0);
	ifptr0=_ifptr0;
	assert(!(flags & IFF_EvalFuncMASK));
	flags=_flags|IFF_EvalFunc0A;
}

void AniInternalFunc_Simple::SetIFPtr(
	void (*_ifptr1)(ANI::ExprValue &,const ANI::ExprValue &),
	int _flags)
{
	assert(!ifptr1);
	assert(nargs==1);
	ifptr1=_ifptr1;
	assert(!(flags & IFF_EvalFuncMASK));
	flags=_flags|IFF_EvalFunc1A;
}

void AniInternalFunc_Simple::SetIFPtr(
	void (*_ifptrN)(ANI::ExprValue &,int,const ANI::ExprValue *),
	int _flags)
{
	assert(!ifptrN);
	ifptrN=_ifptrN;
	flags=_flags;
	assert(!(flags & IFF_EvalFuncMASK));
}


// Special version of CanAssignType() for our "special extensions" 
// (see EVTof_VectorAny(),...)
// Retrurns NULL assignment function for the special cases. 
static int CanAssignType(const ExprValueType &lval,const ExprValueType &rval,
	AssignmentFuncBase **ret_assfunc)
{
	if(ret_assfunc)  *ret_assfunc=NULL;
	
	const Value::CompleteType *lpod=lval.PODType();
	if(lpod)
	{
		const Value::CompleteType *rpod=rval.PODType();
		if(lpod->type==Value::VTVector)
		{
			if(lpod->n==-1)
			{
				// Accept any vector. 
				if(!rpod)  return(0);
				if(rpod->type==Value::VTVector)  return(2);
				// Uaahh... well, could still cast matrix<1,n> to vector<n>,...
				return(0);
			}
			else if(lpod->n==-99)
			{
				// Super-special. Accept ANY TYPE. REALLY ANY. (I.e. even "void")
				return(2);
			}
			assert(lpod->n>=0);
		}
		if(rpod)
		{
			// Do some optimization: 
			// These common cases do not need any assignment function 
			// because no type conversion is performed. Hence, leave 
			// ret_assfunc=NULL. 
			if((lpod->type==Value::VTInteger && rpod->type==Value::VTInteger) || 
			   (lpod->type==Value::VTScalar && rpod->type==Value::VTScalar) || 
			   (lpod->type==Value::VTString && rpod->type==Value::VTString) )
			{  return(2);  }
			if(lpod->type==Value::VTVector && rpod->type==Value::VTVector && 
			   lpod->n==rpod->n)
			{  return(2);  }
		}
	}
	
	// Fallback: 
	return(lval.CanAssignType(rval,ret_assfunc));
}


int AniInternalFunc_Simple::Match_FunctionArguments(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	// This is similar to AniUserFunction's implementation. Hence: 
	//                         STOP. 
	// KEEP YOUR FINGERS OFF THIS FUNCTION UNLESS YOU REALLY 
	//               KNOW WHAT YOU ARE DOING. 
	
	// None of the named args in lfi should occure more than once. 
	// This is checked on higher level (because it would make no 
	// sense to check that for each function -- it has to be done 
	// once per lookup and not once per function). 
	
	// If the caller provides more args than we have, we cannot be 
	// called. If the caller provides less, we cannot be called, too, 
	// because the simple functions to not use default args. 
	if(nargs!=lfi->n_args)
	{  return(0);  }
	
	// Now, do the actual matching. 
	// This maps argument indices: 
	// We have arguments 0...nargs-1. For i in this range, 
	// argidx_map[i] is the index of the provided arg in *lfi 
	// which is to be passed as our argument number i. 
	int argidx_map[nargs];
	for(int i=0; i<nargs; i++)  argidx_map[i]=-1;
	
	// Loop through the passed args: 
	int our_arg_idx=0;
	int args_matching=0;     // args matching
	int args_matching_cv=0;  // args matching with type conversion
	for(int j=0; j<lfi->n_args; j++)
	{
		if(lfi->arg_name[j])
		{
			RefString name=lfi->arg_name[j]->CompleteStr();
			// Name specified. Look it up: (Could be optimized 
			// by first checking argdesc[our_arg_idx] & <dito>+1 if existing). 
			for(our_arg_idx=0; our_arg_idx<nargs; our_arg_idx++)
			{
				if(name==argdesc[our_arg_idx].aname)
				{  goto found;  }
			}
			// No argument named name.str(). 
			return(0);
			found:;
		}
		else if(our_arg_idx>=nargs)  // Don't run off arg list...
		{  return(0);  }
		
		// See if we can use lfi->arg_type[j] for our_arg. 
		assert(our_arg_idx>=0 && our_arg_idx<nargs);
		if(argidx_map[our_arg_idx]>=0)
		{
			// This arg was already assigned. 
			// This means, when the caller wants to call this function, 
			// he would have to assign the current arg more than once. 
			return(0);
		}
		int rv=CanAssignType(argdesc[our_arg_idx].atype,lfi->arg_type[j],NULL);
		if(rv==0)  // No, cannot. Type mismatch. 
		{  return(0);  }
		else if(rv==1)  ++args_matching_cv;
		else  ++args_matching;
		argidx_map[our_arg_idx]=j;
		
		// Step forward one arg in our list: 
		++our_arg_idx;
	}
	assert(args_matching_cv+args_matching==nargs);  // <= If there were default args
	
	// Okay, now... we must ask the return type handler if 
	// this argument combination is okay and what return value 
	// it will result in. 
	ExprValueType return_type;
	{
		ExprValueType arg_types[nargs];  // All args the function accepts. 
		for(int i=0; i<nargs; i++)
		{
			assert(argidx_map[i]>=0);  // <-- No default args allowed. 
			arg_types[i]=lfi->arg_type[argidx_map[i]];
		}
		assert(return_type_hdl);
		return_type=(*return_type_hdl)(nargs,arg_types);
		if(!return_type)  // NULL ref -> illegal combination
		{  return(0);  }
	}
	
	if(store_evalhandler)
	{
		// Store information for caller to be able to quickly reproduce 
		// what we just looked up: 
		if((flags & IFF_EvalFuncMASK)==IFF_EvalFunc0A)
		{
			assert(nargs==0);
			assert(ifptr1);
			EvalFunc_FCallSimpleInternal0A *evalfunc=
				new EvalFunc_FCallSimpleInternal0A(this,ifptr0);
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else if((flags & IFF_EvalFuncMASK)==IFF_EvalFunc1A)
		{
			assert(nargs==1);
			assert(ifptr1);
			EvalFunc_FCallSimpleInternal1A *evalfunc=
				new EvalFunc_FCallSimpleInternal1A(this,ifptr1);
			
			assert(argidx_map[0]==0);
			int rv=CanAssignType(argdesc[0].atype,lfi->arg_type[0],
				&evalfunc->arg_assfunc);
			assert(rv>0);  // Must be 1 or 2. 
			// NOTE: arg_assfunc can be NULL in case no special assignment 
			//       is needed. 
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else if((flags & IFF_EvalFuncMASK)==IFF_EvalFuncClone)
		{
			assert(nargs==0);
			
			EvalFunc_FCallInternalClone *evalfunc=
				new EvalFunc_FCallInternalClone();
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else if((flags & IFF_EvalFuncMASK)==IFF_EvalFuncYield)
		{
			assert(nargs==0);
			
			EvalFunc_FCallInternalYield *evalfunc=
				new EvalFunc_FCallInternalYield();
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else if((flags & IFF_EvalFuncMASK)==IFF_EvalFuncDelay)
		{
			assert(nargs==1);
			
			EvalFunc_FCallInternalDelay *evalfunc=
				new EvalFunc_FCallInternalDelay();
			
			int rv=CanAssignType(argdesc[0].atype,
				lfi->arg_type[0],&evalfunc->arg_assfunc);
			assert(rv>0);  // Must be 1 or 2. 
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else if((flags & IFF_EvalFuncMASK)==IFF_EvalFuncKillS)
		{
			assert(nargs==0);
			
			EvalFunc_FCallInternalKillS *evalfunc=
				new EvalFunc_FCallInternalKillS();
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else if((flags & IFF_EvalFuncMASK)==IFF_EvalFuncAbort)
		{
			assert(nargs==0);
			
			assert(lfi->opfunc);
			EvalFunc_FCallInternalAbort *evalfunc=
				new EvalFunc_FCallInternalAbort(lfi->opfunc);
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		else
		{
			assert(!(flags & IFF_EvalFuncMASK));
			assert(ifptrN);
			EvalFunc_FCallSimpleInternal *evalfunc=
				new EvalFunc_FCallSimpleInternal(this,ifptrN);
			
			// Permutation array: 
			// For the provided arg i, we need our arg number in 
			// range 0...nargs-1. 
			// Furthermore, we may need an offset for return value 
			// on the stack. 
			evalfunc->arg_perm=(int*)LMalloc(lfi->n_args*sizeof(int));
			// These are the argument assignment functions for those args 
			// which are specified (hence [0..nargs-1]): 
			evalfunc->arg_assfunc=(AssignmentFuncBase**)
				LMalloc(nargs*sizeof(AssignmentFuncBase*));
			
			for(int j=0; j<nargs; j++)
			{
				if(argidx_map[j]>=0)
				{
					assert(argidx_map[j]<lfi->n_args);
					evalfunc->arg_perm[argidx_map[j]]=j;
				}
				else assert(0);  // No default args allowed. 
			}
			// Walk the specified args: 
			for(int j=0; j<lfi->n_args; j++)
			{
				int rv=CanAssignType(argdesc[evalfunc->arg_perm[j]].atype,
					lfi->arg_type[j],&evalfunc->arg_assfunc[j]);
				assert(rv>0);  // Must be 1 or 2. 
			}
			
			// Store it: 
			lfi->evalfunc=evalfunc;
		}
		
		// May const fold/subexpr eval?
		lfi->may_try_ieval=(flags & IFF_MayNotTryIEval) ? 0 : 1;
		// Set return type computed above: 
		lfi->ret_evt=return_type;
	}
	
	// Okay, looks good. 
	assert(args_matching+args_matching_cv==nargs);
	return(args_matching_cv ? 1 : 2);
}


String AniInternalFunc_Simple::PrettyCompleteNameString() const
{
	String name((*return_type_hdl)(-1,NULL).TypeString().str());
	name+=" ";
	name+=CompleteName();
	
	String tmp;
	name+="(";
	for(int i=0; i<nargs; i++)
	{
		tmp.sprintf("%s %s%s",
			argdesc[i].aname,argdesc[i].atype.TypeString().str(),
			(i+1)<nargs ? "," : "");
		name+=tmp;
	}
	name+=")";
	
	return(name);
}


void AniInternalFunc_Simple::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	if(attrib & IS_Constructor)
	{  d->Append("[constructor]");  }
	else if(attrib & IS_Destructor)
	{  d->Append("[destructor]");  }
	else
	{  d->Append(name.str());  }
	if((attrib & IS_Constructor) && (attrib & IS_Destructor))
	{  assert(0);  }
	d->Append(":");
	if(attrib & IS_Static)
	{  d->Append(" static");  }
	if(!(attrib & IS_Constructor) && !(attrib & IS_Destructor))
	{
		d->Append(" ");
		ExprValueType return_type=(*return_type_hdl)(-1,NULL);
		d->Append(return_type.TypeString().str());
		if(return_type.IsIncomplete())
		{  d->Append(" [incomplete]");  }
	}
	if(attrib & IS_Const)
	{  d->Append(" const");  }
	
	// Dump args: 
	d->Append(" (");
	for(int i=0; i<nargs; i++)
	{
		do {
			const Value::CompleteType *pod=argdesc[i].atype.PODType();
			if(pod && pod->type==Value::VTVector)
			{
				if(pod->n==-99)
				{  d->Append("<any type>");  break;  }
				else if(pod->n==-1)
				{  d->Append("vector<*>");  break;  }
			}
			d->Append(argdesc[i].atype.TypeString().str());
		} while(0);
		d->Append(" ");
		d->Append(argdesc[i].aname);
		if((i+1)<nargs)  d->Append(",");
	}
	d->Append(")");
	
	d->Append("\n");
}


AniInternalFunc_Simple::AniInternalFunc_Simple(const RefString &_name,
	AniAniSetObj *_parent,
	ExprValueType (*_return_type_hdl)(int,const ExprValueType *),
	int _nargs,const AIFSFuncArgDesc *_args) : 
	AniInternalFunction(_name,_parent,IS_Static|IS_Const)
{
	nargs=_nargs;
	
	return_type_hdl=_return_type_hdl;
	
	if(nargs)
	{
		argdesc=new AIFSFuncArgDesc[nargs];
		for(int i=0; i<nargs; i++)
		{  argdesc[i]=_args[i];  }
	}
	else
	{  argdesc=NULL;  }
	
	flags=0;
	
	ifptr0=NULL;
	//ifptr1=NULL;  // is a...
	//ifptrN=NULL;  // ...union
}

AniInternalFunc_Simple::~AniInternalFunc_Simple()
{
	if(argdesc)
	{  delete[] argdesc;  argdesc=NULL;  }
}
