/*
 * core/anicontext.h
 * 
 * Animation context header. 
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

#ifndef _ANIVISION_CORE_ANIMATION_CONTEXT_H_
#define _ANIVISION_CORE_ANIMATION_CONTEXT_H_

#include <hlib/notifierlist.h>
#include <hlib/orderedarray.h>

#include <valtype/valtypes.h>


// For the animation context we need the exec thread 
// and the calc core interface: 
#include <core/execthread.h>

// For POV command parser: 
#include <pov-core/povparser.h>

#include <core/aniscope.h>

class AniSetting;

namespace CC
{
	class CCIF_Factory;
	class CCIF_Setting;
	class CCIF_Object;
	class CCIF_Value;
}

// Provided by <aniinstance.h>: 
class AniObjectInstance;


// How it works: 
// The complete animation is divided up into settings. 
// When creating the animation, the settings to be computed 
// are called from within the animation block 
// (i.e. "animation NAME { <ani_block> }") like a function 
// (e.g. "setting_name(t=1..3);"). While executing the 
// animation block and during static variable initialisation, 
// we're NOT in animation context (i.e. not animating 
// anything) but as soon as the setting is called from within 
// the animaiton block, we enter animation context. There is 
// never more than ONE ACTIVE animation context at any time. 
// The below structure represents the animation contexts; 
// usually there is one per setting; it is created when the 
// setting is first called. When leaving the setting, the 
// context can be destroyed or kept so that we can resume 
// in the setting several frames later in the animation. 
// (It is even possible to have two or more contexts 
// for a setting to save two or more states to be 
// resumed lateron.) In case no resume is desired, the 
// context should be deleted when leaving the setting. 
// A context always refers to exactly one AniSetting; it 
// may be given a name (to identify several contexts per 
// setting) and it holds all the dynamic execution info; 
// most notably a list of threads (with their stack). 
struct Animation_Context : LinkedListBase<Animation_Context>
{
	// Name of the context: 
	String name;
	
	// The associated setting. 
	AniSetting *setting;
	
	// The setting's calc core interface for this context. 
	// This is actually an array because every registered calc core 
	// has one separate CC::CCIF_Setting here. 
	CC::CCIF_Setting **ccif;
	
	// A list of threads: 
	// NOTE: When the animation context is switched, the 
	//       thread which is currently being executed at the time 
	//       ani context switch stays in Running mode so that 
	//       we can continue properly if the setting is resumed. 
	LinkedList<ExecThread> thread_list;
	
	// This is the special eval thread to be used by the POV output and 
	// function evaluations. (NOTE: It HAS an animation context! [=this one])
	// Note that this may be used recursively since the POV output may 
	// call a function which may in turn call a function again. 
	// One eval thread is allocated per Animation_Context; the destructor 
	// will destroy it if non-NULL. 
	ExecThread *eval_thread;
	
	// Attached POV object representation: 
	struct POVAttachmentNode : LinkedListBase<POVAttachmentNode>
	{
		// Prototype / POV definition: 
		POV::ObjectSpec *osdef;  // Managed by POV::CommandParser in AniAnimation. 
		// Read HOW IT WORKS in a_ifuncpov.cc. 
		// Cloned tree of the object entry (clone of osdef->ospec): 
		// Use this one instead of the one in ospec. 
		// NOTE: ospec is enqueued in the function body (compound 
		//       stmt) of the associated povhook in the main 
		//       tree; you can reach it using: 
		// ((AniObject*)AniObjectInstance::asb)->pov_attach_detach->
		//     GetTreeNode() [cast TNFuncionDef] -> func_body or 
		// using _GetPOVCompound(). 
		POV::PTNObjectSpec *ospec;
		
		_CPP_OPERATORS
		POVAttachmentNode(POV::ObjectSpec *_osdef);
		~POVAttachmentNode();
	};
	
	// One ObjectInstanceNode is allocated for each object instance 
	// (held by this animation context) which has at least one POV 
	// object attached. 
	struct ObjectInstanceNode : NotifyHandler<AniObjectInstance>
	{
		// Back pointer to context: 
		Animation_Context *context_back;
		
		// Pointer to the object instance. It is important that we 
		// do NOT use counted references here. 
		AniObjectInstance *obj_instance;
		
		// List of attached POV objects. 
		LinkedList<POVAttachmentNode> attach_list;
		
		ObjectInstanceNode(Animation_Context *_context,
			AniObjectInstance *_obj_instance);
		~ObjectInstanceNode();
		
		// Find attachment node by name of attached object: 
		POVAttachmentNode *FindAttachmentNodeByName(const RefString &objname);
		
		// Actually attach/detach POV object. 
		// Used by AniAnimation... look there. 
		int DoPOVAttach(POV::ObjectSpec *_ospec);
		int DoPOVDetach(Animation_Context::POVAttachmentNode *an);
		
		// Dump all attach info (i.e. attached objects). 
		// Useful for debugging. 
		void DumpAttachments(FILE *out);
		
		// [overriding virtual from NotifyHandler<AniObjectInstance>.]
		void handlenotify(int ntype,AniObjectInstance *obj_inst);
	};
	
	struct ObjectInstanceNode_OPs
	{
		// Internally used compare functions (used by OrderedArray<>): 
		static inline bool lt(const ObjectInstanceNode *a,
			const ObjectInstanceNode *b)
			{  return(a->obj_instance<b->obj_instance);  }
		static inline bool le(const ObjectInstanceNode *a,
			const ObjectInstanceNode *b)
			{  return(a->obj_instance<=b->obj_instance);  }
		static inline bool eq(const ObjectInstanceNode *a,
			const ObjectInstanceNode *b)
			{  return(a->obj_instance==b->obj_instance);  }
		static inline bool ne(const ObjectInstanceNode *a,
			const ObjectInstanceNode *b)
			{  return(a->obj_instance!=b->obj_instance);  }
		
		static inline bool lt(const AniObjectInstance *key,
			const ObjectInstanceNode *b)
			{  return(key<b->obj_instance);  }
		static inline bool lt(const ObjectInstanceNode *a,
			const AniObjectInstance *key)
			{  return(a->obj_instance<key);  }
		static inline bool eq(const AniObjectInstance *key,
			const ObjectInstanceNode *b)
			{  return(key==b->obj_instance);  }
		
		static inline ObjectInstanceNode * &ass(ObjectInstanceNode * &l,
			ObjectInstanceNode *const &r)  {  return(l=r);  }
		
		static const bool pdt=1;
		static inline size_t size() __attribute__((__const__))
			{  return(sizeof(ObjectInstanceNode *));  }
		static inline void ini(ObjectInstanceNode **) __attribute__((__const__)) {}
		static inline void ini(ObjectInstanceNode **p,
			ObjectInstanceNode *const &a)  {  *p=a;  }
		static inline void clr(ObjectInstanceNode **) __attribute__((__const__)) {}
		
		ObjectInstanceNode_OPs() {}
		~ObjectInstanceNode_OPs() {}
	};
	
	// A list of object instance nodes; each obj instance node belongs 
	// to an object instance which has at least one POV object attached. 
	// USE BELOW FUNCTIONS FOR ACCESS WHENEVER POSSIBLE. 
	OrderedArray<ObjectInstanceNode*,ObjectInstanceNode_OPs> pov_instlist;
	
	// Get the ObjectInstanceNode for the passed object. 
	// Allocate a new one if there is not yet one and alloc_new is set. 
	// Return index in ordered array in *ret_idx. 
	ObjectInstanceNode *_FindAllocObjectInstanceNode(
		AniObjectInstance *obj_instance,ssize_t *ret_idx,int alloc_new);
	
	// Check if the passed object instance node is still neded and 
	// delete it if not. Pass the index of the node instead of the 
	// node itself. 
	// Return value: 
	//  0 -> not deleted
	//  1 -> deleted
	int _CheckDeleteObjectInstanceNode(ssize_t idx);
	
	// Called by the ObjectInstanceNode when the object instance 
	// is being deleted. Must delete the node and all attachments. 
	void _ForceDeleteInstanceNode(ObjectInstanceNode *oin);
	
	// Lookup thread by its ID. Should not be necessary. 
	// (Note: thread IDs are only unique per context and may get 
	// re-used after some time.) 
	ExecThread *LookupThreadByID(int thread_id);
	
	_CPP_OPERATORS
	Animation_Context(const String &name,AniSetting *setting,
		int eval_thread_id);
	~Animation_Context();
};

#endif  /* _ANIVISION_CORE_ANIMATION_CONTEXT_H_ */
