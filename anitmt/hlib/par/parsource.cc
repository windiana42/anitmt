#include "parsource.h"

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#include <assert.h>
#else
#define assert(x)
#endif

namespace par
{

// Used by derived classes to actually read in the parameter: 
PAR::ParamCopy *ParameterSource::CopyAndParseParam(ParamInfo *pi,ParamArg *pa)
{
	// This will copy the param call error handler if necessary 
	// and warn the user if it will be set more than once. 
	ParamCopy *pc=CopyParamCheck(pi,pa);
	if(!pc)
	{  return(NULL);  }
	
	// Parse the value: 
	ParParseState pps=pc->info->vhdl->parse(pi,pc->copyval,pa);
	// pps: >0 -> errors <0 -> warnings
	if(pps)  // >0 -> errors <0 -> warnings
	{  ValueParseError(pps,pa,pc);  }
	if(pps>0)
	{
		_RemoveParamCopy(pc);
		pc=NULL;
		return(NULL);
	}
	else  // <=0
	{
		++pc->nspec;
		// Set the origin...
		pc->porigin=pa->origin;    // (RefString gets referenced here.) 
	}
	return(pc);
}


int ParameterSource::Override(const ParameterSource *below)
{
	int errors=0;
	// For all the SectionNodes in *below, we search for the 
	// corresponding ones in our list: 
	for(SectionNode *bsn=below->snlist.first(); bsn; bsn=bsn->next)
	{
		if(bsn->pclist.is_empty())  continue;
		SectionNode *osn=_FindSectionNodeF(bsn->section);
		if(!osn)
		{
			// We don't have this section node? Time we create it: 
			osn=_AppendNewSectionNode(bsn->section);
			if(!osn)  // allocation failure 
			{  ++errors;  continue;  }
		}
		// Our SectionNode *osn corresponds to *bsn of other par source. 
		errors+=_Override(osn,bsn,below);
	}
	return(errors);
}

// Override call for SectionNode *osn corresponding to *below's 
// SectionNode *bsn. 
int ParameterSource::_Override(SectionNode *osn,SectionNode *bsn,
	const ParameterSource *below)
{
	int errors=0;
	for(ParamCopy *bpc=bsn->pclist.first(); bpc; bpc=bpc->next)
	{
		ParamCopy *opc=_FindParamCopyF(osn,bpc->info);
		// opc may be NULL here. 
		// Our ParamCopy *opc corresponds to *below's *bpc. 
		errors+=_Override(osn,opc,bsn,bpc,below);
	}
	return(errors);
}


// Override call for ParameterCopy *opc in SectionNode *osn 
// corresponding to *below's ParameterCopy *bpc in SectionNode *bsn. 
int ParameterSource::_Override(
	SectionNode *osn,ParamCopy *opc,
	SectionNode * /*bsn*/,ParamCopy *bpc,
	const ParameterSource * /*below*/)
{
	// Simply do it...
	if(!bpc->copyval)
	{
		#if TESTING
		fprintf(stderr,"Oops: bpc->copyval=NULL.\n");
		#endif
		return(1);
	}
	int is_new=0;
	if(!opc)
	{
		// We don't have this param but the other source has? 
		// Then, we must allocate a copy: 
		if(_AllocQueueParam(bpc->info,osn,&opc))
		{  return(1);  }
		is_new=1;
	}
	assert(opc->info==bpc->info);
	
	// Do the overriding: 
	ValueHandler *vhdl=opc->info->vhdl;
	#warning support multiple operations 
	int operation=SOPCopy;
	int reported=0;
	int rv=vhdl->copy(opc->info,opc->copyval,bpc->copyval,operation);
	int errors=0;
	if(rv)
	{  // Operation failed. 
		++errors;
		int oev=OverrideError(opc,bpc,operation,rv);
		reported=1;
		if(oev==1 && operation!=SOPCopy)  // try to simply copy it 
		{
			reported=0;
			rv=vhdl->copy(opc->info,opc->copyval,bpc->copyval);
		}
	}
	if(rv)
	{
		// Operation failed. 
		if(!reported)
		{
			++errors;
			OverrideError(opc,bpc,operation,rv);
			reported=1;
		}
		
		if(is_new)
		{  _FreeParamCopy(osn->pclist.dequeue(opc));  }
	}
	
	// Sum up: 
	opc->nspec+=bpc->nspec;
	
	return(errors);
}


#if TESTING
static const char *_ddsn_warn_blah="parsource.cc:%d: "
	"destroying SectionNode with params assigned to it!\n";
#endif

void ParameterSource::_DequeueDeleteSectionNode(SectionNode *sn)
{
	if(!sn)  return;
	#if TESTING
	if(!sn->pclist.is_empty())
	{  fprintf(stderr,_ddsn_warn_blah,__LINE__);  }
	#endif
	snlist.dequeue(sn);
	delete sn;
}


void ParameterSource::_DeleteSectionNodeParams(SectionNode *sn)
{
	for(;;)
	{
		ParamCopy *pc=sn->pclist.popfirst();  // pop = dequeue
		if(!pc)  break;
		_FreeParamCopy(pc);
	}
}


PAR::ParamCopy *ParameterSource::FindParamCopy(ParamInfo *pi)
{
	if(pi)
	{
		SectionNode *sn=_FindSectionNodeF(pi->section);
		if(sn)
		{  return(_FindParamCopyF(sn,pi));  }
	}
	return(NULL);
}


// Allocate a new ParamCopy and queue the copy at the end of the 
// passed SectionNode. 
// The new ParamCopy is returned in *ret if non-NULL. 
// Return values: 
//   0 -> OK
//  CPSAllocFailed,CPSValAllocFailed -> See CopyParam()
ParameterSource::CopyParamState ParameterSource::_AllocQueueParam(
	ParamInfo *pi,SectionNode * /*sn*/,ParamCopy **ret)
{
	if(ret)  *ret=NULL;
	
	// Alloc new parameter copy: 
	// I allocate the associated value from the same chunk 
	// if this is a value which does not need construction 
	// and may be copied via memcpy(): 
	size_t addsize=pi->vhdl->cpsize();
	int fflag=0;
	ParamCopy *pc=NEWplus<ParamCopy>(addsize,&fflag);
	if(fflag)
	{  if(pc) delete pc;  pc=NULL;  }
	if(!pc)  return(CPSAllocFailed);
	pc->info=pi;
	// Get memory for the parameter: 
	if(addsize)
	{  pc->copyval=(pc+1);  }
	else
	{
		pc->copyval=pi->vhdl->alloc(pi);
		if(!pc->copyval)
		{
			delete pc;  pc=NULL;
			return(CPSValAllocFailed);
		}
	}
	
	if(ret)  *ret=pc;
	return(CPSSuccess);
}


// Allocate a new ParamCopy, copy the parameter and queue the copy 
// at the end of the passed SectionNode. 
// The copied ParamCopy is returned in *ret if non-NULL. 
// Return values: 
//   0 -> OK
//  CPSAllocFailed,CPSValAllocFailed,CPSCopyingFailed -> See CopyParam()
// *cprv returns the return value of ValueHandler::copy() 
// if non-NULL. 
ParameterSource::CopyParamState ParameterSource::_CopyParam(
	ParamInfo *pi,SectionNode *sn,ParamCopy **ret,int *cprv)
{
	if(cprv) *cprv=0;
	if(ret)  *ret=NULL;
	
	ParamCopy *pc;
	CopyParamState rv=_AllocQueueParam(pi,sn,&pc);
	if(rv)  return(rv);
	
	// Copy the parameter: 
	int crv=pi->vhdl->copy(pi,pc->copyval,pi->valptr);
	if(cprv)  *cprv=crv;
	if(crv)
	{
		// Damn! Copying failed. 
		_FreeParamCopy(pc);  pc=NULL;
		return(CPSCopyingFailed);
	}
	
	// Queue this parameter copy: 
	sn->pclist.append(pc);
	
	if(ret)  *ret=pc;
	return(CPSSuccess);
}

// Return value: 
//  1 -> already copied 
//  0 -> OK
// -1 -> allocation failed
// -2 -> pi invalid (section or handler NULL)
// -3 -> failed to allocate value 
// -4 -> copying the value failed 
// *cprv returns the return value of ValueHandler::copy() 
// if non-NULL. 
ParameterSource::CopyParamState ParameterSource::CopyParam(
	ParamInfo *pi,ParamCopy **ret,int *cprv)
{
	if(cprv) *cprv=0;
	if(ret)  *ret=NULL;
	if(!pi)  return(CPSInvalidPI);
	if(!pi->section || !pi->vhdl)  return(CPSInvalidPI);
	
	SectionNode *sn=_FindSectionNodeL(pi->section);
	if(sn)
	{
		ParamCopy *pcp=_FindParamCopyL(sn,pi);
		if(pcp)
		{
			if(ret)  *ret=pcp;
			return(CPSAlreadyCopied);
		}
	}
	else
	{
		// Actually, the section node does not get deleted 
		// if something further down fails. This should not 
		// make any trouble. 
		sn=_AppendNewSectionNode(pi->section);
		if(!sn)
		{  return(CPSAllocFailed);  }
	}
	
	// sn!=NULL here. 
	return(_CopyParam(pi,sn,ret,cprv));
}

// This uses CopyParam() to copy the param, calls the 
// apropriate error function if an error occured and 
// calls the warning function if we're about to set 
// the parameter for the second, third,... time (for 
// this reason, the corresponding ParamArg is needed). 
// Returns NULL on failure. 
PAR::ParamCopy *ParameterSource::CopyParamCheck(
	ParamInfo *pi,const ParamArg *pa)
{
	ParamCopy *pc=NULL;
	int cprv;  // does not have to be set up to 0. 
	CopyParamState cps=CopyParam(pi,&pc,&cprv);
	if(cps<0)  // error
	{  ParamCopyError(cps,cprv,pi);  return(NULL);  }
	
	// cps=0 -> param copied; cps=CPSAlreadyCopied -> param was already copied. 
	// pc!=NULL in both cases. 
	assert(pc);
	
	if(cps==CPSAlreadyCopied && 
	   pa->assmode=='\0' &&  // '\0' -> `='
	   pc->porigin.otype!=ParamArg::_FromNowhere )
	{  WarnAlreadySet(pa,pc);  }
	
	return(pc);
}


// Write all parameters (set by this ParameterSource) 
// `Writing' means changing the prameter consumer's parameter value 
// (overwriting it with our value in the corresponding parameter 
// copy). 
// Return value: 
//  Number of failed parameter write operations. 
int ParameterSource::WriteParams()
{
	int nfailed=0;
	for(SectionNode *sn=snlist.first(); sn; sn=sn->next)
	{
		for(ParamCopy *pc=sn->pclist.first(); pc; pc=pc->next)
		{
			int rv=WriteParam(pc);
			if(rv)
			{  ++nfailed;  }
			//fprintf(stderr,"WP[%d]\n",rv);
		}
	}
	return(nfailed);
}


void ParameterSource::_FreeParamCopy(ParamCopy *pc)
{
	if(!pc)  return;
	if(pc->copyval)  // must free copy val...
	{
		ValueHandler *vhdl=pc->info->vhdl;
		if(!vhdl->cpsize())  //...only if allocated vis vhdl->alloc() 
		{  vhdl->free(pc->info,pc->copyval);  }
	}
	delete pc;
}

// sn: NULL -> search it; else -> no need to look it up (already provided) 
void ParameterSource::_RemoveParamCopy(ParamCopy *pc,SectionNode *sn=NULL)
{
	if(!pc)  return;
	
	// Find associated SectionNode (if not provided as arg): 
	if(!sn)
	{  sn=_FindSectionNodeF(pc->info->section);  }
	assert(sn->section==pc->info->section);
	if(sn)
	{  sn->pclist.dequeue(pc);  }
	#if TESTING
	else
	{  fprintf(stderr,"Strange: Could not file section node for "
		"section %p (%p) %s\n",pc->info->section,pc,
		pc->info->section ? pc->info->section->name : "<NULL>");  }
	#endif
	_FreeParamCopy(pc);
	// See if this SectionNode is still needed: 
	if(sn && sn->pclist.is_empty())
	{  _DequeueDeleteSectionNode(sn);  }
}


ParameterSource::SectionNode *ParameterSource::_FindSectionNodeF(Section *s)
{
	for(SectionNode *sn=snlist.first(); sn; sn=sn->next)
	{  if(sn->section==s)  return(sn);  }
	return(NULL);
}

ParameterSource::SectionNode *ParameterSource::_FindSectionNodeL(Section *s)
{
	for(SectionNode *sn=snlist.last(); sn; sn=sn->prev)
	{  if(sn->section==s)  return(sn);  }
	return(NULL);
}


ParameterSource::SectionNode *ParameterSource::_AppendNewSectionNode(
	Section *s)
{
	SectionNode *sn=NEW<SectionNode>();
	if(sn)
	{
		sn->section=s;
		snlist.append(sn);
	}
	return(sn);
}


PAR::ParamCopy *ParameterSource::_FindParamCopyF(
	SectionNode *sn,ParamInfo *pi)
{
	for(ParamCopy *pc=sn->pclist.first(); pc; pc=pc->next)
	{  if(pc->info==pi)  return(pc);  }
	return(NULL);
}

PAR::ParamCopy *ParameterSource::_FindParamCopyL(
	SectionNode *sn,ParamInfo *pi)
{
	for(ParamCopy *pc=sn->pclist.last(); pc; pc=pc->prev)
	{  if(pc->info==pi)  return(pc);  }
	return(NULL);
}


void ParameterSource::_ClearParamCopies()
{
	for(;;)
	{
		SectionNode *sn=snlist.first();
		if(!sn)  break;
		_DeleteSectionNodeParams(sn);
		_DequeueDeleteSectionNode(sn);
	}
}


// Called by the ParameterManager when the parameter pi in 
// section s is deleted so that the ParameterSource does no 
// longer reference it. 
// INTERNAL USE. 
void ParameterSource::ParamGetsDeleted(Section *s,ParamInfo *pi)
{
	SectionNode *sn=_FindSectionNodeL(s);
	if(!sn)  return;
	ParamCopy *pc=_FindParamCopyL(sn,pi);
	if(!pc)  return;
	
	#if TESTING
	fprintf(stderr,"Aiee: ParamInfo %s (section %s) gets "
		"deleted while used by source\n",
		pi->name,s->name);
	#endif
	
	_RemoveParamCopy(pc,sn);
}


ParameterSource::ParameterSource(ParameterManager *_manager,int *failflag) : 
	LinkedListBase<ParameterSource>(),
	snlist()
{
	int failed=0;
	#warning parsources should register at the manager so that the manager can tell them \
		when parameters get deleted. 
	manager=_manager;
	
	if(manager->RegisterParameterSource(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("ParSource");  }
}

ParameterSource::~ParameterSource()
{
	_ClearParamCopies();
	
	if(manager)
	{  manager->UnregisterParameterSource(this);  }
}

}  // namespace end 
