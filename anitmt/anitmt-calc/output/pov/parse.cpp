/*
 * parse.cpp
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

#warning need a class which contains the expected dest \
  dir content and checks if files have to be renamed. 
// Also: remove path from file statements if one is specified?! (see Should_Copy) 

//###error NEED...
// DONE>>...flags in AValue which check if trans/rot/scale/active is needed (insert statements/Process_Avalue)
// ...object parser for implicit comment named objects
// ...implement obj name prefix support
// DONE>>...toplevel parser plugin for object parser
// ...look for `###'

// Possible speed increase by: 
//  - using a custom operator new for Sub_Parser-derived classes. 


#include <val/val.hpp>
#include <proptree/proptree.hpp>
#include <animation.hpp>

#include "parse.hpp"

#include "htime.h"

namespace output_io
{
namespace POV
{

enum 
{
	st_ND=0x01,  // need deliminator
	st_PP=0x03   // ``preprocessor token; ALL THOSE BEGINNING WITH `#'.''
	             // This has st_ND set automatically. 
};

typedef int _SearchTok_Flags_t;

struct _SearchTok
{
	enum tokID id;
	const char *str;
	_SearchTok_Flags_t flags;
} const searchtokens[]=
{
	{ tString,        "\"",    0 },
	{ tMaskedString,  "\\\"",  0 },
	{ tCOpComment,    "/*",    0 },
	{ tCClComment,    "*/",    0 },
	{ tCppComment,    "//",    0 },
	{ tNewline,       "\n",    0 },
	{ tOpBrace,       "{",     0 },
	{ tClBrace,       "}",     0 },
	{ tAssign,        "=",     0 },
	{ tSemicolon,     ";",     0 },
	{ tNumbersign,    "#",     0 },
	{ tColon,         ":",     0 },
	
	{ tpDeclare,        "declare", st_PP },
	{ tpInclude,        "include", st_PP },
	{ tpFileIdentifier, "png",     st_ND },
	{ tpFileIdentifier, "tga",     st_ND },
	{ tpFileIdentifier, "iff",     st_ND },
	{ tpFileIdentifier, "ppm",     st_ND },
	{ tpFileIdentifier, "pgm",     st_ND },
	{ tpFileIdentifier, "sys",     st_ND },
	{ tpFileIdentifier, "ttf",     st_ND },
	{ tpFileIdentifier, "df3",     st_ND },
	{ tpFileIdentifier, "gif",     st_ND },
	{ tpFileIdentifier, "pot",     st_ND },
	
	{ tpAniInsert,      "insert.", 0 },  // NO deliminator. 
	{ tpAniParse,       "parse.",  0 },  // NO deliminator. 
	
	{ tNone, NULL, 0 }   // LAST ELEMENT 
};


static char *Ani_Type_Name(ComponentInterface::IFType id)
{
	switch(id)
	{
		case ComponentInterface::IFScalar:  return("scalar");
		case ComponentInterface::IFObject:  return("object");
	}
	return("???");
}
static char *Ani_Type_Name(tokID id)
{
	return(Ani_Type_Name(toIFType(id)));
}
static char *Ani_Type_Name_Spc(ComponentInterface::IFType id)
{
	switch(id)
	{
		case ComponentInterface::IFScalar:  return("scalar ");
		case ComponentInterface::IFObject:  return("object ");
	}
	return("");   // OK
}


// Must check if the passed character is allowed in an scalar/object name. 
inline bool allowed_name_char0(char c)  // for the first char
{
	return(isalpha(c) || c=='_');
}
inline bool allowed_name_char1(char c)  // for all following chars
{
	return(allowed_name_char0(c) || isdigit(c));
}

inline bool is_pov_name_char(char c)  // for the first char
{
	return(isalpha(c) || isdigit(c) || c=='_');
}

inline bool is_delim(char c)
{
	return(!isalpha(c) && !isdigit(c) && c!='_');
}

inline bool need_delim(const Find_String::RV *rv)
{
	if(rv->id==taObject || 
	   rv->id==taScalar)
	{  return(true);  }
	_SearchTok_Flags_t *flags=(_SearchTok_Flags_t *)(rv->hook);
	if(!flags)  return(false);
	if(((*flags) & _SearchTok_Flags_t(st_ND))==st_ND)  return(true);
	return(false);
}

inline bool is_pp_command(const Find_String::RV *rv)
{
	if(rv->id==taObject || 
	   rv->id==taScalar)
	{  return(false);  }
	_SearchTok_Flags_t *flags=(_SearchTok_Flags_t *)(rv->hook);
	if(!flags)  return(false);
	if(((*flags) & _SearchTok_Flags_t(st_PP))==st_PP)  return(true);
	return(false);
}


struct File_Parser::Sub_Parser : public message::Message_Reporter
{
	typedef File_Parser::Position File_Parser::Sub_Parser::Position;
	typedef File_Parser::Object_Flags File_Parser::Sub_Parser::Object_Flags;
	
	File_Parser *fp;
	Current_Context *cc;
	bool done;
	int errors;
	tokID parser_type;  // which tok started the parser
	
	void _construct(File_Parser *parent,tokID tok);
	// *tok needed for parser_type. 
	Sub_Parser(File_Parser *parent,Find_String::RV *tok) : 
		message::Message_Reporter(parent->get_consultant())
		{  _construct(parent,tokID(tok->id));  }
	Sub_Parser(File_Parser *parent,tokID tid) : 
		message::Message_Reporter(parent->get_consultant())
		{  _construct(parent,tid);  }
	virtual ~Sub_Parser();
	
	// Used to get the next token if the class does not do 
	// char parsing. 
	// cc->line is increased on tNewline. 
	// Note: rv->hook is an AValue allocated by AValue_List. 
	// with_ani_names: true -> also search for object/scalar names. 
	size_t Find_Tok(Find_String::RV *rv,bool with_ani_names)
		{  return(fp->Find_Tok(rv,with_ani_names));  }
	
	// MAY ONLY BE CALLED ONCE PER SUCCESSFUL Find_Tok() CALL. 
	void Put_Back_Tok(Find_String::RV *rv)
		{  fp->Put_Back_Tok(rv);  }
	
	// To be called by Sub_Parsers if they consumed n chars by doing 
	// string parsing themselves. (encountered nlines `\n' chars).  
	void Consumed(size_t n,int nlines=0)
		{  fp->Consumed(n,nlines);  }
	
	// Write an error/warning header to error/warning stream. 
	message::Message_Stream Error_Header()
		{  return(fp->Recursive_Input_Stream::Error_Header(error(),cc->line));  }
	message::Message_Stream Error_Header(int line)
		{  return(fp->Recursive_Input_Stream::Error_Header(error(),line));  }
	message::Message_Stream Warning_Header()
		{  return(fp->Recursive_Input_Stream::Error_Header(warn(),cc->line));  }
	message::Message_Stream Warning_Header(int line)
		{  return(fp->Recursive_Input_Stream::Error_Header(warn(),line));  }
	
	// Either returns "line xx" or "path/file:xx" depending if 
	// pos referrs to the same file as *cc. 
	std::string Rel_Pos(Position *pos);
	// Stores " starting in [file:]line" if *start-position is 
	// different from current position (*cc). 
	std::string Startpos_Str(Position *start);
	
	// Very special (previous declaration location string). 
	std::string Prev_Decl_Loc_Str(AValue *av);
	
	std::string Get_Path(Recursive_Input_Stream::InFile_Segment *handle)
		{  return(fp->Get_Path(handle));  }
	
	void Abort_Parsing()
		{  done=true;  fp->Abort_Parser();  }
	
	// Register child sub-parser which will be put on 
	// top of the sub parser stack. 
	void Register_Sub_Parser(Sub_Parser *sb);
	
	// Tell Modification_Copy about a declaration or 
	// other object specification (via interpreted comment). 
	// Returns true on success; false on failure. 
	bool Process_AValue(AValue *av)
		{  return(fp->Process_AValue(av));  }
	
	// To skip tCOpComment and tCppComment without 
	// interpreting them. Returns parser if rv->id is 
	// a comment and a comment parser was installed, 
	// otherwise NULL. 
	Comment_Parser *Skip_Comment(Find_String::RV *rv,bool allow_insert_statements);
	
	// Parser must modify cc->line and cc->src, cc->srclen 
	// when parsing. 
	// eof: 1 -> there is nothing left in cc->src and this 
	//           the end of a file with depth>0
	//      2 -> nothing left in src; eof of primary file 
	//           (depth=0)
	//      0 -> otherwise (no eof)
	// Return value: 0 -> go on
	//               1 -> need more input; must fill up buffer 
	virtual int parse(int eof)=0;
	
	// sub parser done. 
	// Called when a sub parser previously registered 
	// via Register_Sub_Parser() is done. 
	// Return value: ??
	virtual int spdone(Sub_Parser *sb)=0;
};

// C and C++ comment parsers: 
class File_Parser::Comment_Parser : public Sub_Parser
{
	private:
		tokID style;  // tCppComment, tCOpComment 
		int nested;  // 0 -> not nested; >0 -> nested level
		bool _start_only_wspace;
		bool comment_begin;
		int in_insert_statement;
		int in_parse_end_statement;
		Position expect_pos;
		
		int _parse(int eof);
		int _parse_flags(Find_String::RV *rv);
		int _parse_insert_which();
		int _parse_parse_end_which();
		void _incomplete_i_pe_statement();
		void _illegal_i_pe_statement();
	protected:
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		// cmt_style->id is either tCppComment (C++-style comment) or 
		// tCOpComment (C-style comment). 
		Comment_Parser(File_Parser *parent,Find_String::RV *cmt_style);
		virtual ~Comment_Parser();
		
		void Set_Object(AValue *_av,const char *_obj_name);
		
		// Can be set by the creator: Interprete the comment (default: 0): 
		// 0 -> no
		// 1 -> search only flags; e.g. /*:E+r */; may not contain name; 
		//      Must call Set_Object() after creation! 
		// 2 -> search for name and flags
		int interprete_obj;
		bool interprete_insert;  // interprete insert statements?
		bool interprete_parse_end;   // interprete parse.end statement?
		
		// Comment start and end position: 
		// Start position is before ``/*'' or ``//''; 
		// end position is after ``*/'' or `\n' (suitable to cut out) 
		Position cmt_start,cmt_end;
		
		// Read in flags and AValue: 
		AValue *av;
		const char *obj_name;  // av->cif.get_name()
		bool name_valid;  // obj_name valid?
		Object_Flags flags;
		bool flags_valid;
		
		// Gets set if interprete_parse_end was set and a parse.end 
		// statement was found. The object AValue is passed in av. 
		bool encountered_parse_end;
};

class File_Parser::String_Parser : public Sub_Parser
{
	private:
		int multiline_warned;
		void _addbytes(char *startptr,char *endptr);
		void _Unterminated_Str();
	protected:
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		String_Parser(File_Parser *parent,Find_String::RV *tok);
		virtual ~String_Parser();
		
		// Allow multi-line strings? 
		bool allow_multiline;  // default: -> config
		// start/end position of string; start is the char after the `"' 
		// and end is the char before `"'. 
		Position str_start,str_end;
		// Parsed in string: 
		std::string string;
};

class File_Parser::Filename_Parser : public Sub_Parser
{
	private:
		tokID waittok;
		char *tok_name;
	protected:
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		// Pass the current token in *tok (include/png/...)
		Filename_Parser(File_Parser *parent,Find_String::RV *tok);
		virtual ~Filename_Parser();
		// Set this after having created the Filename_Parser: 
		//  0 -> simply return if no string was encountered 
		//  1 -> warn (default); 2 -> error if no string is coming next. 
		int warn_no_string;
		
		// start/end position of filename string; 
		// start is the char after the `"' and end is the char before `"'. 
		Position fname_start,fname_end;
		// Parsed in file name: 
		std::string filename;
		// true -> encountered filename; valid string; no errors
		bool filename_valid;
};

class File_Parser::Include_Parser : public Sub_Parser
{
	private:
		size_t inc_start_offset;  // in file
		Filename_Parser *fnparser;
	protected:
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		// Pass the current token in *tok ("include"). 
		Include_Parser(File_Parser *parent,Find_String::RV *tok);
		virtual ~Include_Parser();
};

class File_Parser::Declare_Parser : public Sub_Parser
{
	private:
		tokID waittok;
		struct 
		{
			ComponentInterface::IFType id;  // IFScalar or IFObject
			const char *name;   // object/scalar name
			AValue *av;  // <- rv->hook 
		} expect;
		Position dec_start;
		Position dec_end;
		Position assign_pos;   // after `=' in decl 
		enum
		{ sDeclare,sAssign,sValueStart,sValueEnd }
		state;
		int more_names_error;
		int nbraces;
		bool scalar_brace_warning;
		bool multiline_scalar_decl_warned;
		char *_dectype_of(ComponentInterface::IFType id);
		std::string _startline_str();
	protected:
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		Declare_Parser(File_Parser *parent,Find_String::RV *tok);
		virtual ~Declare_Parser();
};

