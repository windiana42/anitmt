/*****************************************************************************/
/**   This file offers functions to generate C++ Code from AFD data tree   **/
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

#include "gen_cpp.hpp"

#include <fstream>

namespace funcgen
{
  // ****************************************************************
  // code_gen_info:  additional information for the code generation
  
  code_gen_info::code_gen_info( std::string name ) 
    : base_name(name), id_name(name)
  {}

  // **********************************************************
  // Cpp_Code_Translator: translates code peaces to C++ code
  std::string Cpp_Code_Translator::Cpp_Code_Translator::open_block()
  {
    return "{\n";
  }
  std::string Cpp_Code_Translator::close_block()
  {
    return "}\n";
  }
  std::string Cpp_Code_Translator::base_type( std::string name )
  {
    return prefix_base_type + name;
  }
  std::string Cpp_Code_Translator::provider_type( std::string name )
  {
    return prefix_provider_type + name;
  }
  std::string Cpp_Code_Translator::node_type( std::string name )
  {
    return prefix_node_type + name;
  }
  std::string Cpp_Code_Translator::result_function_decl
  ( std::string provider_type, std::string ret_type, std::string par_type )
  {
    return prefix_res_fun + provider_type + '_' + ret_type + '_' + par_type 
      + "( " + prefix_base_type + par_type + " )";
  }
  std::string Cpp_Code_Translator::result_function_impl
  ( std::string provider_type, std::string ret_type, std::string par_type,
    std::string par )
  {
    return prefix_res_fun + ret_type + '_' + par_type 
      + "( " + prefix_base_type + par_type + " " + par + " )";
  }
  std::string Cpp_Code_Translator::result_function_call
  ( std::string provider_type, std::string ret_type, std::string par_type,
    std::string par )
  {
    return prefix_res_fun + ret_type + '_' + par_type 
      + "( " + par + " )";
  }
  std::string Cpp_Code_Translator::start_param
  ( std::string provider_type, std::string ret_type, std::string par_type )
  {
    return prefix_param_range + provider_type + '_' + ret_type + '_' 
      + par_type + "_start_param";
  }
  std::string Cpp_Code_Translator::end_param
  ( std::string provider_type, std::string ret_type, std::string par_type )
  {
    return prefix_param_range + provider_type + '_' + ret_type + '_' 
      + par_type + "_end_param";
  }
  std::string Cpp_Code_Translator::prop_op( std::string name )
  {
    return prefix_prop_op + name;
  }
  std::string Cpp_Code_Translator::prop_op_value( std::string name )
  {
    return prefix_prop_op + name + "()";
  }
  std::string Cpp_Code_Translator::property_type( std::string name )
  {
    if( name == "flag" )
      return "Flag_Property";
    if( name == "scalar" )
      return "Scalar_Property";
    if( name == "vector" )
      return "Vector_Property";
    if( name == "matrix" )
      return "Matrix_Property";
    if( name == "string" )
      return "String_Property";

    // this shouldn't happen in the current specification    
    return "Type_Property<"+name+">";
  }
  std::string Cpp_Code_Translator::operand_type( std::string name )
  {
    return "Operand<"+name+">";
  }
  std::string Cpp_Code_Translator::container( std::string prov_type ) 
  {
    return prefix_container_type + prov_type;
  }  
  std::string Cpp_Code_Translator::serial_container( std::string prov_type ) 
  {
    return prefix_serial_container_type + prov_type;
  }  
  std::string Cpp_Code_Translator::container_name( std::string prov_type ) 
  {
    return prefix_container_name + prov_type;
  }  
  std::string Cpp_Code_Translator::child_result
  ( std::string prov_type, std::string ret_type, std::string par_type,
    std::string par )
  {
    return container_name( prov_type ) + "." 
      + result_function_call( prov_type, ret_type, par_type, par ) ;
  }
  std::string Cpp_Code_Translator::provided_result
  ( std::string prov_type, std::string ret_type, std::string par_type,
    std::string par )
  {
    return result_function_call( prov_type, ret_type, par_type, par );
  }
  std::string Cpp_Code_Translator::first_init( std::string provider_type )
  {
    return prefix_res_fun + provider_type + "_first_init";
  }
  std::string Cpp_Code_Translator::last_init( std::string provider_type )
  {
    return prefix_res_fun + provider_type + "_last_init";
  }

  Cpp_Code_Translator::Cpp_Code_Translator()
    : prefix_base_type(""), prefix_provider_type("_pt_"), 
      prefix_node_type(""), prefix_prop_op("_op_"), prefix_res_fun("_rf_"),
      prefix_param_range("_pr_"), prefix_container_type("_container_"), 
      prefix_serial_container_type("_serial_container_"), 
      prefix_container_name("_cn_")
  {}

