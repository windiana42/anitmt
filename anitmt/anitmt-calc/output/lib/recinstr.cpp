/*
 * recinstr.cpp
 * 
 * Implementation of routines to read in nested files 
 * (Recursive Input Stream). 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   Apr 2001   started writing
 *
 */


// I always forget...
// std::list: call .push_back/.push_front to add entries. 

#include "recinstr.hpp"

#include <iostream>


namespace output_io
{

// Includes can only be nested this often. The only purpose 
// is to avoid infinite recursion; the parameter does NOT 
// change any buffer size etc. 
// This should be enough, he?
int Recursive_Input_Stream::Max_Include_Depth=64;


static int mem_cnt(char *buf,size_t len,char c)
{
	int cnt=0;
	for(char *end=buf+len; buf<end; buf++)
	{
		if(*buf==c)
		{  ++cnt;  }
	}
	return(cnt);
}


// non-static: 
// This can be used to write the number postfix (last two chars) 
// for "1st", "2nd", "3rd", "4th", "5th", ...
const char *Num_Postfix(int n)
{
	switch(n)
	{
		case 1: return("st");
		case 2: return("nd");
		case 3: return("rd");
	}
	return("th");
}


void Recursive_Input_Stream::rewind()
{
	cr.curr=NULL;
	
	// Close all input streams: (important)
	for(std::list<UInput_Stream*>::iterator i=infiles.begin();
		i!=infiles.end(); i++)
	{
		UInput_Stream *is=*i;
		if(is->IsOpen())
		{  is->close();  }
	}
	
	if(start.is)
	{  _Switch_CR(&start);  }  // will not fail here as cr.curr=NULL. 
	
	tot_read_bytes=0;
}


Recursive_Input_Stream::InFile_Segment *Recursive_Input_Stream::_Next_Read_Segment(
	InFile_Segment *ifs)
{
	int sdepth=ifs->depth;
	for(InFile_Segment *i=ifs->next; i; i=i->next)
	{
		if(i->depth>sdepth)  continue;
		if(i->depth<sdepth)  return(NULL);
		assert(ifs->is==i->is);
		return(i);
	}
	return(NULL);
}


ssize_t Recursive_Input_Stream::_read_curr(char *buf,size_t len)
{
	assert(cr.off<=cr.len);
	size_t tocopy=cr.len-cr.off;
	if(tocopy>len)
	{  tocopy=len;  }
	if(!tocopy)  return(0);
	
	assert(cr.curr->is);
	if(!cr.curr->is->IsOpen())
	{
		if(!cr.curr->is->open(/*print_error=*/true))
		{
			// Error already written. 
			return(-1);
		}
	}
	
	size_t pos=cr.curr->is->Off();
	size_t rd=cr.curr->is->read(buf,tocopy);
	tot_read_bytes+=rd;
	if(rd!=tocopy)  // otherwise: internal error or file modified
	{
		error() << cr.curr->is->Path() << ": unexpected EOF encountered "
			"while trying to read " << tocopy << " bytes at position " << 
			pos << ".";
		error() << "Internal error or file was modified during execution.";
		return(-2);
	}
	
	cr.off+=rd;
	return(rd);
}


int Recursive_Input_Stream::_Switch_CR(InFile_Segment *new_ifs)
{
	if(cr.curr)
	{
		if(cr.skip<0)
		{
			// should assert(cr.curr->is->eof_reached) here. 
			cr.curr->is->close();
		}
		else if(cr.skip>0)
		{
			size_t pos=cr.curr->is->Off();
			if(cr.curr->is->skip(cr.skip)!=size_t(cr.skip))
			{
				error() << cr.curr->is->Path() << ": failed to skip " << 
					cr.skip << " at position " << pos << ".";
				error() << "Internal error or file was modified during execution.";
				return(-3);
			}
		}
	}
	
	cr.curr=new_ifs;
	if(cr.curr)
	{
		cr.off=0;
		cr.line=0;
		cr.len=cr.curr->len;
		InFile_Segment *n=_Next_Read_Segment(cr.curr);
		if(n)
		{
			assert(n->off>=cr.curr->off);
			cr.skip = n->off - cr.curr->off;
		}
		else
		{  cr.skip=-1;  }
	}
	
	return(0);
}


size_t Recursive_Input_Stream::read(char *buf,size_t len,File_Context *fc)
{
	fc->handle=NULL;
	fc->path.assign("");
	fc->depth=0;
	fc->line0=0;
	fc->offset=0;
	if(!cr.curr)
	{  return(0);  }
	
	// Skip segments until we can read something: 
	ssize_t rd;
	for(;;)
	{
		rd=_read_curr(buf,len);
		if(rd<0)
		{  fc->depth=int(rd);  return(0);  }
		if(rd>0)  break;
		// rd==0 here; skip ifs
		int rv=_Switch_CR(cr.curr->next);
		if(rv)
		{  fc->depth=rv;  return(0);  }
		if(!cr.curr)
		{
			// EOF of main input file
			return(0);
		}
	}
	
	fc->handle=cr.curr;
	fc->path.assign(cr.curr->is->Path());
	fc->depth=cr.curr->depth;
	fc->line0 = cr.curr->line0 + cr.line;
	fc->offset = cr.curr->off + cr.off - rd;
	
	cr.line+=mem_cnt(buf,rd,'\n');
	
	return(rd);
}


// Really read the file(s) in. 
// Call this or the class will no nothing. 
// Returns -1 if the specified file could not be opened. 
// Returns 0 on success and >0 if errors occured. 
int Recursive_Input_Stream::Start_Parser(const std::string &file)
{
	_Reset();  // sets tot_read_bytes=0
	
	// We do not honor the search path here. 
	UInput_Stream *top_is=new UInput_Stream(file);
	
	if(!top_is->open(/*print_error*/true))
	{
		error() << "Hmm, could not open master scene file; "
			"you may wish to interrupt me.";
		delete top_is;
		return(-1);
	}
	
	infiles.push_back(top_is);
	
	// Generate top level file entry: 
	start.is=top_is;
	start.off=0;
	start.len=0;  // nothing read so far 
	start.depth=0;
	start.line0=1;
	start.lines=0;
	last_ifs=&start;
	
	//start.is->open();  already done above. 
	int rv=_Recursive_Read(&start);
	if(abort_scheduled)  ++rv;
	return(rv);
}


// One recursion instance for each file instance. Depth: ifs->depth. 
int Recursive_Input_Stream::_Recursive_Read(InFile_Segment *ifs)
{
	int errors=0;
	
	Input_Buffer _ibuf(ibuf_default_len);
	// Be sure to restore curr_ib on return: 
	Input_Buffer *orig_ib=curr_ib;
	curr_ib=&_ibuf;
	
	assert(ifs->is);
	
	Parse_Input_Buf pib;
	pib.path=ifs->is->Path();
	pib.depth=ifs->depth;
	
	// ifs->off stays constant. 
	// ifs->len increases as we parse the file. 
	// ifs->line0 stays constant.
	// ifs->lines increases as we read. 
	
	for(;;)
	{
		// Read some input 
		int fus=curr_ib->FillUp(ifs->is,&tot_read_bytes);
		
		// Call virtual function: 
		pib.txt=curr_ib->buf;
		pib.len=curr_ib->len;
		pib.offset=ifs->off+ifs->len;
		pib.line0=ifs->line0+ifs->lines;
		pib.handle=ifs;
		pib.eof_reached = ((fus>=1) ? true : false);
		if(fus==2)  // eof reached; no input left
		{
			Parse_EOF(&pib);
			break;
		}
		assert(pib.len>0);
		size_t parsed=Parse_Input(&pib);
		if(abort_scheduled)  break;
		
		assert(parsed<=pib.len);
		assert(parsed>0);  // may actually be 0 if Set_IBuf_Size() was called. 
		
		// Count the number of parsed lines: 
		ifs->lines+=mem_cnt(curr_ib->buf,parsed,'\n');
		ifs->len+=parsed;
		// remove parsed bytes from input buffer
		curr_ib->Skip(parsed);
		
		// The followinf flag is set if Include_File() was called 
		// successfully. 
		if(include.scheduled)  // must include a file
		{
			if(ifs->depth>=Max_Include_Depth)
			{
				error() << "Error: Max include depth (" << Max_Include_Depth <<
					") exceeded; skipping " << include.is->Path();
				if(include.is)  delete include.is;
				include.is=NULL;
				include.scheduled=false;
			}
		}
		if(include.scheduled)
		{
			if(include.offset+include.inclen!=ifs->off+ifs->len)
			{  fprintf(stderr,"*** %u+%u=%u != %u+%u=%u ***\n",
				include.offset,include.inclen,include.offset+include.inclen,
				ifs->off,ifs->len,ifs->off+ifs->len);  }
			assert(include.offset+include.inclen==ifs->off+ifs->len);
			assert(include.is);
			
			// Yeah, include it. 
			if(include.is->use_cnt==1)  // >1 -> it is already in the list. 
			{  infiles.push_back(include.is);  }
			
			// We need a file segment for the new file and for this 
			// file at the position where we resume. 
			InFile_Segment *inc_ifs=new InFile_Segment();
			inc_ifs->is=include.is;
			inc_ifs->depth=ifs->depth+1;
			assert(!ifs->next && ifs==last_ifs);  // ifs must be the last node in the queue
			_Append_IFS(inc_ifs);
			
			// The new ifs must be allocated NOW (include.* may change) 
			// and queued after _Recursive_Read() (last_ifs may change). 
			InFile_Segment *new_ifs=new InFile_Segment();
			new_ifs->is=ifs->is;
			new_ifs->off=ifs->off+ifs->len;  // just after include statement 
			new_ifs->line0=ifs->line0+ifs->lines;
			new_ifs->depth=ifs->depth;
			
			// Un-schedule the include (as it may be needed by 
			// _Recursive_Read()) 
			include.is=NULL;  // don't delete include.is; it's in infile list 
			include.scheduled=false;
			
			int rr=_Recursive_Read(inc_ifs);
			errors+=rr;
			
			_Append_IFS(new_ifs);  // add at the end of the list 
			ifs=new_ifs;  // work on with this one
			
			if(abort_scheduled)  break;
		}
	}
	
	// close the file: 
	ifs->is->close();
	
	// Restore curr_ib: 
	curr_ib=orig_ib;
	return(errors);
}


int Recursive_Input_Stream::Include_File(
	const std::string &file,
	size_t offset,size_t inclen,
	bool honor_search_path,int search_current_path)
{
	// Another include is already scheduled; Parse_Input() did not 
	// return as it should do. 
	assert(include.scheduled==false);
	
	UInput_Stream *is=new UInput_Stream(file);
	incpath.try_open(is,
		search_current_path,last_ifs->is->Path(),
		honor_search_path);
	
	if(!is->IsOpen())
	{
		delete is;
		return(1);   // could not open file (not found) 
	}
	
	UInput_Stream *lis=_Input_File_In_List(is);
	if(lis)
	{
		delete is;  // don't need this one
		
		int lineno=-1;
		if(offset>=last_ifs->off)
		{  lineno = last_ifs->line0 + 
			mem_cnt(curr_ib->buf,offset-last_ifs->off,'\n');  }
		
		if(_Input_File_In_Hierarchy(lis))
		{
			Error_Header(error(),lineno) << 
				"detected include recursion loop: " << 
				lis->Path();
			return(2);
		}
		
		++lis->use_cnt;  // increase use counter...
		is=lis;          // ...and use it again 
		assert(!is->IsOpen());
		if(!is->open(/*print_error*/true))
		{
			Error_Header(error(),lineno) << "  failed to include the file for "
				"the second time. Are you doing nasty things here?";
			return(3);  // opening failed; error written
		}
		
		if(warn_multiple_include)
		{  Error_Header(error(),lineno) << "warning: " << (lis->Path()) << 
			" included for the " << (lis->use_cnt) << 
			Num_Postfix(lis->use_cnt) << 
			" time. Expect trouble.";  }
	}
	
	include.is=is;
	include.offset=offset;
	include.inclen=inclen;
	include.scheduled=true;
	// Gets added to the infile list by _Recursive_Read() if use_cnt==1. 
	
	return(0);  // success
}


// Returns the InFile_Segment with the depth (ifs->depth-1) or 
// NULL if ifs->depth=0. Uses last_ifs if ifs=NULL. 
Recursive_Input_Stream::InFile_Segment 
	*Recursive_Input_Stream::_Prev_Depth(InFile_Segment *ifs)
{
	if(!ifs)  ifs=last_ifs;
	if(!ifs)  return(NULL);
	int startdepth=ifs->depth;
	if(startdepth<1)  return(NULL);
	for(; ifs; ifs=ifs->prev)
	{
		if(ifs->depth<startdepth)
		{
			assert(ifs->depth==startdepth-1);
			return(ifs);
		}
	}
	assert(0);
	return(NULL);  // should never happen 
}


// Print include file hierarchy; writes: 
// "file3 included from 
//   file2:17 included from
//   file1:48 included from
//   file0:2"
std::string Recursive_Input_Stream::Include_Hierarchy_String()
{
	InFile_Segment *i=last_ifs;
	std::string hs="";
	if(!i)  return(hs);
	hs+=i->is->Path();
	char tmp_line[32];
	for(i=_Prev_Depth(i); i; i=_Prev_Depth(i))
	{
		hs+=" included from\n  ";
		hs+=i->is->Path();
		hs+=":";
		snprintf(tmp_line,32,"%d",(i->line0+i->lines));
		hs+=tmp_line;
	}
	return(hs);
}


message::Message_Stream Recursive_Input_Stream::Error_Header(
	message::Message_Stream os,
	int line,bool force_print_hierarchy)
{
	if(!last_ifs)
		return(os);
	
	bool pr_hr=force_print_hierarchy;  // print hierarchy?
	if(!pr_hr)
	{
		if(!h_p_ifs)
		{  pr_hr=true;  }
		else if(h_p_ifs->depth!=last_ifs->depth)
		{  pr_hr=true;  }
		else
		{
			for(InFile_Segment *i=h_p_ifs; i; i=i->next)
			{
				if(i->depth < h_p_ifs->depth)
				{  pr_hr=true;  break;  }
			}
		}
	}
	
	if(pr_hr)
	{
		if(last_ifs->depth>0)
		{
			os << "In " << Include_Hierarchy_String();
			h_p_ifs=last_ifs;
		}
		else  // do not print hierarchy for depth=0. 
		{  h_p_ifs=NULL;  }
	}
	
	char line_tmp[32];
	if(line<0)  {  strcpy(line_tmp,"???");  }
	else        {  snprintf(line_tmp,32,"%d",line);  }
	os << last_ifs->is->Path() << ":" << line_tmp << ": " << 
		message::noend;
	
	return(os);
}


// Returns 0 if they have identical paths. 
int Path_Cmp(Input_Stream *isA,Input_Stream *isB)
{
	// quick test...
	if(isA->Path() == isB->Path())
	{  return(0);  }
	
	// More could be done here. 
	// Just the most obvious: skip leading "./"
	const char *pathA=isA->CPath();
	while(!strncmp(pathA,"./",2))  pathA+=2;
	const char *pathB=isB->CPath();
	while(!strncmp(pathB,"./",2))  pathB+=2;
	
	if(!strcmp(pathA,pathB))
	{  return(0);  }
	
	#warning we see "file" and "../dir/file" as different files. 
	
	return(1);
}


Recursive_Input_Stream::UInput_Stream 
	*Recursive_Input_Stream::_Input_File_In_List(UInput_Stream *is)
{
	for(std::list<UInput_Stream*>::const_iterator i=infiles.begin();
		i!=infiles.end(); i++)
	{
		if( !Path_Cmp((*i),is) )
		{  return(*i);  }
	}
	return(NULL);
}


bool Recursive_Input_Stream::_Input_File_In_Hierarchy(UInput_Stream *is)
{
	// Starting with depth last_ifs->depth. 
	for(InFile_Segment *i=last_ifs; i; i=_Prev_Depth(i))
	{
		if( !Path_Cmp(i->is,is) )
		{  return(true);  }
	}
	return(false);
}


void Recursive_Input_Stream::_Append_IFS(InFile_Segment *ifs)
{
	assert(last_ifs);
	assert(!last_ifs->next);
	last_ifs->next=ifs;
	ifs->prev=last_ifs;
	last_ifs=ifs;
}


void Recursive_Input_Stream::Set_IBuf_Size(size_t size)
{
	ibuf_default_len=size;
	if(curr_ib)  // resize the currently used input buffer (if any) 
	{  curr_ib->Resize(size);  }
}


Recursive_Input_Stream::InFile_Segment::~InFile_Segment()
{
	// dequeues itself: 
	if(prev)  {  prev->next=next;  }
	if(next)  {  next->prev=prev;  }
	// Be sure...
	prev=NULL;
	next=NULL;
	is=NULL;
}


void Recursive_Input_Stream::Input_Buffer::Skip(size_t slen)
{
	if(slen>=len)
	{  len=0;  return;  }
	len-=slen;
	memmove(buf,&buf[slen],len);
}

// Returns 1 if eof comes close and 2 if ibuflen=0 due to eof, 
// returns 0 otherwise. 
int Recursive_Input_Stream::Input_Buffer::FillUp(UInput_Stream *is,
	size_t *size_inc)
{
	assert(len<=size);
	
	size_t need=size-len;
	if(need)
	{
		assert(is->IsOpen());
		size_t rd=is->read(&buf[len],need);
		len+=rd;
		if(size_inc)  *size_inc+=rd;
		if(!len)  return(2);
		if(rd<need)  return(1);
	}
	return(0);
}

void Recursive_Input_Stream::Input_Buffer::Resize(size_t newsize)
{
	if(newsize<len)  return;
	char *tmp=new char[newsize];
	memcpy(tmp,buf,len);  // <- reason why we do not make ibuf smaller 
	delete[] buf;
	buf=tmp;
	size=newsize;
}

Recursive_Input_Stream::Input_Buffer::Input_Buffer(size_t _size)
{
	len=0;
	size=_size;
	buf=new char[size];
}

Recursive_Input_Stream::Input_Buffer::~Input_Buffer()
{
	if(buf)  delete[] buf;
	size=0;  len=0;   // be sure...
}


Recursive_Input_Stream::Recursive_Input_Stream(message::Message_Consultant *mcons) : 
	message::Message_Reporter(mcons)
{
	ibuf_default_len=4096;
	curr_ib=NULL;
	tot_read_bytes=0;
	
	include.scheduled=false;
	include.is=NULL;
	abort_scheduled=false;
	
	last_ifs=NULL;
	
	cr.curr=NULL;
	h_p_ifs=NULL;
	warn_multiple_include=true;
}


void Recursive_Input_Stream::_Reset()
{
	// Tidy up input file segments
	while(start.next)
	{
		InFile_Segment *tmp=start.next;
		start.next=start.next->next;
		delete tmp;
	}
	start.prev=NULL;  // be sure...
	last_ifs=NULL;
	
	// Tidy up the input streams
	while( infiles.begin()!=infiles.end() )
	{
		UInput_Stream *is=*(infiles.begin());
		infiles.pop_front();
		delete is;
	}
	
	ibuf_default_len=4096;
	curr_ib=NULL;  // DO NOT delete; this points to a stack frame. 
	
	if(include.is)  delete include.is;
	include.scheduled=false;
	abort_scheduled=false;
	
	cr.curr=NULL;
	h_p_ifs=NULL;
	
	tot_read_bytes=0;
	// May not reset warn_multiple_include here (just as searchpath) 
}


Recursive_Input_Stream::~Recursive_Input_Stream()
{
	_Reset();
}

}  // namespace end
