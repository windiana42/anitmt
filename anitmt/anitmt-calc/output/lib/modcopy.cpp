/*
 * modcopy.cpp
 * Copy files while modifying them. 
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

#include "modcopy.hpp"

#include <assert.h>

#include "binstr.hpp"
#include "boutstr.hpp"
#include "recinstr.hpp"

#include "htime.h"

//#include <iostream>
//#include <string>

namespace output_io
{


void Modification_Copy::_Copy_TillEOF(Input_Stream *inf,
	Output_Stream *outf,CopyBuffer *buf)
{
	// This function could probably be made faster with less (!) 
	// buffering as we may choose the nicest IO sizes when copying 
	// as we like to. 
	// However, if you just copy a file beginning with some well-aligned 
	// position (e.g. 0) in both input and output and they both have the 
	// same buffer size (=preferred IO size; e.g. on same filesystem) 
	// then the copying is as fast as it could be because the read() 
	// and write() calls from Buffered_Input/Output_Stream will 
	// directly go throuh to read(2), write(2) without messing ariund 
	// with any buffer. 
	for(;;)
	{
		buf->len=inf->read(buf->buf,buf->size);
		if(!buf->len)  break;
		outf->write(buf->buf,buf->len);
		if(buf->len<buf->size)  break;
	}
}


// Copy until reaching endipos: 
// return value: 
//    0 -> OK
//   -1 -> unexpected EOF
//   +1 -> already further down in input
int Modification_Copy::_Copy_Till(Input_Stream *inf,
	Output_Stream *outf,CopyBuffer *buf,size_t endipos)
{
	size_t ipos=inf->Off();
	if(ipos>endipos)
	{  return(+1);  }
	size_t tocopy=endipos-ipos;
	while(tocopy)
	{
		size_t want=tocopy;
		if(want>buf->size)
		{  want=buf->size;  }
		buf->len=inf->read(buf->buf,want);
		if(buf->len<want)
		{
			error() << "Unexpected EOF in " << inf->CPath() << 
				" at position " << inf->Off();
			return(-1);
		}
		outf->write(buf->buf,buf->len);
		tocopy-=buf->len;  // (buf->len==want)
	}
	return(0);
}


int Modification_Copy::_Skip_Bytes(Buffered_Input_Stream *inf,size_t nbytes)
{
	if(inf->skip(nbytes)<nbytes)
	{
		error() << "Unexpected EOF in " << inf->CPath() << 
			" at position " << inf->Off();
		return(1);
	}
	return(0);
}


int Modification_Copy::_Copy_File(const std::string &output_path,CopyFile *cf)
{
	// Copy the passed file. 
	
	#if CALC_ELAPSED_TIME
		HTime starttime;
		starttime.SetCurr();
	#endif
	
	Buffered_Input_Stream inf(cf->path);
	if(!inf.open(true))
	{  return(1);  }
	
	const char *basename=strrchr(cf->path.c_str(),'/');
	if(basename)  ++basename;
	else  basename=cf->path.c_str();
	bool need_slash=true;
	if(output_path.length()<1)  need_slash=false;
	else if(output_path[output_path.length()-1]=='/')  need_slash=false;
	Buffered_Output_Stream outf(std::string(
		output_path + (need_slash ? "/" : "") + basename));
	if(!outf.open(/*allow_overwrite=*/true))
	{  return(1);  }
	
	verbose(1) << "Copying \"" << inf.CPath() << "\" --> \"" << 
		outf.CPath() << "\"..." << message::noend;
	
	// The performance does not depend hightly upon the buffer size here 
	// but I want to make sure not to use a size which is not too far 
	// from the actual preferred IO size. 
	CopyBuffer buf(
		(inf.Get_Buf_Size() > outf.Get_Buf_Size()) ? 
		 inf.Get_Buf_Size() : outf.Get_Buf_Size() );
	int errors=0;
	int ninsert=0,ndelete=0;
	for(ModNode *n=cf->mod; n; n=n->next)
	{
		if(verbose_level()>3)  // yes...
		{
			verbose(4) << "Next action: " << message::noend;
			if(n->action==MC_Delete)
			{  verbose(4) << "delete " << n->len << " bytes at " << 
				n->pos << ": " << message::noend;  }
			else if(n->action==MC_Insert)
			{  verbose(4) << "insert " << n->len << " bytes at " << 
				n->pos << ": " << message::noend;  }
		}
		int ctv=_Copy_Till(&inf,&outf,&buf,n->pos);
		if(ctv<0)
		{  ++errors;  break;  }
		switch(n->action)
		{
			case MC_Delete:
			{
				size_t skiplen=n->len;
				if(ctv==1)   // overlapping
				{
					size_t nend=n->pos+n->len;
					size_t ipos=inf.Off();
					if(nend<ipos)  skiplen=0;
					else  skiplen=nend-ipos;
				}
				if(verbose_level()>3)
				{  verbose(4) << "skip=" << skiplen << 
					((ctv==1) ? " [overlapping]" : "") << message::noend;  }
				if(_Skip_Bytes(&inf,skiplen))
				{  ++errors;  break;  }
				++ndelete;
			}  break;
			case MC_Insert:
				// This assertion may fail; If this is the case, I have 
				// to decide how to deal with an overlapping deletion/insertion. 
				assert(ctv==0);
				outf.write(n->txt,n->len);
				++ninsert;
				break;
			case _MC_None:  // fall through
			default:
				error() << "Illegal action " << n->action;
				abort();
		}
		if(verbose_level()>3)
		{  verbose(4) << "" /*newline*/;  }
	}
	if(!errors)
	{  _Copy_TillEOF(&inf,&outf,&buf);  }
	
	// Must be queried before closeing them: 
	size_t _inf_bs=inf.Get_Buf_Size();
	size_t _outf_bs=outf.Get_Buf_Size();
	size_t inbytes=inf.Off();
	size_t outbytes=outf.Off();
	
	inf.close();
	outf.close();  // flush buffer 
	
	#if CALC_ELAPSED_TIME
		double elapsed=starttime.ElapsedD(HTime::seconds);
	#endif
	
	if(verbose_level())
	{
		verbose(1) << (errors ? "FAILED" : "done.");
		verbose(1) << "  bytes: " << inbytes << " --> " << outbytes << 
				" (deletions: " << ndelete << 
				"; insertions: " << ninsert << ")";
		verbose(2) << "  buffers: in " << _inf_bs << "; out " << _outf_bs << 
			"; transmit " << buf.size;
		
		#if CALC_ELAPSED_TIME
		size_t nbytes=inbytes+outbytes;
		verbose(1) << "  Input+Output: " << Throughput_Str(nbytes,elapsed);
		#endif
	}
	
	return(errors);
}


