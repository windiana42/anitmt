/*
 * fdump.cpp
 * 
 * Write frame include files (quickly). 
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
 *   May 2001   started writing
 *
 */

#include "fdump.hpp"
#include "outstr.hpp"
#include "convtoa.hpp"

//#include "val.hpp"
#include "tmttype.hpp"
#include "animation.hpp"
#include "scene.hpp"
//#include "proptree.hpp"

#include <iostream>

#include "htime.h"

// Bottom limit for write size (buffer will be twice as large) 
#define MinWriteSize 4096


namespace 
{
	// Don't change the length of these strings too much!!
	const char *_declare_str="#declare";
	const int _declare_str_len=strlen(_declare_str);
	
	const char *_undefined_str="undefined";
	const char *_undefined_cmt_str="  // undefined!";
	
	const char *_active_str="active";
	const char *_trans_str="trans";
	const char *_scale_str="scale";
	const char *_rot_str="rot";
}


namespace output_io
{
namespace POV
{

using namespace anitmt;


// To be exported to other classes: 
// Writes _aniTMT_xxx_yyy to d and returns pointer to next free char 
// (`\0'-terminated, yes, really). 
// xxx is the id passed and yyy is determined by type: 
// `a' -> active; `t' -> translation; `r' -> rotation; `s' -> scale 
// With 128 bytes you are on the safe side. 
char *aniTMT_identifier(char *d,char type,unsigned int id)
{
	d=ObjectIdent(d,id);
	const char *src=NULL;
	switch(type)
	{
		case 'a':  src=_active_str;  break;
		case 't':  src=_trans_str;   break;
		case 's':  src=_scale_str;   break;
		case 'r':  src=_rot_str;     break;
		default:
			assert(0);
			*(d++)='?'; *(d++)='?'; *(d++)='?'; *d='\0';
			return(d);
	}
	for(; *src; src++)
	{  *(d++)=*src;  }
	*d='\0';
	return(d);
}


char *Frame_Dump::Node::_Dump_Scalar2Str(char *d,char *dend,Frame_Dump::Context *ctx)
{
	// Check if the string fits into the buffer: 
	// ``#declare name=1.222e333;\n''
	if((dend-d)<(ctx->ndigits+_declare_str_len+str_len+48))
	{  std::cerr << "Internal error in fdump.cpp:" << __LINE__ << std::endl;  abort();  }
	
	++ctx->nscalars;
	for(char *s=str; *s; s++)
	{  *(d++)=*s;  }
	Ani_Scalar *scl=(Ani_Scalar*)ptn;
	std::pair<bool,Scalar_State> ret=scl->get_return_value(ctx->t);
	
	bool defined=ret.first;
	if(ctx->verbose>3)  // YES!
	{  (*(ctx->vout)) << "scalar: " << scl->get_name() << 
		(defined ? "" : "UNDEFINED") << std::endl;  }
	
	if(defined)
	{  d=Double2Str(d,ret.second.get_value(),ctx->ndigits);  }
	else
	{
		for(const char *s=_undefined_str; *s; s++)
		{  *(d++)=*s;  }
		++ctx->undefined_scalars;
	}
	*(d++)='\n';
	return(d);
}

char *Frame_Dump::Node::_Dump_Object2Str(char *d,char *dend,Frame_Dump::Context *ctx)
{
	/* The object stuff simply HAS to fit into this buffer.        */
	/* There is so much room, the aniTMT identifers are <=64 bytes */
	/* and the digits are limited to 64 (too much anyway) as well  */
	/* so we will never run into trouble here.                     */
	if((dend-d)<((64+64+32)*3+5+64+32))  /* =581 */
	{  std::cerr << "Internal error in fdump.cpp:" << __LINE__ << std::endl;  abort();  }
	
	++ctx->nobjects;
	
	Ani_Object *obj=(Ani_Object*)ptn;
	std::pair<bool,Object_State> ret=
		obj->get_return_value(ctx->t);
	
	bool defined=ret.first;
	bool active=ret.second.is_active();
	
	if(ctx->verbose>3)  // YES!
	{  (*ctx->vout) << "object: " << obj->get_name() << 
		(active ? " [active]" : "") << 
		(defined ? "" : " [UNDEFINED]") << std::endl;  }
	
	if(!defined)
	{
		++ctx->undefined_objects;
		active=false;  // I output undefined objects as inactive. 
	}
	
	enum Dump_Flags scale_flag=DF_Obj_Scale;
	enum Dump_Flags rot_flag=  DF_Obj_Rot;
	enum Dump_Flags trans_flag=DF_Obj_Trans;
	if(active)
	{
		++ctx->active_objects;
		(int)scale_flag|=DF_Obj_AScale;
		(int)rot_flag|=  DF_Obj_ARot;
		(int)trans_flag|=DF_Obj_ATrans;
	}
	
	if(flags & DF_Obj_Active)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_active_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		// ``#declare _aniTMT_xxx_active=''
		
		// I output undefined objects as inactive. 
		if(!defined)
		{
			*(d++)='0';  *(d++)=';';
			for(const char *s=_undefined_cmt_str; *s; s++)
			{  *(d++)=*s;  }
			*(d++)='\n';
		}
		else if(!active)
		{
			*(d++)='0';  *(d++)=';';
			*(d++)='\n';
		}
		else  // active && defined
		{
			*(d++)='1';  *(d++)=';';
			*(d++)='\n';
		}
	}
	
