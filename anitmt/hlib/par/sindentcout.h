#include <stdio.h>
#include <stdarg.h>
#include <hlib/prototypes.h>

#ifndef _HLIB_SimpleIndentConsoleOutput_H_
#define _HLIB_SimpleIndentConsoleOutput_H_ 1

namespace par
{

// Simple class to output indented text to a console (FILE*)
class SimpleIndentConsoleOutput
{
	private:
		int lpos;        // cursor X position (0,...)
		int columns;     // size of console 
		int indent;      // number of indention whitespaces 
		FILE *out;
		
		int _Print(const char *str,va_list ap);
		int _GetTermCols(int fd);
	public:  _CPP_OPERATORS_FF
		// Constructor sets columns to value obtained via ioctl() or 80 
		// and lpos=0. 
		SimpleIndentConsoleOutput(int *failflag=NULL);
		~SimpleIndentConsoleOutput() { }
		
		// Set FILE to write to. Defaults to stdout. 
		// Also tries to query terminal size and change # of columns. 
		void SetFile(FILE *out);
		
		// Get/Set lpos value. 
		int GetLPos()  {  return(lpos);  }
		void SetLPos(int _lpos)
			{  lpos = (_lpos<0) ? 0 : _lpos;  }
		
		// Get/Set columns value; columns cannot be <10. 
		int GetColumns()  {  return(columns);  }
		void SetColumns(int _columns)
			{  columns = (_columns<10) ? 10 : _columns;  }
		
		// Get/Set indent value; setting the indent value does 
		// not perform any indention; this is done when the next 
		// word is to be written. 
		// Indention cannot be larger than columns-10. 
		int GetIndent()  {  return(indent);  }
		void SetIndent(int _indent);
		void AddIndent(int delta)  {  SetIndent(indent+delta);  }
		
		// Actually print the passed string. 
		// If indent is <0, use the value set via SetIndent(), else 
		// use the specified indent value and call implicitly 
		// SetIndent(indent). 
		// The function does not know how to handle various terminal 
		// escape sequences, so don't use them or print them separately 
		// via Puts0(). 
		// Puts() does internal buffering but the buffer is flushed 
		// when it returns. 
		// Print() and operator() internally uses vsnprintf(). They require 
		// a C99-compatible snprintf (glibc>=2.1) and allow for any string 
		// length. Return value: 0 -> OK; 1 -> LMalloc() failed. 
		void Puts(const char *str,int indent=-1);
		int Print(const char *fmt,...);
		int operator()(const char *str,...);
		
		// Used to write special terminal escape strings which do not 
		// change the lpos of the terminal (e.g. change color). 
		void Puts0(const char *special_str)
			{  fprintf(out,"%s",special_str);  }
};

}  // namespace end 

#endif  /* _HLIB_SimpleIndentConsoleOutput_H_ */

