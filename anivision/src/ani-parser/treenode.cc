/*
 * ani-parser/treenode.cc
 * 
 * Basic TreeNode functions (alloc/dealloc/head list...). 
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

#include "ani_scanner.h"
//#include "treenode.h"

#include <stdio.h>
#include <assert.h>


namespace ANI
{

/* Node allocation and deallocation. */
// There is a list of all TreeNode heads for clean de-alloc. 
// (Older nodes at the beginning.) 
LinkedList<TreeNode> TreeNode::head_nodes;

int TreeNode::n_alloc_tree_nodes=0;
size_t TreeNode::tree_nodes_tot_size=0;


void *TreeNode::operator new(size_t s)
{
	TreeNode *tn=(TreeNode*)::LMalloc(s);
	if(tn)
	{  head_nodes.append(tn);  }
	else
	{  CheckMalloc(NULL);  }
	++n_alloc_tree_nodes;
	tree_nodes_tot_size+=s;
	return(tn);
}

void TreeNode::operator delete(void *ptr,size_t s)
{
	if(!ptr)  return;
	TreeNode *tn=(TreeNode*)ptr;
	// Remove from head_nodes if needed. 
	if(!tn->up)
	{
		assert(head_nodes.find(tn));
		head_nodes.dequeue(tn);
	}
	tn->up=NULL;
	::LFree(ptr);
	--n_alloc_tree_nodes;
	assert(tree_nodes_tot_size>=s);
	tree_nodes_tot_size-=s;
}

void TreeNode::_check_stack_error()
{
	#if 0
	int loops=0;
	for(TreeNode *i=head_nodes.last(); i; i=i->prev,loops++)
	{
		if(i!=this)  continue;
		fprintf(stderr,"      check (%d) loops=%d\n",ntype,loops);
		return;
	}
	#endif
	
	assert(!head_nodes.find(this));
	fprintf(stderr,"TreeNode %d not allocated via operator new.\n",ntype);
	abort();
}


void TreeNode::RemoveFromHeadNodes(TreeNode *tn)
{
	assert(!tn->up);  // May not be in head nodes list otherwise. 
	for(TreeNode *i=head_nodes.last(); i; i=i->prev)
	{
		if(i!=tn)  continue;
		head_nodes.dequeue(tn);  // tn==i here. 
		return;
	}
	assert(0);  // OOPS: Passed node not in head nodes list. 
}


void TreeNode::AppendToHeadNodes(TreeNode *tn)
{
	assert(!tn->up);  // May not be in head nodes list otherwise. 
	assert(!head_nodes.find(tn));
	head_nodes.append(tn);
}


void TreeNode::FreeAllHeadNodes(TreeNode *except)
{
	//fprintf(stderr,"FreeAllHeadNodes called!\n");
	
	for(TreeNode *_i=head_nodes.last(); _i; )
	{
		TreeNode *i=_i;
		_i=_i->prev;
		
		if(i==except)  continue;
		assert(!i->up);
		
		delete i;   // Will dequeue i from head_nodes. 
	}
	
	if(except)
	{
		assert(head_nodes.first()==except && head_nodes.last()==except);
		head_nodes.dequeue(except);
	}
	
	assert(head_nodes.is_empty());
}


void TreeNode::DeleteTree()
{
	if(!this)  return;
	
	if(up)
	{  up->RemoveChild(this);  }
	else
	{
		// This is actually a head node. 
		// Add it to the head node list if it is not there. 
		if(!this->prev && !this->next && head_nodes.first()!=this)
		{
			assert(!head_nodes.find(this));
			head_nodes.append(this);
		}
		else
		{  assert(head_nodes.find_rev(this));  }
	}
	
	delete this;
}


void TreeNode::ReplaceBy(TreeNode *new_node)
{
	TreeNode *pr=parent();
	assert(pr);  // May never fail!
	
	// Queue at correct position in the parent: 
	for(TreeNode *i=pr->down.first(); i; i=i->next)
	{
		if(i!=this)  continue;
		// I purposefully do NOT use queueafter() here. 
		// Most expressions are evaluated first-...->last 
		// and we should not re-eval it again and again...
		pr->InsertChildBefore(new_node,i);
		delete pr->RemoveChild(i);  // this==i 
		goto found;
	}
	assert(0);  // OOPS: child not found in parent's down list?!
	found:;
}


// USE THIS INSTEAD OF ASSIGNING this->up DIRECTLY. 
// Read head_nodes for details. 
void TreeNode::SetParent(TreeNode *p)
{
	if(p)
	{
		// Must remove from head_nodes if queued there. 
		if(!up)   // up is set -> has already a parent -> not on head_nodes
		{
			assert(head_nodes.find_rev(this));
			head_nodes.dequeue(this);
		}
	}
	else // p=NULL
	{
		// Must put into head_nodes. 
		if(up)  // up set -> not in head_nodes
		{
			assert(!head_nodes.find(this));
			head_nodes.append(this);
		}
	}
	up=p;
}


void TreeNode::TransferChildrenFrom(TreeNode *src)
{
	while(!src->down.is_empty())
	{
		// Simple transfer; no need to go long way using 
		// SetParent(), AddChild() and head_nodes. 
		TreeNode *i=src->down.popfirst();
		assert(i->up==src);
		i->up=this;
		down.append(i);
	}
}


int TreeNode::CountNodes() const
{
	int n=1;
	for(const TreeNode *tn=down.first(); tn; tn=tn->next)
	{  n+=tn->CountNodes();  }
	return(n);
}


void TreeNode::DumpTree(FILE *out)
{
	StringTreeDump d;
	DumpTree(&d);
	d.Write(out);
}


/******************************************************************************/

CheckIfEvalInfo::CheckIfEvalInfo()
{
	nrefs_to_this_ptr=0;
	nrefs_to_local_stack=0;  
	number_of_values=0;
	nrefs_to_static=0;
}

}  // end of namespace ANI
