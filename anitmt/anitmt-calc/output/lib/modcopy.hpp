/*
 * modcopy.hpp 
 * Copyright (c) 2001 by Wolfgang Wieser 
 * Bugs to wwieser@gmx.de
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

#ifndef _Inc_Modification_Copy_H_
#define _Inc_Modification_Copy_H_

#include <iostream>
#include <string>

namespace output_io
{

class Input_Stream;
class Output_Stream;
class Buffered_Input_Stream;
class Buffered_Output_Stream;

/*
 * Modification_Copy is a class to modify a file while copying it. 
 * You may delete bytes from the file and add bytes to the file. 
 * Modification_Copy will work with binary data without problems. 
 * You can use Modification_Copy also to just copy data without 
 * modifying it nearly without any speed penalty. 
 */

class Modification_Copy
{
	public:
		enum MCAction
		{
			_MC_None=0,  // internal
			MC_Delete,
			MC_Insert
		};
	private:
		struct ModNode
		{
			ModNode *prev,*next;  // sorted by pos 
			MCAction action;
			size_t pos;  
			char *txt;  // only set for MC_Insert NOT TERMINATED. 
			size_t len;  // strlen(txt) or delete length 
			
			ModNode();
			~ModNode();
		};
		struct CopyFile
		{
			CopyFile *prev,*next;
			std::string path;
			ModNode *mod,*lastmod;
			bool cused;    // for DoCopy(Recursive_Input_Stream...)
			
			void Clear();
			CopyFile();
			~CopyFile()  {  Clear();  }
		};
		
		struct CopyBuffer
		{
			char *buf;
			size_t len;
			size_t size;
			
			CopyBuffer(size_t _size)
				{  buf=new char[_size];  len=0;  size=_size;  }
			~CopyBuffer()
				{  if(buf)  delete[] buf;  }
		};
		
		CopyFile *cfirst;  // Random order 
		
		CopyFile *_Find_FileNode(const std::string &path);
		CopyFile *_Ensure_FileNode(const std::string &path);
		
		ModNode *_Alloc_ModNode(MCAction act,size_t pos,const char *txt,size_t len);
		void _Queue_ModNode(CopyFile *cf,ModNode *n);
		
		int _Copy_File(const std::string &output_path,CopyFile *cf);
		int _Skip_Bytes(Buffered_Input_Stream *inf,size_t nbytes);
		int _Copy_Till(Input_Stream *inf,Output_Stream *outf,CopyBuffer *buf,size_t endipos);
		void _Copy_TillEOF(Input_Stream *inf,Output_Stream *outf,CopyBuffer *buf);
		
		void _Reset();
		
		int verbose;
		std::ostream *_vout;
		std::ostream &vout()  {  return(*_vout);  }
	public:
		Modification_Copy();
		~Modification_Copy();
		
		// Set verbosity level and verbose stream: 
		void Set_Verbose(int verbose,std::ostream &vout);
		
		// Add a modification entry: 
		// path: file to which to add the modification 
		// pos: file position from the beginning of the file 
		//      this is the position encountered while reading 
		//      the file and it is corrected with regard to 
		//      prior insertions/deletions. 
		// txt: text to be inserted. 
		// len: length of txt or -1 if strlen() should be 
		//      applied. 
		void MInsert(const std::string &path,size_t pos,
			const char *txt,ssize_t len=-1);
		// len: length of text beginning at pos to be deleted. 
		void MDelete(const std::string &path,size_t pos,
			size_t len);
		
		// Ensure that the specified file gets copied even if 
		// there are no modifications. 
		void MFile(const std::string &path);
		
		// Actually perform the copying: 
		// The input files get their path cut off and 
		// output_path+"/" prepended. 
		// One input file will generate one output file. 
		// Return value: 0 -> OK; >0 -> errors
		int DoCopy(const std::string &output_path);
		
		// Actually perform the copying: 
		// The input is read from the Recursive_Input_Stream and 
		// the output is written the the specified path. 
		// All included files will be merged into one file. 
		// All files which are in the list to copy but which were 
		// not read with Recursive_Input_Stream (e.g. data files 
		// (png texture file, etc.) to be copied but not modified 
		// are then copied as with DoCopy(output_path) above with 
		// the destination name output_path+"/"+basename. 
		int DoCopy(class Recursive_Input_Stream *ris,
			const std::string &output_file,
			const std::string &output_path);
};

}  // namespace end

#endif  /* _Inc_Modification_Copy_H_ */