class File_Parser::Toplevel_Parser : public Sub_Parser
{
	private:
		tokID waittok;
	protected:
		int TL_Parse(Find_String::RV *rv);
		int TL_Parse_Object(Comment_Parser *cp);
		void TL_Object_Parser_Done(Sub_Parser *sb);
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		// NOTE: only the topmost toplevel parser may set tok=tNone. 
		Toplevel_Parser(File_Parser *parent,tokID tok=tNone);
		virtual ~Toplevel_Parser();
};

class File_Parser::Object_Parser : public Toplevel_Parser
{
	private:
		tokID waittok;
		int nbraces;
		AValue *av;
		const char *obj_name;  // av->ptn's name
		bool object_end_coming;
		
		std::string _startline_str();
	protected:
		int parse(int eof);
		int spdone(Sub_Parser *sb);
	public:
		Object_Parser(File_Parser *parent,AValue *av,const char *obj_name);
		virtual ~Object_Parser();
		// Start and end position of the object; 
		// obj_start must be set correctly after construction. 
		Position obj_start,obj_end;
		Position mat_pos;  // where to insert regular scale/rot/trans 
		
		void Set_Braces(int nb)  {  if(!nbraces)  nbraces=nb;  }
		AValue *GetAV()  {  return(av);  }
};


namespace { std::string I2Str(int x)
{
	char tmp[32];
	snprintf(tmp,32,"%d",x);
	return(std::string(tmp));
} }

// To hell, I wanna write code not wasting my time cluttering this file 
// with any number of ``anitmt::''...
using namespace anitmt;   


void File_Parser::_Set_CC(const Parse_Input_Buf *ib,bool eof)
{
	cc.ib=ib;
	cc.src=ib->txt;
	cc.srclen=ib->len;
	cc.fileoff=ib->offset;
	cc.line=ib->line0;
	cc.eof=eof;
}


// Called by Recursive_Input_Stream to feed us with data. 
size_t File_Parser::Parse_Input(const Parse_Input_Buf *ib)
{
	// Available: 
	// ib->handle, ib->path, ib->depth
	// ib->line0, ib->offset
	// ib->txt, ib->len
	// ib->eof_reached (no further data of this file will be delivered 
	//                  beyond  ib->txt+ib->len)
	
	_Set_CC(ib,false);
	
	refill_buffer=false;
	size_t consumed,prev_consumed=0;
	for(;;)
	{
		// Go token hunting...
		int parse_need_input=0;
		for(;;)
		{
			parser_added=0;
			Sub_Parser *tsp=sps.Top();
			assert(tsp);   // at least the Toplevel_Parser must be on the stack 
			
			#ifndef NDEBUG
				#warning Define NDEBUG to switch off this debug check. 
				int _cc_line=cc.line;
				char *_cc_src=cc.src;
			#endif
			parse_need_input+=tsp->parse(/*eof=*/0);
			
			// The parser MUST update cc.line if it does char-matching. 
			#ifndef NDEBUG
				int dline=0;
				for(char *i=_cc_src; i<cc.src; i++)
				{  if(*i=='\n')  ++dline;  }
				// If this assertion fails, this indicates a bug in one of 
				// the sub parsers. 
				assert(_cc_line+dline==cc.line);
			#endif
			
			if(!parser_added)  break;
			// tsp must just have added a new parser (Register_Parser) 
			// It may not terinate at the same time. 
			assert(tsp->done==false);
		}
		
		for(Sub_Parser *tsp=sps.Top();;)
		{
			if(Is_Abort_Scheduled())
			{  ++parse_errors;  return(0);  }  // We may return 0 in that case. 
			if(tsp->done)
			{
				_Sub_Parser_Done();
				tsp=sps.Top();
			}
			else break;
		}
		
		if(refill_buffer)
		{  ++parse_need_input;  }
		
		//std::cerr << "(" << ib->len << "," << cc.srclen << ")\n";
		
		// This may need some explanation: If we could not consume input chars 
		// this means that there is not enough input available for Find_String 
		// to see if some of the tokens match or not (need more bytes to 
		// compare). If we're on EOF, then there are no more chars and the 
		// strings to be searched will definitely not match. So, if we did 
		// not consume any chars and EOF is reached, we declare them consumed 
		// to make Recursive_Input_Stream happy. 
		consumed=ib->len-cc.srclen;
		assert(consumed==size_t(cc.src-ib->txt));
		if(ib->eof_reached && !consumed && may_put_back)
		{  consumed=ib->len;  break;  }
		
		// If nothing consumed and nothing put back or need more input...
		if((consumed<=prev_consumed && may_put_back) || parse_need_input)
		{
			if(consumed)  break;
			// Give error if we need more input (but did not parse anything) 
			assert(!parse_need_input);
			// This should never happen; if it does; a parser probably 
			// forgot to return 1 in parse() (``need input''). 
			assert(0);  
		}  // must read more
		prev_consumed=consumed;
	}
	
	//std::cerr << "  CONSUMED=" << consumed << "\n";
	if(cc.src>ib->txt)
	{  prev_char=*(cc.src-1);  }
	return(consumed);
}


// Called by Recursive_Input_Stream if a file EOF is reached. 
void File_Parser::Parse_EOF(const Parse_Input_Buf *ib)
{
	// Be sure this file gets copied: 
	mcopy->MFile(ib->path);
	
	_Set_CC(ib,true);
	
	int eofval=(ib->depth ? 1 : 2);
	for(;;)
	{
		Sub_Parser *tsp=sps.Top();
		if(!tsp)  break;
		
		tsp->parse(/*eof=*/eofval);
		if(tsp->done)
		{
			_Sub_Parser_Done();
			tsp=NULL;
		}
		else if(eofval==1)  break;
		else
		{
			error() << "Internal error: non-unregistered subparser on hard EOF";
			abort();
		}
	}
	
	if(Is_Abort_Scheduled())
	{  ++parse_errors;  return;  }
	
	// If eofval==1, at teast the Toplevel_Parser must stay here. 
	// If eofval==2, ALL parsers must have quit. 
	if(eofval==1)
	{  assert(sps.Top());  }
	else if(eofval==2)
	{  assert(sps.Top()==NULL);  }
	
	prev_char='\0';
}


int File_Parser::Include_File(const std::string &file,
	size_t offset,size_t inclen,
	bool honor_search_path=true,
	int search_current_path=1)
{
	refill_buffer=true;  // must read on in other file 
	return(Recursive_Input_Stream::Include_File(
		file,offset,inclen,honor_search_path,search_current_path));
	// mcopy->MFile() called by Parse_EOF(). 
}


// Returns number of errors: 
int File_Parser::Should_Copy(const std::string &file,Position *pos)
{
	if(!config.copy_nonsource_files)
	{
		verbose() << "Found non-source file \"" << file << "\" (not copied)";
		return(0);
	}
	
	// Be sure to specify the complete path to mcopy->MFile() below. 
	Search_Path spath;
	Input_Stream test_is(file);
	spath.try_open(&test_is,
		/*search_current=*/+1,cc.ib->path,
		/*honor_search_path=*/false);
	
	if(!test_is.IsOpen())
	{
		// Should still be same file...
		assert(pos->handle==cc.ib->handle);
		Warning_Header(pos->line) << "warning: failed to "
			"open seemingly required file \"" << file << 
			"\". (ignoring it)";
	}
	else
	{
		verbose() << "Adding file \"" << file << "\" to copy list. "
			"(found in " << Get_Path(pos->handle) << ":" << 
			pos->line << ")";
		mcopy->MFile(file);
	}
	
	return(0);
}


size_t File_Parser::_Find_Tok_PP(Find_String::RV *rv,bool with_ani_names)
{
	size_t processed=
		(with_ani_names ? av_finder : tok_finder)->Search(cc.src,cc.srclen,rv);
	if(!processed || !rv->found)
	{  return(processed);  }
	if(rv->id==tNumbersign)
	{
		// See if a recognized preprocessor command follows: 
		// cc.src+processed-1==`#'
		char *c=cc.src+processed,*cend=cc.src+cc.srclen;
		for(; c<cend; c++)
		{  if(*c!=' ' && *c!='\t')  break;  }
		// pp_find_longest is actually the longest length +1 for the 
		// deliminator. 
		if(c+pp_find_longest>=cend)
		{
			// End of buffer reached. Must read more if we can. 
			if(cc.src>cc.ib->txt+1 &&   // +1 for the `#'
			   !cc.ib->eof_reached)
			{
				rv->found=NULL;
				rv->id=-1;
				refill_buffer=true;
				return(processed-1);
			}
			// Can't read more; can't do anything more. 
			if(!cc.ib->eof_reached)
			{
				Warning_Header() << "warning: too many spaces between `#\' and "
					"(probably following) preprocessor command.";
				if(!too_many_spaces_explained)
				{
					Error_Header() << "         (if this line declares an "
						"animated scalar, the `#\' and the spaces will not "
						"be cut out)";
					too_many_spaces_explained=true;
				}
			}
			return(processed);
		}
		// Okay; test for preprocessor command starting at c. 
		Find_String::RV pprv;
		if(pp_finder->Match(c,pp_find_longest,&pprv))
		{
			// pprv.ptr=c+strlen(pprv.found)
			if(is_delim(*pprv.ptr))
			{
				// found preprocessor cmd 
				rv->id=pprv.id;
				rv->hook=pprv.hook;
				rv->found=pprv.found;  // .. but don't trust strlen(rv->found)
				rv->found_len=pprv.ptr-(cc.src+processed-1);
				//rv->ptr stays const
				return(pprv.ptr-cc.src);
			}
		}
		// (Could probably return (cend-c) here to skip spaces.)
	}
	return(processed);
}


size_t File_Parser::Find_Tok(Find_String::RV *rv,bool with_ani_names)
{
	char *old_cc_src=cc.src;
	for(;;)
	{
		if(cc._slt_reset)
		{
			cc._slt_reset=false;
			cc.only_wspace=true;
			cc.with_newline=0;
		}
		
		size_t processed=_Find_Tok_PP(rv,with_ani_names);
		
		if(!rv->found)
		{  rv->found_len=0;  }   // be sure...
		else if(rv->id==tNewline)
		{  ++cc.with_newline;  ++cc.line;  }
		else if(rv->id==tCClComment)
		{
			tokID tpt=sps.Top()->parser_type;
			if(tpt!=tCOpComment && 
			   tpt!=tCppComment && 
			   tpt!=tString)
			{  Error_Header() << "Stray closing C-style comment `*/\'.";
				++sps.Top()->errors;  }
		}
		else
		{  cc._slt_reset=true;  }
		
		if(cc.only_wspace)
			for(char *c=cc.src,*cend=c+processed-rv->found_len; c<cend; c++)
			{
				if(!isspace(*c))
				{  cc.only_wspace=false;  break;  }
			}
		// only_wspace: if there was only white space between the last found tok 
		//              till the end of the processed data (minus length of the 
		//              next tok, if any). 
		
		// Object start position finding logic: 
		#if 0
		{
			tokID tpt=sps.Top()->parser_type;
			if(tpt!=tCOpComment && tpt!=tCppComment)
			{
				char *cend=cc.src+processed;
				if(rv->id==tCOpComment || 
				   rv->id==tCppComment)
				{  cend-=rv->found_len;  }
				char objstart=NULL; ####??
				for(char *c=cc.src; c<cend; c++)
				{
					if(!is_pov_name_char(*c))
					{  objstart=c;  }
				}
			}
		}
		#else
		if(rv->id==tNewline)
		{
			// #### to be fixed
			llast_nl_pos=last_nl_pos;
			last_nl_pos=cc;
			++last_nl_pos.fileoff;  // skip '\n'
		}
		#endif
		
		cc.src+=processed;
		cc.srclen-=processed;
		cc.fileoff+=processed;
		if(!rv->found)  break;
		
		// Be sure that the found strings are real tokens;
		// otherwise we may find ``includex'' as ``include'' token. 
		if(need_delim(rv))
		{
			char *nextchar=cc.src;
			char *prevchar=rv->ptr-1;
			if(prevchar<cc.ib->txt)
			{
				assert(rv->ptr==cc.ib->txt);
				prevchar=&prev_char;
			}
			if(nextchar>=(cc.src+cc.srclen))
			{
				// Next char would be behind buffer end. 
				cc.src-=processed;
				cc.srclen+=processed;
				cc.fileoff-=processed;
				rv->found=NULL;
				rv->id=-1;
				refill_buffer=true;
				break;
			}
			if(!is_delim(*nextchar) || 
			   !is_delim(*prevchar) )
			{  continue;  }   // oh, no deliminator before/after token
		}
		break;
	}
	
	if(rv->found)
	{
		cc.last_found_len=rv->found_len;
		may_put_back=true;
		if(config.dump_tokens && rv->id!=tNewline)
		{
			error() << "  TOK[" << cc.ib->depth << "]" 
				"(" << cc.line << ")=" << rv->found << 
				"  only_wspace=" << cc.only_wspace << 
				" (" << cc.with_newline << " \\n)" << 
				"  (pe=" << parse_errors << "; err=" << 
				(sps.Top() ? (sps.Top()->errors) : (-1)) << ")";
		}
	}
	return(cc.src-old_cc_src);
}


