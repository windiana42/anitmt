/*****************************************************************************/
/**   This file offers a save function in the filled ADL format		    **/
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

#include "save_filled.hpp"

#include <fstream>

namespace anitmt {
  void write_node( std::ofstream &out, Prop_Tree_Node* node, int indent ){
    int i;
    // write type and name
    for( i=0; i<indent; i++ ) out << " ";
    out << node->get_type() << " " << node->get_name() << " {" << std::endl;

    // write properties
    std::list<std::string> properties = node->get_properties_names();
    std::list<std::string>::iterator cp;
    for( cp = properties.begin(); cp != properties.end(); cp++ )
      {
	for( i = 0; i < indent+WN_INDENT_WIDTH; i++ ) out << " ";
	out << *cp << " ";	// write property name
	out << *node->get_property( *cp ) << ';' << std::endl;
      }

    // write childs recursive
    std::list<Prop_Tree_Node*> childs = node->get_all_childs();
    std::list<Prop_Tree_Node*>::iterator cc;
    for( cc = childs.begin(); cc != childs.end(); cc++ )
      {
	// recursive call to write child node indented
	write_node( out, *cc, indent + WN_INDENT_WIDTH );
      }
 
    // write end of block
    for( i=0; i<indent; i++ ) out << " ";
    out << "}" << std::endl;
  }

  void save_filled( std::string filename, Animation *root ){
    std::ofstream out( filename.c_str() );

    // write all scenes
    std::list<Prop_Tree_Node*> childs = root->get_all_childs();
    std::list<Prop_Tree_Node*>::iterator cc;
    for( cc = childs.begin(); cc != childs.end(); cc++ )
      {
	// start recursive write 
	write_node( out, *cc, 0 );
      }
  }
}

