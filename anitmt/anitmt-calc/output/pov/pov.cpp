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
#include <val/val.hpp>
#include <proptree/proptree.hpp>

#include "parse.hpp"

namespace anitmt
{

//! init interface (ex: check if scene file exists)
void Pov_Output::init() throw( )
{
	#warning should create new Message_Consultant (interface...) for POV stuff...
	parser=new output_io::POV::File_Parser(get_consultant());
	
	verbose(2) << "Initializted POV output.";
}


//! check the components (verify them and copy files)
void Pov_Output::check_components() throw( )
{
	assert(parser);
	
	verbose(2) << "POV output: checking components...";
	
	int errors=0;
	Prop_Tree_Interface prop_tree( &ani->ani_root );
	
	// Start parser for all scenes: 
	for( Scene_Interface scene = prop_tree.get_first_scene(); 
	     scene != prop_tree.get_scene_end(); 
	     scene = scene.get_next() )
	{
		if( scene.get_scene_type() != "pov" )
			continue;
		errors+=parser->Go(ani,scene);
	}
	
	verbose(2) << "POV output: checking components: " << 
			(errors ? "FAILED" : "done");
	
	if(!errors)
	{
		Dump_Node dn;
		dn.path = ani->param.ani_dir();
		dn.fdump=parser->Transfer_FDump();
		assert(dn.fdump);
		dn_list.push_back(dn);
	}
	else
	{
		#warning errors not reported. 
		cerr << "Errors in Pov_Output::check_components()." << std::endl;
		abort();
	}
}


//! process the resulting animation (ex: integrate it in scene description)
void Pov_Output::process_results() throw( )
{
	verbose(2) << "POV output: processing results...";
	
	//####what is better: iterating the scenes in the frames or the frames in the scenes?
	//    I do the latter here but the first one would allow us to first 
	//    get all the scalar values/object matrices and then write them so that 
	//    we would calculate them all just once. 
	#warning FIXME...
	for(std::list<Dump_Node>::iterator i=dn_list.begin(); 
		i!=dn_list.end(); i++)
	{
		Dump_Node *dn=&(*i);
		
		std::string file;
		char tmp[32];
		
		// ###must fix frame numbering...
    	int startf = ani->GLOB.param.startframe();
    	int endf   = ani->GLOB.param.endframe();
		double fps = ani->GLOB.param.fps();
    	for(int f=startf; f<=endf; f++ /*###f += ani->GLOB.param.jump()*/ )
    	{
			snprintf(tmp,32,"%07d",f);
			file=dn->path+"f"+tmp+".pov";
			values::Scalar time=f/fps;
			i->fdump->Write(file,time,f);
		}
	}
	
	verbose(2) << "POV output: processing results: done";
}


Pov_Output::Pov_Output(Animation *a) : 
	message::Message_Reporter( a->GLOB.msg.get_consultant() ),
	ani(a)
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


