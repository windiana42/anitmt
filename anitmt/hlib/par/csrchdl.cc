// Default (console) implementation of ParameterSource's 
// (virtual) error/warning handlers. 

#include "parsource.h"

namespace par
{

// Frequently needed string constant...
const char *_three_qm="???";


int ParameterSource::OverrideError(ParamCopy * /*thiscopy*/,
	const ParamCopy * /*belowcopy*/,int operation,int rv)
{
	if(rv==-1)  return(0);  // allocation failure
	
	#warning waiting to get implemented. 
	fprintf(stderr,"Override error: %d (%d)\n",rv,operation);
	
	return(0);
}

#define vpe_size 11
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
	{ PAR::PPSArgUnterminated, "argument unterminated" },
	{ PAR::PPSOptAssign,  "assignment to _option_" },
	{ PAR::PPSIllegalAssMode, "assignment mode not supported" },
	{ PAR::PPSAssFailed,  "value assignment failed" },
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
	
	RefString ostr=arg->origin.OriginStr();
	fprintf(stderr,"%s: %s%.*s in %s: %s\n",
		prg_name,pps<0 ? "warning: " : "",
		int(arg->namelen),arg->name,
		ostr.str(),
		vpe ? vpe->str : _three_qm);
	
	return(0);
}


int ParameterSource::WarnAlreadySet(
	const ParamArg *arg,
	ParamCopy *pc)
{
	RefString ostr=arg->origin.OriginStr();
	RefString pstr=pc->porigin.OriginStr();
	fprintf(stderr,"%s: warning: %.*s in %s previously set in %s\n",
		prg_name,
		int(arg->namelen),arg->name,
		ostr.str(),pstr.str());
	
	return(0);
}


#define cps_size 6
static const struct cps_info
{
	ParameterSource::CopyParamState cps;
	const char *str;
} _cps_info[cps_size]=
{
	{ ParameterSource::CPSAlreadyCopied,  "parameter already copied" },
	{ ParameterSource::CPSSuccess,        "success" },
	{ ParameterSource::CPSAllocFailed,    "allocation failure" },
	{ ParameterSource::CPSInvalidPI,      "invalid paramter info" },
	{ ParameterSource::CPSValAllocFailed, "value allocation failed" },
	{ ParameterSource::CPSCopyingFailed,  "copying failed" }
};


const char *_cprv_str(int cprv)
{
	static char tmp[24];
	switch(cprv)
	{
		case  0:  return("(success)");
		case -1:  return("(allocation failure)");
		case -2:  return("(operation not supported)");
		// default: fall through
	}
	snprintf(tmp,24,"(errcode %d)",cprv);
	return(tmp);
}


int ParameterSource::ParamCopyError(
	CopyParamState cps,
	int cprv,
	ParamInfo *pi)
{
	const char *err_str=_three_qm;
	
	// look error up in list (last-to-first) 
	for(int idx=0; idx<cps_size; idx++)
	{
		if(_cps_info[idx].cps==cps)
		{  err_str=_cps_info[idx].str;  break;  }
	}
	
	// Get full parameter name: 
	size_t len=parmanager()->FullParamName(pi,NULL,0);
	char tmp[len+1];
	parmanager()->FullParamName(pi,tmp,len+1);
	
	const char *add_str="";
	if(cps==ParameterSource::CPSCopyingFailed)
	{  add_str=_cprv_str(cprv);  }
	
	fprintf(stderr,"%s: copying parameter %s: %s%s\n",
		prg_name,tmp,err_str,add_str);
	
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
	
	RefString ostr=arg->origin.OriginStr();
	fprintf(stderr,"%s: %.*s in %s: %s\n",
		prg_name,
		int(arg->namelen),arg->name,
		ostr.str(),
		pet_str);
	
	return(0);
}

}  // namespace end 
