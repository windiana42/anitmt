/*
 * hreadline.h
 * 
 * Header containing class HReadLine, a HLib-compatible GNU readline(3) 
 * interface. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_HReadLine_H_
#define _HLIB_HReadLine_H_ 1

#include <hlib/fdmanager.h>
#include <hlib/fdbase.h>
#include <hlib/refstring.h>


// This is a HLib-compatible GNU readline(3) interface. 
// It requires FDManager and is derived from FDBase. 
// If your application needs to read terminal input from the user 
// and you wish to use a nice prompt with completion, etc., then 
// HReadLine is your friend. 
// It works as follows: 
// In your application, create exactly ONE instance of a HReadLine-derived 
// class. This class will then have to handle the terminal input and 
// custom completion. 
// Note that this derived class may override all the FDBase-provided virtual 
// functions but make sure that you call HReadLine::fdnotify() in case 
// you chose to override fdnotify() in your derived class. 
// HReadLine's fdnotify() will ignore those notifications which are not 
// for it. 
// HReadLine will call rlnotify() when a line or EOF was read. 
class HReadLine : public FDBase
{
	public:
		// There may only be one HReadLine object. 
		// This pointer is needed for readline(3) callbacks. 
		static HReadLine *manager;
		
		// Interface for readline(3): 
		static void _line_callback(char *line);
		
	public:
		struct RLNInfo
		{
			// This is the content of the read line. 
			// This is an empty string, if an empty line was inserted and 
			// a NULL ref if EOF was encountered [or alloc failure]. 
			RefString line;
			// Status value: 
			//  1 -> EOF reached
			//  0 -> OK, command set
			// -1 -> alloc failure
			int status;
			
			_CPP_OPERATORS_FF
			RLNInfo(int *failflag=NULL) : line(failflag)
				{  status=-2;  }
			~RLNInfo() {}
		};
		
	private:
		// Prompt string: 
		RefString prompt;
		// PollID for the terminal: 
		PollID pollid;
		// Input currently enabled?
		bool input_enabled;
		
		// Called by _line_callback(): 
		void line_callback(char *line);
		
	protected:
		// Should be overridden by derived class to handle read lines. 
		// Return value: 
		//  0 -> OK, add this line to history buffer
		//  1 -> do not add this line to history buffer
		// NOTE: You may modify rli->line; in this case the modified 
		//       version is added to the history if 1 is returned. 
		virtual int rlnotify(RLNInfo * /*rli*/)
			{  return(0);  }
		
		// [overriding a virtual from FDBase:]
		// If you also override it, then do not forget to call this one. 
		virtual int fdnotify(FDInfo *fdi);
		
	public:  _CPP_OPERATORS
		HReadLine(int *failflag);
		~HReadLine();
		
		// Call this to enable terminal input. Must be called to enable 
		// polling the terminal file descriptor. 
		void EnableInput();
		// Disable input. You may wish to do this while the application 
		// is working. 
		void DisableInput();
		
		// Set prompt string; use NULL ref for no prompt. 
		void SetPrompt(const RefString &prompt);
		
		// Can be called by abort() handler or so to make sure the 
		// tty is in sane state before aborting. 
		static void EmergencyCleanup();
};

#endif  /* _HLIB_HReadLine_H_ */
