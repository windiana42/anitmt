#ifndef _INC_PAR_ParSource_H_
#define _INC_PAR_ParSource_H_ 1

#include "valhdl.h"
#include "parmanager.h"


namespace par
{

// A ParameterSource is a class derived from ParameterSource. 
// A ParameterSource reads in parameters (from files / command line) and 
// stores each of them temporarily in a parameter copy (if you specify a 
// param twice, they are both put into the same copy or overwrite the 
// previous value in the copy). 
// Say you have more than one ParameterSource (e.g. one for the config 
// file and one for the command line args). You allocate two 
// ParameterSources and let them read in their parameters. After that, 
// the tow sources have the parameter stored (they are not yet accsessible 
// by the parameter consumers). Now, you call the override function to 
// merge the parameters into one ParameterSource and the finally call 
// the write funtion to store the parameters at the parameter consumers 
// and thus make them available. 
// If you did all that, you should call CheckParams() at the parameter 
// manager to check if all required params are there, etc. 
class ParameterSource : 
	private LinkedListBase<ParameterSource>, 
	public PAR
{
	friend class ParameterManager;
	friend class LinkedList<ParameterSource>;
	public:
		enum CopyParamState
		{
			CPSAlreadyCopied=1,
			CPSSuccess=0,        // >0 -> no error; <0 -> error; 0 -> success
			CPSAllocFailed=-1,
			CPSInvalidPI=-2,
			CPSValAllocFailed=-3,
			CPSCopyingFailed=-4
		};
		enum ParameterErrorType
		{
			PETSuccess=0,
			PETUnknown,     // unknown parameter
			PETNotASwitch,  // parameter not a switch as it should be ("-no" prefix)
			_PETLast        // MUST BE LAST ONE
		};
	protected:
		struct SectionNode : public LinkedListBase<SectionNode>
		{
			Section *section;
			LinkedList<ParamCopy> pclist;
			
			_CPP_OPERATORS_FF
			SectionNode(int * /*failflag*/=NULL) : 
				LinkedListBase<SectionNode>(),pclist()  {  section=NULL;  }
			~SectionNode() { }
		};
		LinkedList<SectionNode> snlist;  // section node list
		
		// Search for section nodes: 
		SectionNode *_FindSectionNodeF(Section *s);  // begin with first node 
		SectionNode *_FindSectionNodeL(Section *s);  // begin with last node 
		// Find the ParamCopy of pi in the passed SectionNode: 
		ParamCopy *_FindParamCopyF(SectionNode *sn,ParamInfo *pi); // first->last
		ParamCopy *_FindParamCopyL(SectionNode *sn,ParamInfo *pi); // last->first
		
		// Alloc a section node and append it to the snlist. 
		SectionNode *_AppendNewSectionNode(Section *s);
		// Dequeue and delete a section node (NOT the assigned params!!) 
		inline void _DequeueDeleteSectionNode(SectionNode *sn);
		
		// Delete and dequeue all param copies in the passed section node: 
		void _DeleteSectionNodeParams(SectionNode *sn);
		// Only free()'d (deleted), NOT dequeued: 
		void _FreeParamCopy(ParamCopy *pc);
		// Dequeue and free the passed param copy: 
		// sn: NULL -> search it; else -> no need to look it up (already provided) 
		void _RemoveParamCopy(ParamCopy *pc,SectionNode *sn=NULL);
		
		// Allocate a new ParamCopy, copy the parameter and queue the copy 
		// at the end of the passed SectionNode. 
		// The copied ParamCopy is returned in *ret if non-NULL. 
		// Return values: 
		//   0 -> OK
		//  CPSAllocFailed,CPSValAllocFailed,CPSCopyingFailed -> See CopyParam()
		// *cprv returns the return value of ValueHandler::copy() 
		// if non-NULL. 
		CopyParamState _CopyParam(ParamInfo *pi,SectionNode *sn,
			ParamCopy **ret=NULL,int *cprv=NULL);
		// Allocate a new ParamCopy and queue the copy at the end of the 
		// passed SectionNode. 
		// The new ParamCopy is returned in *ret if non-NULL. 
		// Return values: 
		//   0 -> OK
		//  CPSAllocFailed,CPSValAllocFailed -> See CopyParam()
		CopyParamState _AllocQueueParam(ParamInfo *pi,SectionNode *sn,ParamCopy **ret);
		
		// Override call for SectionNode *osn corresponding to *below's 
		// SectionNode *bsn. 
		int _Override(SectionNode *osn,SectionNode *bsn,
			const ParameterSource *below);
		// Override call for ParameterCopy *opc in SectionNode *osn 
		// corresponding to *below's ParameterCopy *bpc in SectionNode *bsn. 
		// opc may be NULL, then it is created. 
		int _Override(SectionNode *osn,ParamCopy *opc,
			SectionNode *bsn,ParamCopy *bpc, const ParameterSource *below);
		
		// Clear all section nodes and param copies: 
		void _ClearParamCopies();
	
	protected:
		// Called when an error inside the Override() function occured. 
		// The error occured while trying to copy the value in *belowcopy 
		// to *thiscopy using the opertation `operation'. rv is the return 
		// value of the copy routine indicating the error. 
		// [ Copy routine: rv=thiscopy->info->vhdl->copy(thiscopy->info,
		//   thiscopy->copyval,belowcopy->copyval,operation); ]
		// Return value: 
		//  0 -> normally
		//  1 -> try to _copy_ (SOPCopy) the value instead 
		//       (only if operation!=SOPCopy) 
		// NOTE: rv=-1 occurs on allocation failure; it is probably best 
		//       to simply return in that case. 
		virtual int OverrideError(ParamCopy *thiscopy,
			const ParamCopy *belowcopy,int operation,int rv);
		
		// (Called by the parse functions of ParameterSource-derived 
		// classes.) 
		// Gets called if the value handler's parse function failed to 
		// parse in the value. 
		// pps -> contains the return value of the parse function. 
		//        NOTE: pps<0 -> warning only; pps>0 -> errors
		// arg -> the parameter arg and in arg->origin where it comes from 
		// pc ->  parameter copy (ps->copyval is where the parsed 
		//        result should be stored) 
		// Access pc->info to get the ParamInfo struct corresponding to 
		// the parameter and read pi->porigin to get the previous origin 
		// of the parameter (if any). 
		// Return value: ignored; use 0. 
		virtual int ValueParseError(
			ParParseState pps,
			const ParamArg *arg,
			ParamCopy *pc);
		
		// Informs you that copying the parameter (getting a local copy 
		// if it to store the parsed value(s) from this source) failed. 
		// See enum CopyParamState for a description on possible values. 
		// In case cps==CPSCopyingFailed, cprv tells you the value 
		// returned by ValueHandler::copy(). 
		// This is 0 -> success (will never happen here), 
		//        -1 -> allocation failure 
		//        -2 -> operation (!=PAR::SOPCopy) not supported
		//      else -> something else failed 
		// *pi is the parameter which shall be copied. 
		virtual int ParamCopyError(
			CopyParamState cps,
			int cprv,
			ParamInfo *pi);
		
		// This function is called before a parameter is attempted to 
		// be SET more than once in the same source (`+=' and `-=' 
		// assignments do not SET a parameter, they MODIFY it). 
		// This will usually give a warning or is silently ignored. 
		// The previous location of the parameter is stored in 
		// pc->porigin; the location of the argument can be accessed 
		// as arg->origin). 
		// (NOTE: pc->copyval is not yet parsed in.) 
		// Return value: ignored; use 0. 
		virtual int WarnAlreadySet(
			const ParamArg *arg,
			ParamCopy *pc);
		
		// This gets called for parameter errors. 
		// See enum ParameterErrorType above for possible errors. 
		// *arg stores the ParamArg which shall be looked up and parsed 
		//      and triggered the error. 
		// *pi  is the corresponding ParamInfo in case *arg was 
		//      recognized. 
		//      NOTE: For PETUnknown, pi is obviously NULL. 
		// *topsect is the top section pointer (never NULL). *arg must 
		//          be considered relative to *topsect. 
		// Return value: ignored; use 0. 
		virtual int ParameterError(
			ParameterErrorType pet,
			ParamArg *arg,
			ParamInfo *pi,
			Section *topsect);
		
		ParameterManager *manager;
	public: _CPP_OPERATORS_FF
		// Constructor automatically registers ParameterSource at 
		// ParameterManager. 
		ParameterSource(ParameterManager *manager,int *failflag=NULL);
		virtual ~ParameterSource();
		
		// Get pointer to the parameter manager: 
		ParameterManager *parmanager()
			{  return(manager);  }
		
		// Search a ParamInfo corresponding to a parameter name (with 
		// or without sections) at the ParameterManager: 
		// Returns NULL on failure. 
		ParamInfo *LookupParam(const char *name,Section *top=NULL)
			{  return(manager->FindParam(name,top));  }
		ParamInfo *LookupParam(const char *name,size_t namelen,Section *top=NULL)
			{  return(manager->FindParam(name,namelen,top));  }
		
		// Find a (locally allocated) ParamCopy for a specified ParamInfo. 
		// The ParamCopy is generated using e.g. CopyParam(). 
		// Returns NULL if not found. 
		ParamCopy *FindParamCopy(ParamInfo *pi);
		
		// Copy the specified ParamInfo and its value. A ParamInfo must 
		// be copied resulting in a ParamCopy in order to be modified 
		// (to store parsed in value, etc.). 
		// The copied ParamCopy is returned in *ret if non-NULL. 
		// Return value: 
		//  1 -> already copied (CPSAlreadyCopied) 
		//  0 -> OK (CPSSuccess)
		// -1 -> allocation failed (CPSAllocFailed)
		// -2 -> pi invalid (section or handler NULL) (CPSInvalidPI)
		// -3 -> failed to allocate value (CPSValAllocFailed)
		// -4 -> copying the value failed (CPSCopyingFailed)
		// *cprv returns the return value of ValueHandler::copy() 
		// if non-NULL. 
		CopyParamState CopyParam(ParamInfo *pi,ParamCopy **ret=NULL,
			int *cprv=NULL);
		
		// This uses CopyParam() to copy the param, calls the 
		// apropriate error function if an error occured and 
		// calls the warning function if we're about to set 
		// the parameter for the second, third,... time (for 
		// this reason, the corresponding ParamArg is needed). 
		// Returns NULL on failure. 
		ParamCopy *CopyParamCheck(ParamInfo *pi,const ParamArg *pa);
		
		// The parameters in *this overrdide those in *below. 
		// *below is not changed! 
		// The parameters which are set in *below but not in *this, are 
		// copied to *this. Parameters which are to be appended, are 
		// appended to *this. 
		// Return value: 0 -> OK ; >0 -> errors
		// See also OverrideError(). 
		int Override(const ParameterSource *below);
		
		// Finally write a single parameter back to the comsumer 
		// (changing the consumer's value). 
		// See ParameterManager::WriteParam() for details. 
		// To write all parameters, use WriteParams().
		int WriteParam(ParamCopy *pc)
			{  return(manager->WriteParam(pc));  }
		
		// Write all parameters (set by this ParameterSource) 
		// `Writing' means changing the prameter consumer's parameter value 
		// (overwriting it with our value in the corresponding parameter 
		// copy). 
		// Return value: 
		//  Number of failed parameter write operations. 
		int WriteParams();
		
		
		// Called by the ParameterManager when the parameter pi in 
		// section s is deleted so that the ParameterSource does no 
		// longer reference it. 
		// INTERNAL USE. 
		void ParamGetsDeleted(Section *s,ParamInfo *pi);
};

}  // namespace end 

#endif  /* _INC_PAR_ParManager_H_ */
