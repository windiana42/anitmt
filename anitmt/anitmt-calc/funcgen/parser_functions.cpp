/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
/*****************************************************************************/
/**									    **/
/** Author:  Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "parser_functions.hpp"

#include <assert.h>
#include "stdextend.hpp"


namespace funcgen
{
  // **********************************
  // global objects for help functions

  Available_Basic_Operators available_operators;

  // ******************************
  // functions used by the parser and lexer
  // ******************************

  void start_code_block( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    I->lexer->goto_code_copy_mode();
    I->depth_counter = 0;

    //message::Message_Reporter &msg = I->msg;
    //Code_Translator *translator = I->afd->translator;
    Code *code = I->afd->current_code;

    if( code )
    {
      code->start_src_line = I->file_pos.get_line();
      code->start_src_column = I->file_pos.get_column();
    }    
  }
  void finish_code_block( void *info )
  {
  }
  void continue_code_mode( void *info )
  {
    static_cast<afd_info*>(info)->lexer->goto_code_copy_mode();
  }

  // for lexer
  void copy_code_line( afd_info *info, char *line, int len ) 
  {
    //message::Message_Reporter &msg = info->msg;
    Code *code = info->afd->current_code;

    if( code )
    {
      code->code += line;
    }    
  }

  void code_block_escape( afd_info *info ) {}
  void finished_file( afd_info *info ) 
  {
    info->afd->don_t_create_code.pop(); // reduce stack
  }

  //******************************************
  // concrete help functions for parser rules

  void check_id_operator( void *info, std::string identifier )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    if( identifier != "operator" )
    {
      msg.error() << "only version name \"operator\" may be followed by an operator";
    }
  }

  void include_declaration( void *infoptr, const std::string &file )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;

    // determine recursion depth
    if( afd->include_depth >= max_include_depth )
    {
      info->msg.error() << "maximum include depth " << max_include_depth 
			<< " reached";
      return;
    }
    ++afd->include_depth;

    const code_gen_info *code_info = afd->translator->get_info();
    afd->don_t_create_code.push(true);
    std::string::size_type p = file.rfind('.');
    std::string base_name;
    if( p != std::string::npos )
      base_name = file.substr(0,p);
    else
      base_name = file;

    afd->included_basenames.push_back(base_name);
    if( !info->open_file(file, false) )	// if unable to open
    {
      // try all include directories
      std::string filename;
      std::list<std::string>::const_iterator i;
      for( i  = code_info->include_paths.begin();
	   i != code_info->include_paths.end();
	   ++i )
      {
	const std::string &path = *i;
	filename = path;
	if( path.length() )	
	{
	  // check whether path ends with path separator
	  char last_ch = path[ path.length()-1 ]; // get last character
	  if( (last_ch != '/') && (last_ch != '\\') 
	      && (last_ch != code_info->path_separator) )
	  {
	    filename += code_info->path_separator;
	  }
	}
	filename += file;

	if( info->open_file(filename, false) ) // is file in this directory?
	  break;
      }
      if( i == code_info->include_paths.end() ) // if nowhere found
      {
	info->msg.error() << "couldn't open header file " << file;
      }
      
    }
  }

  void include_header( void *info, const std::string &file )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    if( !afd->don_t_create_code.top() )
      afd->header_files.add_header( file );
  }

  bool avoid_recursion( void *infoptr, const std::string &unique_id )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    if( afd->avoid_recursion_of.find( unique_id )
	!= afd->avoid_recursion_of.end() )
    {
      // avoid recursion by closing file
      finished_file(info); if( info->close_file() ) return true;
    }
    else
    {
      afd->avoid_recursion_of.insert( unique_id );
    }
    return false;
  }  

  void priority_list_defined( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    afd->priority_list_defined = true;
    afd->write_priority_list = !afd->don_t_create_code.top();
  }
  void priority_label_add( void *info, std::string label )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    afd->priority_list.add_priority_label( label );
  }

  void declare_base_type( void *info, const std::string &name, 
			  const std::string &type )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    // add to map and a pointer to sequence list
    Base_Type *base_type = &(afd->base_types[name] = Base_Type(type));
    base_type->don_t_create_code = afd->don_t_create_code.top();
    afd->base_types_list.push_back
      (std::pair<std::string,Base_Type*>
       (name, base_type));    
  }

  void declare_base_type_structure( void *info, const std::string &name )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    // add to map and store current pointer
    afd->current_base_type = &afd->base_types[name];
    // add pointer to sequence list
    afd->base_types_list.push_back
      (std::pair<std::string,Base_Type*>(name,afd->current_base_type));    
    afd->current_base_type->don_t_create_code = afd->don_t_create_code.top();
  }
  
  void base_type_structure_element( void *info, const std::string &type, 
				    const std::string &name )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    assert( afd->current_base_type != 0 );
    afd->current_base_type->add_element(type,name);
  }

  void start_provider_type_declaration( void *infoptr, bool serial,
					const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    afd->current_type = &afd->provider_types[name];
    afd->current_type->serial = serial;
    afd->current_type->pos = info->file_pos.duplicate();
    afd->current_type->don_t_create_code = afd->don_t_create_code.top();
  }

  void add_provided_result_type( void *info, const std::string &ret, 
				 const std::string &par )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    assert( afd->current_type != 0 );
    Provider_Type *provider_type = afd->current_type;

    std::set<Result_Type>::iterator i;
    i = provider_type->result_types.find( Result_Type(ret,par) );
    if( i != provider_type->result_types.end() )
    {
      msg.error() 
	<< "result type \"" << ret << "(" << par << ")\""
	<< " is already provided";
    }
    else
      afd->current_type->result_types.insert( Result_Type(ret,par) );
  }

  void start_operator_declaration( void *infoptr, const std::string &type,
				   const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    message::Message_Reporter &msg = info->msg;

    if( afd->operator_declarations.find(name) != afd->operator_declarations.end() )
    {
      msg.error() << "operator " << name << " already defined";
      afd->current_operator_declaration = 0;
    }
    else if( !available_operators.is_operator( type ) )
    {
      msg.error() << "invalid operator base type: " << type;
      afd->current_operator_declaration = 0;
    }
    else
    {
      // insert new operator declaration with name
      afd->current_operator_declaration = &afd->operator_declarations[name];
      // initialize it
      afd->current_operator_declaration->don_t_create_code 
	= afd->don_t_create_code.top();

      afd->current_operator_declaration->pos = info->file_pos.duplicate();
      afd->current_operator_declaration->basic_operator 
	= &available_operators.get_operator( type );
      afd->current_operator_declaration->operator_name = name;
      afd->current_operator_declaration->operator_base_type_name = type;
    }
  }
  void start_operator_function_declaration( void *infoptr, 
					    const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    message::Message_Reporter &msg = info->msg;

    if( afd->current_operator_declaration )
    {
      if( !available_operators.get_operator( afd->current_operator_declaration
					     ->operator_base_type_name )
	  .is_function(name) )
      {
	msg.error() << "unknown function name "<< name <<" for this operator";
	afd->current_operator_declaration->current_function = 0;
      }
      else if( afd->current_operator_declaration->function_code.find(name)
	       != afd->current_operator_declaration->function_code.end() )
      {
	msg.error() << "function "<< name <<" already defined";
	afd->current_operator_declaration->current_function = 0;
      }
      else
      {
	afd->current_operator_declaration->current_function = 
	  &afd->current_operator_declaration->function_code[name];
	afd->current_operator_declaration->current_function->pos 
	  = info->file_pos.duplicate();
      }
    }
  }
  void start_operator_function_code( void *infoptr )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    //message::Message_Reporter &msg = info->msg;

    if( afd->current_operator_declaration )
    {
      if( afd->current_operator_declaration->current_function )
      {
	afd->current_code 
	  = afd->current_operator_declaration->current_function;
      }
    }
  }
  void finish_operator_function_code( void *infoptr )
  {	       
    /* everything is done by copy_code_line() */
  }
  void add_operator_function_parameter( void *infoptr, 
					const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    //message::Message_Reporter &msg = info->msg;

    if( afd->current_operator_declaration )
    {
      if( afd->current_operator_declaration->current_function )
      {
	afd->current_operator_declaration->current_function->parameter_names
	  .push_back( name );
      }
    }
  }
  void start_operator_version( void *infoptr, const std::string &ret_type,
			     const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    //message::Message_Reporter &msg = info->msg;

    if( afd->current_operator_declaration )
    {
      afd->current_operator_declaration->versions
	.push_back(std::list<std::string>());
      afd->current_operator_declaration->current_version =
	&(afd->current_operator_declaration->versions.back());

      afd->current_operator_declaration->current_version
	->push_back( name );
      afd->current_operator_declaration->current_version
	->push_back( ret_type );
    }
  }
  void add_operator_version_parameter( void *infoptr, const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    //message::Message_Reporter &msg = info->msg;

    if( afd->current_operator_declaration )
    {
      if( afd->current_operator_declaration->current_version )
      {
	afd->current_operator_declaration->current_version->push_back( name );
      }
    }
  }
  void finish_operator_version( void *infoptr )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    message::Message_Reporter &msg = info->msg;

    if( afd->current_operator_declaration )
    {
      if( afd->current_operator_declaration->current_version )
      {
	int num_operands = afd->current_operator_declaration
	  ->basic_operator->get_num_operands();
	if( int(afd->current_operator_declaration->current_version->size())
	    != num_operands + 2 )
	{
	  msg.error() << "wrong number of operand type parameters, " 
		      << num_operands << " expected";
	  msg.error(afd->current_operator_declaration->pos) << "see here";
	}
      }
    }
  }

  void start_node_declaration( void *infoptr, bool abstract,
			       const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    afd->current_node = &afd->nodes[name]; 
    afd->current_node->pos = info->file_pos.duplicate();
    if( abstract )		// dont create abstract nodes
      afd->current_node->don_t_create_code = true;
    else
      afd->current_node->don_t_create_code = afd->don_t_create_code.top();
  }
  void node_extends( void *infoptr, const std::string &node )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    assert(afd->current_node != 0);

    // search node
    std::map<std::string,Tree_Node_Type>::iterator i;
    i = afd->nodes.find( node );
    
    if( i == afd->nodes.end() )
      info->msg.error() 
	<< "node \"" << node << "\" to extend doesen't exist";
    else
    {
      if( &(i->second) == afd->current_node )
	info->msg.error() 
	  << "recursive inheritance of node \"" << node << "\" not allowed";
      else
      {
	afd->current_node->merge( i->second ); // merge with node to extend
      }
    }
  }
  void node_provides( void *infoptr, const std::string &type )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    assert(afd->current_node != 0);
    
    // check if provider type exists
    std::map<std::string,Provider_Type>::iterator i;
    i = afd->provider_types.find(type);
    if( i == afd->provider_types.end() )
      info->msg.error() 
	<< "provided type \"" << type << "\" doesn't exist";
    else
    {
      Provider_Type &provider_type = i->second;
      // insert provided type
      Provided_Results &provided_res 
	= afd->current_node->provided_results[type];
      provided_res.type = &provider_type;
      provided_res.type_name = type;

      // insert all result functions of this provider type
      Provider_Type::result_types_type::const_iterator j;
      for( j  = provider_type.result_types.begin();
	   j != provider_type.result_types.end(); ++j )
      {
	provided_res.results[*j]; 
      }
    }
  }

  void node_start_property_type( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);

    node->current_property_type = type;

    // ?!? allow only known property types ?!?
    if( ( type != "flag" ) && ( type != "scalar" ) && ( type != "vector" ) &&
	( type != "matrix" ) && ( type != "string" ) )
    {
      msg.error() << "property type \"" << type << "\" isn't allowed";
      node->current_property_type = "<invalid_type>";
    }
  }
  void node_declare_property( void *info, const std::string &name )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->properties[name] = node->current_property_type;
  }
  void node_declare_property( void *info, const std::string &type,
			      const std::string &name )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->properties[name] = type;
  }

  void node_declare_alias( void *info, const std::string &alias, 
			   const std::string &property )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->aliases.push_back(Alias(alias,property));
  }

  void node_start_operand_type( void *info, const std::string &type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);

    node->current_operand_type = type;
  }
  void node_declare_operand( void *info, const std::string &name )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->operands[name] = node->current_operand_type;
  } 
  void node_declare_operand( void *info, const std::string &type, 
			     const std::string &name )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->operands[name] = type;
  } 

  void node_start_common_declaration( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->current_solve_code = &node->common;
  }
  void node_start_first_declaration( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;    
    assert(node != 0);
    if( type != "" )
    {
      std::map<std::string, Provided_Results>::iterator i;
      i = node->provided_results.find(type);
      if( i == node->provided_results.end() ) //  if not found
      {
	msg.error() 
	  << "type \"" << type << "\" is not provided by this node type";
	msg.verbose(1,node->pos) << "> see here";
	node->current_solve_code = 0;
      }
      else
	// enter declaration in map and set the current pointer
	node->current_solve_code = &node->first[type];
    }
    else
      // enter declaration in map and set the current pointer
      node->current_solve_code = &node->first[type];
  } 
  void node_start_last_declaration( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;    
    assert(node != 0);
    if( type != "" )
    {
      std::map<std::string, Provided_Results>::iterator i;
      i = node->provided_results.find(type);
      if( i == node->provided_results.end() ) //  if not found
      {
	msg.error() 
	  << "type \"" << type << "\" is not provided by this node type";
	msg.verbose(1,node->pos) << "> see here";
	node->current_solve_code = 0;
      }
      else
	// enter declaration in map and set the current pointer
	node->current_solve_code = &node->last[type];
    }
    else
      // enter declaration in map and set the current pointer
      node->current_solve_code = &node->last[type];
    
    // enter declaration in map and set the current pointer
    node->current_solve_code = &node->last[type]; 
  } 

  void node_solve_constraint( void *info, const Expression *exp  )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;    
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->constraints.new_constraint(exp);
    }
    delete exp;
  }

  void node_start_solver( void *info, const std::string &solver )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;    
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.new_solver(solver);
    }
  }
  void node_add_solver_parameter( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_parameter_ref(node->current_reference);
    }
  } // from reference
  void node_finish_solver( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.finish_solver();
    }
  } 
  void node_solve_expression( void *info, const std::string &op,
			      Expression *exp )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.new_expression_solver(op, exp, translator);
    }
    delete exp;
  }

  void node_start_action( void *info, const std::string &name, 
			  std::string priority_label )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Code_Translator *translator = afd->translator;
    Tree_Node_Type *node = afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      if( !(afd->priority_list.is_priority_label(priority_label)) )
      {
	msg.error() << "unknown label for priority level: " << priority_label;
      }
      else
	solve_code->actions.new_action(name, priority_label, translator);
    }
  }
  void node_add_action_parameter_ref( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_ref(node->current_reference,
					    translator);
    }
  }
  void node_add_action_parameter_str( void *info, std::string str )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_str(str,translator);
    }
  }
  void node_add_action_parameter_exp( void *info, Expression *exp )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_exp(exp,translator);
    }
    delete exp;
  }
  void node_finish_action( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.finish_action(translator);
    }
  }

  void node_contains( void *info, bool max1, bool min1, 
		      const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Tree_Node_Type *node = afd->current_node;
    if( node )
    {
      AFD_Root::provider_types_type::iterator i = 
	afd->provider_types.find(type);
      if( i == afd->provider_types.end() )
      {
	msg.error() << "provider type " << type << " doesn't exist";
      }
      else
      {
	Provider_Type &provider_type = i->second;
	node->child_containers.insert( Child_Container(max1,min1,type,
						       provider_type.serial) );
      }
    }
  }

  void node_start_provide( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Tree_Node_Type *node = afd->current_node;
    assert(node != 0);

    if( node )
    {
      // search provided results object
      std::map<std::string, Provided_Results>::iterator i;
      i = node->provided_results.find( type );
      if( i != node->provided_results.end() ) // was it found?
      {
	node->current_provided_results = &(i->second);
				// get the pointer to the provided results code
	node->current_provided_result_type = &afd->provider_types[type];
				// get the pointer to the provider type
      }
      else			// provided type not found?
      {
	msg.error() 
	  << "type \"" << type << "\" is not provided by this node type";
	msg.verbose(1,node->pos) << "> see here";
	node->current_provided_results = 0;
      }
    }
  }

  void node_start_result_code( void *info, const std::string &ret_type, 
			       const std::string &par_type, 
			       const std::string &par_name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Tree_Node_Type *node = afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;

    if( provided )		// is in valid provide section
    {
      Result_Type res_type(ret_type,par_type);
      //check whether this result function is part of current provider type
      Provider_Type::result_types_type::const_iterator i;
      i = provided->type->result_types.find( res_type );
      if( i != provided->type->result_types.end() )
      {				// was it found?
	//start result function
	provided->current_result_code =
	  &provided->results[res_type];
	provided->current_result_code->defined = true;
	provided->current_result_code->return_type = ret_type;
	provided->current_result_code->parameter_type = par_type;
	provided->current_result_code->parameter = par_name;

	// for code mode of lexer
	afd->current_code = provided->current_result_code; 
      }
      else
      {				
	msg.error() 
	  << "result function \"" << ret_type << "( " << par_type 
	  << " )\" is not part of the current provider type";
	msg.verbose(1,provided->type->pos) << "> see here";
	provided->current_result_code = 0;
      }
    }
  }
  void node_result_essential_prop( void *info, const std::string &property )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether property exists !!!
 
	// insert required property
	res_code->required_properties.push_back( property );
      }
    }
  }
  void node_result_essential_child_result( void *info, 
					   const std::string &provider, 
					   const std::string &ret, 
					   const std::string &par )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether result function of child exists exists !!!

	// insert required child result function
	res_code->required_children.push_back
	  (std::pair<std::string,Result_Type>(provider,Result_Type(ret,par)));
      }
    }
  }
  void node_result_essential_result( void *info, const std::string &provider, 
				     const std::string &ret, 
				     const std::string &par )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether result function exists exists !!!

	// insert required child result function
	res_code->required_results.push_back
	  (std::pair<std::string,Result_Type>(provider,Result_Type(ret,par)));
      }
    }
  }

  void write_code(void *info, double scalar )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += std::string("") + scalar + " ";
      }
    }    
  }
  void write_code(void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += id + " " ;
      }
    }    
  }

  void res_ref_property( void *info, std::string prop )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether property exists !!!

	res_code->code += translator->prop_op_value(prop); 
      }
    }    
  }
  void res_ref_child( void *info, std::string provider, 
		      std::string result_type, std::string parameter_type, 
		      std::string parameter, std::string fail_bool_var )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether result function of child exists !!!

	res_code->code += 
	  translator->child_result_with_status( provider, result_type, 
						parameter_type, parameter, 
						fail_bool_var );
      }
    }    
  }
  void res_ref_this( void *info, std::string provider, 
		      std::string result_type, std::string parameter_type, 
		      std::string parameter, std::string fail_bool_var )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;

    // test whether optional provider wasn't specified
    if( result_type == "" )
    {
      result_type = provider;	//!! parameter are used shifted (see parser.yy)
      provider = provided->type_name;
    }

    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether result function of child exists exists !!!

	res_code->code += 
	  translator->provided_result_with_status( provider, result_type, 
						   parameter_type, parameter, 
						   fail_bool_var );
      }
    }    
  }
  void res_ref_ret_child( void *info, std::string provider, 
			  std::string result_type, std::string parameter_type, 
			  std::string parameter )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether result function of child exists !!!

	res_code->code += 
	  translator->child_result( provider, result_type, 
				    parameter_type, parameter );
      }
    }    
  }
  void res_ref_ret_this( void *info, std::string provider, 
			 std::string result_type, std::string parameter_type, 
			 std::string parameter )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;

    // test whether optional provider wasn't specified
    if( result_type == "" )
    {
      result_type = provider;	//!! parameter are used shifted (see parser.yy)
      provider = provided->type_name;
    }

    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	//!!! check whether result function of child exists exists !!!

	res_code->code += 
	  translator->provided_result( provider, result_type, 
				       parameter_type, parameter );
      }
    }    
  }

  void res_ref_start_return_res( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->start_return_res
	  (res_code->return_type);
      }
    }
  }
  void res_ref_finish_return_res( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->finish_return_res
	  (res_code->return_type);
      }
    }
  }
  void res_ref_start_return_prop( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->start_return_prop
	  (res_code->return_type);
      }
    }
  }
  void res_ref_finish_return_prop( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->finish_return_prop
	  (res_code->return_type);
      }
    }
  }
  void res_ref_start_return( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->start_return(res_code->return_type);
      }
    }
  }
  void res_ref_finish_return( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->finish_return(res_code->return_type);
      }
    }
  }
  void res_ref_return_fail( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->return_fail();
      }
    }
  }
  void res_ref_return_if_fail( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert(node != 0);
    Provided_Results *provided = node->current_provided_results;
    if( provided )
    {
      Result_Code *res_code = provided->current_result_code;

      if( res_code )		// in valid result code?
      {
	res_code->code += translator->return_fail();
      }
    }
  }

  Expression *bool_expr( Expression *exp1, 
			 const std::string &op, 
			 Expression *exp2 )
  {
    Expression *res = exp1;
    res->append(op);
    res->append(exp2);
    delete exp2;
    return res;
  }
  Expression *bool_expr( const std::string &op, 
			 Expression *exp )
  {
    Expression *res = new Expression;
    res->append(op);
    res->append(exp);
    delete exp;
    return res;
  }
  Expression *bool_expr( Expression *exp )
  {
    Expression *res = new Expression;
    res->append("(");
    res->append(exp);
    res->append(")");
    delete exp;
    return res;
  }
  Expression *expr_from_ref( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert( node != 0 );
    Expression *res = new Expression( node->current_reference );
    node->current_reference.clear();
    return res;
  }
  Expression *expr_bool( void *info, bool flag )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;

    Expression *res = new Expression();
    res->append( translator->operand_from_bool( flag ) );
    return res;
  }
  Expression *expr_scalar( void *info, double val )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;

    Expression *res = new Expression();
    res->append( translator->operand_from_scalar( val ) );
    return res;
  }
  Expression *expr_string( void *info, std::string str )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;

    Expression *res = new Expression();
    res->append( translator->operand_from_string( str ) ); 
    return res;
  }
  Expression *expr( Expression *exp1, 
		    const std::string &op, 
		    Expression *exp2 )
  {
    Expression *res = exp1;
    res->append(op);
    res->append(exp2);
    delete exp2;
    return res;
  }
  Expression *expr( const std::string &op, 
		    Expression *exp )
  {
    Expression *res = new Expression;
    res->append(op);
    res->append(exp);
    delete exp;
    return res;
  }
  Expression *expr( Expression *exp )
  {
    Expression *res = new Expression;
    res->append("(");
    res->append(exp);		// could be optimized with prefix!!!
    delete exp;
    res->append(")");
    return res;
  }
  Expression *expr_function( const std::string &name )
  {
    Expression *res = new Expression;
    res->append(name+'(');
    res->append(")");
    return res;
  }

  Expression *expr_function( const std::string &name, 
			     Expression *par )
  {
    Expression *res = new Expression;
    res->append(name+'(');
    res->append(par);
    delete par;
    res->append(")");
    return res;
  }
  Expression *expr_array( Expression *exp1, Expression *exp2 )
  {
    Expression *res = exp1;
    res->append("[");
    res->append(exp2);
    delete exp2;
    res->append("]");
    return res;
  }

  void ref_prop_or_op( void *info, const std::string &name )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );
    
    node->current_reference.add_unchecked( translator->prop_op(name), 
					   "<unused>" );
  }
  void ref_node_prop( void *info, const std::string &prop)
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->node_prop(prop),
				 translator->reference_concat_string() );
  }
  void ref_start_param( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert( node != 0 );

    node->current_reference.add_unchecked
      ( translator->start_param( node->current_reference.provider_type,
			       node->current_reference.ret_type,
			       node->current_reference.par_type ), 
	"<unused>" );
  }
  void ref_end_param( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert( node != 0 );

    node->current_reference.add_unchecked
      ( translator->end_param( node->current_reference.provider_type,
			       node->current_reference.ret_type,
			       node->current_reference.par_type ), 
	"<unused>" );
  }
  void ref_provider_type( void *info, const std::string &provider_type, 
			  const std::string &ret_type, 
			  const std::string &par_type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert( node != 0 );

    node->current_reference.provider_type = provider_type;
    node->current_reference.ret_type = ret_type;
    node->current_reference.par_type = par_type;
  }

  void ref_node_local_prev( void *info, std::string provider_type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->prev( provider_type ),
				 translator->reference_concat_string() );
  }
  void ref_node_local_next( void *info, std::string provider_type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->next( provider_type ),
				 translator->reference_concat_string() );
  }
  void ref_node_local_child_first( void *info, const std::string &type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->first_child(type),
				 translator->reference_concat_string() );
  }
  void ref_node_local_child_last( void *info, const std::string &type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->last_child(type),
				 translator->reference_concat_string() );
  }
  void ref_node_local_child( void *info, const std::string &type, double n )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->get_child(type, int(n)),
				 translator->reference_concat_string() );
  }
  void ref_node_prev( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->prev(),
				 translator->reference_concat_string() );
  }
  void ref_node_next( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->next(),
				 translator->reference_concat_string() );
  }
  void ref_node_parent( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->parent(),
				 translator->reference_concat_string() );
  }
  void ref_node_first_child( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->first_child(),
				 translator->reference_concat_string() );
  }
  void ref_node_last_child( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->last_child(),
				 translator->reference_concat_string() );
  }
  void ref_node_child( void *info, double n )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Tree_Node_Type *node = I->afd->current_node;
    assert( node != 0 );

    node->current_reference.add( translator->get_child(int(n)),
				 translator->reference_concat_string() );
  }

}
