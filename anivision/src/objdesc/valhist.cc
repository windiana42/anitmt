/*
 * objdesc/valhist.cc
 * 
 * Value history class. This class can store previous values 
 * (each value being an n-dimensional vector) and allows to interpolate 
 * the value at given time in history. Uses efficient B-trees for 
 * storage. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "valhist.h"


namespace NUM
{


int ValueHistory::GetHistoryPoints(double t,
	const HIPoint **_p,const HIPoint **_n)
{
	HIPoint *p,*n;
	HIPoint *exact=store.FindNeighbours(t,&p,&n);
	if(exact)
	{
		if(_p) *_p=exact;
		if(_n) *_n=NULL;
		return(2);
	}
	if(_p) *_p=p;
	if(_n) *_n=n;
	if(p && n)  return(0);
	if(!p)  return(-1);
	if(!n)  return(+1);
	return(-2);
}


int ValueHistory::GetHistoryPoints(double t,double &tp,double &tn)
{
	const HIPoint *p,*n;
	int rv=GetHistoryPoints(t,&p,&n);
	tp = p ? p->t : NAN;
	tn = n ? n->t : NAN;
	return(rv);
}


int ValueHistory::GetPoint(double t,double *x)
{
	const HIPoint *p,*n;
	int rv=GetHistoryPoints(t,&p,&n);
	switch(rv)
	{
		case 0:
		{
			// Currently, only linear interpolation. 
			// FIXME: Add at least Akima interpolation. 
			double f=(t-p->t)/(n->t-p->t);
			double f1=1.0-f;
			for(int i=0; i<dim; i++)
				x[i] = n->x[i]*f + p->x[i]*f1;
		}	break;
		case -1:
		case +1:
			assert(!"extrapolation not yet implemented.");
			break;
		case 2:
			for(int i=0; i<dim; i++)  x[i]=p->x[i];
			break;
		case -2:
			for(int i=0; i<dim; i++)  x[i]=0.0;
			break;
	}
	return(rv);
}


void ValueHistory::_RemoveOldPoint()
{
	// This should be replaced by someting more clever. 
	if(!store.RemoveSmallest())
	{  --npoints;  }
}


int ValueHistory::Store(double t,double *x)
{
	tmp_hp->t=t;
	for(int i=0; i<dim; i++)  tmp_hp->x[i]=x[i];
	int rv=store.Store(*tmp_hp);
	switch(rv)
	{
		case 0:  ++npoints;  break;
		case 1:  break;   // element replaced
		default:  assert(0);
	}
	
	if(maxpoints && npoints>maxpoints)
	{
		// Time to remove some old point...
		_RemoveOldPoint();
		return(1);
	}
	return(0);
}


void ValueHistory::Clear()
{
	store.Clear();
	npoints=0;
}


ValueHistory::ValueHistory(int _dim,size_t _maxpoints) : 
	dim(_dim),npoints(0),maxpoints(_maxpoints),
	store(32,HLOP_HIPoint(dim))
{
	#warning "Using (more or less random) epsilon value here."
	store.OP.t_epsilon=1e-7;
	
	// Allocate temporary point: 
	tmp_hp=(HIPoint*)LMalloc(store.OP.size());
}

ValueHistory::~ValueHistory()
{
	// Nothing to do. 
	tmp_hp=(HIPoint*)LFree(tmp_hp);
}

}  // end of namespace NUM