int Modification_Copy::DoCopy(const std::string &output_path)
{
	// Copy files one-by-one
	int errors=0;
	for(CopyFile *cf=cfirst; cf; cf=cf->next)
	{
		errors+=_Copy_File(output_path,cf);
	}
	return(errors);
}


int Modification_Copy::DoCopy(class Recursive_Input_Stream *ris,
	const std::string &output_file,
	const std::string &output_path)
{
	int errors=0;
	
	// Reset cused values so that we can see which file got copied. 
	for(CopyFile *cf=cfirst; cf; cf=cf->next)
	{  cf->cused=false;  }
	
	Buffered_Output_Stream output(output_file);
	if(!output.open())
	{  return(++errors);  }
	
	CopyBuffer buf(4096);
	CopyFile *cf=NULL;
	ModNode *mn=NULL;
	for(;;)
	{
		Recursive_Input_Stream::File_Context fc;
		buf.len=ris->read(buf.buf,buf.size,&fc);
		if(!buf.len)
		{
			if(fc.depth<0)  // special meaning: error occured
			{  ++errors;  break;  }
			// EOF reached. 
			break;
		}
		
		// Okay, we got buf.len bytes from fc.path at position fc.offset. 
		CopyFile *ncf=_Find_FileNode(fc.path);
		if(!ncf)
		{  cf=NULL;  mn=NULL;  }
		else if(ncf!=cf)
		{
			// file changed. 
			cf=ncf;
			mn=cf->mod;
			cf->cused=true;
		}
		if(!(cf ? (cf->mod) : (NULL)))
		{
			// no modifications for this one
			output.write(buf.buf,buf.len);
			continue;
		}
		
		// cf!=NULL and cf->mod!=NULL
		// This loop could be made faster by remembering the last value 
		// of mn for each hierarchy instance. 
		char *src=buf.buf;
		size_t srclen=buf.len;
		size_t endpos=fc.offset+srclen;
		for(; mn; mn=mn->next)
		{
			size_t mn_endpos=mn->pos;
			if(mn->action==MC_Delete)
			{  mn_endpos+=mn->len;  }
			if(mn_endpos<fc.offset)  continue;   // already done
			if(mn->pos>=endpos)  break;   // to be done lateron
			// Okay, action to be done at position mn->pos 
			if(mn->pos>fc.offset)
			{
				size_t tocopy=mn->pos-fc.offset;
				assert(tocopy<=srclen);
				output.write(src,tocopy);
				src+=tocopy;
				srclen-=tocopy;
				fc.offset+=tocopy;
			}
			
			// Do action: 
			switch(mn->action)
			{
				case MC_Delete:
				{
					// Gonna skip it...
					size_t skip_end=mn->pos+mn->len;
					size_t toskip=0;
					if(skip_end>=fc.offset)
					{  toskip=skip_end-fc.offset;  }
					if(srclen>=toskip)
					{
						srclen-=toskip;
						src+=toskip;
						fc.offset+=toskip;
						break;
					}
					else
					{
						src+=srclen;
						fc.offset+=srclen;
						srclen=0;
						break;
					}
				}  break;
				case MC_Insert:
					output.write(mn->txt,mn->len);
					break;
				case _MC_None:  // fall through
				default:
					error() << "Illegal action " << mn->action;
					abort();
			}
			
			if(!srclen)  break;
		}
		
		if(srclen)
		{  output.write(src,srclen);  }
	}
	
	output.close();
	
	// Copy those files not read by the Recursive_Input_Stream: 
	for(CopyFile *cf=cfirst; cf; cf=cf->next)
	{
		if(!cf->cused)
		{  errors+=_Copy_File(output_path,cf);  }
	}
	
	return(errors);
}