  // ****************************************
  //! Cpp_Code_Generator: generates C++ code

  void Cpp_Code_Generator::generate_header()
  {
    *decl << "// generated file by funcgen" << std::endl;
    *decl << std::endl;
    *decl << "#ifndef __functionality_"+info->id_name+"__" << std::endl;
    *decl << "#define __functionality_"+info->id_name+"__" << std::endl;
    *decl << std::endl;
    *decl << "#include \""+info->base_name+".hpp\"" << std::endl;
    *decl << std::endl;
    *decl << "#include <list>" << std::endl;
    *decl << "#include <string>" << std::endl;
    *decl << "#include <map>" << std::endl;
    *decl << std::endl;
    *decl << "#include <val.hpp>" << std::endl;
    *decl << "#include <operand.hpp>" << std::endl;
    *decl << "#include <property.hpp>" << std::endl;
    *decl << "#include <proptree.hpp>" << std::endl;
    *decl << std::endl;
    *decl << "namespace functionality" << std::endl;
    *decl << "{" << std::endl;

    *impl << "// generated file by funcgen" << std::endl;
    *decl << std::endl;
    *impl << "namespace functionality" << std::endl;
    *impl << "{" << std::endl;
  }
  void Cpp_Code_Generator::generate_footer()
  {
    *impl << "}" << std::endl;
    *decl << "}" << std::endl;
    *decl << "#endif" << std::endl;
  }

  void Cpp_Code_Generator::generate_base_types()
  {
    *decl << "  // **********************" << std::endl
          << "  // base type declarations" << std::endl
          << "  // **********************" << std::endl
	  << std::endl;

    AFD_Root::base_types_list_type::const_iterator i;
    for( i = afd->base_types_list.begin(); i!=afd->base_types_list.end(); i++ )
    {
      if( i->second->is_structure() ) // is it a structure
      {
	Base_Type::element_types_type::const_iterator j;

	*decl << "  struct " << translator.base_type( i->first ) << std::endl;
	*decl << "  {" << std::endl;
	for( j = i->second->element_begin(); j!=i->second->element_end(); j++ )
	{
	  *decl << "    " << j->second << " " 
		<< translator.base_type(j->first) << ";" << std::endl;
	}
	*decl << "  };" << std::endl;
      }
      else			// is it a simple type
      {
	*decl << "  typedef " << i->second->get_type() << " " 
	      << translator.base_type( i->first ) << ";" << std::endl;
      }
    }
    *decl << std::endl;
  }

