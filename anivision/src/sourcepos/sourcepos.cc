/*
 * sourcepos/sourcepos.cc
 * 
 * Classes for sophisticated source position (file/line/lpos, 
 * include hierarchy) handling. 
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

#include "sourcepos.h"

#include <hlib/lmalloc.h>
#include <assert.h>

// Non-critical assert: 
#define nc_assert(x)  assert(x)


void SourcePositionNS::_forbidden()
{
	nc_assert(!"forbidden");
}


void SourcePositionNS::_allocfail()
{
	fprintf(stderr,"Allocation failure.\n");
	abort();
}


/******************************************************************************/

SourcePosition SourcePositionCache::Find(int line,int lpos)
{
	SourcePosition sp(SourcePosition::NullPos);
	
	// [Need not check for top<0 here, the for() loop is safe.] 
	// Do a linear lookup. A binary search or something like that 
	// would just be overkill at the applied cache size of 4..8. 
	for(int i=cache_size; i>0; i--)  // <-- CORRECT!!
	{
		const SourcePosition *spi=&(cache[(top+i)%cache_size]);
		if((*spi)==SourcePosition::NullPos)  break;
		if(spi->GetLine()==line && spi->GetLPos()==lpos)
		{  sp=*spi;  ++hits;  return(sp);  }
	}
	
	++misses;
	return(sp); 
}


SourcePositionCache::SourcePositionCache(int * /*failflag*/)
{
	top=-1;  // correct
	
	hits=0;
	misses=0;
}

SourcePositionCache::~SourcePositionCache()
{
	fprintf(stderr,"SourcePositionCache: hits=%d, misses=%d\n",
		hits,misses);
}


/******************************************************************************/

InternalSourceFileNode *SourcePositionArchive::AllocISF(
	const RefString &path,SourceFileNode &parent)
{
	InternalSourceFileNode *isf=NEW3<InternalSourceFileNode>(
		path,parent.isf,this);
	if(!isf)
	{  SourcePositionNS::_allocfail();  }
	
	++tot_file_nodes;
	++curr_file_nodes;
	if(max_file_nodes<curr_file_nodes)
	{  max_file_nodes=curr_file_nodes;  }
	
	return(isf);
}

void SourcePositionArchive::DestroyISF(InternalSourceFileNode *isf)
{
	// Normally called by InternalSourceFileNode::_destroy(). 
	if(!isf)  return;
	// We do NOT destroy the file node here to keep the hierarchy 
	// alive (even if this file is not needed at all). 
	// Destruction done by destructor (using _DetachArchive()). 
	#if 0
	delete isf;
	--curr_file_nodes;
	#endif
}


// This will NOT see if we have the required position in the cache. 
// JUST allocate a new one. 
InternalSourcePosition *SourcePositionArchive::AllocISPNoCache(
	const SourceFileNode &fnode,int line,int lpos)
{
	InternalSourcePosition *isp=NEW3<InternalSourcePosition>(
		fnode,line,lpos);
	if(!isp)
	{  SourcePositionNS::_allocfail();  }
	
	++tot_src_pos;
	++curr_src_pos;
	if(max_src_pos<curr_src_pos)
	{  max_src_pos=curr_src_pos;  }
	
	return(isp);
}

void SourcePositionArchive::DestroyISP(InternalSourcePosition *isp)
{
	// Normally called by InternalSourcePosition::_destroy(). 
	if(!isp)  return;
	delete isp;
	
	--curr_src_pos;
}


SourceFileNode SourcePositionArchive::IncludeFile(const RefString &file,
	int line,int lpos,
	filepos_t include_stmt_pos,size_t include_stmt_len)
{
	assert(!!current);
	
	InternalSourceFileNode *isf=AllocISF(file,/*parent=*/current);
	
	isf->parent_include_line=line;
	isf->parent_include_lpos=lpos;
	isf->include_stmt_pos=include_stmt_pos;
	isf->include_stmt_len=include_stmt_len;
	
	current=SourceFileNode(isf);
	return(current);
}


SourceFileNode SourcePositionArchive::EndFile()
{
	assert(!!current);
	
	// Can delete the position cache here because it will no 
	// longer be used. 
	// OTOH, can also keep it because it is small...
	current.isf->_DestroyPosCache();
	
	current=SourceFileNode(current.isf->up);   // ..which may be NULL (primary input EOF)
	return(current);
}


