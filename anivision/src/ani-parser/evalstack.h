/*
 * ani-parser/evalstack.h
 * 
 * Evaluation stack storing thread state for context switches. 
 * This is part of the AniVision project. 
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

#ifndef _ANIVISION_ANI_EVALSTATESTACK_H_
#define _ANIVISION_ANI_EVALSTATESTACK_H_ 1

#include <hlib/cplusplus.h>
#include <ani-parser/exprvalue.h>

// Sadly... needed for stack frame cloning. 
#include <ani-parser/execstack.h>


namespace ANI
{

// This stack holds the eval state (->TreeNode::Eval()) when 
// switching thread contexts. 
class EvalStateStack
{
	public:
		static void _stackpageoops(int n);
		static void _stackemptyoops();
		static void _counteroops(int type,int n);
		static void _illegalclone();
		
		enum _Clone { Clone };
		
		template<class T,int PSIZE=256>class XStack
		{
			public:
				// Clone functions for each of the types: 
				static inline void _DoClone(short int &dest,const short int &src)
					{  dest=src;  }
				static inline void _DoClone(ssize_t &dest,const ssize_t &src)
					{  dest=src;  }
				static inline void _DoClone(TreeNode* &dest,TreeNode *src)
					{  dest=src;  }
				static inline void _DoClone(ExprValue &dest,const ExprValue &src)
					{  dest=src;  }
				static inline void _DoClone(ExecStack::Frame* &dest,
					ExecStack::Frame *src)
					{  dest=new ExecStack::Frame(ExecStack::Frame::Clone,src);  }
				static inline void _DoClone(void* &dest,void *src)
					{  EvalStateStack::_illegalclone();  }
				
				// Clear functions (to avoid stray references):  
				static inline void _DoClear(short int &) {}
				static inline void _DoClear(ssize_t &) {}
				static inline void _DoClear(TreeNode* &) {}
				static inline void _DoClear(ExprValue &x)
					{  x=ExprValue::None;  }
				static inline void _DoClear(ExecStack::Frame* &) {}
				static inline void _DoClear(void* &x)  { x=NULL; }
				
				struct XPage : LinkedListBase<XPage>
				{
					int used;  // number of used values
					T vals[PSIZE];
					
					XPage *Clear(int *nents)
						{  *nents-=used;  used=0;  return(this);  }
					
					_CPP_OPERATORS;
					XPage()   {  used=0;  }
					XPage(_Clone,const XPage *src)
					{
						used=src->used;
						// Note: this does reference, not assign() ExprValues: 
						for(int i=0; i<used; i++)
						{  _DoClone(vals[i],src->vals[i]);  }
					}
					~XPage()  {  if(used) _stackpageoops(used);  }
				};
			private:
				LinkedList<XPage> stack;
				// top: must contain the top element (if any). 
				XPage *top;  // ...which does not have to be stack.last()
				             // if we keep an empty page around. 
			public:  // Public for easy access. 
				// Count number of pages: 
				int npages;
				// Total number of entries: 
				int nents;
				// Max number of entries in all pages together: 
				int maxents;
				// Number of push operations: 
				u_int64_t npush;
				
			private:
				void _Clear()
				{
					while(!stack.is_empty())
					{  delete stack.popfirst()->Clear(&nents);  --npages;  }
					top=NULL;
				}
				void _AdjustTopPop()
				{
 					_DoClear(top->vals[top->used]);
					if(!top->used && top->prev)
					{
						if(top->next)
						{  delete stack.poplast();  --npages;  }
						top=top->prev;
					}
				}
			public:
				// Create empty stack: 
				XStack() : stack()
					{  stack.append(top=new XPage());
						npages=1; nents=0; maxents=0; npush=0U;  }
				// Clone stack: 
				XStack(_Clone,const XStack<T,PSIZE> *src) : stack()
				{
					npages=src->npages; nents=src->nents;
					maxents=0; npush=0U;
					for(XPage *i=src->stack.first(); i; i=i->next)
					{
						XPage *xp=new XPage(Clone,i);
						stack.append(xp);
						if(i==src->top)  top=xp;
					}
				}
				~XStack()
					{
						_Clear();
						if(npages) _counteroops(1,npages);
						if(nents)  _counteroops(2,nents);
					}
				
				// Push value on stack: 
				void push(T v)
				{
					if(top->used>=PSIZE && !(top=top->next))
					{  stack.append(top=new XPage());  ++npages;  }
					top->vals[top->used++]=v;
					if((++nents)>maxents)  maxents=nents;  ++npush;
				}
				
				// Pop from stack or return passed value if empty. 
				T popD(T def)
				{
					if(!top->used)  return(def);  // stack empty
					T tmp=top->vals[--top->used];  --nents;
					_AdjustTopPop();
					return(tmp);
				}
				
				// Pop from stack and abort if empty. 
				void popA(T *save_here)
				{
					if(!top->used)  _stackemptyoops();  // stack empty
					*save_here=top->vals[--top->used];  --nents;
					_AdjustTopPop();
				}
				
				// Check if stack is empty. 
				bool is_empty() const
					{  return(!top->used);  }
		};
		
	private:
		// There are different stacks...
		XStack<short int,256> Sstack;
		XStack<ssize_t,64> Lstack;
		XStack<TreeNode*,192> TNstack;
		XStack<ExprValue,64> EVstack;
		XStack<ExecStack::Frame*,16> EFstack;
		XStack<void*,16> Ptrstack;
		
		// Count number of context switches: 
		u_int64_t n_context_switches;
		
	public: _CPP_OPERATORS
		// Create empty eval stack: 
		EvalStateStack();
		// Clone eval stack: 
		EvalStateStack(_Clone,const EvalStateStack *src);
		~EvalStateStack();
		
		//************************************************************
		//** NOTE: IMPORTANT: WE MUST BE ABLE TO _CLONE_ THE STACK. **
		//**       HENCE: ONLY PUSH CLONABLE VALUES.                **
		//** NEVER PUSH A STACK FRAME POINTER CASTED TO TreeNode*   **
		//**       OR SIMILAR WHICH WILL RESULT IN A HUGE POINTER   **
		//**       MESS AFTER CLONING.                              **
		//** USE pushPtr(), popPtr() to temporarily store pointers  **
		//** which may not be cloned; a clone attempt will then     **
		//** abort.                                                 **
		//************************************************************
		
		// Push/pop state: short integer state: 
		void pushS(short int st)
			{  Sstack.push(st);  }
		short int popS(short int def=0)
			{  return(this ? Sstack.popD(def) : def);  }
		
		// Push/pop state: ssize_t state: 
		void pushL(ssize_t v)
			{  Lstack.push(v);  }
		ssize_t popL(ssize_t def)
			{  return(this ? Lstack.popD(def) : def);  }
		
		// Push/pop state: TreeNode state. 
		void pushTN(TreeNode *tn)
			{  TNstack.push(tn);  }
		TreeNode *popTN(TreeNode *def)
			{  return(this ? TNstack.popD(def) : def);  }
		
		// Push/pop ExprValue...
		void pushEV(const ExprValue &ev)
			{  EVstack.push(ev);  }
		void popEV(ExprValue *store_here)
			{  if(!this) _stackemptyoops();  EVstack.popA(store_here);  }
		
		// Push/pop ExecStack::Frame...
		void pushEF(ExecStack::Frame *ef)
			{  EFstack.push(ef);  }
		ExecStack::Frame *popEF(ExecStack::Frame *def)
			{  return(this ? EFstack.popD(def) : def);  }
		
		// Push/pop arbitrary pointers which may not be cloned. 
		void pushPtr(void *ptr)
			{  Ptrstack.push(ptr);  }
		void *popPtr(void *def)
			{  return(this ? Ptrstack.popD(def) : def);  }
		
		// Check if all stacks are empty: 
		bool is_completely_empty() const
			{  return(Sstack.is_empty() && Lstack.is_empty() && 
				TNstack.is_empty() && EVstack.is_empty() && 
				EFstack.is_empty() && Ptrstack.is_empty() );  }
		// Call this when starting a context switch. 
		// Also checks that the stack is completely empty. 
		void StartingContextSwitch();
		
		// Dump stack usage info: 
		void DumpStackUsage(FILE *out);
};

}  // End of namespace ANI. 

#endif  /* _ANIVISION_ANI_EVALSTATESTACK_H_ */
