/*
 * pov.hpp
 * 
 * Output interface for povray code. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to > wwieser -a- gmx -*- de <
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


#ifndef __AniTMT_Pov_Output_Interface__
#define __AniTMT_Pov_Output_Interface__

#include "../output.hpp"
#include "fdump.hpp"

#include <message/message.hpp>
#include <list>

namespace output_io { namespace POV { 
	class File_Parser; 
	class Frame_Dump;
} }

namespace anitmt
{

//! raw data output format
class Pov_Output : 
	public Output_Interface,
	public message::Message_Reporter
{
	private:
		Animation *ani;
		output_io::POV::File_Parser *parser;
		
		struct Dump_Node
		{
			std::string path;   // path without file
			output_io::POV::Frame_Dump *fdump;
			
			Dump_Node()  {  fdump=NULL;  }
			~Dump_Node()  { }
		};
		
		std::list<Dump_Node> dn_list;
	public:
		//! init interface (ex: check if scene file exists)
		virtual void init() throw( );
		//! check the components (verify them and copy files)
		virtual void check_components() throw( );
		//! process the resulting animation (ex: integrate it in scene description)
		virtual void process_results() throw( ); 
		
		Pov_Output(Animation *ani);
		virtual ~Pov_Output();
};

}  // namespace end 

#endif  /* __AniTMT_Pov_Output_Interface__ */