Modification_Copy::CopyFile *Modification_Copy::_Find_FileNode(
	const std::string &path)
{
	// This is a linear time search but an algorithm with better 
	// runtime would only be needed for quite a lot of input files. 
	// I move the last found entry to the beginning so that the 
	// most likely query (=same path) next time will be answered 
	// very quickly (=cfirst), and the second likely still quickly 
	// (=cfirst->next), etc. 
	for(CopyFile *cf=cfirst; cf; cf=cf->next)
	{
		if(cf->path == path)
		{
			// Move cf to the beginning: 
			if(cf!=cfirst)
			{
				// Dequeue: 
				if(cf->prev)  {  cf->prev->next=cf->next;  }
				if(cf->next)  {  cf->next->prev=cf->prev;  }
				// Insert: 
				cf->next=cfirst;
				cfirst->prev=cf;
				cfirst=cf;
			}
			return(cf);
		}
	}
	return(NULL);
}


// Search for CopyFile node corresponding to path or 
// allocate a new one if it does not exist: 
Modification_Copy::CopyFile *Modification_Copy::_Ensure_FileNode(
	const std::string &path)
{
	CopyFile *cf=_Find_FileNode(path);
	if(cf)  return(cf);
	
	cf=new CopyFile();
	cf->path.assign(path);
	// Queue at the beginning: 
	cf->next=cfirst;
	if(cfirst)
	{  cfirst->prev=cf;  }
	cfirst=cf;
	
	return(cf);
}


