/*
 * ani-parser/execstack.cc
 * 
 * Execution stack for AniVision language. 
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

#include "execstack.h"

//#include <ani-parser/exprvalue.h>

namespace ANI
{

ExecStack::Frame::Frame(_Clone,const Frame *src) : 
	this_ptr(src->this_ptr)
{
	nvals=src->nvals;
	val=nvals ? new ExprValue[nvals] : NULL;
	for(int i=0; i<nvals; i++)
	{
		// Important: We just reference here; we do NOT assign(): 
		val[i]=src->val[i];
	}
}

ExecStack::Frame::Frame(int _nvals,const ScopeInstance &_this_ptr) : 
	this_ptr(_this_ptr)
{
	nvals=_nvals;
	val=nvals ? new ExprValue[nvals] : NULL;
}

inline ExecStack::Frame::~Frame()
{
	if(val)
	{
		delete[] val;
		val=NULL;
	}
	nvals=0;
}


/******************************************************************************/

void ExecStack::_ClearStack()
{
	while(topframe>=0)
	{  DELETE(stack[topframe--]);  }
	stack=(Frame**)LFree(stack);
	stack_size=0;
}


void ExecStack::PushFrame(ExecStack::Frame *f)
{
	if(++topframe>=stack_size)
	{
		// Reallocate stack frame pointer array: 
		stack_size=stack_size+32;
		if(stack_size>max_frames)
		{
			stack_size=max_frames;
			if(topframe>=stack_size)
			{
				fprintf(stderr,"OOPS: Stack overflow (frame %d)\n",topframe);
				abort();
			}
		}
		
		stack=(Frame**)LRealloc(stack,stack_size*sizeof(Frame*));
		
		assert(topframe<stack_size);  // NOT "<=". 
	}
	
	// Add stack frame ("push"): 
	stack[topframe]=f;
}


ExecStack::Frame *ExecStack::Allocate(int nv,const ScopeInstance &_this_ptr)
{
	assert(nv>=0);
	
	// Allocate & push new stack frame: 
	PushFrame(new Frame(nv,_this_ptr));
	
	//fprintf(stderr,"STACK ALLOC: now %d frames, top has %d vars.\n",
	//	topframe+1,stack[topframe]->nvals);
	
	return(stack[topframe]);
}


void ExecStack::_DoDownsize()
{
	//if(stack_size-topframe>129)  <-- checked by _DownsizeIfNeeded()
	//{
		stack_size=((topframe+1+31)/32+1)*32;
		assert(topframe<stack_size);
		stack=(Frame**)LRealloc(stack,stack_size*sizeof(Frame*));
	//}
	
	//fprintf(stderr,"STACK FREE(%d): now %d frames, top has %d vars.\n",
	//	nframes,topframe+1,topframe>=0 ? stack[topframe]->nvals : 0);
}


ExecStack::Frame *ExecStack::PopFrame()
{
	assert(topframe>=0);
	
	--topframe;
	
	_DownsizeIfNeeded();
	
	return(topframe>=0 ? stack[topframe] : NULL);
}


ExecStack::Frame *ExecStack::Free(int nframes)
{
	assert(nframes>=0);
	assert(nframes<=topframe+1);
	
	// Free the frames: 
	while(--nframes>=0)
	{  DELETE(stack[topframe--]);  }
	
	// Check if we make the stack frame pointer array smaller: 
	_DownsizeIfNeeded();
	
	return(topframe>=0 ? stack[topframe] : NULL);
}


void ExecStack::AllocateEmptyFrames2(const ScopeInstance &inst_tmp)
{
	// This could be improved if we have some empty pages lying around 
	// (which are static and hence shared between all exec stacks). 
	
	// Create stack page for function args: 
	Allocate(0,inst_tmp);
	// Create stack page for function compound: 
	Allocate(0,inst_tmp);
}

void ExecStack::FreeEmptyFrames2()
{
	Free(2);
}


ExecStack::Frame *ExecStack::_s_f_a_failassert()
{
	assert(0);
	return(NULL);
}


ExecStack::ExecStack(_Clone,const ExecStack *src)
{
	topframe=src->topframe;  // (integer)
	stack_size=src->stack_size;
	stack=(Frame**)LMalloc(stack_size*sizeof(Frame*));
	
	for(int i=0; i<=topframe; i++)
	{  stack[i]=new Frame(Frame::Clone,src->stack[i]);  }
	
	max_frames=src->max_frames;
}

ExecStack::ExecStack()
{
	topframe=-1;
	stack_size=0;
	stack=NULL;
	
	max_frames=4096;
}

ExecStack::~ExecStack()
{
	_ClearStack();
}

}  // End of namespace ANI. 
