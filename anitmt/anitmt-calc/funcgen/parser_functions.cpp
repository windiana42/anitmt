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
    std::string::size_type p = file.rfind('.');
    std::string base_name;
    if( p != std::string::npos )
      base_name = file.substr(0,p);
    else
      base_name = file;

    if( !afd->don_t_create_code.top() )
      afd->included_basenames.push_back(base_name);

    afd->don_t_create_code.push(true);

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

  void start_node_declaration( void *infoptr, bool abstract,
			       const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    afd->current_node = &afd->nodes[name]; 
    afd->current_node->pos = info->file_pos.duplicate();
    afd->current_node->set_parent_context(afd);
    afd->push_context(afd->current_node);
    if( abstract )		// dont create abstract nodes
      afd->current_node->don_t_create_code = true;
    else
      afd->current_node->don_t_create_code = afd->don_t_create_code.top();
  }
  void finish_node_declaration( void *infoptr )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;

    afd->pop_context();
  }
  void node_extends( void *infoptr, const std::string &node_name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;

    if( afd->current_node )
    {

      // search node
      std::map<std::string,Tree_Node_Type>::iterator i;
      i = afd->nodes.find( node_name );
      
      if( i == afd->nodes.end() )
	info->msg.error() 
	  << "node \"" << node_name << "\" to extend doesen't exist";
      else
      {
	Tree_Node_Type &node = i->second;
	if( &node == afd->current_node )
	  info->msg.error() 
	    << "recursive inheritance of node \"" << node_name 
	    << "\" is not allowed";
	else
	{
	  afd->current_node->merge( node ); // merge with node to extend
	  afd->current_node->set_parent_context( &node );
	}
      }
    }
  }
  void node_provides( void *infoptr, const std::string &type )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    assert(afd->current_node != 0);
    
    // check if provider type exists
    if( !afd->get_context()->is_provider_type(type) )
      info->msg.error() 
	<< "provided type \"" << type << "\" doesn't exist";
    else
    {
      Provider_Type &provider_type = afd->provider_types[type];
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
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
      context->current_property_type = type;

    // ?!? allow only known property types ?!?
    if( ( type != "flag" ) && ( type != "scalar" ) && ( type != "vector" ) &&
	( type != "matrix" ) && ( type != "string" ) )
    {
      msg.error() << "property type \"" << type << "\" isn't allowed";
      context->current_property_type = "<invalid_type>";
    }
  }
  void node_declare_property( void *info, const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_property(name) )
      {
	Property prop(name,context->current_property_type);
	context->property_list.push_back( prop );
	context->properties[name] = &context->property_list.back();
      }
      else
      {
	msg.error() << "property \"" << name << "\" already defined";
      }
    }
  }
  void node_declare_property( void *info, const std::string &type,
			      const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_property(name) )
      {
	context->property_list.push_back( Property(name,type) );
	context->properties[name] = &context->property_list.back();
      }
      else
      {
	msg.error() << "property \"" << name << "\" already defined";
      }
    }
  }

  void node_declare_alias( void *info, const std::string &alias, 
			   const std::string &property )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    assert(node != 0);
    node->aliases.push_back(Alias(alias,property));
  }

  void start_operand_type( void *info, const std::string &type )
  {
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
      context->current_operand_type = type;
  }
  void declare_operand( void *info, const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_operand(name) )
      {
	Operand op(name,context->current_operand_type);
	context->operand_list.push_back( op );
	context->operands[name] = &context->operand_list.back();
      }
      else
      {
	msg.error() << "operand \"" << name << "\" already defined";
      }
    }
  } 
  void declare_operand( void *info, const std::string &type, 
			const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_operand(name) )
      {
	context->operand_list.push_back( Operand(name,type) );
	context->operands[name] = &context->operand_list.back();
      }
      else
      {
	msg.error() << "operand \"" << name << "\" already defined";
      }
    }
  } 
  void declare_variable( void *info, const std::string &type, 
			 const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_variable(name) )
      {
	context->variable_list.push_back( Variable(name,type) );
	context->variables[name] = &context->variable_list.back();
      }
      else
      {
	msg.error() << "variable \"" << name << "\" already defined";
      }
    }
  }
  void declare_special_variable( void *info, const std::string &type, 
				 const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_special_variable(name) )
      {
	context->special_variable_list.push_back
	  ( Variable(name,type) );
	context->special_variables[name] = 
	  &context->special_variable_list.back();
      }
      else
      {
	msg.error() << "variable \"" << name << "\" already defined";
      }
    }
  }
  void declare_container( void *info, const std::string &type, 
			  const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( context->is_provider_type(type) )
      {
	context->container_list.push_back
	  ( Container(context->get_provider_type(type)->serial,type,name) );
	context->containers[name] = &context->container_list.back();
      }
      else
      {
	msg.error() << "provider type expected instead of \""<< type << "\"";
      }
    }    
  }

  void declare_solver_name( void *info, const std::string &type, 
			    const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Context *context = static_cast<afd_info*>(info)->afd->get_context();
    if( context )
    {
      if( !context->is_solver_name(name) )
      {
	context->solver_name_list.push_back( Solver_Name(name,type) );
	context->solver_names[name] = &context->solver_name_list.back();
      }
      else
      {
	msg.error() << "solver name \"" << name << "\" already defined";
      }
    }
  } 

  void node_start_common_declaration( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    afd->current_solve_code = &afd->current_node->common;
  }
  void node_start_first_declaration( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Tree_Node_Type *node = afd->current_node;    
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
	afd->current_solve_code = 0;
      }
      else
	// enter declaration in map and set the current pointer
	afd->current_solve_code = &node->first[type];
    }
    else
      // enter declaration in map and set the current pointer
      afd->current_solve_code = &node->first[type];
  } 
  void node_start_last_declaration( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
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
	afd->current_solve_code = 0;
      }
      else
	// enter declaration in map and set the current pointer
	afd->current_solve_code = &node->last[type];
    }
    else
      // enter declaration in map and set the current pointer
      afd->current_solve_code = &node->last[type];
    
    // enter declaration in map and set the current pointer
    afd->current_solve_code = &node->last[type]; 
  } 

  void node_solve_constraint( void *info, const Expression *exp  )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->constraints.new_constraint(exp);
    }
    delete exp;
  }

  void node_start_solver( void *info, const std::string &solver, 
			  const std::string &name )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.new_solver(solver);

      if( name != "" )
      {
	declare_solver_name( info, solver, name );
	solve_code->solvers.set_solver_identifier(name);
      }
    }
  }

  void node_add_solver_parameter( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_parameter_ref(afd->current_reference);
    }
  } // from reference
  void node_add_solver_const_parameter( void *info, bool b )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_const_parameter
	( afd->translator->operand_from_bool(b) );
    }
  }
  void node_add_solver_const_parameter( void *info, double s )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_const_parameter
	( afd->translator->operand_from_scalar(s) );
    }
  }
  void node_add_solver_const_parameter( void *info, const std::string &s )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_const_parameter
	( afd->translator->operand_from_string(s) );
    }
  }
  void node_add_solver_const_parameter( void *info, 
					const std::string &function_name,
					const std::string &parameters )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_const_parameter
	( afd->translator->operand_from_function( function_name, 
						  parameters ) );
    }
  }
  void solver_code_add_container_parameter( void *info, 
					    const std::string &container )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_const_parameter
	( afd->translator->container_name( container ) );
    }
  } // from reference
  void node_finish_solver( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Solve_System_Code *solve_code = afd->current_solve_code;    

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
    Solve_System_Code *solve_code = I->afd->current_solve_code;    

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
    Solve_System_Code *solve_code = afd->current_solve_code;    

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
    Solve_System_Code *solve_code = I->afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_ref(I->afd->current_reference,
					    translator);
    }
  }
  void node_add_action_parameter_str( void *info, std::string str )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Solve_System_Code *solve_code = I->afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_str(str,translator);
    }
  }
  void node_add_action_parameter_exp( void *info, Expression *exp )
  {
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;
    Solve_System_Code *solve_code = I->afd->current_solve_code;    

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
    Solve_System_Code *solve_code = I->afd->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.finish_action(translator);
    }
  }

  void node_contains( void *info, bool max1, bool min1, 
		      const std::string &type )
  {
    declare_container( info, type, type /*name=type*/ );

    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Tree_Node_Type *node = afd->current_node;
    if( node )
    {
      if( !afd->get_context()->is_provider_type(type) )
      {
	msg.error() << "provider type " << type << " doesn't exist";
      }
      else
      {
	Provider_Type &provider_type = afd->provider_types[type];
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
	afd->return_type = ret_type;
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
  void node_result_essential_solver_function( void *info, 
					      const std::string &solver,
					      const std::string &function )
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
	//!!! check whether solver function exists exists !!!

	// insert required child result function
	res_code->required_solver_functions.push_back
	  (std::pair<std::string,std::string>(solver,function));
      }
    }
  }

  void start_operator_declaration( void *infoptr, const std::string &type,
				   const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;
    message::Message_Reporter &msg = info->msg;

    if( afd->get_context()->is_operator(name) )
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
      afd->current_operator_declaration = &afd->operators[name];
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
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Root *afd = info->afd;

    afd->current_code = 0;
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

  void start_event_solver_parameters( void *info, const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;

    if( !afd->get_context()->is_solver(name) )
    {
      // create new solver
      afd->current_event_solver = &afd->solvers[name];
      afd->current_event_solver->don_t_create_code 
	= afd->don_t_create_code.top();
      // relink kontext of parameter operands to afd root
      afd->current_event_solver->parameters.set_parent_context(afd);
      // choose parameter operands as current context
      afd->push_context( &afd->current_event_solver->parameters );
    }
    else
    {
      msg.error() << "solver \"" << name << "\" already defined";
      afd->current_event_solver = 0;
      afd->push_context( 0 );
    }
  }
  void event_solver_parameter_operand( void *info, const std::string &type, 
				       const std::string &name )
  {
    declare_operand( info, type, name ); // declare operand in context

    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Event_Solver *solver= I->afd->current_event_solver;
    
    if( solver )
    {
      solver->parameters.parameters.push_back( Operand( name, type ) );
    }
  }
  void event_solver_parameter_container( void *info,
					 const std::string &type, 
					 const std::string &name )
  {
    // declare container in context
    declare_container( info, type, name ); 

    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Event_Solver *solver= I->afd->current_event_solver;
    Context *context = I->afd->get_context();
    
    if( solver )
    {
      if( context->is_container(name) )
      // use container just declared in context because serial is detected 
      // there
	solver->
	  parameters.parameters.push_back( *context->get_container(name) );
    }
  }
  void start_event_solver_declaration( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;

    if( afd->current_event_solver )
    {
      afd->pop_context();
      // relink kontext of solver with it's parameter operands
      afd->current_event_solver->set_parent_context
	( &afd->current_event_solver->parameters );
      // choose solver as current context
      afd->push_context( afd->current_event_solver );
    }
  }
  void finish_event_solver_declaration( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    AFD_Root *afd = I->afd;
    if( afd->current_event_solver )
    {
      afd->current_event_solver = 0;
    }
    afd->pop_context();		// context was changed in any case
  }

  //...
  void start_event_solver_init_solvers_block( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    if( afd->current_event_solver )
      afd->current_solve_code = new Solve_System_Code;
    else
      afd->current_solve_code = 0;
  }
  void finish_event_solver_init_solvers_block( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    if( afd->current_solve_code && afd->current_event_solver )
    {
      // copy solver code
      afd->current_event_solver->init_solver_code.merge
	( afd->current_solve_code->solvers );
    }
    if( afd->current_solve_code )
    {
      delete afd->current_solve_code;
      afd->current_solve_code = 0;
    }
  }
  void start_event_solver_init_constraints_block( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    if( afd->current_event_solver )
      afd->current_solve_code = new Solve_System_Code;
    else
      afd->current_solve_code = 0;
  }
  void finish_event_solver_init_constraints_block( void *info )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    if( afd->current_solve_code && afd->current_event_solver )
    {
      // copy solver code
      afd->current_event_solver->init_constraint_code.merge
	( afd->current_solve_code->constraints );
    }
    if( afd->current_solve_code )
    {
      delete afd->current_solve_code;
      afd->current_solve_code = 0;
    }
  }
  void start_event_solver_function( void *info, const std::string &ret_type,
				      const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      solver->functions.push_back( Solver_Function_Code( name, ret_type ) );
      solver->current_function_code 
	= &solver->functions.back();
      solver->current_function_code->set_parent_context( solver );
      afd->push_context( solver->current_function_code );
      afd->current_code = solver->current_function_code;
      afd->return_type = ret_type;
    }
  }
  void finish_event_solver_function( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      afd->pop_context();
    }
    afd->current_code = 0;
  }
  void start_event_solver_event_group( void *info, const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      solver->event_groups.push_back( Event_Group(name) );
      solver->current_event_group = &solver->event_groups.back();
    }
  }
  void finish_event_solver_event_group(void *info)
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      solver->current_event_group = 0;
    }
  }
  void start_event_solver_event(void *info, const std::string &name)
  {  
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( !group )		// if there is no group
      {
	// create a group
	solver->event_groups.push_back( Event_Group("") );
	group = &solver->event_groups.back();
	solver->current_event_group = group;
      }

      group->events.push_back( Event(name) );
      group->current_event = &group->events.back();
    }
  }
  void event_condition( void *info, const std::string &operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  event->required_operands.push_back( operand );
	}
      }
    }
  }
  void event_condition_container( void *info, 
				  const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  event->required_containers.push_back( name );
	}
      }
    }
  }
  void event_condition_container_function( void *info, 
					   const std::string &name,
					   const std::string &return_type,
					   const std::string &parameter_type )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  event->required_container_functions.push_back
	    ( Container_Function( name, return_type, parameter_type ) );
	}
      }
    }
  }
  void event_condition_event( void *info, const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  event->required_events.push_back( name );
	}
      }
    }
  }
  void event_condition_event_group( void *info, const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  event->required_event_groups.push_back( name );
	}
      }
    }
  }
  void event_condition_solver( void *info, const std::string &solver_name, 
			       const std::string &function )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  event->required_solver_functions.push_back
	    ( Solver_Function(solver_name, function) );
	}
      }
    }
  }


  void start_event_solver_init_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      afd->current_code = &solver->init_code;
    }
  }
  void finish_event_solver_init_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    
    afd->current_code = 0;
  }
  void start_event_group_reset_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	afd->current_code = &group->reset_code;
      }
    }
  }
  void finish_event_group_reset_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    
    afd->current_code = 0;
  }
  void start_event_final_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  afd->current_code = &event->final_code;
	}
      }
    }
  }
  void finish_event_final_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    
    afd->current_code = 0;
  }
  void start_event_reset_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  afd->current_code = &event->reset_code;
	}
      }
    }
  }
  void finish_event_reset_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    
    afd->current_code = 0;
  }
  void start_event_test_run_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;
    
    if( solver )
    {
      Event_Group *group = solver->current_event_group;
      if( group )		// if there is no group
      {
	Event *event = group->current_event;
	if( event )
	{
	  afd->current_code = &event->test_run_code;
	}
      }
    }
  }
  void finish_event_test_run_code( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    
    afd->current_code = 0;
  }

  void event_solver_require_operand( void *info, const std::string &op )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      if( solver->current_function_code )
      {
        solver->current_function_code->required_operands.push_back(op);
      }
    }
  }
  void event_solver_require_function( void *info, const std::string &func )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      if( solver->current_function_code )
      {
        solver->current_function_code->required_functions.push_back(func);
      }
    }
  }
  void event_solver_require_solver_func( void *info, 
					   const std::string &solver_id,
					   const std::string &function )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      solver->current_function_code->
	required_solver_functions.push_back
	( Solver_Function(solver_id,function) );
    }
  }
  void event_solver_require_event( void *info, const std::string &event )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      if( solver->current_function_code )
      {
	solver->current_function_code->
	  required_events.push_back( event );
      }
    }
  }
  void event_solver_require_event_group( void *info, const std::string &group )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      if( solver->current_function_code )
      {
        solver->current_function_code->
	  required_event_groups.push_back( group );
      }
    }
  }    
  void event_solver_require_container( void *info, const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      if( solver->current_function_code )
      {
        solver->current_function_code->
	  required_containers.push_back( name );
      }
    }
  }
  void event_solver_require_container_function( void *info, 
						const std::string &name, 
						const std::string &return_type,
						const std::string 
						&parameter_type )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Event_Solver *solver = afd->current_event_solver;

    if( solver )
    {
      if( solver->current_function_code )
      {
	Context *context = afd->get_context();
	assert( context );
	if( context->is_container( name ) )
	{
	  solver->current_function_code->
	    required_container_functions.push_back
	    ( Container_Function( name,
				  context->get_container(name)->provider_type,
				  return_type, parameter_type ) );
	}
	else
	{
	  msg.error() << "`" << name << "' is no container name";
	}
      }
    }
  }

  void event_solver_code_set( void *info, std::string operand, 
			      std::string expression)
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->event_code_set( operand, expression );
    }
  }
  void event_solver_code_try( void *info, std::string operand, 
			      std::string expression)
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->event_code_try( operand, expression );
    }
  }
  void event_solver_code_try_reject( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->event_code_try_reject( afd->string_list );
    }
  }
  void event_solver_code_is_solved_in_try( void *info, std::string operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->event_code_is_solved_in_try( operand );
    }
  }
  void event_solver_code_is_just_solved( void *info, std::string operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->event_code_is_just_solved( operand );
    }
  }
  void user_code_prop_op( void *info, std::string operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->prop_op_value( operand );
    }
  }
  void user_code_prop_op_try( void *info, std::string operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->prop_op_value_try( operand );
    }
  }
  void user_code_solver_function( void *info, std::string solver, 
				  std::string function, 
				  std::string parameter, 
				  std::string opt_fail_bool_var )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;
    
    if( code )
    {
      code->code += translator->solver_function_value
	( solver, function, parameter, opt_fail_bool_var);
    }
  }
  void user_code_store_container_name( void *info, std::string container )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    //Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;

    Context *context = afd->get_context();
    assert( context );
    if( context->is_container( container ) )
    {
      afd->store = container;
    }
    else
    {
	msg.error() << "`" << container << "' is no container name";
    }
  }
  void user_code_container_function( void *info, 
				     std::string return_type, 
				     std::string parameter_type, 
				     std::string parameter, 
				     std::string opt_fail_bool_var )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      Context *context = afd->get_context();
      assert( context );

      code->code += translator->container_function_value
	( afd->store, context->get_container(afd->store)->provider_type,
	  return_type, parameter_type, parameter, 
	  opt_fail_bool_var );
    }
  }
  void user_code_container_first_index( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->container_first_index( afd->store );
    }
  }
  void user_code_container_last_index( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->container_last_index( afd->store );
    }
  }
  void user_code_container_element_function( void *info, std::string container,
					     double index, 
					     std::string return_type, 
					     std::string parameter_type, 
					     std::string parameter, 
					     std::string opt_fail_bool_var )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      Context *context = afd->get_context();
      assert( context );
      if( context->is_container( container ) )
      {
	code->code += translator->container_element_function_value
	  ( container, int(index), 
	    context->get_container(container)->provider_type,
	    return_type, parameter_type, parameter, 
	    opt_fail_bool_var );
      }
      else
      {
	msg.error() << "`" << container << "' is no container name";
      }
    }
  }
  void user_code_for_each_container_element( void *info, std::string element, 
					     std::string container )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      Context *context = afd->get_context();
      assert( context );
      if( context->is_container( container ) )
      {
	code->code += translator->container_for_each_element
	  ( element, container, 
	    context->get_container(container)->provider_type);

	declare_special_variable
	  ( info, context->get_container(container)->provider_type, element );
      }
      else
      {
	msg.error() << "`" << container << "' is no container name";
      }
    }
  }
  void user_code_element_function( void *info, std::string element, 
				   std::string return_type, 
				   std::string parameter_type, 
				   std::string parameter, 
				   std::string opt_fail_bool_var )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      Context *context = afd->get_context();
      assert( context );
      if( context->is_special_variable( element ) )
      {
	code->code += translator->element_function_value
	  ( element, context->get_special_variable(element)->type,
	    return_type, parameter_type, parameter, 
	    opt_fail_bool_var );
      }
      else
      {
	msg.error() << "`" << element
		    << "' is no element identifier in a for_each loop";
      }
    }
  }

  void user_code_return_prop( void *info, std::string operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->start_return_value( afd->return_type );
      code->code += translator->prop_op_value( operand );
      code->code += translator->finish_return_value( afd->return_type );
    }
  }
  void user_code_return_prop_try( void *info, std::string operand )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->start_return_value( afd->return_type );
      code->code += translator->prop_op_value_try( operand );
      code->code += translator->finish_return_value( afd->return_type );
    }
  }
  void user_code_return( void *info, std::string expression )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->start_return_value( afd->return_type );
      code->code += expression;
      code->code += translator->finish_return_value( afd->return_type );
    }
  }
  void user_code_return_solver_function( void *info, 
						 std::string solver, 
						 std::string function, 
						 std::string parameter )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->start_return_result( afd->return_type );
      code->code += translator->solver_function_result( solver, function, 
							parameter );
      code->code += translator->finish_return_result( afd->return_type );
    }
  }
  void user_code_return_fail( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->return_fail();
    }
  }
  void user_code_return_if_fail( void *info )
  {
    afd_info *I=static_cast<afd_info*>(info);
    //message::Message_Reporter &msg = I->msg;
    Code_Translator *translator = I->afd->translator;
    AFD_Root *afd = I->afd;
    Code *code = afd->current_code;

    if( code )
    {
      code->code += translator->return_if_fail();
    }
  }

  void function_parameter( void *info, const std::string &type, 
			       const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_variable(name) )
      {
	// insert variable in list and map
	context->variable_list.push_back( Variable( name, type ) );
	context->variables[name] = &context->variable_list.back();
      }
      else
      {
	msg.error() << "variable \"" << name << "\" already defined";
      }
    }
  }
  void start_variable_declaration( void *info, const std::string &type )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Context *context = afd->get_context();
    if( context )
    {
      context->current_variable_type = type;
    }
  }
  void variable_declaration_name( void *info, const std::string &name )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_variable(name) )
      {
	Variable var( name, context->current_variable_type );
	// insert variable in list and map at once
	context->variable_list.push_back( var );
	context->variables[name] = &context->variable_list.back();
      }
      else
      {
	msg.error() << "variable \"" << name << "\" already defined";
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

	res_code->code += translator->start_return_result( result_type );
	res_code->code += 
	  translator->child_result( provider, result_type, 
				    parameter_type, parameter );
	res_code->code += translator->finish_return_result( result_type );
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

	res_code->code += translator->start_return_result( result_type );
	res_code->code += 
	  translator->provided_result( provider, result_type, 
				       parameter_type, parameter );
	res_code->code += translator->finish_return_result( result_type );
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
	res_code->code += translator->start_return_result
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
	res_code->code += translator->finish_return_result
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
	res_code->code += translator->start_return_value
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
	res_code->code += translator->finish_return_value
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
	res_code->code 
	  += translator->start_return_value(res_code->return_type);
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
	res_code->code 
	  += translator->finish_return_value(res_code->return_type);
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
	res_code->code += translator->return_if_fail();
      }
    }
  }

  void require_identifier( void *info, std::string id, std::string expect )
  {
    if( id != expect )
    {
      afd_info *I=static_cast<afd_info*>(info);
      message::Message_Reporter &msg = I->msg;

      msg.error() << "`" << expect << "' expected instead of `" << id << "'";
    }
  }
  void require_operand( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_operand(id) )
      {
	msg.error() << "operand expected insted of \"" << id << "\"";
      }
    }
  }
  void require_property( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_property(id) )
      {
	msg.error() << "property expected insted of \"" << id << "\"";
      }
    }
  }
  void require_prop_op( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !(context->is_operand(id) || context->is_property(id)) )
      {
	msg.error() << "property or operand expected insted of \"" << id 
		    << "\"";
      }
    }
  }
  void require_provider_type( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_provider_type(id) )
      {
	msg.error() << "provider_type expected insted of \"" << id << "\"";
      }
    }
  }
  void require_container_name( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_container(id) )
      {
	msg.error() << "container name expected insted of \"" << id << "\"";
      }
    }
  }
  void require_variable( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_variable(id) )
      {
	msg.error() << "variable expected insted of \"" << id << "\"";
      }
    }
  }
  void require_special_variable( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_special_variable(id) )
      {
	msg.error() << "special variable expected insted of \"" << id << "\"";
      }
    }
  }
  void require_base_type( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_base_type(id) )
      {
	msg.error() << "base type expected insted of \"" << id << "\"";
      }
    }
  }
  void require_operator( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_operator(id) )
      {
	msg.error() << "operator expected insted of \"" << id << "\"";
      }
    }
  }
  void require_solver( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_solver(id) )
      {
	msg.error() << "solver expected insted of \"" << id << "\"";
      }
    }
  }

  void require_solver_name( void *info, std::string id )
  {
    afd_info *I=static_cast<afd_info*>(info);
    message::Message_Reporter &msg = I->msg;
    AFD_Root *afd = I->afd;
    Context *context = afd->get_context();
    if( context )
    {
      if( !context->is_solver_name(id) )
      {
	msg.error() << "solver name expected insted of \"" << id << "\"";
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
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    Expression *res = new Expression( afd->current_reference );
    afd->current_reference.clear();
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
    
    I->afd->current_reference.add_unchecked( translator->prop_op(name), 
					     "<unused>" );
  }
  void ref_node_prop( void *info, const std::string &prop)
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->node_prop(prop),
				   translator->reference_concat_string() );
  }
  void ref_start_param( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add_unchecked
      ( translator->start_param( I->afd->current_reference.provider_type,
				 I->afd->current_reference.ret_type,
				 I->afd->current_reference.par_type ), 
	"<unused>" );
  }
  void ref_end_param( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add_unchecked
      ( translator->end_param( I->afd->current_reference.provider_type,
			       I->afd->current_reference.ret_type,
			       I->afd->current_reference.par_type ), 
	"<unused>" );
  }
  void ref_provider_type( void *info, const std::string &provider_type, 
			  const std::string &ret_type, 
			  const std::string &par_type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;

    afd->current_reference.provider_type = provider_type;
    afd->current_reference.ret_type = ret_type;
    afd->current_reference.par_type = par_type;
  }

  void ref_node_local_prev( void *info, std::string provider_type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->prev( provider_type ),
				   translator->reference_concat_string() );
  }
  void ref_node_local_next( void *info, std::string provider_type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->next( provider_type ),
				   translator->reference_concat_string() );
  }
  void ref_node_local_child_first( void *info, const std::string &type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->first_child(type),
				 translator->reference_concat_string() );
  }
  void ref_node_local_child_last( void *info, const std::string &type )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->last_child(type),
				 translator->reference_concat_string() );
  }
  void ref_node_local_child( void *info, const std::string &type, double n )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->get_child(type, int(n)),
				 translator->reference_concat_string() );
  }
  void ref_node_prev( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->prev(),
				 translator->reference_concat_string() );
  }
  void ref_node_next( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->next(),
				 translator->reference_concat_string() );
  }
  void ref_node_parent( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->parent(),
				 translator->reference_concat_string() );
  }
  void ref_node_first_child( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->first_child(),
				 translator->reference_concat_string() );
  }
  void ref_node_last_child( void *info )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->last_child(),
				 translator->reference_concat_string() );
  }
  void ref_node_child( void *info, double n )
  {
    //message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    afd_info *I=static_cast<afd_info*>(info);
    Code_Translator *translator = I->afd->translator;

    I->afd->current_reference.add( translator->get_child(int(n)),
				 translator->reference_concat_string() );
  }
  void common_add_identifier( void *info, std::string id )
  {
    AFD_Root *afd = static_cast<afd_info*>(info)->afd;
    afd->string_list.push_back( id );
  }
}
