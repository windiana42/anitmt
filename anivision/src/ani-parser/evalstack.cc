/*
 * ani-parser/evalstack.cc
 * 
 * Eval stack storing thread state on a stack for context switches. 
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

#include "evalstack.h"

//#include <ani-parser/exprvalue.h>

namespace ANI
{

void EvalStateStack::_stackpageoops(int n)
{
	fprintf(stderr,"OOPS: %d entries left in XStack frame.\n",n);
	abort();
}

void EvalStateStack::_stackemptyoops()
{
	fprintf(stderr,"OOPS: EvalStateStack empty at pop().\n");
	abort();
}

void EvalStateStack::_counteroops(int type,int n)
{
	fprintf(stderr,"OOPS: invalid counter value %d (type=%d)\n",n,type);
	assert(0);
}

void EvalStateStack::_illegalclone()
{
	fprintf(stderr,"OOPS: EvalStateStack: illegal void* clone.\n");
	abort();
}


void EvalStateStack::StartingContextSwitch()
{
	assert(is_completely_empty());
	++n_context_switches;
}


void EvalStateStack::DumpStackUsage(FILE *out)
{
	fprintf(out,
		"VM:   EvalStateStack usage:\n"
		"VM:     type: npages  nents  maxents  pushops\n"
		"VM:       S:  %6d  %5d  %7d  %7llu\n"
		"VM:       L:  %6d  %5d  %7d  %7llu\n"
		"VM:       TN: %6d  %5d  %7d  %7llu\n"
		"VM:       EV: %6d  %5d  %7d  %7llu\n"
		"VM:       EF: %6d  %5d  %7d  %7llu\n"
		"VM:       Ptr:%6d  %5d  %7d  %7llu\n",
		Sstack.npages, Sstack.nents, Sstack.maxents, (unsigned long long)Sstack.npush,
		Lstack.npages, Lstack.nents, Lstack.maxents, (unsigned long long)Lstack.npush,
		TNstack.npages,TNstack.nents,TNstack.maxents,(unsigned long long)TNstack.npush,
		EVstack.npages,EVstack.nents,EVstack.maxents,(unsigned long long)EVstack.npush,
		EFstack.npages,EFstack.nents,EFstack.maxents,(unsigned long long)EFstack.npush,
		Ptrstack.npages,Ptrstack.nents,Ptrstack.maxents,(unsigned long long)Ptrstack.npush);
}


EvalStateStack::EvalStateStack() : 
	Sstack(),
	Lstack(),
	TNstack(),
	EVstack(),
	EFstack(),
	Ptrstack()
{
	n_context_switches=0U;
}

EvalStateStack::EvalStateStack(_Clone,const EvalStateStack *src) : 
	Sstack(Clone,&src->Sstack),
	Lstack(Clone,&src->Lstack),
	TNstack(Clone,&src->TNstack),
	EVstack(Clone,&src->EVstack),
	EFstack(Clone,&src->EFstack),
	Ptrstack(Clone,&src->Ptrstack)
{
	n_context_switches=0U;
}

EvalStateStack::~EvalStateStack()
{
	// Dump some statistics: 
	DumpStackUsage(stdout);
	fprintf(stdout,
		"VM:     Number of context switches: %llu\n",
		(unsigned long long)n_context_switches);
}

}  // End of namespace ANI. 
