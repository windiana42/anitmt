/*
 * pov.cpp
 * 
 * Implementation of output routines for povray code. 
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

#include "pov.hpp"

#include <assert.h>
#include "val.hpp"
#include "tmttype.hpp"
#include "scene.hpp"
#include "proptree.hpp"

#include "parse.hpp"

namespace anitmt
{

//! init interface (ex: check if scene file exists)
void Pov_Output::init() throw( EX )
{
	parser=new output_io::POV::File_Parser();
	
	if(verbose())
	{  vout << "Initializted POV output." << std::endl;  }
}


//! check the components (verify them and copy files)
void Pov_Output::check_components() throw( EX )
{
	assert(parser);
	
	if(verbose())
	{  vout << "POV output: checking components..." << std::endl;  }
	
	const Contain<Ani_Scene>::content_type &scenes = 
		ani->get_scenes().get_content();
	int errors=0;
	
	for(Contain<Ani_Scene>::content_type::const_iterator 
		sc=scenes.begin(); sc!=scenes.end(); sc++)  // for all scenes
	{
		errors+=parser->Go(ani,*sc);
	}
	
	if(verbose())
	{
		vout << "POV output: checking components: " << 
			(errors ? "FAILED" : "done") << std::endl;
	}
	
	if(!errors)
	{
		Dump_Node dn;
		dn.path = ani->param.ani_dir();
		dn.fdump=parser->Transfer_FDump();
		assert(dn.fdump);
		dn.fdump->Set_Verbose(verbose(),vout);
		dn_list.push_back(dn);
	}
	
	#warning errors not reported. 
}


//! process the resulting animation (ex: integrate it in scene description)
void Pov_Output::process_results() throw( EX )
{
	if(verbose())
	{  vout << "POV output: processing results..." << std::endl;  }
	
	//####what is better: iterating the scenes in the frames or the frames in the scenes
	//    I do the latter here but the first one would allow us to first 
	//    get all the scalar values/object matrices and then write them so that 
	//    we would calculate them all just once. 
	for(std::list<Dump_Node>::iterator i=dn_list.begin(); 
		i!=dn_list.end(); i++)
	{
		Dump_Node *dn=&(*i);
		
		std::string file;
		char tmp[32];
		
		// ###must fix frame numbering...
    	int startf = ani->param.startframe();
    	int endf   = ani->param.endframe();
		double fps = ani->param.fps();
    	for(int f=startf; f<=endf; f++ /*###f += ani->param.jump()*/ )
    	{
			snprintf(tmp,32,"%05d",f);
			file=dn->path+"frame"+tmp+".inc";
			values::Scalar time=f/fps;
			i->fdump->Write(file,time,f);
		}
	}
	
	if(verbose())
	{  vout << "POV output: processing results: done" << std::endl;  }
}


Pov_Output::Pov_Output(Animation *ani) : 
	Output_Interface(ani),
	vout(cout)
{
	assert(ani!=NULL);
	parser=NULL;
}

Pov_Output::~Pov_Output()
{
	if(parser)  delete parser;  parser=NULL;
	
	for(std::list<Dump_Node>::iterator i;;)
	{
		i=dn_list.begin();
		if(i==dn_list.end())  break;
		Dump_Node *dn=&(*i);
		delete dn->fdump;  // important (see File_Parser::Transfer_FDump())
		dn_list.pop_front();
	}
}

}  // namespace end


