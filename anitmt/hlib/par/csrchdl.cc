// Default (console) implementation of ParameterSource's 
// (virtual) error/warning handlers. 

#include "parsource.h"

namespace par
{

// Frequently needed string constant...
static const char *_three_qm="???";


int ParameterSource::OverrideError(ParamCopy * /*thiscopy*/,
	const ParamCopy * /*belowcopy*/,int operation,int rv)
{
	if(rv==-1)  return(0);  // allocation failure
	
	#warning waiting to get implemented. 
	fprintf(stderr,"Override error: %d (%d)\n",rv,operation);
	
	return(0);
}

#define vpe_size 9
static const struct vpe_info
{
	PAR::ParParseState pps;
	const char *str;
} _vpe_info[vpe_size]=
{
	// errors: 
	{ PAR::PPSSuccess,    "success" },
	{ PAR::PPSValOmitted, "required value omitted" },
	{ PAR::PPSIllegalArg, "argument badly formatted" },
	{ PAR::PPSValOORange, "value out of range" },
	{ PAR::PPSArgTooLong, "argument too long" },
	{ PAR::PPSOptAssign,  "assignment to _option_" },
	{ PAR::PPSIllegalAssMode, "assignment mode not supported" },
	{ PAR::PPSUnknown,    "unknown argument name" },
	// warnings: 
	{ PAR::PPSGarbageEnd, "garbage at end of argument" }
};

int ParameterSource::ValueParseError(
	ParParseState pps,
	const ParamArg *arg,
	ParamCopy *pc)
{
	// Look error up: 
	const vpe_info *vpe=NULL;
	
	// first try: use pps as index: 
	if(pps>=0 && pps<vpe_size)
	{
		if(_vpe_info[pps].pps==pps)
		{  vpe=&_vpe_info[pps];  }
	}
	// second try: look it up in list (last-to-first) 
	if(!vpe)  for(int idx=vpe_size; idx>=0; idx--)
	{
		if(_vpe_info[idx].pps==pps)
		{  vpe=&_vpe_info[idx];  break;  }
	}
	
	#warning waiting to get implemented. 
	fprintf(stderr,"ValueParseError: %s (%.*s; %s)\n",
		vpe ? vpe->str : _three_qm,
		int(arg->namelen),arg->name,pc->info->name);
	
	return(0);
}


int ParameterSource::WarnAlreadySet(
	const ParamArg *arg,
	ParamCopy *pc)
{
	#warning waiting to get implemented. 
	fprintf(stderr,"WarnAlreadySet: %s (%s,%d)\n",
		arg->name,pc->porigin.origin.str(),pc->porigin.opos);
	
	return(0);
}


int ParameterSource::ParamCopyError(
	CopyParamState cps,
	int cprv,
	ParamInfo * /*pi*/)
{
	#warning waiting to get implemented. 
	fprintf(stderr,"ParamCopyError: %d,%d\n",cps,cprv);
	
	return(0);
}


static const char *pet_error_str[ParameterSource::_PETLast]=
{
	"success",
	"unknown parameter",
	"parameter is not a switch"
};

int ParameterSource::ParameterError(
	ParameterErrorType pet,
	ParamArg *arg,
	ParamInfo * /*pi*/,
	Section * /*topsect*/)
{
	const char *pet_str = 
		(pet<0 || pet>=_PETLast) ? _three_qm : pet_error_str[pet];
	
	#warning waiting to get implemented. 
	fprintf(stderr,"ParameterError: %s: %.*s\n",
		pet_str,int(arg->namelen),arg->name);
	
	return(0);
}

}  // namespace end 
