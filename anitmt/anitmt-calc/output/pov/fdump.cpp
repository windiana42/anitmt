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

// This includes the config: 
#include "../lib/lproto.hpp"

#include <iostream>

#include <animation.hpp>

#include "fdump.hpp"
#include "outstr.hpp"
#include "convtoa.hpp"


// Bottom limit for write size (buffer will be twice as large) 
#define MinWriteSize 4096

// FIXME:
// TODO: 
//  - rewrite some parts (just a reminder for me)
//  - #warning (or something) telling you the frame number 
//    (at the end of the code after the #include)
//  - #warning for undefined scalars (and define scalar=0)

namespace 
{
	// Don't change the length of these strings too much!!
	const char *_declare_str="#declare";
	const int _declare_str_len=strlen(_declare_str);
	
	const char *_undefined_str="undefined";
	const char *_undefined_cmt_str="  // undefined!";  // FIXME: not needed any more?
	const char *_warning_scal_str="#warning \"Scalar";
	const char *_is_undefined_str="is undefined\"";
	
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
	
	Scalar_Component_Interface *scl=cif.CVScalar();
	std::pair<bool,values::Scalar> ret=scl->get_value(ctx->t);
	
	bool defined=ret.first;
	if(ctx->msgrep->verbose_level()>3)  // YES!
	{  ctx->msgrep->verbose(4) << "scalar: " << scl->get_name() << 
		(defined ? "" : "UNDEFINED");  }
	
	++ctx->nscalars;
	if(defined)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		d=Double2Str(d,ret.second,ctx->ndigits);
		*(d++)=';';
	}
	else
	{
		for(const char *s=_warning_scal_str; *s; s++)
		{  *(d++)=*s;  }
		for(char *s=str+8; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_is_undefined_str; *s; s++)
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
	
	Object_Component_Interface *obj=cif.CVObject();
	std::pair<bool,Object_State> ret=
		obj->get_state(ctx->t);
	
	bool defined=ret.first;
	// (FIXME) If there was an is_active member, then use 
	// active=ret.second.is_active; instead of
	// active=ret.first; 
	bool active=ret.first;  //ret.second.is_active;
	
	if(ctx->msgrep->verbose_level()>3)  // YES!
	{  ctx->msgrep->verbose(4) << "object: " << obj->get_name() << 
		(active ? " [active]" : "") << 
		(defined ? "" : " [UNDEFINED]");  }
	
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
	if(flags & scale_flag)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_scale_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		d=Vector2Str(d,vect::get_scale_component(ret.second.matrix),ctx->ndigits);
		*(d++)=';';  *(d++)='\n';
	}
	
	if(flags & rot_flag)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_rot_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		d=Vector2Str(d,ret.second.rotate,ctx->ndigits);
		*(d++)=';';  *(d++)='\n';
	}
	
	if(flags & trans_flag)
	{
		for(char *s=str; *s; s++)
		{  *(d++)=*s;  }
		for(const char *s=_trans_str; *s; s++)
		{  *(d++)=*s;  }
		*(d++)='=';
		d=Vector2Str(d,ret.second.translate,ctx->ndigits);
		*(d++)=';';  *(d++)='\n';
	}
	
	return(d);
}


inline char *Frame_Dump::Node::write(char *dest,char *dend,Context *ctx)
{
	switch(cif.GetType())
	{
		case ComponentInterface::IFScalar:
			return(_Dump_Scalar2Str(dest,dend,ctx));
		case ComponentInterface::IFObject:
			return(_Dump_Object2Str(dest,dend,ctx));
	}
	assert(0);   // actually never happens...
	return(dest);
}


void Frame_Dump::_Write_Header(values::Scalar t,int frame)
{
	size_t needbuf=
		ani->get_name().length()+
		scene_if.get_name().length()+
		strlen(VERSION)+
		256;
	char tmp[needbuf+2];
	snprintf(tmp,needbuf+1,
		"// File automatically generated by anitmt-%s\n"
		"// Animation: \"%s\";  Scene: \"%s\"\n"
		"// Frame: %6d;  Time: %f\n\n",
		VERSION,
		ani->get_name().c_str(),scene_if.get_name().c_str(),
		frame,(double)t);
	size_t len=strlen(tmp);
	assert(len<needbuf);
	_Write2Buf(tmp,len);
}


