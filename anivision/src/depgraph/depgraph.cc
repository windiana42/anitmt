#include <hlib/prototypes.h>

#include "depgraph.h"
#include <assert.h>


void DepGraphNodeList::AddNode(DepGraphNode *dgn)
{
	if(!dgn)  return;
	
	for(Chunk *c=chunks.last(); c; c=c->prev)
	{
		if(c->nents>=ents_per_chunk)  continue;
		// There is a free entry in this chunk. 
		assert(!c->ent[c->nents]);
		c->ent[c->nents++]=dgn;
		++nnodes;
		return;
	}
	
	// We need a new chunk. Add it at the end. 
	Chunk *c=(Chunk*)CheckMalloc(LMalloc(sizeof(Chunk)));
	c->ent[0]=dgn;
	for(int i=1; i<ents_per_chunk; i++)
	{  c->ent[i]=NULL;  }
	c->nents=1;
	++nnodes;
	
	chunks.append(c);
	++nchunks;
}


int DepGraphNodeList::IsNodeInList(DepGraphNode *dgn)
{
	for(Chunk *c=chunks.last(); c; c=c->prev)
	{
		// We may not have empty chunks. 
		assert(c->nents);
		for(int i=0; i<c->nents; c++)
		{
			if(c->ent[i]==dgn)
			{  return(1);  }
		}
	}
	return(0);
}


inline void DepGraphNodeList::_TryMerge(Chunk *c,Chunk *n)
{
	if(!c || !n || n->nents+c->nents+2>ents_per_chunk)  return;
	for(int i=0; i<n->nents; i++)
	{  c->ent[c->nents++]=n->ent[i];  }
	chunks.dequeue(n);
	LFree(n);
	--nchunks;
}


int DepGraphNodeList::RemoveNode(DepGraphNode *dgn)
{
	for(Chunk *c=chunks.last(); c; c=c->prev)
	{
		// We may not have empty chunks. 
		assert(c->nents);
		for(int i=0; i<c->nents; c++)
		{
			if(c->ent[i]!=dgn)  continue;
			
			// We have it. 
			if(i!=(--c->nents))
			{  c->ent[i]=c->ent[c->nents];  }
			c->ent[c->nents]=NULL;
			--nnodes;
			
			// See if this chunk and the next one can be merged: 
			// (Leave 2 nodes reserved.)
			_TryMerge(c,c->next);
			// See if this chunk and prev chunk can be merged: 
			_TryMerge(c->prev,c);
			
			return(1);
		}
	}
	return(0);
}


DepGraphNodeList::DepGraphNodeList() : 
	chunks()
{
	nnodes=0;
	nchunks=0;
}

DepGraphNodeList::~DepGraphNodeList()
{
	while(!chunks.is_empty())
	{
		Chunk *c=chunks.popfirst();
		nnodes-=c->nents;
		--nchunks;
		LFree(c);
	}
	assert(!nchunks);
	assert(!nnodes);
}


/******************************************************************************/

int DepGraphNode::DependsOn(DepGraphNode *dn)
{
	if(provides.IsNodeInList(dn))  return(-2);
	if(depends.IsNodeInList(dn))  return(1);
	
	assert(!dn->provides.IsNodeInList(this));
	assert(!dn->depends.IsNodeInList(this));
	
	depends.AddNode(dn);
	dn->provides.AddNode(this);
	return(0);
}


int DepGraphNode::DepSolved(DepGraphNode *dgn)
{
	if(!depends.RemoveNode(dgn))  return(-2);
	assert(dgn->provides.IsNodeInList(this));
	assert(!solved.IsNodeInList(dgn));
	solved.AddNode(dgn);
	return(depends.NNodes());
}