	if(!defined)
	{  return(d);  }  // cannot go on; undefined. 
	
	// Insert scale/rot/trans: 
	#warning write faster versions of get_scale_component(), get_translation_component()
	if(flags & scale_flag)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_scale_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		d=Vector2Str(d,ret.second.get_scale(),ctx->ndigits);
		*(d++)=';';  *(d++)='\n';
	}
	
	if(flags & rot_flag)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_rot_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		d=Vector2Str(d,ret.second.get_rotate(),ctx->ndigits);
		*(d++)=';';  *(d++)='\n';
	}
	
	if(flags & trans_flag)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_trans_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		d=Vector2Str(d,ret.second.get_translate(),ctx->ndigits);
		*(d++)=';';  *(d++)='\n';
	}
	
	return(d);
}


inline char *Frame_Dump::Node::write(char *dest,char *dend,Context *ctx)
{
	switch(type)
	{
		case NT_Scalar:  return(_Dump_Scalar2Str(dest,dend,ctx));
		case NT_Object:  return(_Dump_Object2Str(dest,dend,ctx));
	}
	assert(0);   // actually never happens...
	return(dest);
}


void Frame_Dump::_Write_Header(values::Scalar t,int frame)
{
	size_t needbuf=
		ani->get_name().length()+
		scene->get_name().length()+
		strlen(VERSION)+
		256;
	char tmp[needbuf+2];
	snprintf(tmp,needbuf+1,
		"// File automatically generated by anitmt-%s\n"
		"// Animation: \"%s\";  Scene: \"%s\"\n"
		"// Frame: %6d;  Time: %f\n\n",
		VERSION,
		ani->get_name().c_str(),scene->get_name().c_str(),
		frame,(double)t);
	size_t len=strlen(tmp);
	assert(len<needbuf);
	_Write2Buf(tmp,len);
}


void Frame_Dump::_Write_End()
{
	if(!include_me.length())
	{  return;  }
	
	size_t needbuf=
		include_me.length()+
		32;
	char tmp[needbuf+2];
	snprintf(tmp,needbuf+1,
		"\n#include \"%s\"\n"
		"// eof\n",
		include_me.c_str());
	size_t len=strlen(tmp);
	assert(len<needbuf);
	_Write2Buf(tmp,len);
}


void Frame_Dump::_Write2Buf(const char *str,size_t len)
{
	size_t free=bufend-bufdest;
	
	// fill up the buffer...
	size_t wsize = (free>len) ? len : free;
	memcpy(bufdest,str,wsize);
	bufdest+=wsize;
	str+=wsize;
	len-=wsize;
	_Check_Flush();
	
	if(!len)  return;
	
	assert(bufdest==buf);  // Buffer must be flushed here. 
	wsize=len/(bufhalf-buf);  // (bufhalf-buf) is preferred write size or multiple. 
	if(wsize)
	{
		wsize*=(bufhalf-buf);
		outp->write(str,wsize);
		written_bytes+=wsize;
		str+=wsize;
		len-=wsize;
	}
	
	assert(len<(bufhalf-buf));
	memcpy(bufdest,str,len);
	bufdest+=len;
}


