/*****************************************************************************/
/**   This file offers the output in a raw data format           	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "raw.hpp"

#include <assert.h>
#include <fstream>
#include <val/val.hpp>
//#include "tmttype.hpp"
//#include "scene.hpp"
#include <proptree/proptree.hpp>

namespace anitmt
{
  //! init interface (ex: check if scene file exists)
  void Raw_Output::init() throw()
  {
  }

  //! check the components (verify them and copy files)
  void Raw_Output::check_components() throw()
  {
  }

  //! process the resulting animation (ex: integrate it in scene description)
  void Raw_Output::process_results() throw()
  {
    Prop_Tree_Interface prop_tree( ani->ani_root_original );

    // check whether there is a raw scene
    Scene_Interface first_raw_scene = prop_tree.get_first_scene();
    for( ; 
	 first_raw_scene != prop_tree.get_scene_end(); 
	 first_raw_scene = first_raw_scene.get_next() )
    {
      if( first_raw_scene.get_scene_type() == "raw" )
        break;
    }

    // if there is any raw scene...
    if( first_raw_scene != prop_tree.get_scene_end() )
    {
      std::string dir = ani->GLOB.param.ani_dir();
      std::string basename = "raw_", extension = ".out";
      int startf = ani->GLOB.param.startframe();
      int endf   = ani->GLOB.param.endframe();

      for( int f = startf; f <= endf; f++ /*f += ani->GLOB.param.jump()*/ )
      {
	verbose() << "Writing Frame " << f << "...";

	char str_num[20];
	sprintf( str_num, "%04d", f );
	assert( ani->GLOB.param.fps() != 0 );
	values::Scalar t = f / ani->GLOB.param.fps();
      
	std::string filename = dir + basename + str_num + extension;
	std::ofstream out( filename.c_str() );

	out << "time " << t << ";" << std::endl;
	for( Scene_Interface scene = first_raw_scene; 
	     scene != prop_tree.get_scene_end(); 
	     scene = scene.get_next() )
	{
	  if( scene.get_scene_type() != "raw" )
	    continue;		// continue if scene type is != raw

	  out << "scene " << scene.get_name() << std::endl;
	  out << "{" << std::endl;
	
	  // *************************
	  // output scalar components

	  for( Scalar_Component_Interface scalar = scene.get_first_scalar();
	       scalar != scene.get_scalar_end();
	       scalar = scalar.get_next() )
	  {
	    out << "  scalar " << scalar.get_name() << std::endl;
	    out << "  {" << std::endl;
	  
	    std::pair<bool,values::Scalar> ret = scalar.get_value( t );
	    if( ret.first )
	      out << "    val " << ret.second << ";" << std::endl;
	    else
	      out << "    val <undefined>;" << std::endl;
	  
	    out << "  }" << std::endl;
	  }

	  //*************************
	    // output object components

	    for( Object_Component_Interface object = scene.get_first_object();
		 object != scene.get_object_end();
		 object = object.get_next() )
	    {
	      out << "  object " << object.get_name() << std::endl;
	      out << "  {" << std::endl;
	  
	      std::pair<bool,Object_State> ret = object.get_state( t );
	      if( ret.first )
	      {
		out << "    matrix " << ret.second.matrix << ";" << std::endl;
		out << "    // equals: " << std::endl;
		out << "    translate " << ret.second.translate
		    << ";" << std::endl;
		out << "    rotate "    << ret.second.rotate
		    << ";" << std::endl;
		out << "    // equals: " << std::endl;
		out << "    position "  << ret.second.position
		    << ";" << std::endl;
		out << "    front " << ret.second.front
		    << ";" << std::endl;
		out << "    up-vector " << ret.second.up_vector
		    << ";" << std::endl;
	      }
	      else
		out << "    val <undefined>;" << std::endl;
	  
	      out << "  }" << std::endl;
	    }


	    out << "}" << std::endl;
	}
      }
    }
  }

  Raw_Output::Raw_Output( Animation *a )
    : message::Message_Reporter( a->GLOB.msg.get_consultant() ), ani(a)
  {

  }

}