SourcePosition SourcePositionArchive::GetPos(int line,int lpos)
{
	assert(!!current);
	
	// Query the cache: 
	if(current.isf->pos_cache)
	{
		SourcePosition sp=current.isf->pos_cache->Find(line,lpos);
		if(!!sp)  return(sp);
	}
	
	// Allocate a new position object: 
	SourcePosition sp((line>=0) ? AllocISPNoCache(current,line,lpos) : NULL);
	
	// Add it to cache: 
	if(current.isf->pos_cache)
	{  current.isf->pos_cache->Store(sp);  }
	
	return(sp);
}


// Recursively call _DetachArchive() for all InternalSourceFileNodes. 
void SourcePositionArchive::_RecursiveDetachArchive(
	InternalSourceFileNode *head)
{
	if(!head)  return;
	
	// Make sure we keep a reference: 
	SourceFileNode tmp(head);
	
	for(InternalSourceFileNode *_i=head->down.first(); _i; )
	{
		InternalSourceFileNode *i=_i;
		_i=_i->next;
		
		_RecursiveDetachArchive(i);
	}
	
	head->_DetachArchive();
}

SourcePositionArchive::SourcePositionArchive(const RefString &file,int *) : 
	head(SourceFileNode::NullFile),
	current(head)
{
	tot_file_nodes=0;
	max_file_nodes=0;
	curr_file_nodes=0;
	
	tot_src_pos=0;
	max_src_pos=0;
	curr_src_pos=0;
	
	InternalSourceFileNode *isf=AllocISF(file,
		/*parent=*/current/*, which is NullFile*/);
	head=SourceFileNode(isf);
	current=head;
}

SourcePositionArchive::~SourcePositionArchive()
{
	current=SourceFileNode::NullFile;
	
	// Tell all the SourceFileNodes that the archive no longer exists: 
	_RecursiveDetachArchive(head.isf);
	
	fprintf(stderr,"SourcePositionArchive: files: tot=%d, max=%d (%u bytes)\n",
		tot_file_nodes,max_file_nodes,
		max_file_nodes*sizeof(InternalSourceFileNode));
	fprintf(stderr,"    src pos: tot=%u, max=%u (%u bytes)\n",
		tot_src_pos,max_src_pos,max_src_pos*sizeof(InternalSourcePosition));
}


/******************************************************************************/

void InternalSourceFileNode::_destroy()
{
	if(!pos_archive)
	{
		// Must delete ourselves. 
		//fprintf(stderr,"Hm: InternalSourceFileNode: self-delete.\n");
		delete this;
	}
	else
	{
		// This is clean destruction: Tell the archive 
		// (to keep statistics in sync). 
		pos_archive->DestroyISF(this);
	}
}


InternalSourceFileNode::InternalSourceFileNode(const RefString &file,
	InternalSourceFileNode *parent,SourcePositionArchive *_pos_archive,int *) : 
	path(file),
	down()
{
	pos_cache=NULL;
	
	refcnt=0;
	up=parent;
	
	pos_archive=_pos_archive;
	
	parent_include_line=-1;
	parent_include_lpos=-1;
	include_stmt_pos=-1;
	include_stmt_len=0;  // (unsigned)
	
	pos_cache=NEW<SourcePositionCache>();
	if(!pos_cache)
	{  SourcePositionNS::_allocfail();  }
	
	if(up)
	{  up->_AddChild(this);  }
}

InternalSourceFileNode::~InternalSourceFileNode()
{
	assert(refcnt==0);
	// We may not have refcnt=0 if there are still children. 
	assert(down.is_empty());
	
	// Make sure we delete our down pointer at the parent: 
	if(up)
	{
		up->_DelChild(this);
		up=NULL;
	}
	
	// Destroy position cache if still there: 
	DELETE(pos_cache);
}


/******************************************************************************/

void InternalSourcePosition::_destroy()
{
	if(!file.GetArchive())
	{
		// Must delete ourselves. 
		//fprintf(stderr,"Hm: InternalSourcePosition: self-delete.\n");
		delete this;
	}
	else
	{
		// This is clean destruction: Tell the archive 
		// (to keep statistics in sync). 
		file.GetArchive()->DestroyISP(this);
	}
}


InternalSourcePosition::InternalSourcePosition(const SourceFileNode &fnode,
	int _line,int _lpos,int *) : 
	file(fnode)
{
	line=_line;
	lpos=_lpos;
	refcnt=0;
}

