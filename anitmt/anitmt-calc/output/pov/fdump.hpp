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

#include "val.hpp"
#include "proptree.hpp"
#include "scene.hpp"


namespace output_io
{
namespace POV
{

// To write the frame include files for POV-Ray. 
class Frame_Dump
{
	public:
		enum NType
		{ NT_Scalar,NT_Object };
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
			
			int verbose;
			ostream *vout;
			
			int nscalars;
			int nobjects,active_objects;
			int undefined_scalars;
			int undefined_objects;
			
			Context(double t,int ndigits,int _verbose,ostream &_vout);
			~Context() { }
		};
		struct Node 
		{
			Node *next;
			NType type;
			Dump_Flags flags;
			void *ptn;  // anitmt::Prop_Tree_Node
			char *str;  // scalar: name; object: identifier
			
			inline char *write(char *dest,Context *ctx);
			
			Node(NType type,void *ptn,Dump_Flags flags,unsigned int id);
			~Node();
			
			private:
			char *_Alloc_Scalar_Str(anitmt::Ani_Scalar *ptn);
			char *_Alloc_Object_Str(anitmt::Ani_Object *ptn,unsigned int id);
			char *_Dump_Scalar2Str(char *d,Context *ctx);
			char *_Dump_Object2Str(char *d,Context *ctx);
		};
	private:
		char *buf;      // write buffer
		size_t bufsize;
		
		Node *first,*last;
		
		char *_Write_Header(char *dest,values::Scalar t,int frame);
		
		int verbose;
		ostream *_vout;  // verbose stream
		ostream &vout()  {  return(*_vout);  }
	public:
		Frame_Dump();
		~Frame_Dump();
		
		// Set verbosity level and verbose stream. 
		void Set_Verbose(int verbose,ostream &vout);
		
		// Add an entry to the list; entries are written to the file in 
		// the order which they are added here. 
		// id: value that makes the generated identifiers unique; not 
		//     needed for NT_Scalar. 
		// dump_flags: what shall be dumped (pass 0 for scalars); 
		//    see enum Dump_Flags above. 
		void Add_Entry(NType type,anitmt::Prop_Tree_Node *ptn,
			Dump_Flags dump_flags,unsigned int id=0);
		
		// Delete all entries. 
		void Clear();
		
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
