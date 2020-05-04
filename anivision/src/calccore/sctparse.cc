/*
 * calccore/sctparse.cc
 * 
 * Scope tree parser. 
 * This is part of the AniVision project. 
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

#include "sctparse.h"

#include <stdarg.h>

#include <assert.h>


namespace CC
{

ScopeTreeParser::SCEnt *ScopeTreeParser::Entry::FindSCEnt(int name_tok)
{
	// This could be replaced by a binary search. 
	for(int i=0; i<nents; i++)
		if(scent[i].name_tok==name_tok)  return(&scent[i]);
	return(NULL);
}


ScopeTreeParser::Entry &ScopeTreeParser::Entry::operator+(const SCEnt &se)
{
	if(nents>=alloc_ents)
	{
		alloc_ents+=8;
		scent=(SCEnt*)LRealloc(scent,alloc_ents*sizeof(SCEnt));
	}
	scent[nents++]=se;
	return(*this);
}


ScopeTreeParser::Entry &ScopeTreeParser::Entry::operator=(const Entry &e)
{
	name_tok=e.name_tok;
	has_expression=e.has_expression;
	expr_id=e.expr_id;
	nents=e.nents;
	if(alloc_ents<nents)
	{
		alloc_ents=e.alloc_ents;
		scent=(SCEnt*)LRealloc(scent,alloc_ents*sizeof(SCEnt));
	}
	for(int i=0; i<nents; i++)
		scent[i]=e.scent[i];
	return(*this);
}


ScopeTreeParser::Entry::Entry(const Entry &e)
{
	name_tok=e.name_tok;
	has_expression=e.has_expression;
	expr_id=e.expr_id;
	nents=e.nents;
	alloc_ents=e.alloc_ents;
	scent=(SCEnt*)LMalloc(alloc_ents*sizeof(SCEnt)); // 
	for(int i=0; i<nents; i++)
		scent[i]=e.scent[i];
}

//------------------------------------------------------------------------------

void ScopeTreeParser::EntryList::AddEntry(const Entry &_ent)
{
	if(nents>=alloc_ents)
	{
		alloc_ents+=8;
		ent=(Entry**)LRealloc(ent,alloc_ents*sizeof(Entry*));
	}
	ent[nents++]=new Entry(_ent);
}


void ScopeTreeParser::EntryList::TransferEntries(EntryList *from)
{
	Clear();
	
	alloc_ents=from->alloc_ents;  from->alloc_ents=0;
	nents=from->nents;            from->nents=0;
	ent=from->ent;                from->ent=NULL;
}


void ScopeTreeParser::EntryList::Clear()
{
	for(int i=0; i<nents; i++)
		delete ent[i];
	ent=(Entry**)LFree(ent);
}


ScopeTreeParser::EntryList::EntryList(const EntryList &l)
{
	alloc_ents=l.alloc_ents;
	nents=l.nents;
	ent=(Entry**)LMalloc(alloc_ents*sizeof(Entry*));
	for(int i=0; i<nents; i++)
		ent[i]=new Entry(*l.ent[i]);
}


ScopeTreeParser::EntryList &ScopeTreeParser::EntryList::operator=(
	const EntryList &l)
{
	nents=l.nents;
	if(alloc_ents<nents)
	{
		alloc_ents=l.alloc_ents;
		ent=(Entry**)LRealloc(ent,alloc_ents*sizeof(Entry*));
	}
	for(int i=0; i<nents; i++)
		ent[i]=new Entry(*l.ent[i]);
}

//------------------------------------------------------------------------------

ScopeTreeParser::Scope *ScopeTreeParser::_LookupScope(int scope_id)
{
	for(int i=0; i<nscopes; i++)
		if(scope[i]->scope_id==scope_id)  return(scope[i]);
	return(NULL);
}


int ScopeTreeParser::SetCurrentScope(int scope_id)
{
	Scope *s=_LookupScope(scope_id);
	top=0;
	scs[top].scope=s;
	scs[top].curr_eidx=0;
	return(s ? 0 : 1);
}


int ScopeTreeParser::ParseEnterScope(int scope_name_tok,int *return_name_tok)
{
	if(top<0)  return(-2);
	SCEntry *t=&scs[top];
	
	// Make sure the stack is large enough: 
	if(top+1>=scs_alloc)
	{
		scs_alloc+=32;
		scs=(SCEntry*)LRealloc(scs,scs_alloc*sizeof(SCEntry));
	}
	
	// If error in prev level, push a NULL scope: 
	if(!t->scope)
	{
		++top;
		scs[top].scope=NULL;
		scs[top].curr_eidx=-1;
		return(2);
	}
	
	int rv=4;
	Scope *es=NULL;
	if(t->curr_eidx>=0 && t->curr_eidx<t->scope->nents)
	{
		Entry *e=t->scope->ent[t->curr_eidx];
		SCEnt *sce=e->FindSCEnt(scope_name_tok);
		if(sce)
		{
			es=sce->scope;
			if(return_name_tok)  *return_name_tok=e->name_tok;
			assert(es);  // If this fails, there was an error in InitGrammar(). 
			++t->curr_eidx;  // ...which may be out of range
			rv=0;
		}
		else
		{  rv=3;  }
	}
	
	// Enter scope: 
	++top;
	scs[top].scope=es;
	scs[top].curr_eidx=0;
	
	return(rv);
}


int ScopeTreeParser::ParseEnterScopeN(int scope_name_tok,int name_tok)
{
	if(top<0)  return(-2);
	SCEntry *t=&scs[top];
	
	// Make sure the stack is large enough: 
	if(top+1>=scs_alloc)
	{
		scs_alloc+=32;
		scs=(SCEntry*)LRealloc(scs,scs_alloc*sizeof(SCEntry));
	}
	
	// If error in prev level, push a NULL scope: 
	if(!t->scope)
	{
		++top;
		scs[top].scope=NULL;
		scs[top].curr_eidx=-1;
		return(2);
	}
	
	// Lookup scope: 
	Entry *en=NULL;
	do {
		// First guess, it is the next entry: 
		if(t->curr_eidx>=0 && t->curr_eidx<t->scope->nents)
		{
			Entry *e=t->scope->ent[t->curr_eidx];
			if(e->name_tok==name_tok)
			{
				en=e;
				++t->curr_eidx;  // ...which may be out of range
				break;
			}
		}
		// Try all entries: 
		for(int i=0; i<t->scope->nents; i++)
		{
			Entry *e=t->scope->ent[i];
			if(e->name_tok==name_tok)
			{
				en=e;
				t->curr_eidx=i+1;  // ...which may be out of range
				goto breakout;
			}
		}
	} while(0); breakout:;
	
	int rv;
	Scope *es=NULL;
	if(en)
	{
		SCEnt *sce=en->FindSCEnt(scope_name_tok);
		if(sce)
		{
			es=sce->scope;
			assert(es);  // If this fails, there was an error in InitGrammar(). 
			rv=0;
		}
		else
		{  rv=3;  }
	}
	else
	{  rv=1;  }
	
	// Enter scope: 
	++top;
	scs[top].scope=es;
	scs[top].curr_eidx=0;
	
	return(rv);
}


int ScopeTreeParser::ParseEntry(int *expr_id,int *return_name_tok)
{
	if(top<0)  return(-2);
	SCEntry *t=&scs[top];
	
	// If error in prev level, stop now. 
	if(!t->scope)  return(2);
	
	if(t->curr_eidx>=0 && t->curr_eidx<t->scope->nents)
	{
		Entry *e=t->scope->ent[t->curr_eidx];
		if(e->has_expression)
		{
			if(expr_id)  *expr_id=e->expr_id;
			if(return_name_tok)  *return_name_tok=e->name_tok;
			++t->curr_eidx;  // ...which may be out of range
			return(0);
		}
	}
	
	return(4);
}


int ScopeTreeParser::ParseEntryN(int name_tok,int *expr_id)
{
	if(top<0)  return(-2);
	SCEntry *t=&scs[top];
	
	// If error in prev level, stop now. 
	if(!t->scope)  return(2);
	
	// First guess, it is the next entry: 
	if(t->curr_eidx>=0 && t->curr_eidx<t->scope->nents)
	{
		Entry *e=t->scope->ent[t->curr_eidx];
		if(e->name_tok==name_tok)
		{
			if(e->has_expression)
			{
				if(expr_id) *expr_id=e->expr_id;
				++t->curr_eidx;  // ...which may be out of range
				return(0);
			}
			return(3);
		}
	}
	// Try all entries: 
	for(int i=0; i<t->scope->nents; i++)
	{
		Entry *e=t->scope->ent[i];
		if(e->name_tok!=name_tok)  continue;
		if(e->has_expression)
		{
			if(expr_id) *expr_id=e->expr_id;
			t->curr_eidx=i+1;  // ...which may be out of range
			return(0);
		}
		return(3);
	}
	
	return(1);
}


int ScopeTreeParser::ParseLeaveScope()
{
	if(top<0)  return(-2);
	--top;
	return(0);
}


int ScopeTreeParser::InitGrammar()
{
	int errors=0;
	
	for(int si=0; si<nscopes; si++)
	{
		Scope *sc=scope[si];
		for(int ei=0; ei<sc->nents; ei++)
		{
			Entry *e=sc->ent[ei];
			
			// Lookup the scope pointer for all entries: 
			for(int i=0; i<e->nents; i++)
			{
				int sid=e->scent[i].scope_id;
				int ntok=e->scent[i].name_tok;
				if(!(e->scent[i].scope=_LookupScope(sid)))
				{  ++errors;  continue;  }
				
				// Check for duplicate entries: 
				for(int j=i+1; j<e->nents; j++)
				{
					if(e->scent[j].name_tok==ntok)  ++errors;
				}
			}
			
			// Check for duplicate entries: 
			for(int i=ei+1; i<sc->nents; i++)
			{
				if(e->name_tok==sc->ent[i]->name_tok)  ++errors;
			}
		}
	}
	
	return(errors);
}


ScopeTreeParser::Scope *ScopeTreeParser::_DeclareScope(int scope_id)
{
	Scope *s=_LookupScope(scope_id);
	if(s) return(s);
	if(nscopes>=alloc_scopes)
	{
		alloc_scopes+=16;
		scope=(Scope**)LRealloc(scope,alloc_scopes*sizeof(Scope*));
	}
	s=new Scope(scope_id);
	scope[nscopes++]=s;
	return(s);
}

int ScopeTreeParser::DefineScope(int scope_id,EntryList *elist)
{
	Scope *s=_DeclareScope(scope_id);
	if(!s)  return(1);
	s->TransferEntries(elist);
	return(0);
}


void ScopeTreeParser::Clear()
{
	top=-1;
	
	for(int i=0; i<nscopes; i++)
	{  DELETE(scope[i]);  }
	nscopes=0;
	alloc_scopes=0;
	LFree(scope);
}


ScopeTreeParser::ScopeTreeParser()
{
	scope=NULL;
	nscopes=0;
	alloc_scopes=0;
	
	// Allocate first stack; SetCurrentScope() depends on that. 
	top=-1;
	scs_alloc=32;
	scs=(SCEntry*)LMalloc(scs_alloc*sizeof(SCEntry));
}

ScopeTreeParser::~ScopeTreeParser()
{
	Clear();
	
	// Deallocate stack: 
	scs=(SCEntry*)LFree(scs);
	scs_alloc=0;
}

}  // end of namespace CC