void Frame_Dump::_Write_End(int frame_no)
{
	if(!include_me.length())
	{  return;  }
	
	size_t needbuf=
		include_me.length()+
		32+256;
	char tmp[needbuf+2];
	snprintf(tmp,needbuf+1,
		"\n#include \"%s\"\n"
		"#render \"Frame %d parsed.\"\n"
		"// eof\n",
		include_me.c_str(),frame_no);
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
	
	if(verbose_level())
	{
		verbose(1) << "  Writing \"" << file << "\" " <<
				"Time: " << t << "; Frame: " << frame << message::noend;
		verbose(2) << "";  // <-- newline
		verbose(2) << "      (buffer: " << bufsize << " bytes; "
				"longest identifier: " << longest_identifier_len << " bytes)" << 
				message::noend;
		verbose(1) << " ..." << message::noend;
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
	Context ctx(t,ndigits,this);
	for(Node *n=first; n; n=n->next)
	{
		_Check_Flush();
		bufdest=n->write(bufdest,bufend,&ctx);
	}
	_Write_End(frame);
	_Force_Flush();
	
	outp.close();
	this->outp=NULL;
	
	#if CALC_ELAPSED_TIME
		double elapsed=starttime.ElapsedD(HTime::seconds);
	#endif
	
	if(verbose_level())
	{
		verbose(1) << "done (" << written_bytes << " bytes).";
		verbose(2) << "  Output: " << Throughput_Str(written_bytes,elapsed);
		verbose(3) << "  Dumped: scalars: " << ctx.nscalars << " ("	<< 
				ctx.undefined_scalars << " undefined)";
		verbose(3) << "          objects: " << ctx.nobjects << " (" << 
				ctx.active_objects << " active; " <<
				ctx.undefined_objects << " undefined)";
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


void Frame_Dump::Add_Entry(const ComponentInterface &cif,
	Dump_Flags flags,unsigned int id)
{
	if(cif.GetType()==ComponentInterface::IFScalar)
	{
		size_t len=cif.get_name().length();
		if(len>longest_identifier_len)
		{  longest_identifier_len=len;  }
	}
	
	// Tweak flags: 
	if(flags & DF_Obj_Scale)  (int)flags|=DF_Obj_AScale;
	if(flags & DF_Obj_Rot)    (int)flags|=DF_Obj_ARot;
	if(flags & DF_Obj_Trans)  (int)flags|=DF_Obj_ATrans;
	
	Node *n=new Node(cif,flags,id);
	(last ? last->next : first) = n;
	last=n;
}

static char *NewStr(const char *str)
{
	char *r=new char[strlen(str)+1];
	strcpy(r,str);
	return(r);
}


char *Frame_Dump::Node::_Alloc_Scalar_Str(
	Scalar_Component_Interface *sif)
{
	values::String name=sif->get_name();
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


char *Frame_Dump::Node::_Alloc_Object_Str(
	Object_Component_Interface *,unsigned int id)
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


Frame_Dump::Node::Node(const ComponentInterface &_cif,Dump_Flags _flags,unsigned int id)
{
	next=NULL;
	flags=_flags;
	cif=_cif;
	str=NULL;
	str_len=0;
	
	switch(cif.GetType())
	{
		case ComponentInterface::IFScalar:
			str=_Alloc_Scalar_Str(cif.CVScalar());
			str_len=strlen(str);
			break;
		case ComponentInterface::IFObject:
			str=_Alloc_Object_Str(cif.CVObject(),id);
			str_len=strlen(str);
			break;
		default:  assert(0);  break;
	}
}


Frame_Dump::Node::~Node()
{
	if(str)
	{  delete str;  str=NULL;  }
}


Frame_Dump::Context::Context(double _t,int _ndigits,
	message::Message_Reporter *_msgrep)
{
	t=_t;
	ndigits=_ndigits;
	
	msgrep=_msgrep;
	
	nscalars=0;
	nobjects=0;
	active_objects=0;
	undefined_scalars=0;
	undefined_objects=0;
}


void Frame_Dump::_Clear()
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
}

void Frame_Dump::Clear(anitmt::Animation *new_ani,
	anitmt::Scene_Interface &new_scene_if)
{
	_Clear();
	
	ani=new_ani;
	scene_if=new_scene_if;
}


Frame_Dump::Frame_Dump(
	anitmt::Animation *_ani,
	anitmt::Scene_Interface &_scene_if,
	message::Message_Consultant *m_cons) : 
		message::Message_Reporter(m_cons),
		scene_if(_scene_if)
{
	ani=_ani;
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
	_Clear();
	
	if(buf)
	{  delete buf;  buf=NULL;  }
	
	ani=NULL;
}


} }  // namespace end 