void File_Parser::Put_Back_Tok(Find_String::RV *rv)
{
	assert(may_put_back);
	assert(rv->found);
	size_t delta=rv->found_len;
	//std::cerr << "DELTA=" << delta << "\n";
	cc.src-=delta;
	cc.srclen+=delta;
	cc.fileoff-=delta;
	if(rv->id==tNewline)
	{
		--cc.line;
		if(cc.with_newline>0)
		{  --cc.with_newline;  }
	}
	assert(cc.src>=cc.ib->txt);
	may_put_back=false;
}


void File_Parser::Consumed(size_t n,int nlines)
{
	assert(cc.srclen>=n);
	cc.srclen-=n;
	cc.src+=n;
	cc.fileoff+=n;
	cc.line+=nlines;
}


// type: 's', 'r', 't', 'a', 'e' (active end), 'i' (=s+r+t)
void File_Parser::Insert_Statement(AValue *av,Position *pos,char type)
{
	size_t buflen=256*4;
	char tmp[buflen];
	char *end=tmp;
	if(type=='a')
	{
		// ``#if(_aniTMT_xx_active)''
		strcpy(end,"#if(");  end+=4;
		end=aniTMT_identifier(end,'a',av->serial);
		*(end++)=' ';
		++av->need_active;
		++av->curr_active;  // we enter an if(active) statement
		if(av->curr_active>1 && config.warn_active_mismatch)
		{  Warning_Header() << "warning: nested #if(active) statement for " << 
			Ani_Type_Name(av->type()) << " `" << av->cif.get_name() << 
			"\' (use insert." << av->cif.get_name() << ".actend "
			"instead of #end)";  }
	}
	if(type=='s' || type=='i')
	{
		// ``scale _aniTMT_xx_scale''
		strcpy(end,"scale ");  end+=6;
		end=aniTMT_identifier(end,'s',av->serial);
		*(end++)=' ';
		if(av->curr_active>0)  ++av->need_scale_act;
		else  ++av->need_scale;
	}
	if(type=='r' || type=='i')
	{
		// ``scale _aniTMT_xx_scale''
		strcpy(end,"rotate ");  end+=7;
		end=aniTMT_identifier(end,'r',av->serial);
		*(end++)=' ';
		if(av->curr_active>0)  ++av->need_rot_act;
		else  ++av->need_rot;
	}
	if(type=='t' || type=='i')
	{
		// ``scale _aniTMT_xx_scale''
		strcpy(end,"translate ");  end+=10;
		end=aniTMT_identifier(end,'t',av->serial);
		*(end++)=' ';
		if(av->curr_active>0)  ++av->need_trans_act;
		else  ++av->need_trans;
	}
	if(type=='e')
	{
		// ``#end''
		strcpy(end,"#end ");  end+=5;
		--av->curr_active;
		if(av->curr_active==-1 && config.warn_active_mismatch)
		{  Warning_Header() << "warning: more insert." << av->cif.get_name() << 
			".actend statements than insert." << av->cif.get_name() << 
			".active.";  }
	}
	
	if(end>tmp)
	{
		assert(end<tmp+buflen);
		mcopy->MInsert(Get_Path(pos->handle),pos->fileoff,tmp,end-tmp);
	}
}


// Returns true on success; false on failure. 
bool File_Parser::Process_AValue(AValue *av)
{
	bool retval=true;
	
	assert(av->locked);  // be sure the Sub_Parsers do proper locking 
	if(av->type()==ComponentInterface::IFScalar)
	{
		// Be sure the values are set properly: 
		assert(!av->dec_start.unset());
		assert(!av->dec_end.unset());
		
		// A scalar declaration may not be scattered over several files. 
		// DO NOT USE Same_File() here; we really must have the same 
		// handle pointer otherwise there was another file included 
		// in between them. 
		assert(av->dec_start.handle == av->dec_end.handle);
		assert(av->dec_start.fileoff<=av->dec_end.fileoff);
		
		if(verbose_level()>3)
		{  verbose(4) << "Deleting scalar declaration of `" << av->cif.get_name() << 
			"\' in " << 
			Get_Path(av->dec_start.handle) << ":" << av->dec_start.line << 
			" ... " << 
			Get_Path(av->dec_end.handle) << ":" << av->dec_end.line << 
			" (" << (av->dec_end.fileoff - av->dec_start.fileoff) << 
			" bytes)" << ((av->nfound==1) ? " [first]" : "");  }
		
		// Animated scalar declarations must be snipped out if the code. 
		mcopy->MDelete(
			Get_Path(av->dec_start.handle),
			av->dec_start.fileoff,
			av->dec_end.fileoff - av->dec_start.fileoff);
		
		++av->need_active;
	}
	else if(av->type()==ComponentInterface::IFObject)
	{
		assert(!av->dec_start.unset());
		assert(!av->dec_end.unset());
		
		// To be inserted: (order: scale -> rotate -> translate)
		// ``#if(_aniTMT_xx_active)''
		// ``scale _aniTMT_xx_scale''
		// ``rotate _aniTMT_xx_rot''
		// ``translate _aniTMT_xx_trans''
		// ``#end''
		
		// Yep, here's a potential buffer overflow location. 
		size_t buflen=256*4;
		char tmp[buflen],*end;
		
		if(verbose_level()>3)
		{  verbose(4) << "Inserting pov commands for object `" << 
			av->cif.get_name() << "\' "
			"(start: " << Get_Path(av->dec_start.handle) << ":" << av->dec_start.line << 
			"; matrix: " << Get_Path(av->mat_pos.handle) << ":" << av->mat_pos.line << 
			"; end: " << Get_Path(av->dec_end.handle) << ":" << av->dec_end.line << 
			"):" << 
			(av->flags.insert_active ? " active" : "") << 
			(av->flags.insert_scale ? " scale" : "") << 
			(av->flags.insert_rot ? " rot" : "") << 
			(av->flags.insert_trans ? " trans" : "");  }
		
		bool ins_act=av->flags.insert_active;
		// Cannot use av->curr_active here because of nested objects. 
		// #### (not urgent)
		// We could if we increase av->curr_active when parsing the object 
		//  beginning and the `a' flag was set and decrease it here 
		//  when inserting `#end' some lines below. 
		// Problem: parse errors?
		if(ins_act)
		{
			// ``#if(_aniTMT_xx_active)''
			strcpy(end=tmp,"#if(");  end+=4;
			strcpy(aniTMT_identifier(end,'a',av->serial),")\n");
			mcopy->MInsert(Get_Path(av->dec_start.handle),
				av->dec_start.fileoff, tmp);
			++av->need_active;
		}
		
		end=tmp;
		if(av->flags.insert_scale)
		{
			// ``scale _aniTMT_xx_scale''
			strcpy(end,"scale ");  end+=6;
			end=aniTMT_identifier(end,'s',av->serial);
			if(ins_act)  ++av->need_scale_act;
			else  ++av->need_scale;
		}
		if(av->flags.insert_rot)
		{
			// ``rotate _aniTMT_xx_rot''
			strcpy(end,"\nrotate ");  end+=8;
			end=aniTMT_identifier(end,'r',av->serial);
			if(ins_act)  ++av->need_rot_act;
			else  ++av->need_rot;
		}
		if(av->flags.insert_trans)
		{
			// ``translate _aniTMT_xx_trans''
			strcpy(end,"\ntranslate ");  end+=11;
			end=aniTMT_identifier(end,'t',av->serial);
			if(ins_act)  ++av->need_trans_act;
			else  ++av->need_trans;
		}
		if(end>tmp)
		{
			*(end++)='\n';
			assert(end<=tmp+buflen);  // detect buffer overrun if we can 
			mcopy->MInsert(Get_Path(av->mat_pos.handle),
				av->mat_pos.fileoff, tmp, end-tmp);
		}
		
		if(ins_act)
		{
			// ``#end''
			mcopy->MInsert(Get_Path(av->dec_end.handle),
				av->dec_end.fileoff, "\n#end ");
		}
	}
	else
	{  assert(0);  }   // illegal type.
	
	// Reset the values: 
	//av->dec_start.reset();  // needed to be able to see previous decl if there is more then one
	//av->dec_end.reset();  // needed to be able to see where prev decl ended
	av->mat_pos.reset();
	return(retval);
}


void File_Parser::Comment_Cutout(Position *start,Position *end)
{
	// MUST be the same pointers; comments cannot stretch over EOF 
	// boundaries and as they are comments, no #include can be 
	// executed inside them. 
	assert(start->handle==end->handle);
	assert(end->fileoff>start->fileoff);
	
	if(config.cut_out_comments)
	{
		mcopy->MDelete(
			Get_Path(start->handle),
			start->fileoff,
			end->fileoff-start->fileoff);
	}
}


int File_Parser::_Finder_Check_Copy_Name(const ComponentInterface &cif,
	unsigned int *counter)
{
	std::string str_name=cif.get_name();
	const char *name=str_name.c_str();
	
	// Check if the name is valid: 
	if(!(*name))
	{
		error() << "Error: Object or scalar without a name (ignored).";
		return(1);
	}
	
	bool allowed=allowed_name_char0(*name);
	if(allowed)
		for(const char *c=name+1; *c; c++)
		{
			if(!allowed_name_char1(*c))
			{  allowed=false;  break;  }
		}
	if(!allowed)
	{
		error() << "Error: Object or scalar name \"" << name << 
			"\" contains invalid characters (ignored).";
		return(1);
	}
	
	if(verbose_level()>2)
	{  verbose(3) << name << " " << message::noend;  }
	
	// Store AValue in list: 
	AValue av;
	av.cif=cif;
	av.serial=*counter;
	AValue *avp=av_list.Add(av);
	av_finder->Copy_String(name,toTokID(cif.GetType()),avp);
	++(*counter);
	
	return(0);
}


void File_Parser::_Set_Up_Finder_Const()
{
	assert(!pp_finder && !tok_finder);
	pp_finder = new Find_String();
	tok_finder = new Find_String();
	
	// Store the tokens...
	pp_find_longest=0;
	for(const _SearchTok *st=searchtokens; st->str; st++)
	{
		tok_finder->Add_String(st->str,st->id,&(st->flags));
		if(st->flags & st_PP)
		{
			pp_finder->Add_String(st->str,st->id,&(st->flags));
			size_t l=strlen(st->str);
			if(pp_find_longest<l)
			{  pp_find_longest=l;  }
		}
	}
	++pp_find_longest;   // must be increased by 1 (for deliminator) 
	
	// Sort the names...
	pp_finder->Sort_By_Length();
	tok_finder->Sort_By_Length();
}


int File_Parser::_Set_Up_Finder(Animation * /*ani*/,Scene_Interface &scene_if)
{
	int errors=0;
	assert(!av_finder);
	av_finder = new Find_String();
	
	// Store the tokens...
	for(const _SearchTok *st=searchtokens; st->str; st++)
	{  av_finder->Add_String(st->str,st->id,&(st->flags));  }
	
	// Get a list of scalar components: 
	verbose(2) << "Adding scalar nodes: " << message::noend;
	
	unsigned int count=0;
	for( Scalar_Component_Interface scalar = scene_if.get_first_scalar();
	     scalar != scene_if.get_scalar_end();
	     scalar = scalar.get_next() )
	{
		ComponentInterface cif(scalar);
		if(_Finder_Check_Copy_Name(cif,&count))
		{  ++errors;  }
	}
	verbose(2) << "[" << count << " nodes]";
	
	// Get a list of object components: 
	verbose(2) << "Adding object nodes: " << message::noend;
	count=0;
	for( Object_Component_Interface object = scene_if.get_first_object();
	     object != scene_if.get_object_end();
	     object = object.get_next() )
	{
		ComponentInterface cif(object);
		if(_Finder_Check_Copy_Name(cif,&count))
		{  ++errors;  }
	}
	verbose(2) << "[" << count << " nodes]";
	
	// Now, as all strings are written into the finder, sort them: 
	verbose(2) << "Sorting names..." << message::noend;
	av_finder->Sort_By_Length();
	verbose(2) << "done";
	
	Search_Path_Reset();
	
	#warning searchpath... (FIXME!!!)
	Search_Path_Add("/usr/src/povray/povray31/include");
	Search_Path_Add("/usr/src/film/povray/megapov-install/lib/megapovplus/include");
	Search_Path_Add("/usr/lib/povray/povray31/include");
	Search_Path_Add("/usr/lib/povray/include");
	Search_Path_Add("/usr/local/lib/povray/include");
	Search_Path_Add("/usr/lib/povray/povray31/include");
	Search_Path_Add("/usr/local/lib/povray/povray31/include");
	Search_Path_Add("/usr/lib/povray/megapovplus/include");
	Search_Path_Add("/usr/local/lib/povray/megapovplus/include");
	warn() << "Proprietary search path... (FIXME)";
	
	verbose(2) << "Token finder initialized. (errors: " << errors << ")";
	return(errors);
}


int File_Parser::_Set_Up_MCopy(Animation * /*ani*/,Scene_Interface & /*scene_if*/)
{
	assert(!mcopy);
	
	int errors=0;
	mcopy = new Modification_Copy(get_consultant());
	
	verbose(2) << "File copy routines initialized. (errors: " << errors << ")";
	return(errors);
}