InternalSourcePosition::~InternalSourcePosition()
{
	assert(refcnt==0);
}


/******************************************************************************/

bool SourcePosition::test_equal(const SourcePosition &sp) const
{
	// Compare pos (if available): 
	if(isp && sp.isp)
	{
		if(isp!=sp.isp || 
		   isp->line!=sp.isp->line || 
		   isp->lpos!=sp.isp->lpos || 
		   isp->file!=sp.isp->file )
		{  return(false);  }
	}
	
	return(true);
}


int SourcePosition::PosString(RefString *dest,bool with_file) const
{
	int line=GetLine();
	int lpos=GetLPos();
	int rv=0;
	if(!isp)
	{
		rv=dest->set("(null)");
	}
	else if(line<0)
	{
		if(with_file)  *dest=GetPath();
		else  rv=dest->set("");
	}
	else if(lpos<0)
	{
		if(with_file) rv=dest->sprintf(0,"%s:%d",GetPath().str(),line);
		else rv=dest->sprintf(0,"%d",line);
	}
	else
	{
		if(with_file) rv=dest->sprintf(0,"%s:%d:%d",GetPath().str(),line,lpos);
		else rv=dest->sprintf(0,"%d:%d",line,lpos);
	}
	return(rv ? -1 : 0);
}


int SourcePosition::PosRangeString(const SourcePosition &end,
	RefString *dest,bool with_file) const
{
	int line0=GetLine();
	int lpos0=GetLPos();
	SourceFileNode sfn0=GetFile();
	
	int line1=end.GetLine();
	int lpos1=end.GetLPos();
	SourceFileNode sfn1=end.GetFile();
	
	int fail=0;
	#warning "This may handle cases with line/lpos<0 incorrectly."
	if(sfn0!=sfn1)
	{
		RefString tmp;
		if(PosString(dest,with_file)) ++fail;
		if(dest->append(".."))  ++fail;
		if(end.PosString(&tmp,with_file)) ++fail;
		if(dest->append(tmp))  ++fail;
	}
	else if(line0!=line1 || lpos0<0 || lpos1<0)
	{
do_it_above:;
		// Print first position: 
		if(PosString(dest,with_file)) ++fail;
		if(dest->append(".."))  ++fail;
		// Print second position without path: 
		RefString tmp;
		int rv;
		if(line1<0)
		{  rv=tmp.set("??");  }
		else if(lpos1<0)
		{  rv=tmp.sprintf(0,"%d",line1);  }
		else
		{  rv=tmp.sprintf(0,"%d:%d",line1,lpos1);  }
		if(rv)  ++fail;
		if(dest->append(tmp))  ++fail;
	}
	else if(lpos0!=lpos1)
	{
		if(line0<0 || line1<0)
		{  goto do_it_above;  }
		
		int rv;
		if(with_file)
		{  rv=dest->sprintf(0,"%s:%d:[%d..%d]",
			sfn0.GetPath().str(),line0,lpos0,lpos1);  }
		else
		{  rv=dest->sprintf(0,"%d:[%d..%d]",line0,lpos0,lpos1);  }
		if(rv) ++fail;
	}
	else
	{
		// Start and end are equal. 
		if(PosString(dest,with_file))  ++fail;
	}
	
	return(fail ? -1 : 0);
}


int SourcePosition::PosStringRelative(const SourcePosition &rpos,
	RefString *dest) const
{
	if(GetFile()!=rpos.GetFile())
	{  return(PosString(dest));  }
	int line=GetLine();
	int lpos=GetLPos();
	int rv;
	if(lpos<0)
	{  rv=dest->sprintf(0,"%d",line);  }
	else
	{  rv=dest->sprintf(0,"%d:%d",line,lpos);  }
	return(rv ? -1 : 0);
}


/******************************************************************************/


#if 0
char *prg_name=NULL;
int main()
{
	{
		RefString file;
		file.set("foo");
		SourcePosition xx(SourcePosition::NullPos);
		{
			SourcePositionArchive spa(file);
			SourcePosition sp=spa.GetPos(1,2);
			spa.GetPos(1,2);
			xx=sp;
		}
	}
	
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%sLMalloc: used=%u/%d (max=%u) [%u,%u,%u]%s\n",
		lmu.curr_used ? "**** " : "",
		lmu.curr_used,lmu.used_chunks,
		lmu.max_used,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls,
		lmu.curr_used ? "**** " : "");
	return(0);
}
#endif
