/*
 * parse.hpp
 * 
 * Central POV-Ray file parsing routines. 
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

#ifndef _Inc_IO_PovFileParse_H_
#define _Inc_IO_PovFileParse_H_ 1

#include <stddef.h>

#include "recinstr.hpp"
#include "modcopy.hpp"
#include "findstr.hpp"
#include "fdump.hpp"

#include "animation.hpp"


namespace output_io
{
namespace POV
{

enum tokID
{
	// *** WHEN ADDING TOKENS, REMEMBER TO TAKE ***
	// *** A LOOK AT File_Parser::Find_Tok().   ***
	// General tokens
	tNone=-1,   // keep that
	tString=0,
	tMaskedString,   // \"
	tCOpComment,
	tCClComment,
	tCppComment,
	t_Comment,   // special for tCOpComment and tCppComment
	tNewline,
	tOpBrace,
	tClBrace,
	tAssign,
	tSemicolon,
	tNumbersign,
	tColon,
	
	// POV-Ray tokens
	tpDeclare,
	tpInclude,
	tpFileIdentifier,
	tpAniInsert,  // insert.obj.xx
	tpAniParse,   // parse.obj.xx
	
	// anitmt tokens
	taObject,
	taScalar   // adjust need_delim() if you add identifiers here 
};

class File_Parser : 
	public Recursive_Input_Stream 
{
	class Sub_Parser_Stack;
	class AValue_List;
	
	struct Sub_Parser;
	class Comment_Parser;
	class String_Parser;
	class Filename_Parser;
	class Include_Parser;
	class Declare_Parser;
	class Object_Parser;
	class Toplevel_Parser;
	
	friend class File_Parser::Sub_Parser;
	friend class File_Parser::Include_Parser;  // for Include_File()
	friend class File_Parser::Comment_Parser;  // for Comment_Cutout()
	friend class File_Parser::Toplevel_Parser; // e.g. for Should_Copy()
	
	public:
		struct Current_Context
		{
			const Parse_Input_Buf *ib;
			char *src;
			size_t srclen;
			size_t fileoff;
			int line;
			bool eof;
			
			size_t last_found_len;   // last valid rv->found_len
			
			// tNewline is not a token for these: 
			bool only_wspace;  // only white space since last tok?
			int with_newline;  // newlines since last tok
			bool _slt_reset;  // internal (reset only_wspace/with_newline)
			
			void reset();
			
			Current_Context()  {  reset();  }
			~Current_Context() { }
		};
		struct Position
		{
			InFile_Segment *handle;
			size_t fileoff;
			int line;
			
			bool unset()  {  return((line<0));  }
			void reset()  {  handle=NULL;  fileoff=0;  line=-1;  }
			Position()    {  reset();  }
			Position(Current_Context *cc)
				{  handle=cc->ib->handle; fileoff=cc->fileoff; line=cc->line;  }
			~Position()   { }
			void operator=(const Current_Context &cc)
				{  handle=cc.ib->handle; fileoff=cc.fileoff; line=cc.line;  }
			bool operator!=(const Current_Context &cc)
				{  return(cc.fileoff!=fileoff || cc.ib->handle!=handle || cc.line!=line);  }
		};
		struct Config
		{
			bool allow_nested_comments;
			bool warn_nested_comments;
			bool allow_multiline_strings;
			int max_multi_line_str_lines;  // max no of lines for multi line strings
			// max length of a string in bytes (0 -> unlimited) 
			size_t max_string_length;
			int search_current_path;       // see Recursive_Input_Stream::Include_File() 
			// What to do if an animated scalar is declared more than once: 
			// 0 -> be quiet; 1 -> warn; 2 -> error 
			int scalar_multiple_decl;
			// 1 -> warn; 2 -> error if a #declaration of a scalar 
			// used by the animation system exceeds one line. 
			// You may choose to use one line for each and thus 
			// be able to detect errors (e.g missing `;'). 
			int multiline_scalar_decl;
			// Copy files which are not source files but textures, etc. 
			// (Those encountered after png,ppm,ttf...; 
			// not the #included ones.)
			bool copy_nonsource_files;
			bool cut_out_comments;
			// This is the default value for the +/- object modifier 
			// +1 -> /* my_object:+[srta] */ (`+' is default)
			// -1 -> /* my_object:-[srta] */ (`-' is default)
			int default_obj_flags_mod;
			int default_obj_flags_mod_E;  // for objects with end statement
			// 0 -> ignore; 1 -> warn; 2 -> error if an object 
			// declaration spreads beyond an EOF boundary. 
			int warn_object_eof_boundary;
			// Max whitespace chars between comment begin and 
			// object flags statement in flags-only comment. 
			// (0 -> unlimited)
			size_t max_wspace_cmt_2_flags;
			// Max whitespace chars between comment begin and 
			// object name/insert statement in comment. 
			// (0 -> unlimited)
			size_t max_wspace_cmt_2_cmd;
			// [see above]... to objct name (0 -> unlimited)
			size_t max_wspace_cmt_2_name;
			// Warn if ``insert.'' is found in comment start but 
			// the comment parser is told not to interprete it. 
			bool warn_uninterpreted_insert;
			// Warn if ``parse.'' is found in comment start but 
			// the comment parser is told not to interprete it. 
			bool warn_uninterpreted_parse_end;
			// Warn if files get included more than once?
			bool warn_multiple_include;
			// Warn mismatch on #if(active)..#end 
			// (insert.<obj>.active/insert.<obj>.actend)
			bool warn_active_mismatch;
			// 0 -> dump only used scalars; 
			// 1 -> dump them all into the frame include file
			bool frame_dump_all_scalars;
			// 0 -> only what is needed according to the parser 
			//      (trans/rot/scale are not dumped for inactive 
			//      frames if they are only needed if active; 
			//      trans/rot/scale are independently only dumped if needed) 
			// 1 -> also dump if(active) statements and also dump values for 
			//      inactive frames if they are needed for active objects. 
			// 2 -> dump everything 
			int frame_dump_object_info;
			// Max stack positions in Sub_Parser_Stack. 
			int parser_stack_limit;
			// Only interprete the first line of a C comment? 
			bool cmd_interprete_first_line;
			
			bool dump_tokens;  // dump 'em to cerr? 
			
			Config();
			~Config() { }
		};
		struct Object_Flags
		{
			unsigned int has_end_statement : 1;
			unsigned int unset : 1;  // 1 -> defaults; 0 -> user supplied (for error)
			unsigned int insert_active : 1;  // Insert #if(active)...#end statements?
			unsigned int insert_scale  : 1;
			unsigned int insert_rot    : 1;
			unsigned int insert_trans  : 1;
			unsigned int : (8*sizeof(unsigned int)-6);
			
			void reset(int val=1);
		};
		struct AValue  // animated objects and scalars
		{
			AValue *next;  // AValue_List queuing; do not modify 
			
			tokID type;   // taScalar or taObject
			anitmt::Prop_Tree_Node *ptn;
			int nfound;  // how often found in POV/.. file
			unsigned int serial;  // the _aniTMT_???_ value
			bool locked;   // currently working on this one?
			
			// Of the current declaration; they are reset to 
			// initial (unset) values after the stuff has been 
			// added to Modification_Copy. 
			Position dec_start,dec_end;  // declaration start/end
			Position mat_pos;   // scale/rot/trans or unset. 
			Object_Flags flags;
			
			// Counters for needed values for objects: 
			// (scalars are always needed if they were found (nfound))
			int need_active;   // ...nevertheless incremented for scalars
			// There are counters if trans/rot/scale is needed outside 
			// if(active)...end and ones for trans/rot/scale inside 
			// if(active). The total need is the sum of both values. 
			int need_scale,need_scale_act;
			int need_rot,  need_rot_act;
			int need_trans,need_trans_act;
			// Are we currently inside an if(active) statement?
			int curr_active;
			
			AValue();
			~AValue() { }
		};
		
	private:
		class Sub_Parser_Stack
		{
			private:
				struct Node
				{
					Node *down;
					Sub_Parser *sp;
					
					Node(Sub_Parser *_sp)  {  down=NULL;  sp=_sp;  }
					~Node() { }
				} *top;
				int cnt;
			public:
				Sub_Parser_Stack();
				~Sub_Parser_Stack();
				
				int size()  {  return(cnt);  }
				
				void Push(Sub_Parser *sp);
				Sub_Parser *Pop();
				void Clear();
				Sub_Parser *Top()  {  return(top ? (top->sp) : NULL);  }
		};
		struct AValue_List
		{
			AValue *first,*last;
			
			// Copies the content if the passed AValue and 
			// returns a pointer to the AValue allocated for 
			// the list (which equals this->last). 
			AValue *Add(const AValue &av);
			void Clear();
			
			// Print warnings on unused objects/scalars
			// Returns number of unused things. 
			int Warn_Unused(ostream &os);
			
			AValue_List()  {  first=last=NULL;  }
			~AValue_List()  {  Clear();  }
			private: int _Warn_Unused(ostream &os,tokID type);
		};
		
		Modification_Copy *mcopy;
		Find_String *pp_finder;   // preprocessor cmds only
		Find_String *tok_finder;  // all but object/scalar names
		Find_String *av_finder;   // includig object/scalar names 
		size_t pp_find_longest;  // longest str +1 
		
		int _Finder_Check_Copy_Name(anitmt::Prop_Tree_Node *ptn,tokID id,unsigned int *counter);
		void _Set_Up_Finder_Const();
		int _Set_Up_Finder(anitmt::Animation *ani,anitmt::Ani_Scene *sc);
		int _Set_Up_MCopy(anitmt::Animation *ani,anitmt::Ani_Scene *sc);
		int _Set_Up_FDump(anitmt::Animation *ani,anitmt::Ani_Scene *sc);
		void _Set_Up_SPS();
		void _CleanupUnneeded();
		void _Cleanup();
		
		// overriding virtuals: 
		size_t Parse_Input(const Parse_Input_Buf *ib);
		void Parse_EOF(const Parse_Input_Buf *ib);
		
		int Include_File(const std::string &file,
			size_t offset,size_t inclen,
			bool honor_search_path=true,
			int search_current_path=1);
		int Should_Copy(const std::string &file,Position *pos);
		
		Current_Context cc;
		Sub_Parser_Stack sps;
		int parser_added;
		int parse_errors;
		void _Set_CC(const Parse_Input_Buf *ib,bool eof);
		int Register_Sub_Parser(Sub_Parser *sb);
		void _Sub_Parser_Done();
		
		// Find a token and update Current_Context cc. 
		size_t Find_Tok(Find_String::RV *rv,bool with_ani_names);
		size_t _Find_Tok_PP(Find_String::RV *rv,bool with_ani_names);
		bool too_many_spaces_explained;
		// MAY ONLY BE CALLED ONCE PER SUCCESSFUL Find_Tok() CALL: 
		bool may_put_back;
		bool refill_buffer;
		char prev_char;
		Position llast_nl_pos,last_nl_pos;
		void Put_Back_Tok(Find_String::RV *rv);
		void Consumed(size_t n,int nlines);
		
		ostream &Error_Header()
			{  return(Recursive_Input_Stream::Error_Header(cerr,cc.line));  }
		ostream &Error_Header(int line)
			{  return(Recursive_Input_Stream::Error_Header(cerr,line));  }

		AValue_List av_list;
		Frame_Dump *fdump;  // namespace POV
		bool Process_AValue(AValue *av);
		
		int Setup_Frame_Dump(Frame_Dump *fdump);
		
		// Insert an object insert statement (insert.obj.trans,...)
		void Insert_Statement(AValue *av,Position *pos,char type);
		
		// Warn if too many/few #if(active)/#end statements 
		// (insert.<obj>.active / insert.<obj>.actend) were found. 
		int Warn_Active_Mismatch();
		
		// To cut out comment if user wants it. 
		void Comment_Cutout(Position *start,Position *end);
		
		int verbose;
		ostream &vout;  // verbose stream
	public:
		File_Parser();
		~File_Parser();
		
		// To tweak parser settings: 
		Config config;
		
		// Return 0 on success; !=0 on error 
		int Go(anitmt::Animation *ani,anitmt::Ani_Scene *sc);
		
		// Call this if you want to write frame include files. 
		// This returns a pointer to the Frame_Dump class allocated 
		// by File_Parser. Once you call this function, you are 
		// responsible for Frame_Dump and File_Parser does not 
		// do anything with it any more (not even free it; so you 
		// may delete File_Parser immediately afterwards or call 
		// Go() again in which case a new Frame_Dump is allocated). 
		// Make sure you delete Frame_Dump. 
		Frame_Dump *Transfer_FDump();
};

} }  // namespace end 

#endif  /* _Inc_IO_PovFileParse_H_ */