int File_Parser::_Set_Up_FDump(Animation *ani,Scene_Interface &scene_if)
{
	assert(!fdump);
	
	int errors=0;
	fdump = new Frame_Dump(ani,scene_if,get_consultant());
	
	// File to be included at the end of the frame file. 
	std::string include_file;
	#warning strip path? use "../" as prefix?
	include_file=scene_if.get_filename();
	fdump->Set_Main_File(include_file);
	
	verbose(2) << "Frame writer initialized. (errors: " << errors << ")";
	return(errors);
}

void File_Parser::_Set_Up_SPS()
{
	sps.Clear();   // (should already be Clear())
	parse_errors=0;
	cc.reset();
	
	// Initialize a top level parser: 
	Register_Sub_Parser(new Toplevel_Parser(this));
}


// Returns 1 on error. 
int File_Parser::Register_Sub_Parser(File_Parser::Sub_Parser *sb)
{
	if(sps.size()>=config.parser_stack_limit)
	{  return(1);  }
	sps.Push(sb);
	++parser_added;
	return(0);
}

void File_Parser::_Sub_Parser_Done()
{
	Sub_Parser *top=sps.Top();
	// Remove this Sub_Parser from the stack: 
	sps.Pop();
	Sub_Parser *down=sps.Top();
	if(down)
	{
		// Tell that parser that the one above it is done. 
		down->errors+=top->errors;
		down->spdone(top);
	}
	else
	{  parse_errors+=top->errors;  }
	// And delete the parser. 
	delete top;
}


int File_Parser::Setup_Frame_Dump(Frame_Dump *fdump)
{
	verbose(2) << "Preparing frame output..." << message::noend;
	
	int nent=0;
	for(AValue *av=av_list.first; av; av=av->next)
	{
		bool dump_it=false;
		int dflags=0;
		switch(av->type())
		{
			case ComponentInterface::IFScalar:
			{
				dump_it=(config.frame_dump_all_scalars || av->need_active);
				
				// This should be the case for scalars...?!
				assert(av->need_active==av->nfound);
				
				if(verbose_level()>=4)
				{  verbose(4) << "Scalar \"" << av->cif.get_name() << "\": dump=" << 
					(dump_it ? "yes" : "no");  }
			} break;
			case ComponentInterface::IFObject:
			{
				if(config.frame_dump_object_info>2)  // all info
				{  dflags = Frame_Dump::DF_Obj_Mat | Frame_Dump::DF_Obj_Active;  }
				else if(config.frame_dump_object_info==1)
				{
					dflags = Frame_Dump::DF_Obj_Active;
					if(av->need_scale + av->need_scale_act)
					{  dflags|=Frame_Dump::DF_Obj_Scale;  }
					if(av->need_rot + av->need_rot_act)
					{  dflags|=Frame_Dump::DF_Obj_Rot;  }
					if(av->need_trans + av->need_trans_act)
					{  dflags|=Frame_Dump::DF_Obj_Trans;  }
				}
				else
				{
					if(av->need_active)
					{  dflags|=Frame_Dump::DF_Obj_Active;  }
					if(av->need_scale + av->need_scale_act)
					{  dflags|=(av->need_scale ? Frame_Dump::DF_Obj_Scale : Frame_Dump::DF_Obj_AScale);  }
					if(av->need_rot + av->need_rot_act)
					{  dflags|=(av->need_rot ? Frame_Dump::DF_Obj_Rot : Frame_Dump::DF_Obj_ARot);  }
					if(av->need_trans + av->need_trans_act)
					{  dflags|=(av->need_trans ? Frame_Dump::DF_Obj_Trans : Frame_Dump::DF_Obj_ATrans);  }
				}
				
				dump_it=(dflags ? true : false);
				
				if(verbose_level()>=4)
				{  verbose(4) << "Object \"" << av->cif.get_name() << "\": dump=" << 
					(dump_it ? "yes" : "no") << "; "
					"flags=" << int(dflags) << "; serial=" << av->serial;  }
			} break;
			default:  assert(0);  break;
		}
		
		if(dump_it)
		{
			fdump->Add_Entry(av->cif,(Frame_Dump::Dump_Flags)dflags,av->serial);
			++nent;
		}
	}
	
	verbose(2) << "done [" << nent << " entries]";
	
	return(0);
}


Frame_Dump *File_Parser::Transfer_FDump()
{
	Frame_Dump *tmp=fdump;
	fdump=NULL;
	return(tmp);
}


// Return 0 on success; !=0 on error 
int File_Parser::Go(Animation *ani,Scene_Interface &scene_if)
{
	// Scene name: scene_if.get_name()
	// Scene file: scene_if.get_filename()
	// Animation_Parameters: ani->GLOB.param 
	
	_Cleanup();
	verbose(1) << "processing scene \"" << scene_if.get_name() << "\".";
	
	int errors=0;
	bool dontparse=false;
	values::String file=scene_if.get_filename();
	if(!file)
	{
		error() << "Scene \"" << scene_if.get_name() << "\" has no associated file."; 
		// Don't know if this is an error. 
		//++errors;
		dontparse=true;
	}
	
	if(!errors && !dontparse)
	{
		errors+=_Set_Up_Finder(ani,scene_if);
		errors+=_Set_Up_MCopy(ani,scene_if);
		errors+=_Set_Up_FDump(ani,scene_if);
		_Set_Up_SPS();
		
		verbose(1) << "Starting to parse primary file \"" << file << 
			"\" (scene \"" << scene_if.get_name() << "\").";
		
		Recursive_Input_Stream::warn_multiple_include=config.warn_multiple_include;
		
		#if CALC_ELAPSED_TIME
			HTime starttime;
			starttime.SetCurr();
		#endif
		prev_char='\0';
		errors+=Recursive_Input_Stream::Start_Parser(file);
		#if CALC_ELAPSED_TIME
			double elapsed=starttime.ElapsedD(HTime::seconds);
			size_t nbytes=Recursive_Input_Stream::Read_Bytes();
		#endif
		
		if(verbose_level())
		{
			verbose(1) << "Parsing now done. Status: " << 
				((parse_errors || errors) ? "FAILED" : "OK") << message::noend;
			if(verbose_level()>1 && (parse_errors || errors))
			{  verbose(2) << "  [parse error counter: " << parse_errors << 
				"; other error counter: " << errors << "]" << message::noend;  }
			verbose(1) << "";  // <-- newline
			#if CALC_ELAPSED_TIME
			char tmp[64];
			if(elapsed<=0.0000001)
			{  strcpy(tmp,"lots of ");  }
			else
			{
				double tr=(nbytes/(elapsed*1024));
				if(tr<102.4)
				{  snprintf(tmp,64,"%.0f ",(tr*1024));  }
				else if(tr<102400.0)
				{  snprintf(tmp,64,"%.0f k",tr);  }
				else
				{  snprintf(tmp,64,"%.0f M",(tr/1024.0));  }
			}
			char tim[64];
			if(elapsed<1.0)
			{  snprintf(tim,64,"%.0f m",elapsed*1000.0);  }
			else if(elapsed<60.0)
			{  snprintf(tim,64,"%.1f ",elapsed);  }
			else
			{  snprintf(tim,64,"%.0f ",elapsed);  }
			
			verbose(1) << "  Parsed " << ((nbytes+512)/1024) << " kb in " << 
				tim << "sec (" << tmp << "b/sec)";
			#endif
		}
		
		av_list.Warn_Unused(warn());
		Warn_Active_Mismatch();
		
		// This function writes a verbose message itself: 
		Setup_Frame_Dump(fdump);
	}
	
	// Note: parse_errors has the number of parse errors which 
	//       are not part of the ones counted via errors. 
	if(errors)
	{
		error() << "Aborting work on scene \"" << scene_if.get_name() << 
			"\" due to errors.";
	}
	else
	{
		// Actually copy the files: 
		#warning path names need fixing. 
		std::string dest_path=ani->GLOB.param.ani_dir();
		verbose(1) << "Copying files to \"" << dest_path << "\"...";
		errors+=mcopy->DoCopy(dest_path);
		verbose() << "Copying files " << (errors ? "FAILED" : "done.");
	}
	
	// We may (not) clean up here: 
	// - modcopy: clean up; is done. 
	// - sps: clean up; is done. 
	// - fdump: NO; must dump files. 
	// - av_finder: it has strings copied but they should not have 
	//           been referenced (other than in AValue)
	//           clean it up (it referneces AValues which are cleaned up) 
	// - AValue list: clean it up; fdump has all needed info. 
	// - recinstr: it has InFile_Segment handles which should not 
	//           have been referenced (other than in AValue)
	//           DO NOT clean up; we may need it. 
	_CleanupUnneeded();  // does all that for us...
	
	return(errors+parse_errors);
}


int File_Parser::Warn_Active_Mismatch()
{
	int warnings=0;
	
	bool header_written=false;
	for(AValue *av=av_list.first; av; av=av->next)
	{
		if(av->type()==ComponentInterface::IFScalar)  continue;
		if(av->curr_active<=0) continue;
		++warnings;
		if(!config.warn_active_mismatch)  continue;
		if(!header_written)
		{  warn() << "warning: more if(active) then end statements: " << 
				av->cif.get_name() << message::noend;  header_written=true;  }
		else
		{  warn() << ", " << av->cif.get_name() << message::noend;  }
	}
	if(header_written)
	{  warn() << "";  }  // <-- newline
	
	header_written=false;
	for(AValue *av=av_list.first; av; av=av->next)
	{
		if(av->type()==ComponentInterface::IFScalar)  continue;
		if(av->curr_active>=0) continue;
		++warnings;
		if(!config.warn_active_mismatch)  continue;
		if(!header_written)
		{  warn() << "warning: less if(active) then end statements: " << 
			av->cif.get_name() << message::noend;  header_written=true;  }
		else
		{  warn() << ", " << av->cif.get_name() << message::noend;  }
	}
	if(header_written)
	{  warn() << "";  }  // <-- newline
	
	if(warnings && config.warn_active_mismatch)
	{
		warn() << 
			"         use insert.<object>.actend for active end "
			"statements; maybe you must set" << message::nl <<
			"         ###frame_dump_object_info=1/2### to dump all object vals "
			"(trans/rot/scale)" << message::nl <<
			"         into the frame include file.";
	}
	
	return(warnings);
}


File_Parser::File_Parser(message::Message_Consultant *mcon) : 
	Recursive_Input_Stream(mcon),
	config(this)
{
	mcopy=NULL;
	pp_finder=NULL;
	tok_finder=NULL;
	av_finder=NULL;
	pp_find_longest=0;
	fdump=NULL;
	too_many_spaces_explained=false;
	may_put_back=false;
	refill_buffer=false;
	prev_char='\0';
	
	_Set_Up_Finder_Const();
}


void File_Parser::_CleanupUnneeded()
{
	if(mcopy)      delete mcopy;       mcopy=NULL;
	if(av_finder)  delete av_finder;   av_finder=NULL;
	sps.Clear();
	av_list.Clear();
}

void File_Parser::_Cleanup()
{
	_CleanupUnneeded();
	if(fdump)  delete fdump;  fdump=NULL;
	parse_errors=0;
	cc.last_found_len=0;
	may_put_back=false;
	refill_buffer=false;
}

File_Parser::~File_Parser()
{
	_Cleanup();
	
	pp_find_longest=0;
	if(pp_finder)   delete pp_finder;    pp_finder=NULL;
	if(tok_finder)  delete tok_finder;   tok_finder=NULL;
}


File_Parser::Config::Config(Message_Reporter *msg_rep)
{
	// Set up default config: 
	allow_nested_comments=true;  // povray allows them. 
	warn_nested_comments=true;
	allow_multiline_strings=true;
	max_multi_line_str_lines=10;
	max_string_length=1024*10;  // 10 kb; enough?!?!
	search_current_path=+1;  // before search path
	scalar_multiple_decl=1;
	multiline_scalar_decl=1;   // 1 -> warn; 2 -> error
	copy_nonsource_files=true;
	cut_out_comments=false;
	default_obj_flags_mod=+1;
	default_obj_flags_mod_E=-1;
	warn_object_eof_boundary=1;
	max_wspace_cmt_2_flags=1;
	max_wspace_cmt_2_cmd=0;   // 0 -> unlimited
	max_wspace_cmt_2_name=0;  // 0 -> unlimited
	warn_uninterpreted_insert=true;
	warn_uninterpreted_parse_end=true;
	//#warning next val should be true: 
	warn_multiple_include=true;
	warn_active_mismatch=true;
	frame_dump_object_info=0;  // okay
	parser_stack_limit=1000;   // enough?!?!
	#warning next val should probably be true: 
	cmd_interprete_first_line=true;   // false;
	
	dump_tokens=(msg_rep->verbose_level()>3);
}


inline void File_Parser::Object_Flags::reset(int val)
{
	has_end_statement=0;  // always 
	unset=1;  // always
	insert_active=val;
	insert_scale=val;
	insert_rot=val;
	insert_trans=val;
}


