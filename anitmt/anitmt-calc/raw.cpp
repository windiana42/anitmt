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
#include "val.hpp"
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
    std::list<Prop_Tree_Node*> scenes = ani->get_all_children();
    std::list<Prop_Tree_Node*>::iterator s;

    std::string basename = "raw_", extension = ".out";
    int startf = ani->param.startframe();
    int endf   = ani->param.endframe();

    for( int f = startf; f <= endf; f++ /*f += ani->param.jump()*/ )
    {
      char str_num[15];
      sprintf( str_num, "%04u", f );
      assert( ani->param.fps() != 0 );
      values::Scalar t = f / ani->param.fps();
      
      std::string filename = basename + str_num + extension;
      ofstream out( filename.c_str() );

      out << "time " << t << ";" << std::endl;
      for( s = scenes.begin(); s != scenes.end(); ++s )
      {
	out << "scene " << (*s)->get_name() << std::endl;
	out << "{" << std::endl;
	
	Ani_Scene *scene = dynamic_cast<Ani_Scene*>( *s );
	assert( scene != 0 );
	
	//*************************
	// output scalar components

	const Contain_Return<Scalar_State>::content_type &scalars = 
	  scene->get_scalars().get_content();
	
	Contain_Return<Scalar_State>::content_type::const_iterator scal;
	for( scal = scalars.begin(); scal != scalars.end(); ++scal )
	{
	  Prop_Tree_Node *node = dynamic_cast<Prop_Tree_Node*>( *scal );
	  out << "  scalar " << node->get_name() << std::endl;
	  out << "  {" << std::endl;
	  
	  std::pair<bool,Scalar_State> ret = (*scal)->get_return_value( t );
	  if( ret.first )
	    out << "    val " << ret.second.val << ";" << std::endl;
	  else
	    out << "    val <undefined>;" << endl;
	  
	  out << "  }" << std::endl;
	}

	//*************************
	// output object components


	out << "}" << std::endl;
      }
    }
  }
}