inline void Frame_Dump::_Check_Flush()
{
	if(bufdest>=bufend)
	{
		outp->write(buf,bufsize);
		written_bytes+=bufsize;
		bufdest=buf;
	}
	if(bufdest>=bufhalf)
	{
		size_t wsize=bufhalf-buf;
		outp->write(buf,wsize);
		written_bytes+=wsize;
		size_t dirty=bufdest-bufhalf;
		if(dirty)
		{  memcpy(buf,bufdest,dirty);  }
		bufdest=buf+dirty;
	}
}


inline void Frame_Dump::_Force_Flush()
{
	if(bufdest>buf)
	{
		size_t wsize=bufdest-buf;
		outp->write(buf,wsize);
		written_bytes+=wsize;
		bufdest=buf;
	}
}


int Frame_Dump::Write(const std::string &file,values::Scalar t,int frame)
{
	// UNBUFFERED; that is important as WE do the buffering HERE. 
	Output_Stream outp(file);
	if(!outp.open(/*print_error=*/true))
	{  return(1);  }
	
	this->outp=&outp;
	
	size_t wsize=outp.Preferred_IO_Size();
	while(wsize<2*(longest_identifier_len+128))  // 128 for `#declare' and value. 
	{  wsize*=2;  }
	while(wsize<MinWriteSize)
	{  wsize*=2;  }
	
	size_t bsize=wsize*2;
	if(bsize!=bufsize)  // Re-alloc buffer only if necessary. 
	{
		delete[] buf;
		bufsize=bsize;
		buf=new char[bufsize];
	}
	
	bufhalf=buf+wsize;
	bufend=buf+bufsize;
	bufdest=buf;  // buf empty
	written_bytes=0;
	
	if(verbose)
	{
		vout() << "  Writing \"" << file << "\" " <<
			"Time: " << t << "; Frame: " << frame;
		if(verbose>2)
		{  vout() << std::endl << 
			"    (buffer: " << bufsize << " bytes; "
			"longest identifier: " << longest_identifier_len << " bytes)";  }
		vout() << " ...";
		vout().flush();
	}
	
	#warning get ndigits value (precsison)
	int ndigits=6;
	if(ndigits>64)   // necessary avoiding buffer overflows 
	{  ndigits=64;  }
	
	#if CALC_ELAPSED_TIME
		HTime starttime;
		starttime.SetCurr();
	#endif
	
	_Write_Header(t,frame);
	Context ctx(t,ndigits,verbose,vout());
	for(Node *n=first; n; n=n->next)
	{
		_Check_Flush();
		bufdest=n->write(bufdest,bufend,&ctx);
	}
	_Write_End();
	_Force_Flush();
	
	outp.close();
	this->outp=NULL;
	
	#if CALC_ELAPSED_TIME
		double elapsed=starttime.ElapsedD(HTime::seconds);
	#endif
	
	if(verbose)
	{
		vout() << "done (" << written_bytes << " bytes)." << std::endl;
		if(verbose>1)
		{  vout() << "  Output: " << Throughput_Str(written_bytes,elapsed) << 
			std::endl;  }
		if(verbose>2)
		{
			vout() << "  Dumped: scalars: " << ctx.nscalars << " ("	<< 
				ctx.undefined_scalars << " undefined)" << std::endl;
			vout() << "          objects: " << ctx.nobjects << " (" << 
				ctx.active_objects << " active; " <<
				ctx.undefined_objects << " undefined)" << std::endl;
		}
	}
	if(ctx.undefined_scalars || ctx.undefined_objects)
	{
		std::cerr << "  warning: in frame " << frame << ": UNDEFINED: ";
		if(ctx.undefined_scalars)
		{  std::cerr << ctx.undefined_scalars << " scalars; ";  }
		if(ctx.undefined_objects)
		{  std::cerr << ctx.undefined_objects << " objects; ";  }
		std::cerr << std::endl;
	}
	
	return(0);
}


namespace { template<class Dest> void *cast_void(Prop_Tree_Node *ptn)
{
	Dest *d=dynamic_cast<Dest *>(ptn);
	assert(d);
	return(d);
} }