inline File_Parser::AValue::AValue() : cif()
{
	next=NULL;
	nfound=0;
	serial=0;   // NOT -1 as unsigned. 
	locked=false;
	flags.reset();
	need_active=0;
	need_scale=0;  need_scale_act=0;
	need_rot=0;    need_rot_act=0;
	need_trans=0;  need_trans_act=0;
	curr_active=0;
}

inline File_Parser::AValue::AValue(const AValue &src) : cif(src.cif)
{
	next=NULL;
	
	nfound=src.nfound;
	serial=src.serial;   // NOT -1 as unsigned. 
	
	// If this assert fails, that might be ok; I'm just not 
	// aware that this will happen: 
	assert(!src.locked);
	locked=false;
	
	dec_start=src.dec_start;
	dec_end=src.dec_end;
	mat_pos=src.mat_pos;
	
	flags=src.flags;
	need_active=src.need_active;
	need_scale=src.need_scale;  need_scale_act=src.need_scale_act;
	need_rot=  src.need_rot;    need_rot_act=  src.need_rot_act;
	need_trans=src.need_trans;  need_trans_act=src.need_trans_act;
	curr_active=src.curr_active;
}

inline File_Parser::AValue::~AValue()
{
}

void File_Parser::Current_Context::reset()
{
	ib=NULL;
	src=NULL;
	srclen=0;
	fileoff=0;
	line=-1;
	eof=false;
	last_found_len=0;
	only_wspace=true;
	with_newline=0;
	_slt_reset=false;
}

/******************************************************************************/

File_Parser::AValue *File_Parser::AValue_List::Add(const AValue &av)
{
	AValue *n=new AValue(av);
	n->next=NULL;  // be sure...
	if(last)
	{  last->next=n;  }
	else
	{  first=n;  }
	last=n;
	return(n);
}

void File_Parser::AValue_List::Clear()
{
	while(first)
	{
		AValue *tmp=first;
		first=first->next;
		delete tmp;
	}
	last=NULL;
}

int File_Parser::AValue_List::_Warn_Unused(message::Message_Stream os,
	ComponentInterface::IFType type)
{
	int nunused=0;
	bool header_written=false;
	for(AValue *av=first; av; av=av->next)
	{
		if(av->nfound || av->type()!=type)  continue;
		if(!header_written)
		{
			os << "warning: unused " << Ani_Type_Name(type) << "s: " << 
				av->cif.get_name() << message::noend;
			header_written=true;
		}
		else
		{  os << ", " << av->cif.get_name() << message::noend;  }
		++nunused;
	}
	if(nunused)
	{  os << " [" << nunused << "]"; }
	else
	{  os << message::killmsg;  }
	return(nunused);
}

int File_Parser::AValue_List::Warn_Unused(message::Message_Stream os)
{
	int nunused=0;
	nunused+=_Warn_Unused(os,ComponentInterface::IFScalar);
	nunused+=_Warn_Unused(os,ComponentInterface::IFObject);
	if(!nunused)  // <- Make sure no empty "warning:" line appears. 
	{  os << message::killmsg;  }
	return(nunused);
}

/******************************************************************************/

void File_Parser::Sub_Parser_Stack::Push(File_Parser::Sub_Parser *sp)
{
	Node *n=new Node(sp);
	n->down=top;
	top=n;
	++cnt;
}

File_Parser::Sub_Parser *File_Parser::Sub_Parser_Stack::Pop()
{
	if(!top)
	{  return(NULL);  }
	Sub_Parser *sp=top->sp;
	Node *tmp=top;
	top=top->down;
	delete tmp;
	--cnt;
	return(sp);
}

File_Parser::Sub_Parser_Stack::Sub_Parser_Stack()
{
	top=NULL;
	cnt=0;
}

void File_Parser::Sub_Parser_Stack::Clear()
{
	while(Top())
	{  delete Pop();  }
	assert(!cnt);
}

File_Parser::Sub_Parser_Stack::~Sub_Parser_Stack()
{
	Clear();
}


File_Parser::Comment_Parser *File_Parser::Sub_Parser::Skip_Comment(
	Find_String::RV *rv,bool allow_insert_statements)
{
	if(rv->id==tCOpComment || rv->id==tCppComment)
	{
		Comment_Parser *cp=new Comment_Parser(fp,rv);
		cp->interprete_insert=allow_insert_statements;
		Register_Sub_Parser(cp);
		return(cp);
	}
	return(NULL);
}

std::string File_Parser::Sub_Parser::Rel_Pos(Position *pos)
{
	std::string ret;
	if(fp->Same_File(pos->handle,cc->ib->handle))
	{
		char tmp[64];
		snprintf(tmp,64,"line %d",pos->line);
		ret.assign(tmp);
	}
	else
	{
		ret.assign(fp->Get_Path(pos->handle));
		ret.append(":");
		ret.append(I2Str(pos->line));
	}
	return(ret);
}

std::string File_Parser::Sub_Parser::Startpos_Str(Position *start)
{
	std::string ret;
	assert(start->line>=0);   // otherwise is makes no sense calling this function 
	if(cc->line!=start->line || 
	   !fp->Same_File(start->handle,cc->ib->handle))
	{
		ret.assign(" starting in ");
		ret.append(Rel_Pos(start));
	}
	return(ret);
}

// Very special...
// Previous declaration location string
std::string File_Parser::Sub_Parser::Prev_Decl_Loc_Str(AValue *av)
{
	std::string str;
	if(av->nfound && !av->locked)
	{
		assert(!av->dec_start.unset());
		assert(!av->dec_end.unset());
		bool all_same = 
			fp->Same_File(av->dec_start.handle,av->dec_end.handle) && 
			fp->Same_File(av->dec_start.handle,cc->ib->handle);
		str.assign(" previously declared in " + 
			(all_same ? 
				std::string("lines ") : 
				std::string(Get_Path(av->dec_start.handle))+":") + 
			I2Str(av->dec_start.line) + " ... " + 
			(fp->Same_File(av->dec_start.handle,av->dec_end.handle) ? 
				std::string("") : 
				std::string(Get_Path(av->dec_end.handle))+":") + 
			I2Str(av->dec_end.line));
	}
	else if(!av->dec_start.unset())
	{
		str.assign(" previous declaration started in " + 
			(fp->Same_File(av->dec_start.handle,cc->ib->handle) ? 
				std::string("line ") : 
				(std::string(Get_Path(av->dec_start.handle))+":")) + 
			I2Str(av->dec_start.line));
	}
	else
	{  str.assign(" not yet declared.");  }
	return(str);
}

void File_Parser::Sub_Parser::Register_Sub_Parser(Sub_Parser *sb)
{
	if(fp->Register_Sub_Parser(sb))
	{
		Error_Header() << "parser ran out of stack (" << 
			fp->sps.size() << " instances).";
		error() << "This happens either due to a great "
			"misinterpretation of the input" << message::nl << 
			"by the parser or as the input is too deeply nested." << message::nl << 
			"In this case, increase ###parser_stack_limit.";
		Abort_Parsing();
	}
}

void File_Parser::Sub_Parser::_construct(File_Parser *parent,tokID tok)
{
	fp=parent;
	cc=&parent->cc;
	done=false;
	errors=0;
	parser_type=tok;  // tNone for toplevel parser ONLY. 
}

File_Parser::Sub_Parser::~Sub_Parser()
{
}

/******************************************************************************/

// Return value: 0 -> must return (e.g. sub parser installed)
//               1 -> must return; need input
//              -1 -> go on
int File_Parser::Toplevel_Parser::TL_Parse(Find_String::RV *rv)
{
	switch(rv->id)
	{
		case tString:
			Register_Sub_Parser(new String_Parser(fp,rv));
			waittok=tString;
			return(0);
		// case tCClComment: break; // Error about stray comment end written elsewhere. 
		// case tNumbersign: break; // reached if preprocessor cmd unknown 
		
		// POV-Ray tokens
		case tpDeclare:
			Register_Sub_Parser(new Declare_Parser(fp,rv));
			waittok=tpDeclare;
			return(0);
		case tpInclude:
			Register_Sub_Parser(new Include_Parser(fp,rv));
			waittok=tpInclude;
			return(0);
		case tpFileIdentifier:
		{
			Filename_Parser *np=new Filename_Parser(fp,rv);
			np->warn_no_string=1;
			Register_Sub_Parser(np);
			waittok=tpFileIdentifier;
			return(0);
		}
	}
	
	return(-1);
}

int File_Parser::Toplevel_Parser::parse(int eof)
{
	if(eof==2)  // eof==1 does not interest Toplevel_Parser. 
	{
		done=true;
		return(0);
	}
	
	// We must parse the POV-file. Let's come to the hard part...
	Find_String::RV rv;
	
	for(;;)
	{
		Find_Tok(&rv,false);
		if(!rv.found)  break;
		
		if(rv.id==tNewline)
			continue;
		
		{
			Comment_Parser *cp=Skip_Comment(&rv,/*interprete_insert_statements=*/true);
			if(cp)
			{
				cp->interprete_obj=2;   // interprete object name
				cp->interprete_parse_end=true;  // for errors
				waittok=t_Comment;
				return(0);
			}
		}
		
		int val=TL_Parse(&rv);
		if(val>=0)  return(val);
	}
	
	return(0);
}


int File_Parser::Toplevel_Parser::TL_Parse_Object(Comment_Parser *cp)
{
	if(!cp->name_valid)  return(0);
	
	AValue *av=cp->av;
	av->locked=true;  // Object parser does not do that. 
	if(cp->flags_valid)
	{  av->flags=cp->flags;  }
	else  // use defaults: 
	{  av->flags.reset();  }
	
	Object_Parser *op=new Object_Parser(fp,av,cp->obj_name);
	
	// Object_Parser must know the start point of the object 
	//####this is dirty: last_nl_pos is unset if have an object name cmt 
	// right after file begin (which is invalid so we should give a warning 
	// and ignore)
	op->obj_start = (cp->parser_type==tCppComment) ? 
		(fp->llast_nl_pos) : (fp->last_nl_pos);
	// ####fixme too: assumed opening brace before comment: 
	op->Set_Braces(1);
	
	Register_Sub_Parser(op);
	return(1);
}


void File_Parser::Toplevel_Parser::TL_Object_Parser_Done(File_Parser::Sub_Parser *sb)
{
	Object_Parser *op=(Object_Parser *)sb;
	assert(op->GetAV()->locked);
	// must unlock AValue. 
	op->GetAV()->locked=false;
}


int File_Parser::Toplevel_Parser::spdone(File_Parser::Sub_Parser *sb)
{
	assert(waittok!=tNone);
	
	tokID newwaittok=tNone;
	if(waittok==tpFileIdentifier)
	{
		Filename_Parser *p=(Filename_Parser *)sb;
		if(!errors && p->filename_valid)
		{
			assert(!p->errors);
			std::string file=p->filename;
			if(file.length())
			{  errors+=fp->Should_Copy(file,&p->fname_start);  }
		}
	}
	else if(waittok==t_Comment)
	{
		Comment_Parser *cp=(Comment_Parser *)sb;
		if(cp->encountered_parse_end)
		{
			assert(cp->av);
			Warning_Header() << "warning: encountered stray parse.end "
				"statement for " << Ani_Type_Name(cp->av->type()) << " `" << 
				cp->av->cif.get_name() << "\'.";
			Warning_Header() << "         `" << cp->av->cif.get_name() << 
				"\' was" << Prev_Decl_Loc_Str(cp->av);
		}
		if(TL_Parse_Object(cp))  // only if cp->name_valid
		{  newwaittok=taObject;  }
	}
	else if(waittok==taObject)
	{
		TL_Object_Parser_Done(sb);
	}
	
	waittok=newwaittok;
	return(0);
}

File_Parser::Toplevel_Parser::Toplevel_Parser(File_Parser *parent,tokID tok) : 
	Sub_Parser(parent,tok)
{
	waittok=tNone;
	//std::cerr << "  ADD:  Toplevel_Parser\n";
}

File_Parser::Toplevel_Parser::~Toplevel_Parser()
{
	//std::cerr << "  DONE: Toplevel_Parser\n";
}

/******************************************************************************/

int File_Parser::Include_Parser::parse(int eof)
{
	if(eof)  // eof==1 || eof==2
	{
		Error_Header() << "failed to include missing file.";
		++errors;
		done=true;
		return(0);
	}
	
	// ``include'' was already encountered and processed. 
	// We must scan in the file name. 
	inc_start_offset = cc->fileoff - cc->last_found_len;
	assert(fnparser);
	Register_Sub_Parser(fnparser);
	
	return(0);
}

int File_Parser::Include_Parser::spdone(File_Parser::Sub_Parser *sb)
{
	Filename_Parser *p=(Filename_Parser *)sb;
	assert(p==fnparser);
	fnparser=NULL;  // it's deleted when this function returns; we may not delete it 
	
	if(!errors && p->filename_valid)  // Only if there are no errors. (errors=this->errors+sb->errors)
	{
		assert(!p->errors);
		std::string file=p->filename;
		
		// We're a special friend of File_Parser. 
		int rv=fp->File_Parser::Include_File(file,
			/*offset=*/ inc_start_offset,
			/*inclen=*/ (cc->fileoff-inc_start_offset),
			/*honor_search_path=*/true,
			/*search_current_path=*/fp->config.search_current_path);
		if(rv==1)
		{  Error_Header() << "failed to include \"" << file << "\".";  }
		if(rv)
		{  ++errors;  }
	}
	
	done=true;
	return(0);
}

