/*****************************************************************************/
/**   This file offers functions and macros for the parser                  **/
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

#include "parser_functions.hpp"

#include "stdextend.hpp"

namespace funcgen
{
  // ******************************
  // functions used by the parser and lexer
  // ******************************

  void start_code_block( void *info )
  {
    static_cast<afd_info*>(info)->lexer->goto_code_copy_mode();
  }
  void finish_code_block( void *info )
  {
  }
  void continue_code_mode( void *info )
  {
    static_cast<afd_info*>(info)->lexer->goto_code_copy_mode();
  }

  // for lexer
  void copy_code_line( char *line, int len ) {}
  void code_block_escape( afd_info *info ) {}

  //******************************************
  // concrete help functions for parser rules

  void declare_base_type( void *info, const std::string &name, 
			  const std::string &type )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
    afd->base_types[name] = Base_Type(type);
  }

  void declare_base_type_structure( void *info, const std::string &name )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
    afd->current_base_type = &afd->base_types[name];
  }
  
  void base_type_structure_element( void *info, const std::string &type, 
				    const std::string &name )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
    afd->current_base_type->add_element(type,name);
  }

  void start_provider_type_declaration( void *info, bool serial,
					const std::string &name )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
    afd->current_type = &afd->types[name];
    afd->current_type->serial = serial;
  }

  void add_provided_result_type( void *info, const std::string &ret, 
				 const std::string &par )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
    std::map<std::string,Result_Type>::iterator i;
    i = afd->current_type->result_types.find( ret );
    if( i != afd->current_type->result_types.end() )
    {
      msg.error() << "result type " << ret << " is already provided";
    }
    else
      afd->current_type->result_types.insert
	( std::pair<std::string,Result_Type>(ret,Result_Type(ret,par)) );
  }

  void start_node_declaration( void *infoptr, const std::string &name )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Manager *afd = info->afd;
    afd->current_node = &afd->nodes[name];
    afd->current_node->pos = info->file_pos.duplicate();
  }
  void node_extends( void *infoptr, const std::string &node )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Manager *afd = info->afd;

    // search node
    std::map<std::string,Tree_Node_Type>::iterator i;
    i = afd->nodes.find( node );
    
    if( i == afd->nodes.end() )
      info->msg.error() << "Node " << node << " to extend doesen't exist";
    else
    {
      afd->current_node->merge( i->second ); // merge with node to extend
    }
  }
  void node_provides( void *infoptr, const std::string &type )
  {
    afd_info *info = static_cast<afd_info*>(infoptr);
    AFD_Manager *afd = info->afd;
    
    // check if provider type exists
    std::map<std::string,Provider_Type>::iterator i;
    i = afd->types.find(type);
    if( i == afd->types.end() )
      info->msg.error() << "Provided type " << type << " doesn't exist";
    else
    {
      // insert provided type
      afd->current_node->provided_results[type];
    }
  }

  void node_start_property_type( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    if( type == "flag" )
      node->current_properties = &node->flag_properties;
    else if( type == "scalar" )
      node->current_properties = &node->scalar_properties;
    else if( type == "vector" )
      node->current_properties = &node->vector_properties;
    else if( type == "matrix" )
      node->current_properties = &node->matrix_properties;
    else if( type == "string" )
      node->current_properties = &node->string_properties;
    else
    {
      msg.error() << "Operand type " << type << " doesn't exist";
      node->current_properties = 0;
    }
  }
  void node_declare_property( void *info, const std::string &name )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    if( node->current_properties != 0 )
      node->current_properties->insert(name);
  }

  void node_declare_alias( void *info, const std::string &alias, 
			   const std::string &property )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    node->aliases.push_back(Alias(alias,property));
  }

  void node_start_operand_type( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    if( type == "flag" )
      node->current_operands = &node->flag_operands;
    else if( type == "scalar" )
      node->current_operands = &node->scalar_operands;
    else if( type == "vector" )
      node->current_operands = &node->vector_operands;
    else if( type == "matrix" )
      node->current_operands = &node->matrix_operands;
    else if( type == "string" )
      node->current_operands = &node->string_operands;
    else
    {
      msg.error() << "Operand type " << type << " doesn't exist";
      node->current_operands = 0;
    }
  }
  void node_declare_operand( void *info, const std::string &name )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    if( node->current_operands != 0 )
      node->current_operands->insert(name);
  } 

  void node_start_common_declaration( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    node->current_solve_code = &node->common;
  }
  void node_start_first_declaration( void *info, const std::string &type="" )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;    
    if( type != "" )
    {
      std::map<std::string,Solve_System_Code>::iterator i;
      i = node->first.find(type);
      if( i == node->first.end() )
      {
	msg.error() << "Type " << type << " is not provided by this node type";
	node->current_solve_code = 0;
      }
      else
      {
	node->current_solve_code = &i->second; // set solve code pointer
      }
    }
    else
    {
      node->current_solve_code = &node->first[""]; // use default first
    }    
  } 
  void node_start_last_declaration( void *info, const std::string &type="" )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;    
    if( type != "" )
    {
      std::map<std::string,Solve_System_Code>::iterator i;
      i = node->last.find(type);
      if( i == node->last.end() )
      {
	msg.error() << "Type " << type << " is not provided by this node type";
	node->current_solve_code = 0;
      }
      else
      {
	node->current_solve_code = &i->second; // set solve code pointer
      }
    }
    else
    {
      node->current_solve_code = &node->last[""]; // use default last
    }    
  } 

  void node_solve_constraint( void *info, const Expression *exp  )
  {
    Solve_System_Code *solve_code = 
      static_cast<afd_info*>(info)->afd->current_node->current_solve_code;    

    if( solve_code )
    {
      solve_code->constraints.new_constraint(exp);
    }
    delete exp;
  }

  void node_start_solver( void *info, const std::string &solver )
  {
    Solve_System_Code *solve_code = 
      static_cast<afd_info*>(info)->afd->current_node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.new_solver(solver);
    }
  }
  void node_add_solver_parameter( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.add_parameter_ref(node->current_reference);
    }
  } // from reference
  void node_finish_solver( void *info )
  {
    Solve_System_Code *solve_code = 
      static_cast<afd_info*>(info)->afd->current_node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.finish_solver();
    }
  } 
  void node_solve_expression( void *info, const std::string &property,
			      Expression *exp )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->solvers.new_expression_solver(exp);
    }
    delete exp;
  }

  void node_start_action( void *info, const std::string &name, 
			  double priority )
  {
    Solve_System_Code *solve_code = 
      static_cast<afd_info*>(info)->afd->current_node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.new_action(name,priority);
    }
  }
  void node_add_action_parameter_ref( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_ref(node->current_reference);
    }
  }
  void node_add_action_parameter_exp( void *info, Expression *exp )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    Solve_System_Code *solve_code = node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.add_parameter_exp(exp);
    }
    delete exp;
  }
  void node_finish_action( void *info )
  {
    Solve_System_Code *solve_code = 
      static_cast<afd_info*>(info)->afd->current_node->current_solve_code;    

    if( solve_code )
    {
      solve_code->actions.finish_action();
    }
  }

  void node_contains( void *info, bool max1, bool min1, 
		      const std::string &type )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    node->child_containers.insert( Child_Container( max1,min1,type ) );
  }

  void node_start_provide( void *info, const std::string &type )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
  }

  void node_start_result_code( void *info, const std::string &ret_type, 
			       const std::string &par_type, 
			       const std::string &par_name )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
  }

  void node_result_essential_prop( void *info, const std::string &property )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
  }
  void node_result_essential_child_result( void *info, 
					   const std::string &provider, 
					   const std::string &ret="", 
					   const std::string &par="" )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
  }
  void node_result_essential_result( void *info, const std::string &provider, 
				     const std::string &ret="", 
				     const std::string &par="" )
  {
    AFD_Manager *afd = static_cast<afd_info*>(info)->afd;
  }

  void res_ref_property( void *info, std::string prop )
  {
    
  }
  void res_ref_child( void *info, std::string provider, 
		      std::string result_type, std::string parameter )
  {

  }
  void res_ref_this( void *info, std::string provider, 
		      std::string result_type, std::string parameter )
  {

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
  Expression *expr_from_ref( void *info )
  {
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    Expression *res = new Expression( node->current_reference );
    node->current_reference.clear();
    return res;
  }
  Expression *expr_scalar( double val )
  {
    Expression *res = new Expression();
    std::string str; str += val;
    res->append( str );
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
  Expression *expr( Expression *exp )
  {
    Expression *res = new Expression;
    res->append("(");
    res->append(exp);		// could be optimized with prefix!!!
    delete exp;
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

  void ref_prop_or_op( void *info, const std::string &name )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;
    
    node->current_reference.add_unchecked( name );
  }
  void ref_node_prop( void *info, const std::string &prop)
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add_unchecked( prop );
  }
  void ref_start_param( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add_unchecked( "start_param" );
  }
  void ref_end_param( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add_unchecked( "end_param" );
  }
  void ref_provider_type( void *info, const std::string &provider_type, 
			  const std::string &ret_type, 
			  const std::string &par_type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    //!!! an C-Code anpassen... !!!
    node->current_reference.add_unchecked
      ( provider_type + '.' + ret_type + '(' + par_type + ')' );
  }

  void ref_node_local_prev( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "prev" );
  }
  void ref_node_local_next( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "next" );
  }
  void ref_node_local_child_first( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "first_child" );
  }
  void ref_node_local_child_last( void *info, const std::string &type )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "last_child" );
  }
  void ref_node_local_child( void *info, const std::string &type, double n )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( std::string("get_child(") + n + ")" );
  }
  void ref_node_prev( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "prev" );
  }
  void ref_node_next( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "next" );
  }
  void ref_node_parent( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "parent" );
  }
  void ref_node_first_child( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "first_child" );
  }
  void ref_node_last_child( void *info )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( "last_child" );
  }
  void ref_node_child( void *info, double n )
  {
    message::Message_Reporter &msg = static_cast<afd_info*>(info)->msg;
    Tree_Node_Type *node = static_cast<afd_info*>(info)->afd->current_node;

    node->current_reference.add( std::string("get_child(") + n + ")" );
  }

}