void Frame_Dump::Add_Entry(NType type,Prop_Tree_Node *ptn,Dump_Flags flags,
	unsigned int id)
{
	// Special hack...
	void *ptr=NULL;
	switch(type)
	{
		case NT_Scalar:
		{
			ptr=cast_void<Ani_Scalar>(ptn);
			size_t len=ptn->get_name().length();
			if(len>longest_identifier_len)
			{  longest_identifier_len=len;  }
		}  break;
		case NT_Object:
			ptr=cast_void<Ani_Object>(ptn);
			break;
		// default: ptr stays NULL and will trap the assertion. 
	}
	assert(ptr);
	
	// Tweak flags: 
	if(flags & DF_Obj_Scale)  (int)flags|=DF_Obj_AScale;
	if(flags & DF_Obj_Rot)    (int)flags|=DF_Obj_ARot;
	if(flags & DF_Obj_Trans)  (int)flags|=DF_Obj_ATrans;
	
	Node *n=new Node(type,ptr,flags,id);
	if(last)
	{  last->next=n;  }
	else
	{  first=n;  }
	last=n;
}


static char *NewStr(const char *str)
{
	char *r=new char[strlen(str)+1];
	strcpy(r,str);
	return(r);
}


char *Frame_Dump::Node::_Alloc_Scalar_Str(Ani_Scalar *ptn)
{
	values::String name=ptn->get_name();
	int len=name.length();
	
	// Write ``#declare name='' to str. 
	len+=_declare_str_len+3;
	char *str=new char[len];
	char *end=str;
	for(const char *s=_declare_str; *s; s++)
	{  *(end++)=*s;  }
	*(end++)=' ';
	for(const char *s=name.c_str(); *s; s++)
	{  *(end++)=*s;  }
	*(end++)='=';
	*(end++)='\0';
	
	#warning remove this assert: 
	assert(end == (str+len));
	return(str);
}


char *Frame_Dump::Node::_Alloc_Object_Str(anitmt::Ani_Object *,unsigned int id)
{
	// Write ``#declare _aniTMT_xxx_'' to str. 
	char tmp[128];   // must be enough
	char *end=tmp;
	for(const char *s=_declare_str; *s; s++)
	{  *(end++)=*s;  }
	*(end++)=' ';
	end=ObjectIdent(end,id);
	*(end++)='\0';
	
	return(NewStr(tmp));
}


Frame_Dump::Node::Node(NType _type,void *_ptn,Dump_Flags _flags,unsigned int id)
{
	next=NULL;
	type=_type;
	flags=_flags;
	ptn=_ptn;
	str=NULL;
	str_len=0;
	
	switch(type)
	{
		case NT_Scalar:
			str=_Alloc_Scalar_Str((Ani_Scalar*)ptn);
			str_len=strlen(str);
			break;
		case NT_Object:
			str=_Alloc_Object_Str((Ani_Object*)ptn,id);
			str_len=strlen(str);
			break;
		default:
			std::cerr << "Frame_Dump: Illegal type " << int(type) << "." << std::endl;
			abort();
	}
}


Frame_Dump::Node::~Node()
{
	if(str)  delete str;
	str=NULL;
}


Frame_Dump::Context::Context(double _t,int _ndigits,
	int _verbose,std::ostream &_vout)
{
	t=_t;
	ndigits=_ndigits;
	
	verbose=_verbose;
	vout=&_vout;
	
	nscalars=0;
	nobjects=0;
	active_objects=0;
	undefined_scalars=0;
	undefined_objects=0;
}


void Frame_Dump::Clear(anitmt::Animation *new_ani,anitmt::Ani_Scene *new_scene)
{
	while(first)
	{
		Node *tmp=first;
		first=first->next;
		delete tmp;
	}
	last=NULL;
	longest_identifier_len=0;
	include_me="";
	outp=NULL;
	
	ani=new_ani;
	scene=new_scene;
}


void Frame_Dump::Set_Verbose(int _verbose,std::ostream &vs)
{
	verbose=_verbose;
	_vout=&vs;
}


Frame_Dump::Frame_Dump(anitmt::Animation *_ani,anitmt::Ani_Scene *_scene)
{
	verbose=0;
	_vout=&std::cout;
	
	ani=_ani;
	scene=_scene;
	outp=NULL;
	
	first=NULL;
	last=NULL;
	buf=NULL;
	bufsize=0;
	bufhalf=bufdest=bufend=NULL;
	written_bytes=0;
	
	longest_identifier_len=0;
}

Frame_Dump::~Frame_Dump()
{
	Clear(NULL,NULL);
	
	if(buf)  delete buf;
	buf=NULL;
}


} }  // namespace end 
