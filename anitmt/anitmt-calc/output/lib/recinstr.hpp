/*
 * recinstr.hpp
 * 
 * Header for routines to read in nested files 
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


#ifndef _Inc_Recursive_Input_Stream_H_
#define _Inc_Recursive_Input_Stream_H_ 1

#include <list>
#include <message/message.hpp>

#include "binstr.hpp"
#include "searchpath.hpp"

namespace output_io
{

// This can be used to write the number postfix (last two chars) 
// for "1st", "2nd", "3rd", "4th", "5th", ...
extern const char *Num_Postfix(int n);

// Class to read in files which can #include other files 
// in turn etc. 
class Recursive_Input_Stream : 
	public message::Message_Reporter
{
	private:
		struct UInput_Stream : public Buffered_Input_Stream
		{
			UInput_Stream(const std::string &path) : Buffered_Input_Stream(path) 
				{  use_cnt=1;  }
			~UInput_Stream() throw() { }
			
			int use_cnt;   // file use counter 
		};
		struct Input_Buffer
		{
			char *buf;
			size_t size;   // size of buf 
			size_t len;    // length of data in buf 
			
			void Skip(size_t len);  // ``scroll'' the buffer 
			// Returns 1 if eof comes close and 2 if ibuflen=0 due to eof. 
			int FillUp(UInput_Stream *is,size_t *byte_counter=NULL);
			void Resize(size_t newsize);
			Input_Buffer(size_t size);
			~Input_Buffer();
		};
	public:
		class InFile_Segment
		{ private:  friend class Recursive_Input_Stream;
			InFile_Segment *prev,*next;
			UInput_Stream *is;  // pointer to infiles list entry
			size_t off;       // start offset in file
			size_t len;       // length of the segment
			int line0;        // start line number at off (1)
			int lines;        // number of lines relative to off
			int depth;        // recurse depth (0 -> top level)
			
			InFile_Segment()
			{ prev=NULL; next=NULL; is=NULL; 
				off=len=0; depth=-1; lines=0; line0=1;  }
			~InFile_Segment();  // dequeues itself. 
		};
		struct File_Context
		{
			InFile_Segment *handle;  // corresponding InFile_Segment
			std::string path;  // file name
			int depth;         // recursion depth 
			int line0;         // start line number
			size_t offset;     // file offset (bytes from beginning)
		};
		struct Parse_Input_Buf : File_Context
		{
			char *txt;
			size_t len;     // length of data in txt 
			bool eof_reached;
		};
		
	private:
		size_t ibuf_default_len;
		Input_Buffer *curr_ib;  // NOT via new; pointer to stack frame!
		size_t tot_read_bytes;
		
		// File hierarchy: 
		InFile_Segment start;  // top level input file start point
		InFile_Segment *last_ifs;  // pointer to last InFile_Segment
		
		// Returns the InFile_Segment with the depth (ifs->depth-1) or 
		// NULL if ifs->depth=0. Uses last_ifs if ifs=NULL. 
		InFile_Segment *_Prev_Depth(InFile_Segment *ifs);
		
		// A list of all input files read during parsing; do not 
		// assume anything about the order. 
		std::list<UInput_Stream*> infiles;
		
		Search_Path incpath;
		
		struct 
		{
			bool scheduled;   // Is there an include to be done?
			UInput_Stream *is;  // open input stream 
			// As specified with Include_File(): 
			size_t offset;
			size_t inclen;
		} include;
		bool abort_scheduled;
		
		int _Recursive_Read(InFile_Segment *ifs);
		
		// Checks if is->path is already in infiles list. 
		// Returns first UInput_Stream * or NULL. 
		UInput_Stream *_Input_File_In_List(UInput_Stream *is);
		// Checks if is->path is already in the hierarchy. 
		bool _Input_File_In_Hierarchy(UInput_Stream *is);
		
		// Add an InFile_Segment after last_ifs. 
		void _Append_IFS(InFile_Segment *ifs);
		
		InFile_Segment *h_p_ifs;  // hierarchy printed InFile_Segment 
		
		struct 
		{
			InFile_Segment *curr;
			size_t off;
			int line;
			size_t len;
			ssize_t skip;  // -1 -> close Input_Stream when read 
		} cr;   // current read
		
		bool _Try_Open_In_Current_Path(UInput_Stream *is,const std::string &file);
		bool _Try_Open_In_Searchpath(UInput_Stream *is,const std::string &file,
			bool honor_search_path,int search_current_path);
		
		int _Switch_CR(InFile_Segment *new_ifs);
		ssize_t _read_curr(char *buf,size_t len);
		InFile_Segment *_Next_Read_Segment(InFile_Segment *ifs);
		
		// Clear everything...
		// Puts class in same state as directly after construction 
		// except that the search path is unchanged 
		// (use Search_Path_Reset()). 
		void _Reset();
	protected: 
		// Most formats should be parsable with an ibuf size of 
		// 4096 (default). Use this if you need more. 
		// This sets ibuf_default_len as well as the current and 
		// ibuf size. Making the ibuf smaller will fail silently 
		// unless it is empty enough. 
		void Set_IBuf_Size(size_t size);
		
		// If, during the parsing, you find a statement which tells 
		// you to include a file, call this function and the class 
		// will open the file and read on there. 
		// honor_search_path: Shall the file be searched using the 
		//                    search path?
		// offset: byte offset in the file where the #include statement 
		//         begins (from beginning of the file). 
		// inclen: size of the #include statement (which has to be 
		//         skipped when we read on in the file). 
		// search_current_path: See Search_Path::try_open()
		// NOTE: see Search_Path::try_open() for further information 
		//       on path search and the values above. 
		// NOTE: When calling Include_File(), you must immediately 
		//       return from Parse_Input(). Parse_Input() must 
		//       have parsed exactly until the end of the include 
		//       statement. 
		// Return value: 
		//   0 -> OK; file found and opened
		//   1 -> could not open file (not found) YOU MUST PRINT ERROR. 
		//   2 -> not opened (include rcursion loop; error written)
		//   3 -> wanted to include file the second/third.. time but this 
		//        time opening failed. Error written. 
		int Include_File(const std::string &file,
			size_t offset,size_t inclen,
			bool honor_search_path=true,
			int search_current_path=1);
		
		// Useful for error messages: 
		// Prints include file hierarchy; writes: 
		// "file3 included from 
		//   file2:17 included from
		//   file1:48 included from
		//   file0:2"
		std::string Include_Hierarchy_String();
		
		// Print error header. Must be called from within 
		// Parse_Input() or Parse_EOF(). 
		message::Message_Stream Error_Header(message::Message_Stream os,
			int line,bool force_print_hierarchy=false);
		
		// Abort paring; Start_Parser() will return an error. 
		// Must return from Parse_Input() / Parse_EOF() after 
		// calling Abort_Parser(); the return value does not 
		// matter. 
		void Abort_Parser()
			{  abort_scheduled=true;  }
		bool Is_Abort_Scheduled()
			{  return(abort_scheduled);  }
		
		// The class reads in the input and passes it down here. 
		// The passed text to parse is in ib->txt; its size is ib->len. 
		// The function returns the number of parsed bytes. 
		// The next time the function gets called, ib->txt will not 
		// contain the bytes already parsed (according to the return 
		// value). 
		// BE SURE NOT TO DELETE ANY '\n' IN THE INPUT BUFFER. 
		virtual size_t Parse_Input(const Parse_Input_Buf *ib)=0;
		
		// This function gets called when end of file of the currently 
		// parsed file is reached and before we go on in the file 
		// which is one level higher. 
		// ib->len will be 0 here. 
		// This is thought to check for braces left to be closed 
		// etc. (hint: check ib->depth; depth=0 for top level) 
		virtual void Parse_EOF(const Parse_Input_Buf *ib)=0;
	public:
		Recursive_Input_Stream(message::Message_Consultant *mcons);
		virtual ~Recursive_Input_Stream();
		
		// Warn if files get included more than once?
		bool warn_multiple_include;
		
		// Really read the file(s) in. 
		// Call this or the class will no nothing. 
		// Pass toplevel file to read as top_file. 
		// Returns -1 if the specified file could not be opened. 
		// Returns 0 on success and >0 if errors occured. 
		int Start_Parser(const std::string &top_file);
		
		// Add a directory to the search path: 
		void Search_Path_Add(const std::string &dir)
			{  incpath.add(dir);  }
		void Search_Path_Reset()
			{  incpath.clear();  }
		
		// Includes can only be nested this often. The only purpose 
		// is to avoid infinite recursion; the parameter does NOT 
		// change any buffer size etc. 
		static int Max_Include_Depth;
		
		// Used to get information by a handle as passed in File_Context. 
		const char *Get_Path(InFile_Segment *handle)
			{  return(handle ? (handle->is->CPath()) : "???");  }
		
		// Are the two specified handles referring to the same file? 
		bool Same_File(InFile_Segment *handleA,InFile_Segment *handleB)
			{  return((handleA->is == handleB->is));  }
		
		// Get the number of bytes read in so far (in all files). 
		size_t Read_Bytes()  {  return(tot_read_bytes);  }
		
		// After having read in all files (Start_Parser() returned), 
		// you can re-read all the input (with all the included 
		// files) using read(). 
		// Up to len bytes are read in and copied into buf. 
		// The context is stored in fc if the retval is >0. 
		// (No context (besides fc->depth) is stored for retval=0) 
		// Note that <len bytes are only returned if the context 
		// of the next byte would belong to a different context 
		// than the bytes before. 
		// Return value: number of read bytes and 0 on EOF. 
		// NOTE: fc->depth is set to something negative and 0 is 
		//       returned if an error occured. 
		size_t read(char *buf,size_t len,File_Context *fc);
		// Function to rewind the read pointer for read(). 
		// MUST be called before read() is called the first time. 
		void rewind();
};

}  // namespace end 

#endif  /* _Inc_Recursive_Input_Stream_H_ */