File_Parser::Include_Parser::Include_Parser(File_Parser *parent,
	Find_String::RV *tok) : 
	Sub_Parser(parent,tok)
{
	inc_start_offset=0;
	fnparser=new Filename_Parser(fp,tok);
	fnparser->warn_no_string=2;  // error
	//std::cerr << "  ADD:  Include_Parser\n";
}

File_Parser::Include_Parser::~Include_Parser()
{
	if(fnparser)  delete fnparser;
	//std::cerr << "  DONE: Include_Parser\n";
}

/******************************************************************************/

int File_Parser::Filename_Parser::parse(int eof)
{
	if(eof)  // eof==1 || eof==2
	{
		Error_Header() << "missing filename (EOF reached)";
		++errors;
		done=true;
		return(0);
	}
	
	// We must scan in the file name. 
	// [CMT] "filename" 
	if(fname_start.unset())
	{  fname_start=*cc;  }
	
	Find_String::RV rv;
	for(;;)
	{
		Find_Tok(&rv,false);
		if(!rv.found)  break;
		
		if(!cc->only_wspace || 
		   (rv.id!=tString && rv.id!=tNewline && 
		    rv.id!=tCOpComment && rv.id!=tCppComment))
		{
			if(warn_no_string)
			{  ((warn_no_string>1) ? Error_Header() : (Warning_Header() << "warning: ")) <<
				"filename string expected after `" << tok_name << "\'.";  }
			if(warn_no_string>1)  ++errors;
			Put_Back_Tok(&rv);
			done=true;
			break;
		}
		// #warning allowing no insert statement interpretation here. 
		if(Skip_Comment(&rv,/*allow insert statements*/false))
		{
			waittok=t_Comment;
			return(0);
		}
		if(rv.id==tString)
		{
			String_Parser *child=new String_Parser(fp,&rv);
			child->allow_multiline=false;
			Register_Sub_Parser(child);
			waittok=tString;
			return(0);
		}
	}
	
	return(0);
}

int File_Parser::Filename_Parser::spdone(File_Parser::Sub_Parser *sb)
{
	if(waittok==tString)
	{
		String_Parser *p=(String_Parser*)sb;
		filename=p->string;
		fname_start=p->str_start;
		fname_end=p->str_end;
		done=true;
		if(!p->errors)
		{  filename_valid=true;  }
		return(0);
	}
	return(0);
}

File_Parser::Filename_Parser::Filename_Parser(File_Parser *parent,
	Find_String::RV *tok) : 
	Sub_Parser(parent,tok)
{
	waittok=tNone;
	filename_valid=false;
	warn_no_string=1;
	
	assert(tok->found);
	tok_name=new char[tok->found_len+1];
	strncpy(tok_name,tok->found,tok->found_len);
	tok_name[tok->found_len]='\0';
	//std::cerr << "  ADD:  Filename_Parser\n";
}

File_Parser::Filename_Parser::~Filename_Parser()
{
	delete[] tok_name;
	//std::cerr << "  DONE: Filename_Parser\n";
}

/******************************************************************************/

void File_Parser::String_Parser::_addbytes(char *startptr,char *endptr)
{
	if(endptr<=startptr)  return;
	size_t alen=endptr-startptr;
	size_t slen=string.length();
	if(fp->config.max_string_length && 
	   slen+alen>fp->config.max_string_length)
	{
		if(fp->config.max_string_length>slen)
		{  string.append(startptr,fp->config.max_string_length-slen);  }
		_Unterminated_Str();
	}
	else
	{  string.append(startptr,alen);  }
}

void File_Parser::String_Parser::_Unterminated_Str()
{
	Error_Header(str_start.line) << "unterminted or too long string";
	++errors;
	string.append("...");
	// We cannot parse on if we found an unterminated string. 
	Abort_Parsing();  // sets done=true
}

int File_Parser::String_Parser::parse(int eof)
{
	if(eof)  // eof==1 || eof==2
	{
		Error_Header(str_start.line) << "unterimated string at EOF";
		++errors;
		done=true;
		return(0);
	}
	
	// We must scan in the string. 
	// ``String \" masked string "''  (the first `"' is already consumed) 
	Find_String::RV rv;
	
	if(str_start.unset())
	{  str_start=*cc;  }  // This is correct; the `"' is already consumed. 
	
	for(char *startptr=cc->src;;)
	{
		Find_Tok(&rv,false);
		if(!rv.found)
		{
			_addbytes(startptr,cc->src);
			break;
		}
		
		if(rv.id==tString)
		{
			// Okay, found end of string. 
			_addbytes(startptr,rv.ptr);
			str_end=*cc;
			str_end.fileoff-=cc->last_found_len;
			done=true;
			break;
		}
		else if(rv.id==tMaskedString)
		{
			// Found a ``\"''; replace with ``"''
			_addbytes(startptr,rv.ptr);
			string.append("\"");
			startptr=cc->src;
		}
		else if(rv.id==tNewline)
		{
			if(!allow_multiline)
			{
				Error_Header(str_start.line) << "string terminated by newline.";
				_addbytes(startptr,rv.ptr-1);
				++errors;
				done=true;
				break;
			}
			if(!multiline_warned)
			{
				Warning_Header(str_start.line) << "warning: multi-line string.";
			}
			++multiline_warned;
			_addbytes(startptr,rv.ptr);
			if(multiline_warned>fp->config.max_multi_line_str_lines && 
			   (!fp->config.max_string_length || 
			    string.length()<=fp->config.max_string_length))
			{
				_Unterminated_Str();
				break;
			}
			startptr=cc->src;
		}
	}
	
	return(0);
}

int File_Parser::String_Parser::spdone(Sub_Parser *)
{
	return(0);
}

File_Parser::String_Parser::String_Parser(File_Parser *parent,
	Find_String::RV *tok) : 
	Sub_Parser(parent,tok)
{
	multiline_warned=0;
	allow_multiline=fp->config.allow_multiline_strings;
	//std::cerr << "  ADD:  String_Parser\n";
}

File_Parser::String_Parser::~String_Parser()
{
	//std::cerr << "  DONE: String_Parser\n";
}

/******************************************************************************/

int File_Parser::Comment_Parser::_parse_flags(Find_String::RV *rv)
{
	if(cc->srclen<32 && !cc->ib->eof_reached)  // 32 is really enough for all flags
	{  return(1);  }   // need more input
	
	assert(rv->id==tColon);
	char *txt=rv->ptr;  // *txt=':';
	char *endtxt=cc->src+cc->srclen;
	++txt;
	bool _hes=false;
	if(txt<endtxt)
	{
		if(*txt=='e' || *txt=='E')
		{  _hes=true;  ++txt;  }
	}
	int bitop = _hes ? 
		fp->config.default_obj_flags_mod_E : fp->config.default_obj_flags_mod;
	if(txt<endtxt)
	{
		     if(*txt=='+')  {  bitop=+1;  ++txt;  }
		else if(*txt=='-')  {  bitop=-1;  ++txt;  }
	}
	flags.reset((bitop>0) ? 0 : 1);
	flags.has_end_statement=_hes;
	if(bitop<0)  bitop=0;
	int illegal=0;
	for(; txt<endtxt; txt++)
	{
		if(!isalpha(*txt))
		{  --txt;  break;  }
		switch(*txt)
		{
			case 'a':  flags.insert_active=bitop;  break;
			case 's':  flags.insert_scale=bitop;  break;
			case 'r':  flags.insert_rot=bitop;  break;
			case 't':  flags.insert_trans=bitop;  break;
			default:  if((++illegal)>2)  goto leavefor;  break;
		}
	}
	leavefor:;
	
	int parsed=(txt-rv->ptr)-illegal;
	if(txt==rv->ptr)
	{  Warning_Header() << "warning: flags for object `" << obj_name << 
		"\' expected.";  }
	else if(parsed)
	{
		if(illegal)
		{  Error_Header() << "illegal flags for object `" << obj_name << 
			"\'.";  ++errors;  }
		
		flags_valid=true;
		flags.unset=false;
	}
	
	Consumed(txt-rv->ptr);
	
	if(flags_valid && verbose_level()>3)
	{  verbose(4) << "Object `" << obj_name << "\' flags: " << 
		(flags.has_end_statement ? 'E' : '-') << 
		(flags.insert_active ? 'a' : '-') << 
		(flags.insert_scale ? 's' : '-') << 
		(flags.insert_rot ? 'r' : '-') << 
		(flags.insert_trans ? 't' : '-');  }
	
	return(0);
}


namespace
{
	// Returns true if equal
	bool str_compare(char *src,char *srcend,char *str)
	{
		for(;;)
		{
			if(!(*str))  return(true);
			if(src>=srcend)  break;
			if(*str!=*src)  break;
			++str;
			++src;
		}
		return(false);
	}
}


int File_Parser::Comment_Parser::_parse_insert_which()
{
	assert(in_insert_statement);
	
	if(cc->srclen<32 && !cc->ib->eof_reached)  // 32 is really enough for a insert specs
	{  return(1);  }   // need more input
	
	char *txt=cc->src;
	char *endtxt=cc->src+cc->srclen;
	
	char type='i';
	if(txt<endtxt)
	{
		if(*txt=='.')
		{  type='\0';  }
		else if(!isspace(*txt))
		{  _illegal_i_pe_statement();  return(0);  }
		++txt;
	}
	if(!type && txt<endtxt)
	{
		     if(str_compare(txt,endtxt,"rot"))       type='r';
		else if(str_compare(txt,endtxt,"scale"))     type='s';
		else if(str_compare(txt,endtxt,"trans"))     type='t';
		else if(str_compare(txt,endtxt,"activeend")) type='e';
		else if(str_compare(txt,endtxt,"actend"))    type='e';
		else if(str_compare(txt,endtxt,"active"))    type='a';
		// else type stays 0 and gives error 
	}
	if(!type || txt>=endtxt)
	{  _illegal_i_pe_statement();  return(0);  }
	
	if(verbose_level()>3)
	{  verbose(4) << "Insert statement for " << Ani_Type_Name(av->type()) << 
		" `" << obj_name << "\' of type >" << type << "<.";  }
	fp->Insert_Statement(av,&cmt_start,type);
	in_insert_statement=0;  // done. 
	return(0);
}


int File_Parser::Comment_Parser::_parse_parse_end_which()
{
	assert(in_parse_end_statement);
	
	if(cc->srclen<32 && !cc->ib->eof_reached)  // 32 is really enough for a parse end spec
	{  return(1);  }   // need more input
	
	char *txt=cc->src;  // *txt=':';
	char *endtxt=cc->src+cc->srclen;
	
	if(txt<endtxt)
	{
		if(*txt!='.')
		{  _illegal_i_pe_statement();  return(0);  }
		++txt;
	}
	if(str_compare(txt,endtxt,"end"))
	{
		encountered_parse_end=true;
		if(verbose_level()>3)
		{  verbose(4) << "Parse end statement for " << Ani_Type_Name(av->type()) << 
			" `" << obj_name << "\' found.";  }
	}
	else
	{  _illegal_i_pe_statement();  return(0);  }
	
	in_parse_end_statement=0;  // done. 
	return(0);
}


void File_Parser::Comment_Parser::_incomplete_i_pe_statement()
{
	Warning_Header(cmt_start.line) << "warning: incomplete " << 
		(in_insert_statement ? "insert" : "parse") << 
		" statement?";
	in_insert_statement=0;
	in_parse_end_statement=0;
}

void File_Parser::Comment_Parser::_illegal_i_pe_statement()
{
	Warning_Header(cmt_start.line) << "warning: illegal or incomplete " << 
		(in_insert_statement ? "insert" : "parse") << " statement?";
	in_insert_statement=0;
	in_parse_end_statement=0;
}


