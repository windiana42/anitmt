/*
 * ani-parser/execstack.h
 * 
 * Execution stack definition: stack for AniVision language execution. 
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

#ifndef _ANIVISION_ANI_EXECSTACK_H_
#define _ANIVISION_ANI_EXECSTACK_H_ 1

#include <hlib/cplusplus.h>
#include <ani-parser/exprvalue.h>

namespace ANI
{

// This stack holds the "stack variables", i.e. automatic vars 
// and function call args as well as "this" pointers. 
// There is another stack for the state saving done when 
// switching threads, see EvalStateStack. 
class ExecStack
{
	public:
		struct Frame
		{
			// This is the actual stack: a bunch of values. 
			// val is an array of size nvals;
			int nvals;
			ExprValue *val;
			// "this" pointer or NULL ref: 
			ANI::ScopeInstance this_ptr;
			
			// Get value on stack: 
			// NOTE: This MUST return a reference. 
			inline ExprValue &GetValue(int aidx)
			{  if(aidx<0 || aidx>=nvals)  ExecStack::_s_f_a_failassert();
				return(val[aidx]);  }
			
			enum _Clone { Clone };
			
			// Construct frame with specified number of entries and passed 
			// "this" pointer entry: 
			Frame(int _nvals,const ANI::ScopeInstance &_this_ptr);
			// Clone stack frame: 
			Frame(_Clone,const Frame *src);
			inline ~Frame();
			_CPP_OPERATORS
		};
		
		enum _Clone { Clone };
		
	private:
		// The stack consists of several frames. 
		// This is an array of stack_size elements. 
		// Each element is a pointer to a stack frame. 
		// Only the first topframe+1 elements are valid; the 
		// top of the stack is the element [topframe]. 
		// The stack is dynamically reallocated. 
		int topframe;
		int stack_size;
		Frame **stack;
		
		// This is the maximum stack size in stack frames. 
		int max_frames;
		
		// Clear the complete stack: 
		void _ClearStack();
		
		// Stack _frame_ access assertion failure. 
		static Frame *_s_f_a_failassert();
		
		// Shrink stack if needed: 
		void _DoDownsize();  // Always use _DownsizeIfNeeded(). 
		void _DownsizeIfNeeded()
			{  if(stack_size-topframe>129) _DoDownsize();  }
	public: _CPP_OPERATORS
		// Create new (empty) stack: 
		ExecStack();
		// Clone existing stack: 
		ExecStack(_Clone,const ExecStack *src);
		~ExecStack();
		
		// Get the top frame: 
		// If there is none, assertion will fail
		Frame *TopFrame()
			{  return(topframe>=0 ? stack[topframe] : _s_f_a_failassert());  }
		
		// Get index of top frame, i.e. "stack depth". 
		int TopFrameIdx() const
			{  return(topframe);  }
		
		// Get the frame which is nframes below top. 
		// nframes=0 will result in TopFrame(). 
		// Too high values for nframes will result in 
		// failed assertion. 
		Frame *GetFrame(int nframes)
			{  return( (topframe>=nframes && nframes>=0) ? 
				stack[topframe-nframes] : _s_f_a_failassert());  }
		
		// Get ExprValue with addressation index aidx in the frame 
		// nframes below top. USE THIS INSTEAD OF TopFrame()/GetFrame(). 
		ExprValue &GetValue(int aidx,int nframes=0)
			{  return(GetFrame(nframes)->GetValue(aidx));  }
		
		// Get the "this" pointer on the topmost stack position: 
		const ScopeInstance &GetThisPtr()
			{  return(GetFrame(0)->this_ptr);  }
		
		// Allocate a new stack frame with the passed number 
		// of values. Pass the "this" pointer in this_ptr. 
		// Returns pointer to the newly created stack frame. 
		Frame *Allocate(int nvals,const ANI::ScopeInstance &this_ptr);
		
		// Free the nframes topmost stack frames and 
		// return the topmost one left (which can be NULL). 
		Frame *Free(int nframes=1);
		
		// You can also manually allocate a stack frame using 
		// new ExecStack::Frame(nvals,tptr) to store variables but 
		// without putting the frame on the stack (e.g. while 
		// evaluating function call args). You can then manually 
		// add the frame on the stack using: 
		void PushFrame(Frame *f);
		
		// Manually pop top frame without deallocating it. 
		Frame *PopFrame();
		
		// This is a very special function: It will allocate two 
		// stack frames with no variables and a specified this pointer. 
		// This is needed by special eval threads which do not need 
		// access to any variables on the stack. 
		// (The newly allocated frames will be on top of the stack.)
		// Use FreeEmptyFrames2() to pop the frames again. 
		void AllocateEmptyFrames2(const ScopeInstance &inst_tmp);
		void FreeEmptyFrames2();
};

}  // End of namespace ANI. 

#endif  /* _ANIVISION_ANI_EXECSTACK_H_ */
