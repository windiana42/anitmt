/*****************************************************************************/
/**   This file offers functions to generate C++ Code from AFD data tree   **/
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

#include "gen_cpp.hpp"

#include <fstream>
#include <bitset>

#include "stdextend.hpp"

namespace funcgen
{
  // **********************************************************
  // Cpp_Code_Translator: translates code peaces to C++ code
  std::string Cpp_Code_Translator::open_block()
  {
    return "{\n";
  }
  std::string Cpp_Code_Translator::close_block()
  {
    return "}\n";
  }
  std::string Cpp_Code_Translator::priority_label( std::string name )
  {
    return prefix_priority_label + name;
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
  std::string Cpp_Code_Translator::node_base_type()
  {
    return "proptree::Prop_Tree_Node";
  }
  std::string Cpp_Code_Translator::open_action( std::string action, 
						std::string priority )
  {
    if( action == "push" )
    {
      return std::string("solve::establish_Push_Connection( "
			 "info->priority_system,")
	+ priority_label( priority );
    }
    if( action == "condition_push" )
    {
      return std::string("solve::establish_Condition_Push_Connection( "
			 "info->priority_system,")
	+ priority_label( priority );
    }
    if( action == "default" )
    {
      return std::string("solve::establish_Default_Value( "
			 "info->priority_system,") 
	+ priority_label( priority );
    }
    // if any other action ??? could also output error ???
    return "solve::" + action + "( info->priority_system, " 
      + priority_label( priority );
  }
  std::string Cpp_Code_Translator::parameter_add( std::string param )
  {
    return ", " + param;
  }
  std::string Cpp_Code_Translator::close_function()
  {
    return ");";
  }
  std::string Cpp_Code_Translator::solver_identifier( std::string name )
  {
    return prefix_solver_identifier + name;
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
    return prefix_res_fun + provider_type + '_' + ret_type + '_' + par_type 
      + "( " + prefix_base_type + par_type + " " + par + " )";
  }
  std::string Cpp_Code_Translator::result_function_call
  ( std::string provider_type, std::string ret_type, std::string par_type,
    std::string par )
  {
    return prefix_res_fun + provider_type + '_' + ret_type + '_' + par_type 
      + "( " + par + " )";
  }
  std::string Cpp_Code_Translator::start_return_res( std::string )
  {
    return "return(";
  }
  std::string Cpp_Code_Translator::finish_return_res( std::string )
  {
    return ")";
  }
  std::string Cpp_Code_Translator::start_return_prop( std::string return_type )
  {
    return "return std::pair< bool," + base_type(return_type) + " >(true,";
  }
  std::string Cpp_Code_Translator::finish_return_prop( std::string )
  {
    return ")";
  }
  std::string Cpp_Code_Translator::start_return( std::string return_type )
  {
    return "return std::pair< bool," + base_type(return_type) + " >(true,";
  }
  std::string Cpp_Code_Translator::finish_return( std::string )
  {
    return ")";
  }
  std::string Cpp_Code_Translator::return_fail()
  {
    return "return no_res";
  }
  std::string Cpp_Code_Translator::return_if_fail()
  {
    return "if( did_any_result_fail ) return no_res";
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
  std::string Cpp_Code_Translator::node_prop( std::string name )
  {
    return "get_property( \"" + name + "\" )";
  }
  std::string Cpp_Code_Translator::property_type( std::string name )
  {
    if( name == "flag" )
      return "proptree::Flag_Property";
    if( name == "scalar" )
      return "proptree::Scalar_Property";
    if( name == "vector" )
      return "proptree::Vector_Property";
    if( name == "matrix" )
      return "proptree::Matrix_Property";
    if( name == "string" )
      return "proptree::String_Property";

    // this shouldn't happen in the current specification    
    return "proptree::Type_Property< "+name+" >";
  }
  std::string Cpp_Code_Translator::operand_type( std::string name )
  {
    return "solve::Operand< "+name+" >";
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
  std::string Cpp_Code_Translator::child_result_with_status
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par, std::string fail_bool_var  )
  {
    if( fail_bool_var == "" ) fail_bool_var = "did_result_fail";
    return "extract_status( " + child_result(provider_type,ret,par_type,par)
      + ", " + fail_bool_var + ", did_any_result_fail )";
  }
  std::string Cpp_Code_Translator::provided_result_with_status
    ( std::string provider_type, std::string ret, std::string par_type,
      std::string par, std::string fail_bool_var )
  {
    if( fail_bool_var == "" ) fail_bool_var = "did_result_fail";
    return "extract_status( " + provided_result(provider_type,ret,par_type,par)
      + ", " + fail_bool_var + ", did_any_result_fail )";
  }

  std::string Cpp_Code_Translator::first_init( std::string provider_type )
  {
    return prefix_res_fun + provider_type + "_first_init()";
  }
  std::string Cpp_Code_Translator::last_init( std::string provider_type )
  {
    return prefix_res_fun + provider_type + "_last_init()";
  }
  std::string Cpp_Code_Translator::is_avail
  ( std::string provider_type, std::string ret_type, std::string par_type )
  {
    return prefix_is_avail + provider_type + '_' + ret_type + '_' 
      + par_type + "_is_avail";
  }
  std::string Cpp_Code_Translator::prev()
  {
    return "get_prev()";
  }
  std::string Cpp_Code_Translator::next()
  {
    return "get_next()";
  }
  std::string Cpp_Code_Translator::prev( std::string provider_type )
  {
    return "get_prev_" + provider_type + "()";
  }
  std::string Cpp_Code_Translator::next( std::string provider_type )
  {
    return "get_next_" + provider_type + "()";
  }
  std::string Cpp_Code_Translator::parent()
  {
    return "get_parent()";
  }
  std::string Cpp_Code_Translator::first_child()
  {
    return "get_first_child()";
  }
  std::string Cpp_Code_Translator::last_child()
  {
    return "get_last_child()";
  }
  std::string Cpp_Code_Translator::get_child(int n)
  {
    return std::string("get_child(") + n + ")";
  }
  std::string Cpp_Code_Translator::first_child( std::string child_type )
  {
    return container_name(child_type) + ".get_first_element()";
  }
  std::string Cpp_Code_Translator::last_child( std::string child_type )
  {
    return container_name(child_type) + ".get_last_element()";
  }
  std::string Cpp_Code_Translator::get_child( std::string child_type, int n)
  {
    return container_name(child_type) + ".get_last_element(" + n + ")";
  }
  std::string Cpp_Code_Translator::reference_concat_string()
  {
    return "->";
  }
  std::string Cpp_Code_Translator::operator_class_name( std::string name )
  {
    return prefix_operator_class_name + name;
  }
  std::string Cpp_Code_Translator::solver_class_name( std::string name )
  {
    return prefix_solver_class_name + name;
  }
  std::string Cpp_Code_Translator::operand_from_bool( bool flag )
  {
    return std::string("solve::const_op( values::Flag( ") + (flag?"true":"false") + " ), get_consultant() )";
  }
  std::string Cpp_Code_Translator::operand_from_scalar( double val )
  {
    return std::string("solve::const_op( values::Scalar( ") + val + 
      " ), get_consultant() )";
  }
  std::string Cpp_Code_Translator::operand_from_string( std::string str )
  {
    return std::string("solve::const_op( values::String( \"") + str + 
      "\" ), get_consultant() )";
  }
  std::string Cpp_Code_Translator::operand_from_function( std::string name, 
							  std::string par )
  {
    return "solve::const_op( " + name + "( " + par + " ), get_consultant() )";
  }
  std::string Cpp_Code_Translator::function( std::string function_name,
					     std::string parameters )
  {
    return function_name + "( " + parameters + " )";
  }

  Cpp_Code_Translator::Cpp_Code_Translator( code_gen_info *info )
    : Code_Translator(info), prefix_priority_label("_pl_"),
      prefix_base_type(""), prefix_provider_type("_pt_"), 
      prefix_node_type("node_"), prefix_prop_op("_op_"),prefix_res_fun("_rf_"),
      prefix_is_avail("_av_"), prefix_param_range("_pr_"), 
      prefix_container_type("_container_"), 
      prefix_serial_container_type("_serial_container_"), 
      prefix_container_name("_cn_"),
      prefix_solver_identifier(""), // user should access this in C++ code
      prefix_operator_class_name("_oc_"),
      prefix_solver_class_name("_sc_")
  {}

  // ****************************************
  //! Cpp_Code_Generator: generates C++ code

  void Cpp_Code_Generator::generate_header()
  {
    *decl << "// ********************************************" << std::endl;
    *decl << "// generated file by funcgen (www.anitmt.org)" << std::endl;
    *decl << "// requires:" << std::endl;
    *decl << "//   - libmessage" << std::endl;
    *decl << "//   - libval" << std::endl;
    *decl << "//   - libsolve" << std::endl;
    *decl << "//   - libproptree" << std::endl;
    *decl << "// ********************************************" << std::endl;
    *decl << std::endl;
    *decl << "#ifndef __functionality_"+info->id_name+"__" << std::endl;
    *decl << "#define __functionality_"+info->id_name+"__" << std::endl;
    *decl << std::endl;
    *decl << "#include <val/val.hpp>" << std::endl;
    *decl << "#include <solve/operand.hpp>" << std::endl;
    *decl << "#include <solve/operator.hpp>" << std::endl;
    *decl << "#include <solve/solver.hpp>" << std::endl;
    *decl << "#include <proptree/property.hpp>" << std::endl;
    *decl << "#include <proptree/proptree.hpp>" << std::endl;
    *decl << "#include <message/message.hpp>" << std::endl;
    *decl << std::endl;
    *decl << "#include <list>" << std::endl;
    *decl << "#include <string>" << std::endl;
    *decl << "#include <map>" << std::endl;
    *decl << "#include <math.h>" << std::endl;
    *decl << std::endl;
    std::list<std::string>::const_iterator i;
    for( i  = afd->included_basenames.begin();
	 i != afd->included_basenames.end(); ++i )
    {
      *decl << "#include \"" << *i << ".hpp\"" << std::endl;
    }
    for( i  = afd->header_files.begin();
	 i != afd->header_files.end(); ++i )
    {
      *decl << "#include \"" << *i << "\""<< std::endl;
    }
    *decl << "namespace " << info->namespace_name << std::endl;
    *decl << "{" << std::endl;

    *impl << "// ********************************************" << std::endl;
    *impl << "// generated file by funcgen (www.anitmt.org)" << std::endl;
    *impl << "// requires:" << std::endl;
    *impl << "//   - libmessage" << std::endl;
    *impl << "//   - libval" << std::endl;
    *impl << "//   - libsolve" << std::endl;
    *impl << "//   - libproptree" << std::endl;
    *impl << "// ********************************************" << std::endl;
    *impl << std::endl;
    *impl << "#include <solve/constraint.hpp>" << std::endl;
    *impl << "#include \""+info->base_name+".hpp\"" << std::endl;
    *impl << std::endl;
    *impl << "namespace functionality" << std::endl;
    *impl << "{" << std::endl;
    *impl << "  // ****************************" << std::endl;
    *impl << "  // help functions" << std::endl;
    *impl << "  // ****************************" << std::endl;
    *impl << std::endl;
    *impl << "  template<class T>" << std::endl;
    *impl << "  inline T extract_status( std::pair<bool,T> value, "
	  << "bool &status, " << std::endl;
    *impl << "			   bool &any_false )" << std::endl;
    *impl << "  {" << std::endl;
    *impl << "    status = value.first;" << std::endl;
    *impl << "    any_false |= !value.first;" << std::endl;
    *impl << "    return value.second;" << std::endl;
    *impl << "  }" << std::endl;
  }
  void Cpp_Code_Generator::generate_footer()
  {
    *impl << "}" << std::endl;
    *decl << "}" << std::endl;
    *decl << "#endif" << std::endl;
  }

  void Cpp_Code_Generator::generate_priority_list()
  {
    if( afd->write_priority_list )
    {
      *decl << "  // **********************" << std::endl;
      *decl << "  // priority list labels  " << std::endl;
      *decl << "  // **********************" << std::endl;
      *decl << std::endl;
      std::list<std::string>::const_iterator i;
      int num = 1;
      for( i = afd->priority_list.begin(); i!=afd->priority_list.end(); ++i )
      {
	*decl << "  const double " << translator.priority_label(*i) 
	      << " = " << num++ << ";" << std::endl;
      }        
    }
  }

  void Cpp_Code_Generator::generate_base_types()
  {
    *decl << "  // **********************" << std::endl
          << "  // base type declarations" << std::endl
          << "  // **********************" << std::endl
	  << std::endl;

    AFD_Root::base_types_list_type::const_iterator i;
    for( i = afd->base_types_list.begin(); i!=afd->base_types_list.end(); ++i )
    {
      if( i->second->don_t_create_code ) continue;

      if( i->second->is_structure() ) // is it a structure
      {
	Base_Type::element_types_type::const_iterator j;

	*decl << "  struct " << translator.base_type( i->first ) << std::endl;
	*decl << "  {" << std::endl;
	for( j = i->second->element_begin(); j!=i->second->element_end(); ++j )
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
    for( i = afd->provider_types.begin(); i != afd->provider_types.end(); ++i )
    {
      const std::string &provides = i->first;
      const Provider_Type &provider_type = i->second;

      if( provider_type.don_t_create_code ) continue;

      // ****************
      // provider class

      *decl << "  class " << translator.provider_type( provides ) 
	    << " : virtual public " << translator.node_base_type()
	    << std::endl
            << "  {" << std::endl
            << "  private:" << std::endl
            << "    // ** type specific node connection **" << std::endl
            << "    " << translator.provider_type( provides ) << " *_tc_prev_" 
	    << provides << ";" << std::endl
            << "    " << translator.provider_type( provides ) << " *_tc_next_" 
	    << provides << ";" << std::endl
            << "  public:" << std::endl
            << "    // ** type specific node connection **" << std::endl
            << "    " << translator.provider_type( provides ) << " *get_prev_" 
	    << provides << "();" << std::endl
            << "    " << translator.provider_type( provides ) << " *get_next_" 
	    << provides << "();" << std::endl
            << "    void set_prev_" 
	    << provides << "( " << translator.provider_type( provides )
	    << "* );" << std::endl
            << "    void set_next_" 
	    << provides << "( " << translator.provider_type( provides )
	    << "* );" << std::endl
	    << std::endl;

      *impl << "  // ** type specific node connection **" << std::endl
            << "  " << translator.provider_type( provides ) << " *"
	    << translator.provider_type( provides ) << "::get_prev_" 
	    << provides << "()" << std::endl
	    << "  {" << std::endl
	    << "    return _tc_prev_" << provides << ";" << std::endl
	    << "  }" << std::endl
            << "  " << translator.provider_type( provides ) << " *"
	    << translator.provider_type( provides ) << "::get_next_" 
	    << provides << "()" << std::endl
	    << "  {" << std::endl
	    << "    return _tc_next_" << provides << ";" << std::endl
	    << "  }" << std::endl
            << "  void " << translator.provider_type( provides ) 
	    << "::set_prev_" << provides 
	    << "( " << translator.provider_type( provides ) << "*prev )" 
	    << std::endl
	    << "  {" << std::endl
	    << "    _tc_prev_" << provides << " = prev;" << std::endl
	    << "  }" << std::endl
            << "  void " << translator.provider_type( provides ) 
	    << "::set_next_" << provides 
	    << "( " << translator.provider_type( provides ) << "*next )" 
	    << std::endl
	    << "  {" << std::endl
	    << "    _tc_next_" << provides << " = next;" << std::endl
	    << "  }" << std::endl
	    << std::endl;

      *decl << "    // ** result functions **" << std::endl;

      Provider_Type::result_types_type::const_iterator j;
      for( j  = provider_type.result_types.begin(); 
	   j != provider_type.result_types.end(); ++j )
      {
	*decl << "    virtual std::pair< bool," 
	      << translator.base_type(j->return_type) << " > "
	      << translator.result_function_decl( provides, j->return_type, 
						  j->parameter_type )
	      << " = 0;" << std::endl;
	if( provider_type.serial )	// is serial provider type?
	{			// ... create start/end-parameter operands
	  *decl << "    solve::Operand< " << j->parameter_type << " > " 
		<< translator.start_param( provides, j->return_type, 
					   j->parameter_type )
		<< ";" << std::endl
		<< "    solve::Operand< " << j->parameter_type << " > " 
		<< translator.end_param( provides, j->return_type, 
					 j->parameter_type )
		<< ";" << std::endl;
	}
      }
      *decl << "    // ** is result availible **" << std::endl;
      for( j  = provider_type.result_types.begin(); 
	   j != provider_type.result_types.end(); ++j )
      {
	*decl << "    solve::Operand<bool> " 
	      << translator.is_avail( provides, j->return_type, 
				      j->parameter_type ) << ";"
	      << std::endl;
      }
      *decl << "    // ** init functions **" << std::endl
	    << "    virtual void " << translator.first_init( provides )
	    << "{/*optional*/}" << std::endl
	    << "    virtual void " << translator.last_init( provides )
	    << "{/*optional*/}" << std::endl;
      
      *decl << "    // ** constructor **" << std::endl
	    << "    " << translator.provider_type( provides ) 
	    << "( message::Message_Consultant *consultant );" << std::endl;

      *impl << "  // ** constructor **" << std::endl
	    << "  " << translator.provider_type( provides ) << "::"
	    << translator.provider_type( provides ) 
	    << "( message::Message_Consultant *c )" << std::endl
	    << "    : " << translator.node_base_type() << "(\"\",\"\",0,c), "
	    << "/* should never be used */" << std::endl
	    << "      _tc_prev_" << provides << "(0)," << std::endl
	    << "      _tc_next_" << provides << "(0)";
      if( provider_type.serial )	// is serial provider type?
      {				 // ... init start/end-parameter operands
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *impl << ",\n      "
		<< translator.start_param( provides, j->return_type, 
					   j->parameter_type ) << "(c)"
		<< "," << std::endl 
		<< "      " 
		<<  translator.end_param( provides, j->return_type, 
					  j->parameter_type ) << "(c)";
	}
      }
      for( j  = provider_type.result_types.begin(); 
	   j != provider_type.result_types.end(); ++j )
      {
	*impl << ",\n      "
	      << translator.is_avail( provides, j->return_type, 
				      j->parameter_type ) << "(c)";
      }
      *impl << std::endl
	    << "  {" << std::endl
	    << "  }" << std::endl
	    << std::endl;

      *decl << "    // ** virtual destructor **" << std::endl
	    << "    virtual ~" << translator.provider_type( provides ) 
	    << "() {}" << std::endl;

      *decl << "  };" << std::endl
	    << std::endl;
    }

    *decl << "  // *****************" << std::endl
	  << "  // container classes" << std::endl
	  << std::endl;

    *impl << "  // *****************" << std::endl
	  << "  // container classes" << std::endl
	  << std::endl;
    for( i = afd->provider_types.begin(); i != afd->provider_types.end(); ++i )
    {
      const std::string &provides = i->first;
      const Provider_Type &provider_type = i->second;

      if( provider_type.don_t_create_code ) continue;

      // ****************
      // container class

      *impl << "  // ********************************************************"
	    << "************" << std::endl
	    << "  // serial container for nodes that provide " << provides 
	    << ":" << std::endl
	    << "  //   " << translator.serial_container(provides) << std::endl
	    << std::endl;
 
      if( provider_type.serial )	// is serial provider type?
      {				// ... create serial container class
	*decl << "  class " << translator.serial_container( provides )
	      << std::endl
	      << "  {" << std::endl
	      << "  public:" << std::endl
	      << "    typedef std::list<" 
	      << translator.provider_type(provides) << "*>"
	      <<        "elements_type;" << std::endl
	      << "  private:" << std::endl
	      << "    bool max1; // maximal one element" << std::endl 
	      << "    bool min1; // minimal one element" << std::endl
	      << "    elements_type elements;" << std::endl;
	Provider_Type::result_types_type::const_iterator j;
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *decl << "    solve::Multi_And_Operator *avail_operator_" 
		<< j->return_type << "_" << j->parameter_type << ";"
		<< std::endl;
	}

	*decl << "    typedef std::map<std::string, "
	      << "proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >*> "
	      << "node_factories_type;" << std::endl;
	*decl << "    static node_factories_type node_factories;" << std::endl;

	*impl << "  " << translator.serial_container( provides ) << "::"
	      << "node_factories_type "
	      << translator.serial_container( provides ) << "::"
	      << "node_factories;" 
	      << std::endl;

	*decl << "  public:" << std::endl;
	*decl << "    static void add_node_factory( std::string name, "
	      << "proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >* );" << std::endl;

	*impl << "  void "
	      << translator.serial_container( provides ) << "::"
	      << "add_node_factory( std::string name, "
	      << "proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >* nf )" << std::endl;
	*impl << "  {" << std::endl;
	*impl << "    node_factories[name] = nf;" << std::endl;
	*impl << "  }" << std::endl;
	*impl << std::endl;

	*decl << "    proptree::Prop_Tree_Node *add_child( std::string type, "
	      << "std::string name, proptree::tree_info *info, "
	      << "message::Message_Consultant *msg, "
	      << "proptree::Prop_Tree_Node *already_obj );" << std::endl;

	*impl << "  proptree::Prop_Tree_Node *"
	      << translator.serial_container( provides ) << "::" << std::endl;
	*impl << "  add_child( std::string type, std::string name, "
	      << "proptree::tree_info *info, " << std::endl;
	*impl << "             message::Message_Consultant *msg, "
	      << "proptree::Prop_Tree_Node *already_obj )" << std::endl
	      << "  {" << std::endl
	      << "    node_factories_type::iterator i;" << std::endl
	      << "    i = node_factories.find(type);" << std::endl
	      << "    if( i == node_factories.end() ) return already_obj;"
	      << std::endl
	      << "    proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >* &nf = i->second;"
	      << std::endl
	      << "    proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >::node_return_type "
	      << "node;" << std::endl
	      << "    if( already_obj != 0 ) " << std::endl
	      << "      node = nf->cast(already_obj);" << std::endl
	      << "    else" << std::endl
	      << "      node = nf->create(name,info,msg);" << std::endl
	      << "    if( !elements.empty() ) " << std::endl
	      << "    { // link contained elements if not empty" << std::endl
	      << "      " << translator.provider_type(provides) << " *last = "
	      << "*(--elements.end());"<< std::endl
	      << "      last->set_next_" << provides << "( node.first );" 
	      << std::endl
	      << "      node.first->set_prev_" << provides << "( last );" 
	      << std::endl
	      << "    }" << std::endl
	      << "    elements.push_back(node.first); " << std::endl;
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *impl << "    avail_operator_" << j->return_type << "_" 
		<< j->parameter_type
		<< "->add_operand( node.first->" 
		<< translator.is_avail( provides, j->return_type, 
					j->parameter_type ) << " );"
		<< std::endl;
	}
	*impl << "// store provided type pointer" << std::endl
	      << "    return node.second;                      "
	      << "// return general prop tree node pointer" << std::endl
	      << "  }" << std::endl
	      << std::endl;

	*decl << "  " << translator.provider_type(provides) 
	      << "* get_first_element();" << std::endl;
	*impl << "  " << translator.provider_type(provides) << "* "
	      << translator.serial_container( provides ) << "::"
	      << "get_first_element()" << std::endl
	      << "  {" << std::endl 
	      << "    if( elements_begin() == elements_end() ) return 0;" << std::endl 
	      << "    else return *elements_begin();" << std::endl 
	      << "  }" << std::endl;
	
	*decl << "    " << translator.provider_type(provides) 
	      << "* get_last_element();" << std::endl;
	*impl << "  " << translator.provider_type(provides) << "* "
	      << translator.serial_container( provides ) << "::"
	      << "get_last_element()" << std::endl
	      << "  {" << std::endl 
	      << "    if( elements_begin() == elements_end() ) return 0;" << std::endl 
	      << "    else return *(--elements_end());" << std::endl 
	      << "  }" << std::endl;

	*decl << "    " << translator.provider_type(provides) 
	      << "* get_element( int n );" << std::endl;
	*impl << "  " << translator.provider_type(provides) << "* "
	      << translator.serial_container( provides ) << "::"
	      << "get_element( int n )" << std::endl
	      << "  {" << std::endl 
	      << "    elements_type::iterator i; int z;" << std::endl 
	      << "    for( i = elements_begin(), z=0; i != elements_end(); ++i, ++z )" << std::endl 
	      << "      if( z == n ) return *i;" << std::endl 
	      << "    return 0;" << std::endl 
	      << "  }" << std::endl;

	*decl << "    // ** result functions **" << std::endl;

	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *decl << "    virtual std::pair< bool," 
		<< translator.base_type(j->return_type) << " > " 
		<< translator.result_function_decl( provides, j->return_type, 
						    j->parameter_type )
		<< ";" << std::endl;

	  *impl << "  std::pair< bool," << translator.base_type(j->return_type)
		<< " > " << translator.serial_container( provides ) << "::"
		<< translator.result_function_impl( provides, j->return_type, 
						    j->parameter_type,"_par_" )
		<< std::endl
		<< "  {" << std::endl
		<< "    elements_type::iterator i;" << std::endl
	    //!!! COULD BE OPTIMIZED BY STORING LAST HIT !!!
		<< "    for( i = elements.begin(); i != elements.end(); ++i )"
		<< std::endl
		<< "    {" << std::endl
		<< "      if( _par_ <= (*i)->" 
		<< translator.end_param(provides, j->return_type, 
					j->parameter_type) << "() )" 
		<< std::endl
		<< "      {" << std::endl
		<< "        // does it match both limits?" << std::endl
		<< "        if( _par_ >= (*i)->" 
		<< translator.start_param(provides, j->return_type, 
					j->parameter_type) << "() )" 
		<< std::endl
		<< "        {" << std::endl
		<< "          return (*i)->" 
		<< translator.result_function_call( provides, j->return_type, 
						    j->parameter_type,"_par_" )
		<< "; " << std::endl
		<< "        }" << std::endl
		<< "        else // must be undefined range" << std::endl
		<< "        {" << std::endl
		<< "          break;" << std::endl
		<< "        }" << std::endl
		<< "      }" << std::endl
		<< "    }" << std::endl
		<< "    // undefined range" << std::endl
		<< "    std::pair< bool," 
		<< translator.base_type(j->return_type) << " > ret; " 
		<< std::endl
		<< "    ret.first = false;" << std::endl
		<< "    return ret;" << std::endl
		<< "  }" << std::endl
		<< std::endl;
	}
	*decl << "    // ** is result availible **" << std::endl;
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *decl << "    solve::Operand<bool> " 
		<< translator.is_avail( provides, j->return_type, 
					j->parameter_type ) << ";" 
		<< std::endl;
	}
	*decl << "    // ** access functions **" << std::endl
	      << "    elements_type::iterator elements_begin(); " << std::endl
	      << "    elements_type::iterator elements_end(); " << std::endl
	      << "    bool elements_empty(); " << std::endl
	      << "    // ** constructor **" << std::endl
	      << "    " << translator.serial_container( provides ) 
	      << "(bool max1, bool min1, message::Message_Consultant* );" 
	      << std::endl
	      << "    // ** virtual destructor **" << std::endl
	      << "    virtual ~" << translator.serial_container( provides ) 
	      << "() {}" << std::endl
	      << "    // ** Don't call the following functions! ** " 
	      << std::endl
	      << "    //! function that is called after hierarchy was set up "
	      << "for each node" << std::endl
	      << "    void hierarchy_final_init();" << std::endl
	      << "  };" << std::endl
	      << std::endl;

	*impl << "  " << translator.serial_container( provides ) 
	      << "::elements_type::iterator "
	      << translator.serial_container(provides) << "::elements_begin()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.begin();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.serial_container(provides) 
	      << "::elements_type::iterator "
	      << translator.serial_container( provides ) << "::elements_end()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.end();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  bool "
	      << translator.serial_container(provides) << "::elements_empty()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.empty();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.serial_container( provides ) << "::"
	      << translator.serial_container( provides )
	      << "(bool _max1, bool _min1, message::Message_Consultant *c)" 
	      << std::endl
	      << "    : max1(_max1), min1(_min1)";
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *impl << "," << std::endl
		<< "      " << translator.is_avail( provides, j->return_type, 
						    j->parameter_type )
		<< "(c)";
	}
	*impl << std::endl
	      << "  {" << std::endl;
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *impl << "    avail_operator_" << j->return_type << "_" 
		<< j->parameter_type << " = new solve::Multi_And_Operator(c);"
		<< std::endl
		<< "    " << translator.is_avail( provides, j->return_type, 
						  j->parameter_type )
		<< " = avail_operator_" << j->return_type << "_" 
		<< j->parameter_type << "->get_result();"
		<< std::endl;
	}
	*impl << "  }" << std::endl
	      << "  //! function that is called after hierarchy was set up "
	      << "for each node" << std::endl
	      << "  void " << translator.serial_container( provides ) << "::"
	      << "hierarchy_final_init()" << std::endl
	      << std::endl
	      << "  {" << std::endl;
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *impl << "    avail_operator_" << j->return_type << "_" 
		<< j->parameter_type << "->finish_adding();" << std::endl;
	}
	*impl << "  }" << std::endl;
      }
      else
      {				// ... create container class
	*decl << "  class " << translator.container( provides )
	      << std::endl
	      << "  {" << std::endl
	      << "  public:" << std::endl
	      << "    typedef std::list<" 
	      << translator.provider_type(provides) << "*>"
	      <<        "elements_type;" << std::endl
	      << "  private:" << std::endl
	      << "    bool max1; // maximal one element" << std::endl 
	      << "    bool min1; // minimal one element" << std::endl
	      << "    elements_type elements;" << std::endl
	      << "    typedef std::map<std::string, "
	      << "proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >*> "
	      << "node_factories_type;" << std::endl
	      << "    static node_factories_type node_factories;" << std::endl
	      << "  public:" << std::endl
	      << "    static void add_node_factory( std::string name, "
	      << "proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >* );" << std::endl
	      << "    proptree::Prop_Tree_Node *add_child( std::string type, "
	      << "std::string name, proptree::tree_info *info, "
	      << "message::Message_Consultant *msg, "
	      << "proptree::Prop_Tree_Node *already_obj );" << std::endl
	      << "    // ** result functions **" << std::endl;

	*impl << "  " << translator.container( provides ) << "::"
	      << "node_factories_type "
	      << translator.container( provides ) << "::"
	      << "node_factories;" 
	      << std::endl
	      << "  void "
	      << translator.container( provides ) << "::"
	      << "add_node_factory( std::string name, "
	      << "proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >* nf )" << std::endl
	      << "  {" << std::endl
	      << "    node_factories[name] = nf;" << std::endl
	      << "  }" << std::endl
	      << "  proptree::Prop_Tree_Node *"
	      << translator.container( provides ) << "::" << std::endl
	      << "  add_child( std::string type, std::string name, "
	      << "proptree::tree_info *info, " << std::endl 
	      << "             message::Message_Consultant *msg, "
	      << "proptree::Prop_Tree_Node *already_obj )" << std::endl
	      << "  {" << std::endl
	      << "    node_factories_type::iterator i;" << std::endl
	      << "    i = node_factories.find(type);" << std::endl
	      << "    if( i == node_factories.end() ) return already_obj;"
	      << std::endl
	      << "    proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >* &nf = i->second;"
	      << std::endl
	      << "    proptree::Basic_Node_Factory< "
	      << translator.provider_type(provides) <<" >::node_return_type "
	      << "node;" << std::endl
	      << "    if( already_obj != 0 ) " << std::endl
	      << "      node = nf->cast(already_obj);" << std::endl
	      << "    else" << std::endl
	      << "      node = nf->create(name,info,msg);" << std::endl
	      << "    elements.push_back(node.first); "
	      << "// store provided type pointer" << std::endl
	      << "    return node.second;                      "
	      << "// return general prop tree node pointer" << std::endl
	      << "  }" << std::endl
	      << std::endl;

	*decl << "    elements_type::iterator elements_begin(); " << std::endl
	      << "    elements_type::iterator elements_end(); " << std::endl
	      << "    bool elements_empty(); " << std::endl
	      << "    // ** constructor **" << std::endl
	      << "    " << translator.container( provides ) 
	      << "(bool max1, bool min1, message::Message_Consultant* );" 
	      << std::endl
	      << "    // ** virtual destructor **" << std::endl
	      << "    virtual ~" << translator.container( provides ) 
	      << "() {}" << std::endl
	      << "    //! function that is called after hierarchy was set up "
	      << "for each node" << std::endl
	      << "    void hierarchy_final_init();" << std::endl
	      << "  };" << std::endl
	      << std::endl;

	*impl << "  " << translator.container( provides ) 
	      << "::elements_type::iterator "
	      << translator.container( provides ) << "::elements_begin()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.begin();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.container( provides ) 
	      << "::elements_type::iterator "
	      << translator.container( provides ) << "::elements_end()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.end();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  bool "
	      << translator.container( provides ) << "::elements_empty()"
	      << std::endl
	      << "  {" << std::endl
	      << "    return elements.empty();" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "  " << translator.container( provides ) << "::"
	      << translator.container( provides )
	      << "(bool _max1, bool _min1, message::Message_Consultant * )" 
	      << std::endl
	      << "    : max1(_max1), min1(_min1)" << std::endl
	      << "  {" << std::endl
	      << "  }" << std::endl
	      << std::endl
	      << "    // ** Don't call the following functions! ** " 
	      << std::endl
	      << "  //! function that is called after hierarchy was set up "
	      << "for each node" << std::endl
	      << "  void " << translator.container( provides ) << "::"
	      << "hierarchy_final_init()" << std::endl
	      << std::endl
	      << "  {" << std::endl
	      << "  }" << std::endl;
      }
    }
    *decl << std::endl;
  }

  void write_constraint_statements( std::ostream *os, 
				    const Constraint_Code &constraint_code,
				    Code_Translator &translator )
  {
    std::list<Constraint_Declaration>::const_iterator i;
    for( i  = constraint_code.constraint_declarations.begin();
	 i != constraint_code.constraint_declarations.end(); ++i )
    {
      std::list<std::string>::const_iterator j;
      if( !i->essentials.empty() )
      {
	*os << "    if( ";
	bool first = true;
	for( j = i->essentials.begin(); j != i->essentials.end(); ++j )
	{
	  *os << (first?first=false,"":" && ") << (*j);
	}
	*os << " )" << std::endl;
	*os << "    {" << std::endl;
	*os << "      solve::constraint( " << i->constraint_code << " );"
	    << std::endl;
	*os << "    }" << std::endl;
      }
      else
	*os << "    solve::constraint( " << i->constraint_code << " );" 
	    << std::endl;
    }
  }

  void write_solver_statements( std::ostream *os, 
				const Solver_Code &solve_code,
				Code_Translator &translator )
  {
    std::list<Solver_Declaration>::const_iterator i;
    for( i  = solve_code.solver_declarations.begin();
	 i != solve_code.solver_declarations.end(); ++i )
    {
      std::string indent = "    ";
      std::list<std::string>::const_iterator j;
      if( !i->essentials.empty() )
      {
	*os << "    if( ";
	bool first = true;
	for( j = i->essentials.begin(); j != i->essentials.end(); ++j )
	{
	  *os << (first?first=false,"":" && ") << (*j);
	}
	*os << " )" << std::endl;
	*os << "    {" << std::endl;
	indent = "      ";
      }
      if( i->is_expression )
      {
	*os << indent << translator.prop_op(i->dest_operand) << ".assign( "
	    << i->expression_code << " );" << std::endl;
      }
      else
      {
	*os << indent;
	if( i->identifier != "" )
	  *os << translator.solver_identifier(i->identifier) << " = ";
	*os << i->solver_type << "( ";
	std::list<std::string>::const_iterator par;
	for( par = i->parameters.begin(); par != i->parameters.end(); ++par )
	{
	  *os << *par << ", ";
	}
	*os << "get_consultant() );" << std::endl;
      }
      if( !i->essentials.empty() )
      {
	*os << "    }" << std::endl;
      }
    }
  }

  void write_action_statements( std::ostream *os, 
				const Action_Code &actions,
				Code_Translator &translator )
  {
    std::list<Action_Declaration>::const_iterator i;
    for( i  = actions.action_declarations.begin();
	 i != actions.action_declarations.end(); ++i )
    {
      std::list<std::string>::const_iterator j;
      if( !i->essentials.empty() )
      {
	*os << "    if( ";
	bool first = true;
	for( j = i->essentials.begin(); j != i->essentials.end(); ++j )
	{
	  *os << (first?first=false,"":" && ") << (*j);
	}
	*os << " )" << std::endl;
	*os << "    {" << std::endl;
	*os << "      " << i->action_code << std::endl;
	*os << "    }" << std::endl;
      }
      else
	*os << "    " << i->action_code << std::endl;
    }
  }

  void write_solve_code( std::ostream *os, 
			 const Solve_System_Code &solsys_code,
			 Code_Translator &translator )
  {
    // ** Constraints **
    write_constraint_statements( os, solsys_code.constraints, translator );
    // ** Solvers **
    write_solver_statements( os, solsys_code.solvers, translator );
    // ** Actions **
    write_action_statements( os, solsys_code.actions, translator );
  }

  void write_solver_declarations( std::ostream *os, 
				  const Solver_Code &solve_code,
				  Code_Translator &translator )
  {
    std::list<Solver_Declaration>::const_iterator i;
    for( i  = solve_code.solver_declarations.begin();
	 i != solve_code.solver_declarations.end(); ++i )
    {
      if( !i->is_expression )
      {
	if( i->identifier != "" )
	{
	  *os << "    " << translator.solver_class_name(i->solver_type) 
	      << " *" << translator.solver_identifier(i->identifier) << ";"
	      << std::endl;
	}
      }
    }
  }

  std::string get_type_name( op_par_type t )
  {
    switch( t )
    {
    case none:		return "void";
    case boolean:	return "bool";
    case result:	return "T_Result";
    case operand:	return "const T_Operand1 &"; // user alias
    case operand1:	return "const T_Operand1 &";
    case operand2:	return "const T_Operand2 &";
    case operand3:	return "const T_Operand3 &";
    case operand4:	return "const T_Operand4 &";
    case operand5:	return "const T_Operand5 &";
    case operand6:	return "const T_Operand6 &";
    case operand7:	return "const T_Operand7 &";
    case operand8:	return "const T_Operand8 &";
    case operand9:	return "const T_Operand9 &";
    case info:		return "solve::Solve_Run_Info*";
    }
    assert(0);
    return "";
  }

  // *******************************************
  void Cpp_Code_Generator::generate_operators()
  {
    *decl << "  // ********************" << std::endl;
    *decl << "  // ********************" << std::endl;
    *decl << "  // operator declartions" << std::endl;
    *decl << "  // ********************" << std::endl;
    *decl << "  // ********************" << std::endl;
    *decl << std::endl;
    *decl << "}" << std::endl;
    *decl << "namespace solve" << std::endl;
    *decl << "{" << std::endl;
    *decl << std::endl;

    int i; bool first;
    AFD_Root::operators_type::iterator op;
    for( op = afd->operators.begin();
	 op != afd->operators.end(); ++op )
    {
      const std::string &op_name = op->first;
      const Operator_Declaration &op_decl = op->second;
      int op_decl_operands = op_decl.basic_operator->get_num_operands();

      if( op_decl.don_t_create_code ) continue;

      // ************************
      // generate operator class
      
      *decl << "  // *******************************************" << std::endl;
      *decl << "  // " << op_decl.operator_base_type_name << " " << op_name 
	    << std::endl;
      *decl << "  // *******************************************" << std::endl;
      *decl << std::endl;

      *decl << "  template< class T_Result";
      for( i=1; i <= op_decl_operands; ++i )
      {
	*decl << ", class T_Operand" << i;
      }
      *decl << " >" << std::endl;
      *decl << "  class " << translator.operator_class_name(op_name)
	    << std::endl;
      *decl << "    : public solve::" 
	    << op_decl.basic_operator->get_real_name()
	    << "< T_Result";
      for( i=1; i <= op_decl_operands; ++i )
      {
	*decl << ", T_Operand" << i;
      }
      *decl << " >" << std::endl;
      *decl << "  {" << std::endl;
      *decl << "  public:" << std::endl;
      *decl << "    " << translator.operator_class_name(op_name) 
	    << "( ";
      for( i=1; i <= op_decl_operands; ++i )
      {
	*decl << (i>1?", ":"") 
	      << "solve::Operand< T_Operand" << i << " > &operand"<< i;
      }
      *decl << " )" << std::endl;
      *decl << "      : solve::" << op_decl.basic_operator->get_real_name()
	    << "< T_Result";
      for( i=1; i <= op_decl_operands; ++i )
      {
	*decl << ", T_Operand" << i;
      }
      *decl << " > " << std::endl;
      *decl << "        ( ";
      for( i=1; i <= op_decl_operands; ++i )
      {
	*decl << (i>1?", ":"") << "operand"<< i;
      }
      *decl << " )" << std::endl;
      *decl << "    {" << std::endl;
      *decl << "      init();" << std::endl;
      *decl << "    }" << std::endl;
      *decl << "  private:" << std::endl;

      // *******************
      // generate functions
      
      // check whether all required functions were specified...
      const std::list<std::string> &required_function_list
	= op_decl.basic_operator->get_required_functions();
      std::set<std::string> required_functions( required_function_list.begin(),
						required_function_list.end() );
      std::set<std::string>::iterator r;

      std::map< std::string, Function_Code >::const_iterator f;
      for( f  = op_decl.function_code.begin(); 
	   f != op_decl.function_code.end(); ++f )
      {
	const std::string   &function_name = f->first;
	const Function_Code &function_code = f->second;

	assert( op_decl.basic_operator->is_function( function_name ) );
	if( (r = required_functions.find( function_name ))
	    != required_functions.end() ) // is function required?
	{
	  required_functions.erase(r); // not required any more...
	}
	  
	const std::list<op_par_type> &types 
	  = op_decl.basic_operator->get_types( function_name );
	assert( !types.empty() );
	std::list<op_par_type>::const_iterator t = types.begin();
	
	*decl << "    virtual " << get_type_name( *t ) << " " << function_name
	      << "( ";
	first = true;
	std::list<std::string>::const_iterator param_name;	
	for(++t, param_name = function_code.parameter_names.begin(); 
	    t != types.end(); ++t, ++param_name )
	{
	  if( param_name == function_code.parameter_names.end() )
	  {
	    info->msg.error(function_code.pos) 
	      << "too view parameters for function " << function_name
	      << message::nl 
	      << op_decl.basic_operator->
	      get_function_specification( function_name );
	    break;
	  }
	  *decl << (first?first=false,"":", ") << get_type_name(*t) << " " 
		<< *param_name;
	}
	*decl << " )" << std::endl;

	*decl << "    { // user code: ";
	function_code.pos->write2stream(*decl,2);
	*decl << std::endl;
	for( i=0; i< function_code.start_src_column; i++ )
	  *decl << " ";
	*decl << function_code.code << std::endl;
	*decl << "    }" << std::endl;
      }
      if( !required_functions.empty() )
      {
	for( r=required_functions.begin(); r!=required_functions.end(); ++r )
	{
	  info->msg.error(op_decl.pos) 
	    << "essential function " << *r << " has to be implemented"
	    << message::nl 
	    << op_decl.basic_operator->get_function_specification(*r);
	}
      }
      *decl << "  };" << std::endl;
      *decl << std::endl;

      // *******************
      // generate versions

      std::list< std::list<std::string> >::const_iterator v;
      for( v = op_decl.versions.begin(); v != op_decl.versions.end(); ++v )
      {
	const std::list<std::string> &version_types = *v;
	assert( version_types.size() >= 2 ); // return type and name

	// generate all combinations of parameters as operand and constant
	// num_comb holds the operand/constant flags (0: constant, 1:operand)
	// num_comb = 0 is omitted, as one operand is required
	assert( op_decl_operands <= 31 ); // int bits have to be enough
	unsigned max_comb = 1 << op_decl_operands;
	for( unsigned num_comb = 1; num_comb < max_comb; ++num_comb )
	{
	  std::bitset<32> combination(num_comb);
	  int first_operand = -1; // index of first non constant operand
	  for( i=1; i <= op_decl_operands; ++i )
	    if( combination[i-1] )
	    {
	      first_operand = i;
	      break;
	    }

	  std::list<std::string>::const_iterator vt = version_types.begin();
	  const std::string &version_name = *vt; ++vt;
	  std::string version_type = info->namespace_name + "::" + *vt;
	  *decl << "  inline solve::Operand< " << version_type
		<< " >& " << std::endl
		<< "  " << version_name << "( ";
	  for( i=1, ++vt; i <= op_decl_operands; ++i, ++vt )
	  {
	    if( vt == version_types.end() )
	    {
	      info->msg.error(op_decl.pos) 
		<< "too few operands in version declaration " 
		<< version_name << ", " << op_decl_operands << " expected";
	      break;
	    }
	    version_type = info->namespace_name + "::" + *vt;
	    *decl << (i>1?", ":"") 
		  << ( combination[i-1]? // operand(1) or constant(0) ?
		       "solve::Operand< " + version_type + " > &":
		       "const " + version_type + " &" )
		  << " operand"<< i;
	  }
	  if( vt != version_types.end() )
	  {
	    info->msg.error(op_decl.pos) 
	      << "too many of operands in version declaration " 
	      << version_name << ", " << op_decl_operands << " expected";
	  }

	  *decl << " )" << std::endl;
	  *decl << "  {" << std::endl;
	  *decl << "    return (new "
		<< translator.operator_class_name(op_name) << "< ";
	  first = true;
	  for( vt = ++version_types.begin(); vt != version_types.end(); ++vt )
	  {
	    version_type = info->namespace_name + "::" + *vt;
	    *decl << (first?first=false,"":", ")
		  << version_type;
	  }
	  *decl << " >( ";
	  for( i=1; i <= op_decl_operands; ++i )
	  {
	    *decl << (i>1?", ":"");
	    if( combination[i-1] )
	    {
	      *decl << "operand" << i;
	    }
	    else
	    {
	      *decl << "solve::const_op( operand" << i 
		    << ", operand" << first_operand << ".get_consultant() )";
	    }
	  }
	  *decl << " ) )->get_result();" << std::endl;
	  *decl << "  }" << std::endl;
	  *decl << std::endl;
	}
      }
    }
    *decl << "}" << std::endl;
    *decl << "namespace " << info->namespace_name << std::endl;
    *decl << "{" << std::endl;
    *decl << std::endl;
  }

  // *******************************************
  void Cpp_Code_Generator::generate_solvers()
  {
    *decl << "  // ********************" << std::endl;
    *decl << "  // ********************" << std::endl;
    *decl << "  // solver declartions" << std::endl;
    *decl << "  // ********************" << std::endl;
    *decl << "  // ********************" << std::endl;
    *decl << std::endl;
    //    *decl << "}" << std::endl;
    //    *decl << "namespace solve" << std::endl;
    //    *decl << "{" << std::endl;
    //    *decl << std::endl;
    *impl << "  // ********************" << std::endl;
    *impl << "  // ********************" << std::endl;
    *impl << "  // solver declartions" << std::endl;
    *impl << "  // ********************" << std::endl;
    *impl << "  // ********************" << std::endl;
    *impl << std::endl;

    bool first;
    AFD_Root::complex_solver_type::iterator solver;
    for( solver = afd->solvers.begin();
	 solver != afd->solvers.end(); ++solver )
    {
      const std::string &solver_name = solver->first;
      const Complex_Solver &solver_decl = solver->second;

      if( solver_decl.don_t_create_code ) continue;

      *decl << "  class " << translator.solver_class_name( solver_name ) << ";"
	    << std::endl;
    }

    for( solver = afd->solvers.begin();
	 solver != afd->solvers.end(); ++solver )
    {
      const std::string &solver_name = solver->first;
      const Complex_Solver &solver_decl = solver->second;

      if( solver_decl.don_t_create_code ) continue;

      *decl << "  class " << translator.solver_class_name( solver_name ) 
	    << " : public message::Message_Reporter"
	    << std::endl;
      *decl << "  {" << std::endl;
      *decl << "  public:" << std::endl;

      // *******************
      // provided functions
      std::list<Solver_Function_Code>::const_iterator func;
      for( func = solver_decl.functions.begin();
	   func != solver_decl.functions.end(); ++func )
      {
	*decl << "    " << func->return_type << " " << func->name << "( ";
	*impl << "  " << func->return_type << " " 
	      << translator.solver_class_name( solver_name ) << "::" 
	      << func->name << "( ";
	std::list<Variable>::const_iterator var;
	first = true;
	for( var = func->variable_list.begin(); 
	     var != func->variable_list.end(); ++var )
	{
	  if( first ) first = false;
	  else { *decl << ", "; *impl << ", "; }
	  *decl << var->type << " " << var->name;
	  *impl << var->type << " " << var->name;
	}
	*decl << " );" << std::endl;
	*impl << " )" << std::endl;
	*impl << "  {" << std::endl;
	*impl << "    // *** user code following... line:" 
	      << func->start_src_line << " ***" << std::endl;
	// indent like in AFD source file
	for( int cnt = 0; cnt < func->start_src_column; ++cnt )
	  *impl << ' ';
	*impl << func->code << std::endl;
	*impl << "  }" << std::endl;

	*decl << "  solve::Operand<bool> " 
	      << translator.is_avail( func->name ) << ";"
	      << std::endl;
      }      
      *decl << std::endl;

      // ************
      // constructor
      *decl << "    " << translator.solver_class_name( solver_name ) 
	    << "( ";
      *impl << "  " << translator.solver_class_name( solver_name ) << "::"
	    << translator.solver_class_name( solver_name ) << "( ";
      std::list<Operand>::const_iterator par;
      for( par = solver_decl.parameter.operand_list.begin();
	   par != solver_decl.parameter.operand_list.end(); ++par )
      {
	*decl << translator.operand_type( par->type ) << "&"
	      << par->name << ", ";
	*impl << translator.operand_type( par->type ) << "& " 
	      << "_" << translator.prop_op( par->name ) << ", ";
      }
      *decl << " message::Message_Consultant* );" << std::endl;
      *impl << " message::Message_Consultant* consultant )" << std::endl;

      *impl << "    : message::Message_Reporter(consultant)";
      // avail variables of functions
      for( func = solver_decl.functions.begin();
	   func != solver_decl.functions.end(); ++func )
      {
	*impl << "," << std::endl;
	*impl << "      " 
	      << translator.is_avail( func->name ) 
	      << "( get_consultant() )";
      }
      // parameter operands
      for( par = solver_decl.parameter.operand_list.begin();
	   par != solver_decl.parameter.operand_list.end(); ++par )
      {
	*impl << "," << std::endl;
	*impl << "      " << translator.prop_op( par->name ) << "(" 
	      << "_" << translator.prop_op(par->name) << ")";
      }
      // other operands
      std::list<Operand>::const_iterator op;
      for( op = solver_decl.operand_list.begin();
	   op != solver_decl.operand_list.end(); ++op )
      {
	*impl << "," << std::endl;
	*impl << "      " << translator.prop_op( op->name ) 
	      << "( get_consultant() )";
      }
      *impl << std::endl;
      *impl << "  {" << std::endl;
      if( !solver_decl.functions.empty() )
	*impl << "    solve::Multi_And_Operator *m_and;" << std::endl;
      // avail variables of functions
      for( func = solver_decl.functions.begin();
	   func != solver_decl.functions.end(); ++func )
      {
	*impl << "    " 
	      << "m_and = new solve::Multi_And_Operator( get_consultant() );"
	      << std::endl;
	*impl << "    "
	      << translator.is_avail( func->name ) << " = "
	      <<"m_and->get_result();" << std::endl;
	std::list<std::string>::const_iterator i;
	for( i = func->required_operands.begin();
	     i != func->required_operands.end(); ++i )
	{
	  *impl << "    m_and->add_operand( solve::is_solved(" 
		<< translator.prop_op( *i ) << ") );" << std::endl;
	}
	for( i = func->required_functions.begin();
	     i != func->required_functions.end(); ++i )
	{
	  *impl << "    m_and->add_operand( " << translator.is_avail( *i ) 
		<< " );" << std::endl;
	}
	std::list<Solver_Function>::const_iterator sol_func;
	for( sol_func = func->required_solver_functions.begin();
	     sol_func != func->required_solver_functions.end(); ++sol_func )
	{
	  *impl << "    m_and->add_operand( " 
  		<< sol_func->solver << "->" 
		<< translator.is_avail( sol_func->function ) 
		<< " );" << std::endl;
	}
	*impl << "    m_and->finish_adding();" << std::endl;
      }
      // constraints & solvers in init blocks
      write_constraint_statements( impl, solver_decl.init_constraint_code, 
				     translator );
      write_solver_statements( impl, solver_decl.init_code, translator );
      *impl << "  }" << std::endl;

      *decl << std::endl;
      *decl << "  private:" << std::endl;
      write_solver_declarations( decl, solver_decl.init_code, translator );
      // *********
      // operands

      // parameter references
      for( par = solver_decl.parameter.operand_list.begin();
	   par != solver_decl.parameter.operand_list.end(); ++par )
      {
	*decl << "    " << translator.operand_type( par->type ) << "& "
	      << translator.prop_op( par->name ) << ";" << std::endl;
      }
      // other operands
      for( op = solver_decl.operand_list.begin();
	   op != solver_decl.operand_list.end(); ++op )
      {
	*decl << "    " << translator.operand_type( op->type ) << " "
	      << translator.prop_op( op->name ) << ";" << std::endl;
      }
      // *************
      // declarations
      std::list<Variable>::const_iterator var;
      for( var = solver_decl.variable_list.begin();
	   var != solver_decl.variable_list.end(); ++var )
      {
	*decl << "    " << var->type << " "
	      << var->name << ";" << std::endl;
      }
      *decl << "  };" << std::endl;
      *decl << std::endl;
      *decl << "  " << translator.solver_class_name( solver_name ) << "* "
	    << solver_name << "( ";
      *impl << "  " << translator.solver_class_name( solver_name ) << "* "
	    << solver_name << "( ";
      for( par = solver_decl.parameter.operand_list.begin();
	   par != solver_decl.parameter.operand_list.end(); ++par )
      {
	*decl << translator.operand_type( par->type ) << "&"
	      << par->name << ", ";
	*impl << translator.operand_type( par->type ) << "& " 
	      << "_" << translator.prop_op( par->name ) << ", ";
      }
      *decl << "message::Message_Consultant * );" << std::endl;
      *decl << std::endl;
      *impl << "message::Message_Consultant *msgc )" << std::endl;
      *impl << "  {" << std::endl;
      *impl << "    return new " << translator.solver_class_name( solver_name )
	    << "( ";
      for( par = solver_decl.parameter.operand_list.begin();
	   par != solver_decl.parameter.operand_list.end(); ++par )
      {
	*impl << "_" << translator.prop_op( par->name ) << ", ";
      }
      *impl << "msgc );" << std::endl;
      *impl << "  }" << std::endl;
    }
  }

  // ***************************************
  void Cpp_Code_Generator::generate_nodes()
  {
    *decl << "  // ***************************" << std::endl
          << "  // tree node type declarations" << std::endl
          << "  // ***************************" << std::endl
	  << std::endl;

    *impl << "  // *****************************" << std::endl
          << "  // *****************************" << std::endl
          << "  // tree node type implementation" << std::endl
          << "  // *****************************" << std::endl
	  << "  // *****************************" << std::endl
          << std::endl;

    AFD_Root::nodes_type::const_iterator i;
    for( i = afd->nodes.begin(); i != afd->nodes.end(); ++i )
    {
      const std::string &node_name = i->first;
      const Tree_Node_Type &node = i->second;

      if( node.don_t_create_code ) continue;

      *impl << "  // *****************************" << std::endl
	    << "  // " << node_name << ": node type" <<  std::endl
	    << "  // *****************************" << std::endl
	    <<  std::endl;
      
      *decl << "  // *****************************" << std::endl
	    << "  // node type " << node_name <<  std::endl
	    << std::endl
            << "  class " << translator.node_type( node_name ) << std::endl
	    << "    : virtual public " << translator.node_base_type();
      std::map<std::string, Provided_Results>::const_iterator prov;
      for( prov  = node.provided_results.begin(); 
	   prov != node.provided_results.end(); ++prov )
      {
	const std::string &provides = prov->first;
	*decl << ", " << std::endl
	      << "      public " << translator.provider_type(provides);
      }
      *decl << std::endl
	    << "  {" << std::endl
	    << "  protected:" << std::endl
	    << "    // ** properties **" << std::endl;
      std::map<std::string,Property*>::const_iterator j;
      for( j=node.properties.begin(); j!=node.properties.end(); ++j )
      {
	*decl << "    " << translator.property_type(j->second->type) << " "
	      << translator.prop_op(j->first) << ";" << std::endl;
      }
      *decl << std::endl;

      std::map<std::string,Operand*>::const_iterator op;
      *decl << "    // ** operands **" << std::endl;
      for( op=node.operands.begin(); op!=node.operands.end(); ++op )
      {
	*decl << "    " << translator.operand_type(op->second->type) << " "
	      << translator.prop_op(op->first) << ";" << std::endl;
      }
      *decl << std::endl;

      *decl << "    // ** virtual tree node functions **" << std::endl
	    << "    virtual proptree::Prop_Tree_Node *try_add_child( "
	    << "std::string type, std::string name ) throw();" << std::endl
	    << "    //! custom initialization after hierarchy was set up"
	    << std::endl
	    << "    virtual void custom_hierarchy_final_init();" << std::endl;

      *impl << "  // ** virtual tree node functions **" << std::endl
	    << "  proptree::Prop_Tree_Node *" 
	    << translator.node_type( node_name )
	    << "::try_add_child( std::string type,"
	    << " std::string name ) throw()"<< std::endl
	    << "  {" << std::endl
	    << "    proptree::Prop_Tree_Node *node = 0;" << std::endl;

      std::set<Child_Container>::const_iterator k;
      for( k  = node.child_containers.begin(); 
	   k != node.child_containers.end(); ++k )
      {
	*impl << "    node = " 
	      << translator.container_name( k->provider_type ) 
	      << ".add_child( type, name, info, get_consultant(), node );" 
	      << std::endl;
      }
      *impl << "    return node;" << std::endl
	    << "  }" << std::endl
	    << "  void " << translator.node_type( node_name ) 
	    << "::custom_hierarchy_final_init()" << std::endl
	    << "  {" << std::endl;
      for( k  = node.child_containers.begin(); 
	   k != node.child_containers.end(); ++k )
      {
	*impl << "    " << translator.container_name( k->provider_type ) 
	      << ".hierarchy_final_init();" << std::endl;
      }
      *impl << "  }" << std::endl
	    << std::endl;

      *decl << "  public:" << std::endl;

      *decl << "    // ** child container **" << std::endl;
      for( k  = node.child_containers.begin(); 
	   k != node.child_containers.end(); ++k )
      {
	if( afd->provider_types[ k->provider_type ].serial )
	{
	  *decl << "    " << translator.serial_container( k->provider_type )
		<< " " << translator.container_name( k->provider_type ) << ";"
		<< " // " << (k->max1?"max1 ":"") << (k->min1?"min1":"")
		<< std::endl;
	}
	else
	{
	  *decl << "    " << translator.container( k->provider_type )
		<< " " << translator.container_name( k->provider_type ) << ";"
		<< " // " << (k->max1?"max1 ":"") << (k->min1?"min1":"")
		<< std::endl;
	}
      }
      *decl << std::endl;
      
      *decl << "    // ** constraint, solver and action establishing "
	    << "functions **" << std::endl
	    << "    //! establish general dependencies (common)" 
	    <<	      " for each node" << std::endl
	    << "    virtual void common_init();" << std::endl;

      // write Solve_System_Code (contraints,solvers,actions)
      *impl << "  void " << translator.node_type( node_name )
	    << "::common_init()" << std::endl
	    << "  {" << std::endl;
      write_solve_code( impl, node.common, translator );
      if( !node.child_containers.empty() )
	*impl << "    // ** invoke first_/last_init() for each child container"
	      << " **" << std::endl;
      for( k  = node.child_containers.begin(); 
	   k != node.child_containers.end(); ++k )
      {
	*impl << "    if( !" << translator.container_name( k->provider_type )
	      << ".elements_empty() )" << std::endl
	      << "    {" << std::endl
	      << "      (*" << translator.container_name( k->provider_type )
	      << ".elements_begin())->" 
	      << translator.first_init( k->provider_type ) << ";" 
	      << std::endl
	      << "      (*(--" << translator.container_name( k->provider_type )
 	      << ".elements_end()))->" 
	      << translator.last_init( k->provider_type ) << ";" 
	      << std::endl
	      << "    }" << std::endl;
      }

      bool first = true;
      std::map<std::string, Provided_Results>::const_iterator n;
      for( n = node.provided_results.begin(); n != node.provided_results.end();
	   ++n )
      {
	const std::string &provides = n->first;
	const Provided_Results &results = n->second;
	std::map<Result_Type, Result_Code>::const_iterator o;
	for( o = results.results.begin(); o != results.results.end(); ++o )
	{
	  const Result_Type &res_type = o->first;
	  const Result_Code &res_code = o->second;
	  // is nothing required?
	  if( (res_code.required_properties.empty()) &&
	      (res_code.required_children.empty()) &&
	      (res_code.required_results.empty()) )
	  {
	    *impl << "    " 
		  << translator.is_avail( provides, res_type.return_type, 
					  res_type.parameter_type ) 
		  << " = solve::const_op( true, get_consultant() );" 
		  << std::endl;
	  }
	  else			// if something is required
	  {
	    if( first )		// declare operator only once
	    {
	      *impl << "    // ** establish availability checks **" 
		    << std::endl
		    << "    solve::Multi_And_Operator *m_and;" << std::endl;
	      first = false;
	    }
	    *impl << "    m_and = "
		  << "new solve::Multi_And_Operator(get_consultant());"
		  << std::endl
		  << "    " 
		  << translator.is_avail( provides, res_type.return_type, 
					  res_type.parameter_type ) 
		  << " = m_and->get_result();" 
		  << std::endl;
	    std::list<std::string>::const_iterator req_prop;
	    for( req_prop = res_code.required_properties.begin();
		 req_prop != res_code.required_properties.end();
		 ++req_prop )
	    {
	      *impl << "    m_and->add_operand( solve::is_solved( " 
		    << translator.prop_op( *req_prop ) << " ) );" << std::endl;
	    }
	    std::list< std::pair<std::string,Result_Type> >::const_iterator 
	      req_child;
	    for( req_child = res_code.required_children.begin();
		 req_child != res_code.required_children.end();
		 ++req_child )
	    {
	      const std::string &child_type = req_child->first;
	      const std::string &return_type = req_child->second.return_type;
	      const std::string &parameter_type 
		= req_child->second.parameter_type;

	      *impl << "    m_and->add_operand( " 
		    << translator.container_name( child_type ) << "."
		    << translator.is_avail( child_type, return_type, 
					    parameter_type ) << " );" 
		    << std::endl;
	    }
	    std::list< std::pair<std::string,Result_Type> >::const_iterator 
	      req_res;
	    for( req_res = res_code.required_results.begin();
		 req_res != res_code.required_results.end();
		 ++req_res )
	    {
	      const std::string &provider_type = req_res->first;
	      const std::string &return_type = req_res->second.return_type;
	      const std::string &parameter_type 
		= req_res->second.parameter_type;

	      *impl << "    m_and->add_operand( "
		    << translator.is_avail( provider_type, return_type, 
					    parameter_type ) << " );" 
		    << std::endl;
	    }
	    *impl << "    m_and->finish_adding();" << std::endl;
	  }
	}
      }
      *impl << "  }" << std::endl
	    << std::endl;

      std::map<std::string,Solve_System_Code>::const_iterator m;
      for( m = node.first.begin(); m != node.first.end(); ++m )
      {
	const std::string &provides = m->first;
	const Solve_System_Code &solve_code = m->second;

	std::string function_name;
	if( provides != "" )
	  function_name = translator.first_init(provides);
	else
	  function_name = "first_init()";

	*decl << "    virtual void " << function_name
	      << ";" << std::endl;
	// write Solve_System_Code (contraints,solvers,actions)
	*impl << "  void " << translator.node_type( node_name )
	      << "::" << function_name << std::endl
	      << "  {" << std::endl;
	write_solve_code( impl, solve_code, translator );
	*impl << "  }" << std::endl
	      << std::endl;
      }
      for( m = node.last.begin(); m != node.last.end(); ++m )
      {
	const std::string &provides = m->first;
	const Solve_System_Code &solve_code = m->second;

	std::string function_name;
	if( provides != "" )
	  function_name = translator.last_init(provides);
	else
	  function_name = "last_init()";

	*decl << "    virtual void " << function_name
	      << ";" << std::endl;
	// write Solve_System_Code (contraints,solvers,actions)
	*impl << "  void " << translator.node_type( node_name )
	      << "::" << function_name << std::endl
	      << "  {" << std::endl;
	write_solve_code( impl, solve_code, translator );
	*impl << "  }" << std::endl
	      << std::endl;
      }
      *decl << std::endl;

      *decl << "    // ** result functions **"
	    << std::endl;
      *impl << "  // ** result functions **"
	    << std::endl;

      for( n = node.provided_results.begin(); n != node.provided_results.end();
	   ++n )
      {
	const std::string &provides = n->first;
	const Provided_Results &results = n->second;
	std::map<Result_Type, Result_Code>::const_iterator o;
	for( o = results.results.begin(); o != results.results.end(); ++o )
	{
	  const Result_Type &res_type = o->first;
	  const Result_Code &res_code = o->second;
	  *decl << "    virtual std::pair< bool," 
		<< translator.base_type(res_type.return_type) << " > "
		<< translator.result_function_decl( provides,
						    res_type.return_type, 
						    res_type.parameter_type )
		<< ";" << std::endl;

	  if( !res_code.defined )
	  {
	    //!!! position reporting !!!
	    info->msg.error(node.pos) 
	      << "result function \"" << res_type.return_type
	      << "( " << res_type.parameter_type << " )\"" 
	      << " of provider type \"" << provides << "\" in node \"" 
	      << node_name << "\" is not defined";
	    continue;
	  }

	  *impl << "  std::pair< bool," 
		<< translator.base_type(res_type.return_type) << " > "
		<< translator.node_type( node_name ) << "::"
		<< translator.result_function_impl( provides,
						    res_type.return_type, 
						    res_type.parameter_type,
						    res_code.parameter )
		<< std::endl
		<< "  {" << std::endl
		<< "    std::pair< bool," 
		<< translator.base_type(res_type.return_type) << " > no_res;"
		<< std::endl
		<< "    no_res.first = false;" << std::endl
		<< "    bool did_any_result_fail;" << std::endl
		<< "    bool did_result_fail;" << std::endl
		<< "    did_any_result_fail = false;" << std::endl
		<< "    did_result_fail = false;" << std::endl;

	  if( !res_code.required_properties.empty() )
	  {
	    *impl << "    // ** check for required properties " << std::endl
		  << "    if( ";
	    std::list<std::string>::const_iterator p;
	    bool first=true;
	    for( p  = res_code.required_properties.begin();
		 p != res_code.required_properties.end(); ++p )
	    {
	      const std::string &prop = *p;
	      *impl << (first?first=false,"":" || ")
		    << "!" << translator.prop_op(prop) << ".is_solved()";
	    }
	    *impl << " )" << std::endl
		  << "    {" << std::endl
		  << "      error() << \"required property wasn't solved\";" 
		  << std::endl
		  << "      return no_res;" << std::endl
		  << "    }" << std::endl;
	  }

	  if( !res_code.required_children.empty() )
	  {
	    *impl << "    // ** check for required children" << std::endl
		  << "    if( ";
	    std::list< std::pair<std::string,Result_Type> >::const_iterator q;
	    bool first=true;
	    for( q  = res_code.required_children.begin();
		 q != res_code.required_children.end(); ++q )
	    {
	      const std::string &child_provider = q->first;
	      const Result_Type &res_type = q->second;
	      *impl << (first?first=false,"":" || ")
		    << "!" << translator.container_name(child_provider) << "."
		    << translator.is_avail( child_provider, 
					    res_type.return_type,
					    res_type.parameter_type )
		    << ".is_solved()";
	    }
	    *impl << " )" << std::endl
		  << "    {" << std::endl 
		  << "      error() << \"required child container "
		  << "wasn't solved\";" << std::endl
		  << "      return no_res;" << std::endl
		  << "    }" << std::endl;
	  }

	  if( !res_code.required_results.empty() )
	  {
	    *impl << "    // ** check for required children " << std::endl
		  << "    if( ";
	    std::list< std::pair<std::string,Result_Type> >::const_iterator q;
	    bool first=true;
	    for( q  = res_code.required_results.begin();
		 q != res_code.required_results.end(); ++q )
	    {
	      const std::string &provides = q->first;
	      const Result_Type &res_type = q->second;
	      *impl << (first?first=false,"":" || ")
		    << "!" << translator.is_avail( provides, 
						   res_type.return_type,
						   res_type.parameter_type )
		    << ".is_solved()";
	    }
	    *impl << " )" << std::endl
		  << "    {" << std::endl 
		  << "      error() << \"required result for provided type " 
		  << " wasn't solved\";" << std::endl
		  << "      return no_res;" << std::endl
		  << "    }" << std::endl;
	  }

	  *impl << "    // *** user code following... line:" 
		<< res_code.start_src_line << " ***"
		<< std::endl;
	  // indent like in AFD source file
	  for( int cnt = 0; cnt < res_code.start_src_column; ++cnt )
	    *impl << ' ';
	  *impl << res_code.code << std::endl
		<< "  }" << std::endl
		<< std::endl;
	}
      }
      *decl << std::endl;

      /*
      *decl << "    // ** infrastructure functions **" << std::endl
	    << std::endl
	    << "    //! function that is called after hierarchy was set up" 
	    <<	      " for each node" << std::endl
	    << "    virtual void hierarchy_final_init();" << std::endl;

      *impl << "  // ** infrastructure functions **" << std::endl
	    << std::endl
	    << "  //! function that is called after hierarchy was set up" 
	    <<	    " for each node" << std::endl
	    << "  void " << translator.node_type( node_name )
	    << "::hierarchy_final_init()" << std::endl
	    << "  {" << std::endl
	    << "    common_init();" << std::endl;
      
      *impl << "  }" << std::endl
	    << std::endl;
      */

      *decl << "    // ** infrastructure functions **" << std::endl
	    << "    static void make_availible();" << std::endl;

      *impl << "  // ** infrastructure functions **" << std::endl
	    << "  void " << translator.node_type( node_name )
	    << "::make_availible()" << std::endl
	    << "  {" << std::endl;
      for( n = node.provided_results.begin(); n != node.provided_results.end();
	   ++n )
      {
	const std::string &provides = n->first;
	bool serial = afd->provider_types[provides].serial;
	*impl << "    " 
	      << (serial?translator.serial_container(provides):
		  translator.container(provides))
	      << "::add_node_factory( \"" << node_name 
	      << "\", new proptree::Node_Factory<" 
	      << translator.provider_type(provides) << "," 
	      << translator.node_type( node_name ) << " > );"
	      << std::endl;
      }
      *impl << "  }" << std::endl;


      *decl << "    // ** constructor **" << std::endl
	    << "    " << translator.node_type( node_name ) 
	    << "( std::string name, proptree::tree_info *info, " << std::endl
	    << "      message::Message_Consultant *msg_consultant );" 
	    << std::endl;
      *impl << "  // ** constructor **" << std::endl
	    << "  " << translator.node_type( node_name ) << "::"
	    << translator.node_type( node_name ) 
	    << "( std::string name, proptree::tree_info *info, " << std::endl
	    << "    message::Message_Consultant *msg_consultant )" << std::endl
	    << "    : " << translator.node_base_type() 
	    << "( \"" << node_name << "\", name, info, msg_consultant )";
      // init all provided types (base classes)
      for( prov  = node.provided_results.begin(); 
	   prov != node.provided_results.end(); ++prov )
      {
	const std::string &provides = prov->first;
	*impl << ", " << std::endl
	      << "      " << translator.provider_type(provides) 
	      << "( msg_consultant )";
      }
      // init all properties in constructor
      for( j=node.properties.begin(); j!=node.properties.end(); ++j )
      {
	*impl << "," << std::endl
	      << "      " << translator.prop_op(j->first) 
	      << "( \"" << j->first << "\", this )";
      }
      // init all operands in constructor
      for( op=node.operands.begin(); op!=node.operands.end(); ++op )
      {
	*impl << "," << std::endl
	      << "      " << translator.prop_op(op->first) 
	      << "( get_consultant() )";
      }
      // init child containers with max1/min1
      for( k  = node.child_containers.begin(); 
	   k != node.child_containers.end(); ++k )
      {
        *impl << "," << std::endl
	      << "      " << translator.container_name( k->provider_type )
	      << "(" << (k->max1?"true":"false") << ", " 
	      << (k->min1?"true":"false") << ", get_consultant() )";
      }
      *impl << std::endl
	    << "  {" << std::endl
	    << "    // ********************" << std::endl
	    << "    // Register Properties " << std::endl;
      for( j=node.properties.begin(); j!=node.properties.end(); ++j )
      {
	*impl << "    add_property( \"" << j->first << "\", &"
	      << translator.prop_op(j->first)
	      << " );" << std::endl;
      }
      *impl << std::endl
	    << "    // *****************" << std::endl
	    << "    // Register Aliases " << std::endl;

      std::list<Alias>::const_iterator alias;
      for( alias=node.aliases.begin(); alias!=node.aliases.end(); ++alias )
      {
	*impl << "    add_property( \"" << alias->alias << "\", &"
	      << translator.prop_op(alias->src)
	      << " );" << std::endl;
      }
      *impl << "  }" << std::endl
	    << std::endl;
      
      *decl << "  private:";
      *decl << "    // solver identifiers in common block" << std::endl;
      write_solver_declarations( decl, node.common.solvers, translator );
      *decl << "    // solver identifiers in any first block" << std::endl;
      for( m = node.first.begin(); m != node.first.end(); ++m )
      {
	write_solver_declarations( decl, m->second.solvers, translator );
      }
      *decl << "    // solver identifiers in any last block" << std::endl;
      for( m = node.last.begin(); m != node.last.end(); ++m )
      {
	write_solver_declarations( decl, m->second.solvers, translator );
      }
      *decl << "  };" << std::endl;
      *decl << std::endl;
    }

    *decl << "  // *****************************" << std::endl
	  << "  // make nodes availible " << std::endl
	  << std::endl
	  << "  void make_" << info->id_name << "_nodes_availible();" 
	  << std::endl;

    *impl << "  // *****************************" << std::endl
	  << "  // make nodes availible " <<  std::endl
	  << "  // *****************************" << std::endl
	  << std::endl
	  << "  void make_" << info->id_name << "_nodes_availible()" 
	  << "  {" << std::endl;
    
    for( i = afd->nodes.begin(); i != afd->nodes.end(); ++i )
    {
      if( i->second.don_t_create_code ) continue;

      const std::string &node_name = i->first;

      *impl << "    " << translator.node_type(node_name) 
	    << "::make_availible();" <<  std::endl;  
    }
    *impl << "  }" << std::endl;
  }

  Cpp_Code_Translator *Cpp_Code_Generator::get_translator()
  {
    return &translator;
  }

  void Cpp_Code_Generator::generate_code( AFD_Root *afd_root )
  {
    afd = afd_root;
    decl = new std::ofstream( (info->base_name+".hpp").c_str() );
    impl = new std::ofstream( (info->base_name+".cpp").c_str() );
    generate_header();
    generate_priority_list();
    generate_base_types();
    generate_types();
    generate_operators();
    generate_solvers();
    generate_nodes();
    generate_footer();
  }

  Cpp_Code_Generator::Cpp_Code_Generator( code_gen_info *i ) 
    : info(i), translator(i) 
  {
  }
}