void Modification_Copy::MInsert(const std::string &path,size_t pos,
	const char *txt,ssize_t len=-1)
{
	if(len<0)  len=strlen(txt);
	if(!len)  return;
	
	CopyFile *cf=_Ensure_FileNode(path);
	ModNode *n=_Alloc_ModNode(MC_Insert,pos,txt,len);
	
	_Queue_ModNode(cf,n);
}


void Modification_Copy::MDelete(const std::string &path,size_t pos,
	size_t len)
{
	if(!len)  return;
	
	CopyFile *cf=_Ensure_FileNode(path);
	ModNode *n=_Alloc_ModNode(MC_Delete,pos,NULL,len);
	
	_Queue_ModNode(cf,n);
}


void Modification_Copy::MFile(const std::string &path)
{
	_Ensure_FileNode(path);
	// That's all. 
}


// Queues the ModNode in cf where it belongs 
void Modification_Copy::_Queue_ModNode(CopyFile *cf,ModNode *n)
{
	// We run last-to-first as the insertion/deletion is most likely 
	// at the end as files are normally read downwards. 
	ModNode *i=cf->lastmod;
	for(; i; i=i->prev)
	{  if(n->pos>=i->pos)  break;  }
	if(i)
	{
		// Insert node n after i 
		n->next=i->next;
		if(i->next)  i->next->prev=n;
		i->next=n;
		n->prev=i;
		if(i==cf->lastmod)
		{  cf->lastmod=n;  }
	}
	else
	{
		// Insert node n at the beginning of the list
		n->next=cf->mod;
		if(cf->mod)
		{  cf->mod->prev=n;  }
		cf->mod=n;
		if(!cf->lastmod)
		{  cf->lastmod=n;  }
	}
}


Modification_Copy::ModNode *Modification_Copy::_Alloc_ModNode(
	MCAction act,size_t pos,const char *txt,size_t len)
{
	ModNode *n=new ModNode();
	n->action=act;
	n->pos=pos;
	if(txt)
	{
		n->txt=new char[len];
		memcpy(n->txt,txt,len);
	}
	n->len=len;
	return(n);
}


void Modification_Copy::_Reset()
{
	while(cfirst)
	{
		CopyFile *tmp=cfirst;
		cfirst=cfirst->next;
		delete tmp;  // clears up all the ModNodes
	}
}


Modification_Copy::Modification_Copy(message::Message_Consultant *mcons) : 
	message::Message_Reporter(mcons)
{
	cfirst=NULL;
}


Modification_Copy::~Modification_Copy()
{
	_Reset();
}


/******************************************************************************/

Modification_Copy::ModNode::ModNode()
{
	action=_MC_None;
	prev=NULL; next=NULL;
	pos=0;
	txt=NULL;
	len=0;
}

Modification_Copy::ModNode::~ModNode()
{
	if(txt)  delete[] txt;
}


void Modification_Copy::CopyFile::Clear()
{
	while(mod)
	{
		ModNode *tmp=mod;
		mod=mod->next;
		delete tmp;
	}
	lastmod=NULL;
}

Modification_Copy::CopyFile::CopyFile()
{
	prev=NULL;  next=NULL;
	mod=NULL;  lastmod=NULL;
	cused=false;
}

}  // namespace end 


#if 0
	// Now, do the adjustment: 
	// NOTE: Overlapping insertions/deletion is not supported and will 
	//       not be detected. 
	//       You don't need them probably (e.g. insert something 
	//       at a posotion inside another insertion?!). 
	switch(n->action)
	{
		case MC_Delete:
		{
			size_t delta=n->len;
			for(i=n->next; i; i=i->next)
			{
				assert(i->pos>=delta);
				i->pos-=delta;
			}
		} break;
		case MC_Insert:
		{
			size_t delta=n->len;
			for(i=n->next; i; i=i->next)
			{  i->pos+=delta;  }
		} break;
		case _MC_None:  // fall through
		default:
			error() << "Illegal action " << n->action;
			abort();
	}
#endif
