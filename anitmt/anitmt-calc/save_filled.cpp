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
    for( i=0; i<indent; i++ ) out << WN_INDENT_STRING;
    out << node->get_type() << " " << node->get_name() << " {" << std::endl;

    // write properties
    std::list<std::string> properties = node->get_properties_names();
    std::list<std::string>::iterator cp;
    for( cp = properties.begin(); cp != properties.end(); cp++ )
      {
	for( i = 0; i <= indent; i++ ) out << WN_INDENT_STRING;
	out << *cp << " ";	// write property name

	// Write property value
	// Special handling of string properties ("" around them)
	Property *p=node->get_property(*cp);
	bool str_mode=dynamic_cast<String_Property*>(p);
	if (str_mode) out << '"';
	out << *p;
	if (str_mode) out << '"';
	out << ';' << std::endl;
      }

    // write childs recursive
    std::list<Prop_Tree_Node*> childs = node->get_all_childs();
    std::list<Prop_Tree_Node*>::iterator cc;
    for( cc = childs.begin(); cc != childs.end(); cc++ )
      {
	// recursive call to write child node indented
	write_node( out, *cc, indent+1);
      }
 
    // write end of block
    for( i=0; i<indent; i++ ) out << WN_INDENT_STRING;
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

