/*****************************************************************************/
/**   This file offers the output in a raw data format           	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "raw.hpp"

#include <assert.h>
#include <fstream>
#include <val/val.hpp>
#include "tmttype.hpp"
#include "scene.hpp"
#include "proptree.hpp"

namespace anitmt
{
  //! init interface (ex: check if scene file exists)
  void Raw_Output::init() throw( EX )
  {
  }

  //! check the components (verify them and copy files)
  void Raw_Output::check_components() throw( EX )
  {
  }

  //! process the resulting animation (ex: integrate it in scene description)
  void Raw_Output::process_results() throw( EX )
  {
    const Contain<Ani_Scene>::content_type &scenes 
      = ani->get_scenes().get_content();
    Contain<Ani_Scene>::content_type::const_iterator scene;

    std::string dir = ani->param.ani_dir();
    std::string basename = "raw_", extension = ".out";
    int startf = ani->param.startframe();
    int endf   = ani->param.endframe();

    for( int f = startf; f <= endf; f++ /*f += ani->param.jump()*/ )
    {
      char str_num[20];
      sprintf( str_num, "%04d", f );
      assert( ani->param.fps() != 0 );
      values::Scalar t = f / ani->param.fps();
      
      std::string filename = dir + basename + str_num + extension;
      ofstream out( filename.c_str() );

      out << "time " << t << ";" << std::endl;
      for( scene = scenes.begin(); scene != scenes.end(); ++scene )
      {
	out << "scene " << (*scene)->get_name() << std::endl;
	out << "{" << std::endl;
	
	//*************************
	// output scalar components

	const Contain< Ani_Scalar >::content_type &scalars = 
	  (*scene)->get_scalars().get_content();
	Contain< Ani_Scalar >::content_type::const_iterator scalar;

	for( scalar = scalars.begin(); scalar != scalars.end(); ++scalar )
	{
	  out << "  scalar " << (*scalar)->get_name() << std::endl;
	  out << "  {" << std::endl;
	  
	  std::pair<bool,Scalar_State> ret = (*scalar)->get_return_value( t );
	  if( ret.first )
	    out << "    val " << ret.second.get_value() << ";" << std::endl;
	  else
	    out << "    val <undefined>;" << endl;
	  
	  out << "  }" << std::endl;
	}

	//*************************
	// output object components

	const Contain< Ani_Object >::content_type &objects = 
	  (*scene)->get_objects().get_content();
	Contain< Ani_Object >::content_type::const_iterator object;

	for( object = objects.begin(); object != objects.end(); ++object )
	{
	  out << "  object " << (*object)->get_name() << std::endl;
	  out << "  {" << std::endl;
	  
	  std::pair<bool,Object_State> ret = (*object)->get_return_value( t );
	  if( ret.first )
	  {
	    out << "    matrix " << ret.second.get_matrix()<< ";" << std::endl;
	    out << "    // equals: " << endl;
	    out << "    scale "     << ret.second.get_scale() 
		<< ";" << std::endl;
	    out << "    translate " << ret.second.get_translate()
		<< ";" << std::endl;
	    out << "    rotate "    << ret.second.get_rotate()
		<< ";" << std::endl;
	  }
	  else
	    out << "    val <undefined>;" << endl;
	  
	  out << "  }" << std::endl;
	}


	out << "}" << std::endl;
      }
    }
  }
}