  void Cpp_Code_Generator::generate_types()
  {
    *decl << "  // **************************" << std::endl
          << "  // provider type declarations" << std::endl
          << "  // **************************" << std::endl
	  << std::endl;
    *impl << "  // ****************************" << std::endl
          << "  // provider type implementation" << std::endl
          << "  // ****************************" << std::endl
	  << std::endl;
    *decl << "  // ****************" << std::endl
	  << "  // provider classes" << std::endl
	  << std::endl;
    AFD_Root::provider_types_type::const_iterator i;
    for( i = afd->provider_types.begin(); i != afd->provider_types.end(); i++ )
    {
      // ****************
      // provider class

      *decl << "  class " << translator.provider_type( i->first )
	    << std::endl
            << "  {" << std::endl
            << "  public:" << std::endl
            << "    //** result functions **" << std::endl;

      Provider_Type::result_types_type::const_iterator j;
      for( j  = i->second.result_types.begin(); 
	   j != i->second.result_types.end(); j++ )
      {
	*decl << "    virtual " << translator.base_type(j->return_type) << " "
	      << translator.result_function_decl( i->first, j->return_type, 
						  j->parameter_type )
	      << " = 0;" << std::endl;
	if( i->second.serial )	// is serial provider type?
	{			// ... create start/end-parameter operands
	  *decl << "    Oparand<" << j->parameter_type << "> " 
		<< translator.start_param( i->first, j->return_type, 
					   j->parameter_type )
		<< ";" << std::endl
		<< "    Oparand<" << j->parameter_type << "> " 
		<< translator.end_param( i->first, j->return_type, 
					 j->parameter_type )
		<< ";" << std::endl;
	}
      }
      *decl << "    //** init functions **" << std::endl
	    << "    virtual void " << translator.first_init( i->first )
	    << "{/*optional*/}" << std::endl
	    << "    //** init functions **" << std::endl
	    << "    virtual void " << translator.last_init( i->first )
	    << "{/*optional*/}" << std::endl;

      *decl << "  };" << std::endl
	    << std::endl;
    }

    *decl << "  // *****************" << std::endl
	  << "  // container classes" << std::endl
	  << std::endl;

    *impl << "  // *****************" << std::endl
	  << "  // container classes" << std::endl
	  << std::endl;
    for( i = afd->provider_types.begin(); i != afd->provider_types.end(); i++ )
    {
      // ****************
      // container class

      *impl << "  // ********************************************************"
	    << "************" << std::endl
	    << "  // serial container for nodes that provide " << i->first 
	    << ":" << std::endl
	    << "  //   " << translator.serial_container(i->first) << std::endl
	    << std::endl;
 
      if( i->second.serial )	// is serial provider type?
      {				// ... create serial container class
	*decl << "  class " << translator.serial_container( i->first )
	      << std::endl
	      << "  {" << std::endl
	      << "    typedef std::list<" 
	      << translator.provider_type(i->first) << "*>"
	      <<        "elements_type;" << std::endl
	      << "    elements_type elements;" << std::endl
	      << "  public:" << std::endl
	      << "    // ** result functions **" << std::endl;

	Provider_Type::result_types_type::const_iterator j;
	for( j  = i->second.result_types.begin(); 
	     j != i->second.result_types.end(); j++ )
	{
	  *decl << "    virtual std::pair<bool," 
		<< translator.base_type(j->return_type) << "> " 
		<< translator.result_function_decl( i->first, j->return_type, 
						    j->parameter_type )
		<< ";" << std::endl;

	  *impl << "  std::pair<bool," << translator.base_type(j->return_type)
		<< "> " << translator.serial_container( i->first ) << "::"
		<< translator.result_function_impl( i->first, j->return_type, 
						    j->parameter_type,"_par_" )
		<< std::endl
		<< "  {" << std::endl
		<< "    elements_type::iterator i;" << std::endl
	    //!!! COULD BE OPTIMIZED BY STORING LAST HIT !!!
		<< "    for( i = elements.begin(); i != elements.end(); i++ )"
		<< std::endl
		<< "    {" << std::endl
		<< "      if( _par_ <= (*i)->" 
		<< translator.end_param(i->first, j->return_type, 
					j->parameter_type) << "() )" 
		<< std::endl
		<< "      {" << std::endl
		<< "        // does it match both limits?" << std::endl
		<< "        if( _par_ >= (*i)->" 
		<< translator.start_param(i->first, j->return_type, 
					j->parameter_type) << "() )" 
		<< std::endl
		<< "        {" << std::endl
		<< "          return std::pair<bool," 
		<< translator.base_type(j->return_type) << ">(true," 
		<< "(*i)->" 
		<< translator.result_function_call( i->first, j->return_type, 
						    j->parameter_type,"_par_" )
		<< "); " << std::endl
		<< "        }" << std::endl
		<< "        else // must be undefined range" << std::endl
		<< "        {" << std::endl
		<< "          std::pair<bool," 
		<< translator.base_type(j->return_type) << "> ret; " 
		<< std::endl
		<< "          ret->first = false;" << std::endl
		<< "          return ret;" << std::endl
		<< "        }" << std::endl
		<< "      }" << std::endl
		<< "    }" << std::endl
		<< "  }" << std::endl
		<< std::endl;
	}
	*decl << "    // ** access functions **" << std::endl
	      << "    elements_type::iterator elements_begin(); "
	      << std::endl
	      << "    elements_type::iterator elements_end(); "
	      << std::endl
	      << "    // ** constructor **" << std::endl
	      << "    " << translator.serial_container( i->first ) 
	      << "(bool max1, bool min1);" << std::endl
	      << "  };" << std::endl
	      << std::endl;

	*impl << "  " << translator.container( i->first ) 
	      << "::elements_type::iterator "
	      << translator.container( i->first ) << "::elements_begin()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.begin();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.container( i->first ) 
	      << "::elements_type::iterator "
	      << translator.container( i->first ) << "::elements_end()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.end();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.serial_container( i->first ) << "::"
	      << translator.serial_container( i->first )
	      << "(bool _max1, bool _min1)" << std::endl
	      << "    : max1(_max1), min1(_min1)" << std::endl
	      << "  {" << std::endl
	      << "  }" << std::endl
	      << std::endl;
      }
      else
      {				// ... create container class
	*decl << "  class " << translator.container( i->first )
	      << std::endl
	      << "  {" << std::endl
	      << "    typedef std::list<" 
	      << translator.provider_type(i->first) << "*>"
	      <<        "elements_type;" << std::endl
	      << "    elements_type elements;" << std::endl
	      << "  public:" << std::endl
	      << "    elements_type::iterator elements_begin(); "
	      << std::endl
	      << "    elements_type::iterator elements_end(); "
	      << std::endl
	      << "  };" << std::endl
	      << std::endl;

	*impl << "  " << translator.container( i->first ) 
	      << "::elements_type::iterator "
	      << translator.container( i->first ) << "::elements_begin()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.begin();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.container( i->first ) 
	      << "::elements_type::iterator "
	      << translator.container( i->first ) << "::elements_end()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.end();" << std::endl
	      << "  }" << std::endl
	      << std::endl;
      }
    }
    *decl << std::endl;
  }