int File_Parser::Comment_Parser::_parse(int eof)
{
	if(!(!av || obj_name))
	{
		error() << av << obj_name;
		assert(!av || obj_name);   // obj_name should have been assigned 
	}
	
	if(eof)  // eof==1 || eof==2
	{
		if(in_insert_statement || in_parse_end_statement)
		{
			Warning_Header() << "warning: " << 
				(in_insert_statement ? "insert" : "parse") << 
				" statement terminated by EOF?";
			in_insert_statement=0;
			in_parse_end_statement=0;
		}
		if(style==tCOpComment)
		{
			Error_Header() << "unterminated C-style comment (EOF reached)";
			++errors;
		}
		// style==tCppComment: 
		// A C++-style comment can also be terminated legally by EOF. 
		cmt_end=*cc;
		done=true;
		return(0);
	}
	
	//  We must scan in the comment...
	// style==tCOpComment: ``blah blah blah*/''
	// style==tCppComment: ``blah blah blah\n''
	
	if(cmt_start.unset())
	{
		cmt_start=*cc;
		cmt_start.fileoff-=cc->last_found_len;
	}
	
	Find_String::RV rv;
	for(;;)
	{
		bool need_ani_names=(in_insert_statement || in_parse_end_statement) || 
			(!nested && interprete_obj>1 && comment_begin);
		Find_Tok(&rv,need_ani_names);
		if(!rv.found)  break;
		
		// in_insert_statement: 0 -> no; 1 -> ``insert.'' seen; 
		// 2 -> obj name seen; 
		// 3 -> insert spec (trans/rot/scale) seen 
		if(in_insert_statement || in_parse_end_statement)
		{
			assert(!(in_insert_statement && in_parse_end_statement));
			if(expect_pos.handle!=cc->ib->handle || 
			   expect_pos.fileoff+rv.found_len!=cc->fileoff)
			{  _illegal_i_pe_statement();  }
			if(in_insert_statement==1 || in_parse_end_statement==1)
			{
				if(rv.id==taObject)
				{
					av=(AValue*)(rv.hook);
					obj_name=rv.found;  // OKAY! (allocated by Find_String)
					expect_pos=*cc;
					// must make 0 -> 0 and 1 -> 2
					in_insert_statement*=2;
					in_parse_end_statement*=2;
				}
				else if(rv.id==taScalar)
				{  Warning_Header() << "warning: ignoring " << 
					(in_insert_statement ? "insert" : "parse") << " statement "
					"for " << Ani_Type_Name(((AValue*)(rv.hook))->type()) << 
					" `" << rv.found << "\'.";
				   in_insert_statement=0;  in_parse_end_statement=0;  }
				else
				{  _illegal_i_pe_statement();  }
			}
			// <-- NO else
			if(in_insert_statement==2)
			{
				// Gonna parse ourselves: 
				if(_parse_insert_which())
				{  return(1);  }  // more input
			}
			if(in_parse_end_statement==2)
			{
				if(_parse_parse_end_which())
				{  return(1);  }  // more input
			}
		}
		if(interprete_obj>1 && name_valid && comment_begin && rv.id==tColon)
		{
			assert(!in_insert_statement && !in_parse_end_statement);
			if(_parse_flags(&rv))
			{
				Put_Back_Tok(&rv);
				return(1);   // more input
			}
		}
		
		if(comment_begin && cc->only_wspace && !nested)
		{
			// +2: two chars for `/*' or `//' 
			size_t delta_off=cc->fileoff-(cmt_start.fileoff+2)-rv.found_len;
			bool cmd_space_ok = (!fp->config.max_wspace_cmt_2_cmd || 
			        delta_off <= fp->config.max_wspace_cmt_2_cmd);
			if(rv.id==tColon && interprete_obj==1 && 
			   (!fp->config.max_wspace_cmt_2_flags || 
			   delta_off <= fp->config.max_wspace_cmt_2_flags))  // flags only
			{
				if(_parse_flags(&rv))
				{
					Put_Back_Tok(&rv);
					return(1);   // more input
				}
			}
			else if(rv.id==tpAniInsert && cmd_space_ok)
			{
				if(interprete_insert)
				{
					assert(!in_insert_statement);
					in_insert_statement=1;
					expect_pos=*cc;  // where object name must be 
				}
				else if(fp->config.warn_uninterpreted_insert)
				{  Warning_Header() << "warning: possible insert statement "
					"not interpreted.";  }
			}
			else if(rv.id==tpAniParse && cmd_space_ok)
			{
				if(interprete_parse_end)
				{
					assert(!in_parse_end_statement);
					in_parse_end_statement=1;
					expect_pos=*cc;  // where object name must be 
				}
				else if(fp->config.warn_uninterpreted_parse_end)
				{  Warning_Header() << "warning: possible object parse end "
					"statement not interpreted.";  }
			}
		}
		if(interprete_obj>1 && comment_begin && 
		   cc->only_wspace && 
		   /*!cc->with_newline && */ (cmt_start.line==cc->line) && 
		   !nested && !name_valid)
		{
			size_t delta_off=cc->fileoff-(cmt_start.fileoff+2)-rv.found_len;
			// ###this check must be put at the name prefix code (once hacked)
			if(!fp->config.max_wspace_cmt_2_name || 
			   delta_off <= fp->config.max_wspace_cmt_2_name)
			{
				if(rv.id==taObject)
				{
					// Not sure about the first two asserts...
					assert(!obj_name);
					assert(!av);
					assert(!in_insert_statement && !in_parse_end_statement);
					obj_name=rv.found;   // allocated by Find_String. 
					av=(AValue*)(rv.hook);
					name_valid=true;
					// still gonna check for flags
					break;   // <- necessary to stay incomment_begin mode 
				}
				else if(rv.id==taScalar)
				{  Warning_Header() << "warning: name of " << 
					Ani_Type_Name(tokID(rv.id)) << " `" << rv.found << "\' "
					"in comment (ignored).";  }
			}
		}
		
		if(style==tCppComment)
		{
			comment_begin=false;
			if(rv.id==tNewline)
			{  done=true;  }
		}
		else // style==tCOpComment
		{
			if(rv.id!=tNewline || fp->config.cmd_interprete_first_line)
			{  comment_begin=false;  }
			
			// Pov-Ray allows nested C-style comments. 
			if(rv.id==tCOpComment)
			{
				if(nested)  // already in nested comment
				{  ++nested;  }
				else
				{
					if(fp->config.allow_nested_comments)
					{
						if(fp->config.warn_nested_comments)
						{  Warning_Header(cmt_start.line) << 
							"warning: nested C-style comment; "
							"next instance in line " << cc->line;  }

						Comment_Parser *cp=new Comment_Parser(fp,&rv);
						// Nested comment: 
						cp->nested=1;
						// (don't look for object names in nested comments)
						Register_Sub_Parser(cp);
						return(0);
					}
					else if(fp->config.warn_nested_comments)
					{  Warning_Header() << 
						"warning: non-followed nested C-style comment beginning "
						"on line " << cmt_start.line;  }
				}
			}
			else if(rv.id==tCClComment)
			{
				if(nested)  --nested;
				if(!nested)
				done=true;
			}
		}
		
		if(done)  break;   // NOT RETURN. 
	}
	
	if(done)
	{
		if(cmt_end.unset())
		{  cmt_end=*cc;  }
		
		if(in_insert_statement || in_parse_end_statement)
		{  _incomplete_i_pe_statement();  }
		
		// This is tricky; we must set cc->only_wspace=true here to 
		// the  value it had when the comment started. 
		// Otherwise, something like ``png // needed \n "testfile.png"'' 
		// could not be parsed correctly. 
		cc->only_wspace=_start_only_wspace;
	}
	
	return(0);
}

int File_Parser::Comment_Parser::parse(int eof)
{
	int rv=_parse(eof);
	if(done && !nested)
	{
		assert(!cmt_start.unset());
		assert(!cmt_end.unset());
		fp->Comment_Cutout(&cmt_start,&cmt_end);
	}
	return(rv);
}

int File_Parser::Comment_Parser::spdone(File_Parser::Sub_Parser *)
{
	// The nested comment instance is done. So what?
	return(0);
}

void File_Parser::Comment_Parser::Set_Object(AValue *_av,const char *_obj_name)
{
	av=_av;
	obj_name=_obj_name;
}

File_Parser::Comment_Parser::Comment_Parser(File_Parser *parent,
	Find_String::RV *cmt_style) : 
	Sub_Parser(parent,cmt_style)
{
	style=tokID(cmt_style->id);
	assert(style==tCOpComment || style==tCppComment);
	interprete_obj=0;
	interprete_insert=false;
	interprete_parse_end=false;
	flags_valid=false;
	name_valid=false;
	flags.reset();
	nested=0;
	comment_begin=true;
	in_insert_statement=0;
	in_parse_end_statement=0;
	encountered_parse_end=false;
	_start_only_wspace=cc->only_wspace;
	av=NULL;
	obj_name=NULL;
	//std::cerr << "  ADD:  Comment_Parser (errors=" << errors << ")\n";
}

File_Parser::Comment_Parser::~Comment_Parser()
{
	//std::cerr << "  DONE: Comment_Parser (errors=" << errors << ")\n";
}

/******************************************************************************/

char *File_Parser::Declare_Parser::_dectype_of(ComponentInterface::IFType id)
{
	char *rv=Ani_Type_Name_Spc(id);
	if(*rv=='?')  return("");  // OK.
	return(rv);
}

std::string File_Parser::Declare_Parser::_startline_str()
{
	return(Startpos_Str(&dec_start));
}

