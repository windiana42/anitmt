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
    return "functionality::Prop_Tree_Node";
  }
  std::string Cpp_Code_Translator::open_action( std::string action, 
						double level )
  {
    if( action == "push" )
    {
      return std::string("solve::establish_Push_Connection( "
			 "info->priority_system,") + level;
    }
    if( action == "default" )
    {
      return std::string("solve::establish_Default_Value( "
			 "info->priority_system,") + level;
    }
    // if any other action ??? could also output error ???
    return "solve::" + action + "( info->priority_system, " + level;
  }
  std::string Cpp_Code_Translator::parameter_add( std::string param )
  {
    return ", " + param;
  }
  std::string Cpp_Code_Translator::close_function()
  {
    return ");";
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
    return std::string("/*!!!insert child ")/* + n*/ + "!!!*/" ;
  }

  Cpp_Code_Translator::Cpp_Code_Translator()
    : prefix_base_type(""), prefix_provider_type("_pt_"), 
      prefix_node_type("node_"), prefix_prop_op("_op_"),prefix_res_fun("_rf_"),

      prefix_is_avail("_av_"), prefix_param_range("_pr_"), 
      prefix_container_type("_container_"), 
      prefix_serial_container_type("_serial_container_"), 
      prefix_container_name("_cn_")
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
    *decl << "#include <list>" << std::endl;
    *decl << "#include <string>" << std::endl;
    *decl << "#include <map>" << std::endl;
    *decl << std::endl;
    *decl << "#include <message/message.hpp>" << std::endl;
    *decl << "#include <val/val.hpp>" << std::endl;
    *decl << "#include <solve/operand.hpp>" << std::endl;
    *decl << "#include <proptree/property.hpp>" << std::endl;
    *decl << "#include <proptree/proptree.hpp>" << std::endl;
    *decl << std::endl;
    *decl << "namespace functionality" << std::endl;
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
    *impl << "#include \""+info->base_name+".hpp\"" << std::endl;
    *impl << std::endl;
    *impl << "#include <solve/operator.hpp>" << std::endl;
    *impl << "#include <solve/solver.hpp>" << std::endl;
    *impl << std::endl;
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
    for( i = afd->base_types_list.begin(); i!=afd->base_types_list.end(); ++i )
    {
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
      // ****************
      // provider class

      *decl << "  class " << translator.provider_type( provides )
	    << std::endl
            << "  {" << std::endl
            << "  public:" << std::endl
            << "    // ** result functions **" << std::endl;

      Provider_Type::result_types_type::const_iterator j;
      for( j  = provider_type.result_types.begin(); 
	   j != provider_type.result_types.end(); ++j )
      {
	*decl << "    virtual " << translator.base_type(j->return_type) << " "
	      << translator.result_function_decl( provides, j->return_type, 
						  j->parameter_type )
	      << " = 0;" << std::endl;
	if( provider_type.serial )	// is serial provider type?
	{			// ... create start/end-parameter operands
	  *decl << "    solve::Operand<" << j->parameter_type << "> " 
		<< translator.start_param( provides, j->return_type, 
					   j->parameter_type )
		<< ";" << std::endl
		<< "    solve::Operand<" << j->parameter_type << "> " 
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
	    << "    : ";
      bool first = true;
      if( provider_type.serial )	// is serial provider type?
      {				 // ... init start/end-parameter operands
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *impl << (first?first=false,"":",\n      ")
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
	*impl << (first?first=false,"":",\n      ")
	      << translator.is_avail( provides, j->return_type, 
				      j->parameter_type ) << "(c)";
      }
      *impl << "  {" << std::endl
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
	      << "  private:" << std::endl
	      << "    bool max1; // maximal one element" << std::endl 
	      << "    bool min1; // minimal one element" << std::endl
	      << "    typedef std::list<" 
	      << translator.provider_type(provides) << "*>"
	      <<        "elements_type;" << std::endl
	      << "    elements_type elements;" << std::endl
	      << "  public:" << std::endl
	      << "    // ** result functions **" << std::endl;

	Provider_Type::result_types_type::const_iterator j;
	for( j  = provider_type.result_types.begin(); 
	     j != provider_type.result_types.end(); ++j )
	{
	  *decl << "    virtual std::pair<bool," 
		<< translator.base_type(j->return_type) << "> " 
		<< translator.result_function_decl( provides, j->return_type, 
						    j->parameter_type )
		<< ";" << std::endl;

	  *impl << "  std::pair<bool," << translator.base_type(j->return_type)
		<< "> " << translator.serial_container( provides ) << "::"
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
		<< "          return std::pair<bool," 
		<< translator.base_type(j->return_type) << ">(true," 
		<< "(*i)->" 
		<< translator.result_function_call( provides, j->return_type, 
						    j->parameter_type,"_par_" )
		<< "); " << std::endl
		<< "        }" << std::endl
		<< "        else // must be undefined range" << std::endl
		<< "        {" << std::endl
		<< "          break;" << std::endl
		<< "        }" << std::endl
		<< "      }" << std::endl
		<< "    }" << std::endl
		<< "    // undefined range" << std::endl
		<< "    std::pair<bool," 
		<< translator.base_type(j->return_type) << "> ret; " 
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
	      << "  {" << std::endl
	      << "  }" << std::endl
	      << std::endl;
      }
      else
      {				// ... create container class
	*decl << "  class " << translator.container( provides )
	      << std::endl
	      << "  {" << std::endl
	      << "  private:" << std::endl
	      << "    bool max1; // maximal one element" << std::endl 
	      << "    bool min1; // minimal one element" << std::endl
	      << "    typedef std::list<" 
	      << translator.provider_type(provides) << "*>"
	      <<        "elements_type;" << std::endl
	      << "    elements_type elements;" << std::endl
	      << "  public:" << std::endl
	      << "    elements_type::iterator elements_begin(); " << std::endl
	      << "    elements_type::iterator elements_end(); " << std::endl
	      << "    bool elements_empty(); " << std::endl
	      << "    // ** constructor **" << std::endl
	      << "    " << translator.container( provides ) 
	      << "(bool max1, bool min1, message::Message_Consultant* );" 
	      << std::endl
	      << "    // ** virtual destructor **" << std::endl
	      << "    virtual ~" << translator.container( provides ) 
	      << "() {}" << std::endl
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
	      << std::endl;
      }
    }
    *decl << std::endl;
  }

  void write_solve_code( std::ostream *os, 
			 const Solve_System_Code &solve_code )
  {
    // ** Constraints **
    {
      std::list<Constraint_Declaration>::const_iterator i;
      for( i  = solve_code.constraints.constraint_declarations.begin();
	   i != solve_code.constraints.constraint_declarations.end(); ++i )
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
	  *os << "      " << i->constraint_code << std::endl;
	  *os << "    }" << std::endl;
	}
	else
	  *os << "    " << i->constraint_code << std::endl;
      }
    }
    // ** Solvers **
    {
      std::list<Solver_Declaration>::const_iterator i;
      for( i  = solve_code.solvers.solver_declarations.begin();
	   i != solve_code.solvers.solver_declarations.end(); ++i )
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
	  *os << "      " << i->solver_code << std::endl;
	  *os << "    }" << std::endl;
	}
	else
	  *os << "    " << i->solver_code << std::endl;
      }
    }
    // ** Actions **
    {
      std::list<Action_Declaration>::const_iterator i;
      for( i  = solve_code.actions.action_declarations.begin();
	   i != solve_code.actions.action_declarations.end(); ++i )
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
  }

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

      *impl << "  // *****************************" << std::endl
	    << "  // " << node_name << ": node type" <<  std::endl
	    << "  // *****************************" << std::endl
	    <<  std::endl;
      
      *decl << "  // *****************************" << std::endl
	    << "  // node type " << node_name <<  std::endl
	    << std::endl
            << "  class " << translator.node_type( node_name ) << std::endl
	    << "    : public " << translator.node_base_type();
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
      std::map<std::string,std::string>::const_iterator j;
      for( j=node.properties.begin(); j!=node.properties.end(); ++j )
      {
	*decl << "    " << translator.property_type(j->second) << " "
	      << translator.prop_op(j->first) << ";" << std::endl;
      }
      *decl << std::endl;

      *decl << "    // ** operands **" << std::endl;
      for( j=node.operands.begin(); j!=node.operands.end(); ++j )
      {
	*decl << "    " << translator.operand_type(j->second) << " "
	      << translator.prop_op(j->first) << ";" << std::endl;
      }
      *decl << std::endl;

      *decl << "    // ** child container **" << std::endl;
      std::set<Child_Container>::const_iterator k;
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
      
      *decl << "  public:" << std::endl
	    << "    // ** constraint, solver and action establishing "
	    << "functions **" << std::endl
	    << "    //! establish general dependencies (common)" 
	    <<	      " for each node" << std::endl
	    << "    virtual void common_init();" << std::endl;

      // write Solve_System_Code (contraints,solvers,actions)
      *impl << "  void " << translator.node_type( node_name )
	    << "::common_init()" << std::endl
	    << "  {" << std::endl;
      write_solve_code( impl, node.common );
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
	write_solve_code( impl, solve_code );
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
	write_solve_code( impl, solve_code );
	*impl << "  }" << std::endl
	      << std::endl;
      }
      *decl << std::endl;

      *decl << "    // ** result functions **"
	    << std::endl;
      *impl << "  // ** result functions **"
	    << std::endl;
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
	  *decl << "    virtual std::pair<bool," 
		<< translator.base_type(res_type.return_type) << "> "
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

	  *impl << "  std::pair<bool," 
		<< translator.base_type(res_type.return_type) << "> "
		<< translator.node_type( node_name ) << "::"
		<< translator.result_function_impl( provides,
						    res_type.return_type, 
						    res_type.parameter_type,
						    res_code.parameter )
		<< std::endl
		<< "  {" << std::endl
		<< "    std::pair<bool," 
		<< translator.base_type(res_type.return_type) << "> no_res;"
		<< std::endl
		<< "    no_res.first = false;" << std::endl;

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
		    << "!" << translator.prop_op(prop) << ".is_solve()";
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

      *decl << "    // ** constructor **" << std::endl
	    << "    " << translator.node_type( node_name ) 
	    << "( std::string name, tree_info *info, " << std::endl
	    << "      message::Message_Consultant *msg_consultant );" 
	    << std::endl;
      *impl << "  // ** constructor **" << std::endl
	    << "  " << translator.node_type( node_name ) << "::"
	    << translator.node_type( node_name ) 
	    << "( std::string name, tree_info *info, " << std::endl
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
	      << "( \"" << translator.prop_op(j->first) << "\", this )";
      }
      // init all operands in constructor
      for( j=node.operands.begin(); j!=node.operands.end(); ++j )
      {
	*impl << "," << std::endl
	      << "      " << translator.prop_op(j->first) 
	      << "( get_consultant() )";
      }
      // init child containers with max1/min1
      for( k  = node.child_containers.begin(); 
	   k != node.child_containers.end(); ++k )
      {
        *impl << "," << std::endl
	      << "      " << translator.container_name( k->provider_type )
	      << "(" << (k->max1?"true":"false") << "," 
	      << (k->min1?"true":"false") << ")"
	      << " // " << (k->max1?"max1 ":"") << (k->min1?"min1":"");
      }
      *impl << std::endl
	    << "  {" << std::endl
	    << "    // ********************" << std::endl
	    << "    // Register Properties " << std::endl;
      for( j=node.properties.begin(); j!=node.properties.end(); ++j )
      {
	*impl << "    add_property( \""
	      << translator.prop_op(j->first) << "\", "
	      << translator.prop_op(j->first)
	      << " );" << std::endl;
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
