/*
 * ani-parser/strtreedump.cc
 * 
 * Expandable, indent-capable, alloc-ahead string buffer for tree 
 * node dump. 
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

#include "strtreedump.h"

#include <assert.h>


void StringTreeDump::Append(const char *s)
{
	Entry *ent=ents.last();
	if(!ent || ent->indent!=curr_indent)
	{
		// Need new entry. 
		// First, tighten previous entry. 
		if(ents.last())  ents.last()->TightSize();
		// Alloc and queue new entry: 
		ent=new Entry(curr_indent);
		ents.append(ent);
	}
	ent->Append(s);
	indent_just_added=0;
}

void StringTreeDump::RemoveEnd(char c)
{
	Entry *ent=ents.last();
	assert(ent);  // Otherwise we may not call this function...
	ent->RemoveEnd(c);
}


size_t StringTreeDump::Write(FILE *fp)
{
	size_t totbytes=0;
	int cpos=0;  // current position
	int indent_size=4;
	char indent_char=' ';
	// One entry to hold a line in the dump (temp): 
	Entry tmp(0);
	Entry *ent=ents.first();
	if(ent) for(const char *ent_str=ent->str;;)
	{
		// One loop per line. 
		// First, find the text: 
		const char *add_start=ent_str;
		for(; *ent_str; ent_str++)
		{
			if(*ent_str=='\n')  break;
		}
		const char *add_end=ent_str;
		bool was_nl=0;
		if(*ent_str=='\n')
		{  was_nl=1;  ++ent_str;  }
		
		if(add_end<=add_start)
		{
			if(was_nl)
			{
				//fprintf(stderr,"<->\n");
				assert(tmp.len==0);
				fprintf(fp,"\n");
				++totbytes;
			}
			else
			{
				assert(*ent_str=='\0');
				ent=ent->next;
				if(!ent)  goto done;
				ent_str=ent->str;
			}
			continue;
		}
		
		// Add indention: 
		int add_indent=ent->indent-cpos;
		if(add_indent)
		{
			char itmp[add_indent*indent_size+1];
			for(int i=0; i<add_indent*indent_size; i++)
			{  itmp[i]=indent_char;  }
			itmp[add_indent*indent_size]='\0';
			tmp.Append(itmp);
		}
		
		// Then, add text: 
		tmp.Append(add_start,add_end-add_start);
		
		// Write it: 
		fprintf(fp,"%s\n",tmp.str);
		totbytes+=tmp.len+1;
		tmp.Zap();
	}
	done:;
	return(totbytes);
}


StringTreeDump::StringTreeDump() : 
	ents()
{
	curr_indent=0;
	indent_just_added=0;
}

StringTreeDump::~StringTreeDump()
{
	while(!ents.is_empty())
	{  delete ents.poplast();  }
}


/******************************************************************************/

void StringTreeDump::Entry::Append(const char *s,size_t l)
{
	if(!s)  return;
	len+=l;
	
	// This seems inefficient but as the appended strings are 
	// always small, this is not a problem. 
	if(len>=size)
	{
		if(!size)  size=32;
		while(len>=size)   // len+1 > size
		{
			if(size<1024)  size*=2;
			else size+=1024;
		}
		str=(char*)LRealloc(str,size);
	}
	
	strncpy(str+len-l,s,l);
	str[len]='\0';
}


void StringTreeDump::Entry::RemoveEnd(char c)
{
	if(len && str[len-1]==c)
	{  str[--len]='\0';  }
	else
	{
		fprintf(stderr,"Warning: Attempt to remove `%c' from tree dump but "
			"`%c' is last char.\n",c,len ? str[len-1] : '\0');
	}
}


void StringTreeDump::Entry::TightSize()
{
	if(size>len+1)
	{
		size=len+1;
		str=(char*)LRealloc(str,size);
		assert(str);
	}
}


StringTreeDump::Entry::Entry(int _indent)
{
	size=0;
	len=0;
	str=NULL;
	indent=_indent;
	assert(indent>=0);
}

StringTreeDump::Entry::~Entry()
{
	str=(char*)LFree(str);
}