int File_Parser::Declare_Parser::parse(int eof)
{
	if(eof)
	{
		if(eof==2 || expect.id==ComponentInterface::IFScalar || 
			fp->config.warn_object_eof_boundary>1)
		{
			// Under some circumstance, this content may be written twice...
			Error_Header() << 
				_dectype_of(expect.id) << "declaration" << 
				(expect.name ? 
					(std::string(" of `")+expect.name+"\'") : 
					std::string("")) << _startline_str() << 
				" terminated by EOF.";
			++errors;
			done=true;
			if(expect.av)
			{  expect.av->locked=false;  }
		}
		else if(fp->config.warn_object_eof_boundary)
		  // object declarations may spread over several files 
		{
			Warning_Header(dec_start.line) << "warning: " << 
				_dectype_of(expect.id) << "declaration" << 
				(expect.name ? 
					(std::string(" of `")+expect.name+"\'") : 
					std::string("")) << _startline_str() << 
				" expands across EOF boundary.";
		}
		return(0);
	}
	
	if(dec_start.unset())
	{
		// ``declare'' was already encountered and processed. 
		// We must scan in var/value: 
		// either: ``[CMT] var [CMT] = [CMT] scalar [CMT] ;''   (note: semicolon)
		// or: ``[CMT] var [CMT] = [CMT] object [CMT] { ... }'' (no semicolon needed)
		dec_start=*cc;
		dec_start.fileoff-=cc->last_found_len;
	}
	
	Find_String::RV rv;
	for(;;)
	{
		bool need_ani_names=(state==sDeclare);
		Find_Tok(&rv,need_ani_names);
		if(!rv.found)  break;
		
		{
			Comment_Parser *cp=Skip_Comment(&rv,/*allow insert statements*/true);
			if(cp)  // there is a comment coming 
			{
				if((state==sDeclare && expect.id!=ComponentInterface::IFNone) || 
				    state==sAssign)
				{
					cp->interprete_obj=1;  // flags only 
					cp->Set_Object(expect.av,expect.name);
				}
				// Subparser already registered. 
				waittok=t_Comment;
				return(0);
			}
		}
		if(state==sDeclare)
		{
			if(rv.id==taScalar || rv.id==taObject)
			{
				if(!cc->only_wspace)
				{  Warning_Header() << "warning: in " << 
					_dectype_of(toIFType(tokID(rv.id))) << "declaration: "
					"garbage before identifier `" << rv.found << "\'.";  }
				if(expect.id==ComponentInterface::IFNone)
				{
					expect.id=toIFType(tokID(rv.id));
					expect.av=(AValue*)(rv.hook);
					expect.name=rv.found;  // OKAY! (allocated by Find_String)
					if(expect.av->locked)
					{
						Error_Header() << "cannot declare " << 
							_dectype_of(expect.id) << "`" << expect.name << "\' " 
							"inside an instace of itself. ";
						Error_Header() << "Previous instance beginning in " << 
							Rel_Pos(&dec_start) << ".";
						++errors;
						done=true;
						return(0);  // keep that
					}
					if(expect.av->nfound && fp->config.scalar_multiple_decl)
					{  ((fp->config.scalar_multiple_decl>1) ? Error_Header() : 
							(Warning_Header() << "warning: ")) << "encountering " << 
						(expect.av->nfound+1) << Num_Postfix(expect.av->nfound+1) << 
						" declaration of " << _dectype_of(expect.id) << 
						"`" << expect.name << "\'.";
					   Warning_Header() << "         previous one begins at " << 
						Rel_Pos(&expect.av->dec_start) << ".";
					   if(fp->config.scalar_multiple_decl>1)  ++errors;
					}
					expect.av->locked=true;
					expect.av->flags.reset(/*1*/);  // all insertions are set if user says nothing. 
					expect.av->dec_start=dec_start;  // needed for error reporting
				}
				else if(expect.id==rv.id)
				{
					if(!more_names_error)
					{  Error_Header() << "woops: encountered more than one " << 
						_dectype_of(expect.id) << "name (second: `" << rv.found << 
						"\') in declaration" << _startline_str() << ".";  }
					++more_names_error;
					++errors;
				}
				else
				{  Error_Header() << "woops: encountered " << 
					_dectype_of(toIFType(tokID(rv.id))) << "name `" << rv.found << 
					"\' after " << _dectype_of(expect.id) << "name `" << 
					expect.name << "\' in same declaration" << _startline_str() << 
					".";
				   ++errors;  }
			}
			else if(rv.id==tAssign)
			{
				if(expect.id==ComponentInterface::IFNone)
				{
					// We did not encounter a name after the declaration & 
					// before the assignment `=' either because there was 
					// none or the one(s) specified is/are not one(s) we 
					// are looking for. Nothing to be done. 
					done=true;
				}
				assign_pos=*cc;
				state=sAssign;
			}
			else if(rv.id==tSemicolon)
			{
				// Empty declaration?!
				if(expect.id!=ComponentInterface::IFNone)
				{  Warning_Header() << "warning: " << _dectype_of(expect.id) << 
					"declaration of `" << expect.name << "\'" << 
					_startline_str() << " is without assignment.";  }
				if(expect.id==ComponentInterface::IFScalar)
				{  goto scalar_decl_end;  /* ugly goto. */  }
				else if(expect.id==ComponentInterface::IFObject)
				{
					Warning_Header() << "warning: empty declaration of " << 
						Ani_Type_Name(expect.id) << " `" << expect.name << 
						"\' ignored.";
					// With scalars, we don't need the value, with objects we do. 
					// And an object typically is not terminated by a `;'. 
				}
				// Don't care. 
				done=true;
			}
			else 
			{
				if(rv.id==tpDeclare)
				{  Warning_Header() << "warning: declaration inside declaration" << 
					_startline_str() << " (inner one processed).";  }
				else if(rv.id==tNumbersign || is_pp_command(&rv))
				{  Warning_Header() << "warning: preprocessor command inside "
					"declaration" << _startline_str() << " (decl ignored).";  }
				else
				{  Warning_Header() << "warning: skipping wired declaration" << 
					_startline_str() << ".";  }
				Put_Back_Tok(&rv);
				done=true;
			}
		}
		else if(state==sAssign)   // already scanned ``#declare id =''
		{
			if(rv.id==tpDeclare)
			{
				Warning_Header() << "warning: declaration inside declaration "
					"of " << _dectype_of(expect.id) << "`" << expect.name << 
					"\'" << _startline_str() << " (inner one processed; "
					"missing `;\'?).";
				Put_Back_Tok(&rv);
				done=true;
			}
			else if(rv.id==tNumbersign || is_pp_command(&rv))
			{
				Error_Header() << "preprocessor command inside declaration "
					"rvalue of " << _dectype_of(expect.id) << "`" << 
					expect.name << "\'" << _startline_str() << ".";
				++errors;
				Put_Back_Tok(&rv);
				done=true;
			}
			if(expect.id==ComponentInterface::IFScalar)
			{
				if(rv.id==tSemicolon)
				{
					scalar_decl_end:;
					
					if(nbraces>0)
					{
						Error_Header() << "too few `}\' in scalar "
							"declaration of `" << expect.name << "\'" << 
							_startline_str() << ".";
						++errors;
						done=true;
					}
					
					// Okay, found end of declaration. 
					dec_end=*cc;  // semicolon already consumed 
					
					++expect.av->nfound;
					if(verbose_level()>2)
					{  verbose(3) << "Found definition of " << 
						Ani_Type_Name(expect.av->type()) << " `" << 
						expect.name << "\' in " << 
						Get_Path(dec_start.handle) << ":" << dec_start.line << 
						"..." << dec_end.line;  }
					
					AValue *av=expect.av;
					assert(av->type()==expect.id);
					av->dec_start=dec_start;
					av->dec_end=dec_end;
					// Insert positions are not needed for scalar 
					// declarations. 
					
					state=sValueEnd;
					done=true;
				}
				else if(rv.id==tOpBrace || rv.id==tClBrace)
				{
					if(rv.id==tOpBrace)
					{
						if(!scalar_brace_warning)
						{
							Warning_Header() << "warning: braces encountered in "
								"scalar declaration of `" << expect.name << 
								"\'" << _startline_str() << ".";
							scalar_brace_warning=true;
						}
						++nbraces;
					}
					else
					{
						--nbraces;
						if(nbraces<0)
						{
							Error_Header() << "too many `}\' in scalar "
								"declaration of `" << expect.name << "\'" << 
								_startline_str() << ".";
							++errors;
							done=true;
						}
					}
				}
				else if(rv.id==tString)
				{
					Register_Sub_Parser(new String_Parser(fp,&rv));
					waittok=tString;
					return(0);
				}
			}
			else if(expect.id==ComponentInterface::IFObject)
			{
				if(rv.id==tSemicolon)
				{
					Error_Header() << "no object definition for `" << 
						expect.name << "\' found in declaration" << 
						_startline_str() << ".";
					++errors;
					done=true;
				}
				else if(rv.id==tClBrace)
				{
					Error_Header() << "object declaration" << _startline_str() << 
						" begins with closing brace.";
					++errors;
					done=true;
				}
				else if(rv.id==tOpBrace)
				{
					Put_Back_Tok(&rv);  // object parser wants to parse opening brace 
					assert(!assign_pos.unset());
					// Must correct start value (was set to beginning of #declare)
					expect.av->dec_start=assign_pos;
					Object_Parser *op=new Object_Parser(fp,expect.av,expect.name);
					// Values to be set by us: 
					op->obj_start=assign_pos;
					Register_Sub_Parser(op);
					waittok=taObject;
					return(0);
				}
			}
		}
		
		if(fp->config.multiline_scalar_decl && 
		   !multiline_scalar_decl_warned && 
		   expect.id==ComponentInterface::IFScalar && 
		   cc->line!=dec_start.line)
		{
			bool err = (fp->config.multiline_scalar_decl>1);
			(err ? Error_Header(dec_start.line) : 
				(Warning_Header(dec_start.line) << "warning: " )) << 
				"scalar declaration " << 
				(expect.name ? (std::string("of `")+expect.name+"\' ") : "") << 
				"exceeds one line. (Missing `;\'?)";
			if(err)
			{  ++errors;  done=true;  }
			multiline_scalar_decl_warned=true;
		}
		
		if(done)  break;  // DO NOT RETURN HERE!
	}
	
	if(state==sValueEnd && done)
	{
		assert(expect.id!=ComponentInterface::IFNone);
		assert(expect.av);
		
		// Tell Modification_Copy: 
		if(!Process_AValue(expect.av))
		{  ++errors;  }
	}
	
	// Be sure to unlock the AValue. 
	if(done && expect.av)
	{   expect.av->locked=false;  }
	
	return(0);
}

int File_Parser::Declare_Parser::spdone(File_Parser::Sub_Parser *sb)
{
	assert(waittok!=tNone);
	
	if(waittok==taObject)
	{
		// Object parser done. Then we're done, too. 
		done=1;
		assert(expect.av);
		expect.av->locked=false;
	}
	else if(waittok==t_Comment)
	{
		Comment_Parser *cp=(Comment_Parser *)sb;
		if(cp->interprete_obj==1 && cp->flags_valid)
		{
			assert(expect.av);
			if(!expect.av->flags.unset)
			{
				Error_Header() << "declaration of " << 
					_dectype_of(expect.id) << "`" << expect.name << "\'" << 
					_startline_str() << " has flags set more than once.";
				++errors;
			}
			else
			{
				expect.av->flags=cp->flags;
				if(state==sAssign)
				{  Warning_Header() << "warning: should place flags comment "
					"before assignment in declaration of " << 
					_dectype_of(expect.id) << "`" << expect.name << "\'" << 
					_startline_str() << ".";  }
			}
		}
	}
	
	waittok=tNone;
	return(0);
}

File_Parser::Declare_Parser::Declare_Parser(File_Parser *parent,
	Find_String::RV *tok) : 
	Sub_Parser(parent,tok)
{
	waittok=tNone;
	expect.id=ComponentInterface::IFNone;
	expect.name=NULL;
	expect.av=NULL;
	state=sDeclare;
	more_names_error=0;
	nbraces=0;
	scalar_brace_warning=false;
	multiline_scalar_decl_warned=false;
	//std::cerr << "  ADD:  Declare_Parser (errors=" << errors << ")\n";
}

File_Parser::Declare_Parser::~Declare_Parser()
{
	//std::cerr << "  DONE: Declare_Parser (errors=" << errors << ")\n";
}

/******************************************************************************/

std::string File_Parser::Object_Parser::_startline_str()
{
	return(Startpos_Str(&obj_start));
}

int File_Parser::Object_Parser::parse(int eof)
{
	if(eof)
	{
		Toplevel_Parser::parse(eof);   // rv always 0 here. 
		
		if((eof==1 && fp->config.warn_object_eof_boundary>1) || eof==2)
		{
			Error_Header() << "declaration of object `" << obj_name << 
				"\'" << _startline_str() << " terminated by EOF.";
			++errors;
			done=1;
			return(0);
		}
		else if(fp->config.warn_object_eof_boundary)
		{  Warning_Header() << "warning: declaration of object `" << 
			obj_name << "\'" << _startline_str() << 
			" spreads across EOF boundary.";  }
	}
	
	Find_String::RV rv;
	for(;;)
	{
		bool need_ani_names=false;
		Find_Tok(&rv,need_ani_names);
		if(!rv.found)  break;
		
		{
			Comment_Parser *cp=Skip_Comment(&rv,/*interprete insert=*/true);
			if(cp)
			{
				cp->interprete_parse_end=!object_end_coming;  // also if we do not expect it (for errors)
				cp->interprete_obj=2;  // objects
				waittok=t_Comment;
				return(0);
			}
		}
		if(rv.id==tOpBrace)
		{
			++nbraces;
			if(object_end_coming)
			{
				Error_Header() << "opening brace `{\' encountered after "
					"parse.end statement in object declaration of `" << 
					obj_name << "\'" << _startline_str() << ".";
				++errors;
				done=true;
				Put_Back_Tok(&rv);
				break;
			}
		}
		else if(rv.id==tClBrace)
		{
			--nbraces;
			if(nbraces<0 && !av->flags.has_end_statement)
			{
				Error_Header() << "too many closing braces `}\' in object "
					"declaration of `" << obj_name << "\'" << _startline_str() << 
					".";
				++errors;
				done=true;
				Put_Back_Tok(&rv);
				break;
			}
			if((!nbraces && !av->flags.has_end_statement) || 
			    object_end_coming)
			{
				// Object end reached. 
				obj_end=*cc;
				
				mat_pos=*cc;
				mat_pos.fileoff-=rv.found_len;
				
				++av->nfound;
				if(verbose_level()>2)
				{  verbose(3) << "Found definition of " << Ani_Type_Name(av->type()) << 
					" `" << obj_name << "\' in " << 
					Get_Path(obj_start.handle) << ":" << obj_start.line << 
					" ... " <<
					Get_Path(obj_end.handle) << ":" << obj_end.line;  }

				av->dec_start=obj_start;
				av->dec_end=obj_end;
				av->mat_pos=mat_pos;
				// av->flags are set (or have defaults). 
				
				if(!Process_AValue(av))
				{  ++errors;  }
				
				done=true;
				break;
			}
		}
		else
		{
			int val=Toplevel_Parser::TL_Parse(&rv);
			if(val>=0)  return(val);
		}
		
		if(done)  break;
	}
	
	return(0);
}

int File_Parser::Object_Parser::spdone(File_Parser::Sub_Parser *sb)
{
	if(waittok==tNone)
	{  return(Toplevel_Parser::spdone(sb));  }
	
	tokID newwaittok=tNone;
	if(waittok==t_Comment)
	{
		Comment_Parser *cp=(Comment_Parser *)sb;
		if(cp->encountered_parse_end)
		{
			assert(cp->av);
			if(cp->av!=av)
			{
				Error_Header() << "object end statement of " << 
					Ani_Type_Name(cp->av->type()) << " `" << 
					cp->av->cif.get_name() << "\' found inside object "
					"declaration of `" << obj_name << "\'" << 
					_startline_str() << ".";
				if(cp->av->locked)
				{
					Error_Header() << "  object `" << obj_name << "\' "
						"seems to be nested inside declaration of " << 
						Ani_Type_Name(cp->av->type()) << " `" << 
						cp->av->cif.get_name() << "\'" << 
						Startpos_Str(&cp->av->dec_start) << ".";
					// #warning should we set "done" for us and the parser having locked the AValue? Can we?
				}
				++errors;
			}
			else
			{
				if(!av->flags.has_end_statement)
				{  Warning_Header() << "warning: declaration of " << 
					Ani_Type_Name(av->type()) << " `" << obj_name << 
					_startline_str() << " contains parse end statement.";
				   Error_Header() << "         probably you forgot to set "
				    "the `E\' flag; trusting end statement for now.";  }
				object_end_coming=true;
			}
		}
		if(Toplevel_Parser::TL_Parse_Object(cp))  // only if cp->name_valid
		{  newwaittok=taObject;  }
	}
	else if(waittok==taObject)
	{
		TL_Object_Parser_Done(sb);
	}
	
	waittok=newwaittok;
	return(0);
}

File_Parser::Object_Parser::Object_Parser(File_Parser *parent,AValue *_av,
	const char *_obj_name) : 
	Toplevel_Parser(parent,taObject)
{
	waittok=tNone;
	nbraces=0;
	object_end_coming=false;
	av=_av;
	obj_name=_obj_name;
	assert(av->locked);  // Caller must set lock and unset lock when we're done. 
	//std::cerr << "  ADD:  Object_Parser (errors=" << errors << ")\n";
}

File_Parser::Object_Parser::~Object_Parser()
{
	//std::cerr << "  DONE: Object_Parser (errors=" << errors << ")\n";
}

/******************************************************************************/


} }  // namespace end 