  void Cpp_Code_Generator::generate_nodes()
  {
    *decl << "  // ***************************" << std::endl
          << "  // tree node type declarations" << std::endl
          << "  // ***************************" << std::endl
	  << std::endl;

    *impl << "  // *****************************" << std::endl
          << "  // tree node type implementation" << std::endl
          << "  // *****************************" << std::endl
	  << std::endl;

    AFD_Root::nodes_type::const_iterator i;
    for( i = afd->nodes.begin(); i != afd->nodes.end(); i++ )
    {
      *impl << "  // *****************************" << std::endl
	    << "  // node type " << i->first <<  std::endl
	    <<  std::endl;
      
      *decl << "  // *****************************" << std::endl
	    << "  // node type " << i->first <<  std::endl
	    << std::endl
            << "  class " << translator.node_type( i->first ) 
	    << " : public Prop_Tree_Node" << std::endl
	    << "  {" << std::endl
	    << "  private:" << std::endl
	    << "    //** properties **" << std::endl;
      std::map<std::string,std::string>::const_iterator j;
      for( j=i->second.properties.begin(); j!=i->second.properties.end(); j++ )
      {
	*decl << "    " << translator.property_type(j->second) << " "
	      << translator.prop_op(j->first) << ";" << std::endl;
      }
      *decl << std::endl;

      *decl << "    //** operands **" << std::endl;
      for( j=i->second.operands.begin(); j!=i->second.operands.end(); j++ )
      {
	*decl << "    " << translator.operand_type(j->second) << " "
	      << translator.prop_op(j->first) << ";" << std::endl;
      }
      *decl << std::endl;

      *decl << "    //** child container **" << std::endl;
      std::set<Child_Container>::const_iterator k;
      for( k  = i->second.child_containers.begin(); 
	   k != i->second.child_containers.end(); k++ )
      {
	if( afd->provider_types[ k->provider_type ].serial )
	{
	  *decl << "    " << translator.serial_container( k->provider_type )
		<< " " << translator.container_name( k->provider_type )
		<< "(" << (k->max1?"true":"false") << "," 
		<< (k->min1?"true":"false") << ");" 
		<< " // " << (k->max1?"max1 ":"") << (k->min1?"min1":"")
		<< std::endl;
	}
      }
      *decl << std::endl;
      
      *decl << "    //** infrastructure functions **" << std::endl
	    << std::endl
	    << "    //! constraints, solvers and actions " 
	    <<	      " for each node" << std::endl
	    << "    virtual void establish_dependencies();" << std::endl
	    << "    //! function that is called after hierarchy was set up" 
	    <<	      " for each node" << std::endl
	    << "    virtual void hierarchy_final_init();" << std::endl;

      *impl << "  //** infrastructure functions **" << std::endl
	    << std::endl
	    << "  //! function that is called after hierarchy was set up" 
	    <<	    " for each node" << std::endl
	    << "  void " << translator.node_type( i->first )
	    << "::hierarchy_final_init()" << std::endl
	    << "  {" << std::endl;
      // for each child container
      for( k  = i->second.child_containers.begin(); 
	   k != i->second.child_containers.end(); k++ )
      {
	*impl << "    if( !" << translator.container_name( k->provider_type )
	      << ".elements_empty() )" << std::endl
	      << "    {" << std::endl
	      << "      (*" << translator.container_name( k->provider_type )
	      << ".elements_begin())->" 
	      << translator.first_init( k->provider_type ) << "();" 
	      << std::endl
	      << "      (*(--" << translator.container_name( k->provider_type )
 	      << ".elements_end()))->" 
	      << translator.last_init( k->provider_type ) << "();" 
	      << std::endl
	      << "    }" << std::endl;
      }
      *impl << "  }" << std::endl
	    << std::endl;
      
      *decl << "  };" << std::endl;
      *decl << std::endl;
    }
  }

  Cpp_Code_Translator *Cpp_Code_Generator::get_translator()
  {
    return &translator;
  }

  void Cpp_Code_Generator::generate_code( AFD_Root *afd_root, 
					  code_gen_info *I )
  {
    afd = afd_root;
    info = I;
    decl = new std::ofstream( (info->base_name+".hpp").c_str() );
    impl = new std::ofstream( (info->base_name+".cpp").c_str() );
    generate_header();
    generate_base_types();
    generate_types();
    generate_nodes();
    generate_footer();
  }
}
