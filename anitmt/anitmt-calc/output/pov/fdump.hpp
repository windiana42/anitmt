/*
 * fdump.hpp
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

#ifndef _Inc_IO_FrameDump_H_
#define _Inc_IO_FrameDump_H_ 1

#include <string>

#include <val/val.hpp>

#include "../lib/compif.hpp"
#include "tokid.hpp"


namespace output_io
{

class Output_Stream;

namespace POV
{

// To write the frame include files for POV-Ray. 
class Frame_Dump : 
	public message::Message_Reporter
{
	public:
		enum Dump_Flags
		{
			DF_Obj_Scale= 0x01,  // always needed
			DF_Obj_Rot=   0x02,
			DF_Obj_Trans= 0x04,
			DF_Obj_Mat=   0x07,  // scale+rot+trans
			DF_Obj_AScale=0x10,  // only needed if object active
			DF_Obj_ARot=  0x20,
			DF_Obj_ATrans=0x40,
			DF_Obj_AMat=  0x70,  // active scale+rot+trans
			DF_Obj_Active=0x08   // if(active) itself
			// NOTE: The internal meaning is differently; it gets 
			//       changed by Add_Entry(). 
		};
		struct Context
		{
			double t;
			int ndigits;
			
			message::Message_Reporter *msgrep;
			
			int nscalars;
			int nobjects,active_objects;
			int undefined_scalars;
			int undefined_objects;
			
			Context(double t,int ndigits,message::Message_Reporter *msgrep);
			~Context() { }
		};
		struct Node 
		{
			Node *next;
			Dump_Flags flags;
			ComponentInterface cif;
			char *str;  // scalar: name; object: identifier
			size_t str_len;
			
			inline char *write(char *dest,char *dend,Context *ctx);
			
			Node(const ComponentInterface &cif,Dump_Flags flags,unsigned int id);
			~Node();
			
			private:
			char *_Alloc_Scalar_Str(anitmt::Scalar_Component_Interface *sif);
			char *_Alloc_Object_Str(anitmt::Object_Component_Interface *oif,unsigned int id);
			char *_Dump_Scalar2Str(char *d,char *dend,Context *ctx);
			char *_Dump_Object2Str(char *d,char *dend,Context *ctx);
		};
	private:
		anitmt::Animation *ani;
		anitmt::Scene_Interface scene_if;
		
		char *buf;      // write buffer
		char *bufhalf;
		char *bufdest;  // where to append data
		char *bufend;
		size_t bufsize;  // multiple of peferred write size 
		size_t written_bytes;
		
		Output_Stream *outp;
		
		Node *first,*last;
		size_t longest_identifier_len;
		std::string include_me;
		
		void _Write2Buf(const char *str,size_t len);
		inline void _Check_Flush();
		inline void _Force_Flush();
		
		void _Write_Header(values::Scalar t,int frame);
		void _Write_End(int frame_no);
		
		void _Clear();
	public:
		Frame_Dump(anitmt::Animation *ani,
			anitmt::Scene_Interface &scene_if,
			message::Message_Consultant *m_cons);
		~Frame_Dump();
		
		// include_me is the path to be put into the #include statement 
		// at the end of the frame. 
		void Set_Main_File(const std::string &_include_me)
			{  include_me=_include_me;  }
		
		// Add an entry to the list; entries are written to the file 
		// in the order which they are added here. 
		// oif/sif: allocated (and freed) by higher level 
		//     (e.g. File_Parser). 
		// id: value that makes the generated identifiers unique; not 
		//     needed for NT_Scalar. 
		// dump_flags: what shall be dumped; see enum 
		//     Dump_Flags above. 
		// ComponentInterface copied...
		void Add_Entry(const ComponentInterface &cif,
			Dump_Flags dump_flags,unsigned int id=0);
		
		// Delete all entries. 
		void Clear(anitmt::Animation *ani,
			anitmt::Scene_Interface &scene_if);
		
		// Actually write the file; Time is passed in t and the 
		// frame number in f. 
		// Returns 0 on success and !=0 on failure. 
		int Write(const std::string &file,values::Scalar t,int frame);
};

// Writes _aniTMT_xxx_yyy to d and returns pointer to next free char 
// (`\0'-terminated, yes, really). 
// xxx is the id passed and yyy is determined by type: 
// `a' -> active; `t' -> translation; `r' -> rotation; `s' -> scale 
// With 128 bytes you are on the safe side. 
extern char *aniTMT_identifier(char *d,char type,unsigned int id);

} }  // namespace end 

#endif  /* _Inc_IO_FrameDump_H_ */
