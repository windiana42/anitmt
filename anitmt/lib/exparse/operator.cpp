/*
 * operator.cpp
 * 
 * Abstraction of operator tree node in expression tree 
 * for input lib / exparse. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "operator.hpp"

#include <stdio.h>

namespace exparse
{

inline void Operator::_zap_val()
{
	if(otype!=OTValue)  return;
	if(free_val && val)
	{  delete val;  }
	val=NULL;
	free_val=0;
}


int Operator::attach(Value *_val)
{
	if(otype!=OTValue)
	{  return(1);  }
	_zap_val();
	if(_val)
	{
		val=_val;
		free_val=1;
	}
	return(0);
}


int Operator::assign(Value *_val)
{
	if(otype!=OTValue)
	{  return(1);  }
	_zap_val();
	val=_val;
	return(0);
}


Value *Operator::detach()
{
	if(otype!=OTValue || 
	   !free_val)  // no value attached
	{  return(NULL);  }
	Value *rv=val;
	val=NULL;
	free_val=0;
	return(rv);
}


// Special function, called during expression tree optimization. 
void Operator::AttachSubtreeResult(Value *v)
{
	if(otype!=OTValue)  // ...which will always be the case here
	{
		// Get rid of down pointers (the caller must have deleted 
		// these operators). 
		if(down)  delete[] down;  down=NULL;
		nchld=0;
		otype=OTValue;
		val=NULL;    // overwrites fdesc with NULL
		free_val=0;  // ..since there is none
	}
	attach(v);
}


inline void Operator::_SetType(OperatorType type)
{
	otype=type;
	odesc=OperatorDescription(otype);
	assert(odesc);
	if(otype!=OTValue)
	{  fdesc=odesc->fdesc;  }  // NULL for OTValue 
}

int Operator::SetType(OperatorType type)
{
	if(otype==_OTLast)
	{
		_SetType(type);
		return(0);
	}
	return(type==otype);
}

int Operator::ForceSetType(OperatorType type)
{
	if(type==otype)  return(0);
	if(otype==OTValue)
	{  if(val)  return(1);  }
	else
	{  if(nchld)  return(1);  }
	
	_SetType(type);
	return(0);
}


int Operator::SetFunction(const FunctionDesc *_fdesc)
{
	if(otype!=OTFunction)  return(1);
	if(fdesc)  return(1);  // other function already set
	fdesc=_fdesc;  // MAY NEVER MODIFY odesc->fdesc. 
	return(0);
}


int Operator::AddChild(Operator *child)
{
	if(otype==OTValue)  return(-1);
	if(otype>=_OTLast)  return(-2);
	if(!fdesc)  return(-3);
	if(otype==OTFunction)
	{
		const int alloc_thresh=8;   // alloc ahead 
		int new_nchld=nchld+1;
		if((*fdesc->check_nargs)(new_nchld)==2)  // too many
		{  return(1);  }
		if(!(nchld%alloc_thresh))   // got to alloc some down pointers
		{
			#warning should use mcache here: 
			Operator **new_down = new Operator*[nchld+alloc_thresh];
			if(nchld)
			{  memcpy(new_down,down,nchld*sizeof(Operator*));  }
			delete[] down;
			down=new_down;
		}
		down[nchld++]=child;
		child->up=this;
		return(0);
	}
	
	// Assign to next free down pointer: 
	if(nchld>=odesc->noperands)
	{  return(1);  }
	down[nchld++]=child;
	child->up=this;
	return(0);
}


int Operator::CountChildren()
{
	if(otype==OTValue || otype==_OTLast)
	{  return(0);  }
	int nc=nchld;
	for(int i=0; i<nchld; i++)
	{  nc+=down[i]->CountChildren();  }
	return(nc);
}


Operator::Operator()
{
	up=NULL;
	down=NULL;
	otype=_OTLast;
	odesc=NULL;
	val=NULL;   // sets fdesc=NULL, too (union) 
	free_val=0;  // sets nchld=0, too (union) 
}

Operator::Operator(OperatorType ot)
{
	up=NULL;
	down=NULL;
	otype=ot;
	odesc=OperatorDescription(otype);
	assert(odesc);
	if(otype==OTValue)
	{
		val=NULL;
		free_val=0;
	}
	else
	{
		fdesc=odesc->fdesc;
		nchld=0;
	}
	
	if(odesc->noperands>0)   // NOTE: is -1 for OTFunction. 
	{
		down=new Operator*[odesc->noperands];
		for(int i=0; i<odesc->noperands; i++)
		{  down[i]=NULL;  }
	}
}

Operator::Operator(Value *_val,bool attach_val)
{
	up=NULL;
	down=NULL;
	otype=OTValue;
	val=_val;
	free_val = attach_val ? 1 : 0;
	odesc=OperatorDescription(otype);
	assert(odesc);
}

Operator::Operator(const FunctionDesc *_fdesc)
{
	up=NULL;
	down=NULL;
	otype=OTFunction;
	odesc=OperatorDescription(otype);
	assert(odesc);
	fdesc=_fdesc;
	nchld=0;
}

Operator::~Operator()
{
	_zap_val();
	up=NULL;
	if(down)  delete[] down;  down=NULL;
	otype=_OTLast;
}

}  // namespace end 
