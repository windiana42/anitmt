/*****************************************************************************/
/**   Basic classes to store all information of afd files       	    **/
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

#include "afdbase.hpp"

#include <iostream>

#include "stdextend.hpp"

namespace funcgen
{
  void Header_Files::add_header( std::string header )
  {
    headers.push_back( header );
  }
  std::list<std::string>::const_iterator Header_Files::begin()
  {
    return headers.begin();
  }
  std::list<std::string>::const_iterator Header_Files::end()
  {
    return headers.end();
  }

  void Priority_List::add_priority_label( std::string label )
  {
    priority_labels.push_back( label );
    priority_label_set.insert( label );
  }
  bool Priority_List::is_priority_label( std::string label )
  {
    return priority_label_set.find(label) != priority_label_set.end();
  }
  std::list<std::string>::const_iterator Priority_List::begin()
  {
    return priority_labels.begin();
  }
  std::list<std::string>::const_iterator Priority_List::end()
  {
    return priority_labels.end();
  }

  //! add element type to structure
  void Base_Type::add_element( std::string type, std::string name )
  {
    element_types[name] = type;
  }

  bool Base_Type::is_structure() const
  {
    return structure;
  }
  
  //! returns the type name
  std::string Base_Type::get_type() const
  {
    if( structure )
    {
      std::string type = "struct {";
      std::map<std::string,std::string>::const_iterator i;
      for( i = element_types.begin(); i != element_types.end(); i++ )
	type += i->first + " " + i->second + ";";
      type += "}";
      return type;
    }
    else
      return type_name;
  }

  Base_Type::element_types_type::const_iterator Base_Type::element_begin()
    const
  {
    return element_types.begin();
  }
  Base_Type::element_types_type::const_iterator Base_Type::element_end() 
    const
  {
    return element_types.end();
  }

  //! new type is structure
  Base_Type::Base_Type() : structure(true)
  {
  }

  //! new type is same as old type
  Base_Type::Base_Type( std::string old_type ) 
    : structure(false), type_name(old_type)
  {    
  }

  //! print, just for debug purposes
  void Base_Type::print() const
  {
    if( structure )
    {
      std::cout << "{";
      std::map<std::string,std::string>::const_iterator i;
      bool first = true;
      for( i = element_types.begin(); i != element_types.end(); i++ )
      {
	std::cout << (first?first=false,"":", ") 
		  << i->first << " " << i->second;
      }
      std::cout << '}';
    }
    else
    {
      std::cout << type_name;
    }
  }

  // special functions for identification
  bool Provider_Type::operator==( Provider_Type& type )
  {
    return get_id() == type.get_id();
  }

  //! for insertion in set/map containers:
  bool Result_Type::operator<( const Result_Type &rt ) const
  {
    return (return_type < rt.return_type) 
      || ( (return_type == rt.return_type) 
	   && (parameter_type < rt.parameter_type) );
  }

  /*! Result_Type uses pair as subclass to inherit comparison behaviour for 
    use in container classes */
  Result_Type::Result_Type( std::string ret, std::string par )
    : return_type(ret), parameter_type(par)
  {
  }

  //! print, just for debug purposes
  void Provider_Type::print( std::string name ) const
  {
    std::cout << (serial?"serial ":"") << "type " << name << " {" << std::endl;
    std::set<Result_Type>::const_iterator i;
    for( i = result_types.begin(); i != result_types.end(); i++ )
      std::cout << "  provides " << i->return_type << "( "
		<< i->parameter_type << " );" << std::endl;
    std::cout << "}" << std::endl;
  }

  Property::Property( std::string n, std::string t ) : name(n), type(t)
  {
  }
  Operand::Operand( std::string n, std::string t ) : name(n), type(t)
  {
  }
  Variable::Variable( std::string n, std::string t ) : name(n), type(t)
  {
  }
  Solver_Name::Solver_Name( std::string n, std::string t ) : name(n), type(t)
  {
  }

  //**********
  // Context

  bool Context::is_property( std::string name )
  {
    if( properties.find(name) != properties.end() )
      return true;
    if( parent ) return parent->is_property(name);
    return false;
  }
  Property *Context::get_property( std::string name )
  {
    std::map<std::string,Property*>::iterator i = properties.find(name);
    if( i != properties.end() )
      return i->second;

    if( parent ) return parent->get_property(name);
    return 0;
  }

  bool Context::is_operand( std::string name )
  {
    if( operands.find(name) != operands.end() )
      return true;
    if( parent ) return parent->is_operand(name);
    return false;
  }
  Operand *Context::get_operand( std::string name )
  {
    std::map<std::string,Operand*>::iterator i = operands.find(name);
    if( i != operands.end() )
      return i->second;

    if( parent ) return parent->get_operand(name);
    return 0;
  }
  
  bool Context::is_container( std::string name )
  {
    if( containers.find(name) != containers.end() )
      return true;
    if( parent ) return parent->is_container(name);
    return false;
  }
  Container *Context::get_container( std::string name )
  {
    std::map<std::string,Container*>::iterator i = containers.find(name);
    if( i != containers.end() )
      return i->second;

    if( parent ) return parent->get_container(name);
    return 0;
  }

  bool Context::is_variable( std::string name )
  {
    if( variables.find(name) != variables.end() )
      return true;
    if( parent ) return parent->is_variable(name);
    return false;
  }
  Variable *Context::get_variable( std::string name )
  {
    std::map<std::string,Variable*>::iterator i = variables.find(name);
    if( i != variables.end() )
      return i->second;

    if( parent ) return parent->get_variable(name);
    return 0;
  }

  bool Context::is_special_variable( std::string name )
  {
    if( special_variables.find(name) != special_variables.end() )
      return true;
    if( parent ) return parent->is_special_variable(name);
    return false;
  }
  Variable *Context::get_special_variable( std::string name )
  {
    std::map<std::string,Variable*>::iterator i = special_variables.find(name);
    if( i != special_variables.end() )
      return i->second;

    if( parent ) return parent->get_special_variable(name);
    return 0;
  }

  bool Context::is_base_type( std::string name )
  {
    if( base_types.find(name) != base_types.end() )
      return true;
    if( parent ) return parent->is_base_type(name);
    return false;
  }
  Base_Type *Context::get_base_type( std::string name )
  {
    std::map<std::string,Base_Type>::iterator i = base_types.find(name);
    if( i != base_types.end() )
      return &(i->second);

    if( parent ) return parent->get_base_type(name);
    return 0;
  }

  bool Context::is_provider_type( std::string name )
  {
    if( provider_types.find(name) != provider_types.end() )
      return true;
    if( parent ) return parent->is_provider_type(name);
    return false;
  }
  Provider_Type *Context::get_provider_type( std::string name )
  {
    std::map<std::string,Provider_Type>::iterator i=provider_types.find(name);
    if( i != provider_types.end() )
      return &(i->second);

    if( parent ) return parent->get_provider_type(name);
    return 0;
  }

  bool Context::is_operator( std::string name )
  {
    if( operators.find(name) != operators.end() )
      return true;
    if( parent ) return parent->is_operator(name);
    return false;
  }
  Operator_Declaration *Context::get_operator( std::string name )
  {
    std::map<std::string,Operator_Declaration>::iterator i 
      = operators.find(name);
    if( i != operators.end() )
      return &(i->second);

    if( parent ) return parent->get_operator(name);
    return 0;
  }
  bool Context::is_solver( std::string name )
  {
    if( solvers.find(name) != solvers.end() )
      return true;
    if( parent ) return parent->is_solver(name);
    return false;
  }
  Event_Solver *Context::get_solver( std::string name )
  {
    std::map<std::string,Event_Solver>::iterator i 
      = solvers.find(name);
    if( i != solvers.end() )
      return &(i->second);

    if( parent ) return parent->get_solver(name);
    return 0;
  }

  bool Context::is_solver_name( std::string name )
  {
    if( solver_names.find(name) != solver_names.end() )
      return true;
    if( parent ) return parent->is_solver_name(name);
    return false;
  }
  Solver_Name *Context::get_solver_name( std::string name )
  {
    std::map<std::string,Solver_Name*>::iterator i = solver_names.find(name);
    if( i != solver_names.end() )
      return i->second;

    if( parent ) return parent->get_solver_name(name);
    return 0;
  }

  Code::Code() : start_src_line(0), start_src_column(0), code("") {}

  Solver_Function::Solver_Function( std::string s, std::string f ) 
    : solver(s), function(f) {}

  Solver_Function_Code::Solver_Function_Code( std::string n, 
					      std::string ret ) 
    : return_type(ret), name(n) {}

  Container::Container( bool serial, std::string provider_type, 
			std::string name )
    : serial(serial), provider_type(provider_type), name(name) {}

  Container_Function::Container_Function( std::string name, 
					  std::string provider_type, 
					  std::string return_type, 
					  std::string parameter_type )
    : name(name), provider_type(provider_type), return_type(return_type), 
      parameter_type(parameter_type) {}

  Solver_Parameter::Solver_Parameter( const Operand&   operand   )
    : parameter_type(t_operand), operand( new Operand(operand) ) {}
    
  Solver_Parameter::Solver_Parameter( const Container& container )
    : parameter_type(t_container), container( new Container(container) ) {}

  Solver_Parameter::Solver_Parameter( Solver_Parameter &src )
    : parameter_type(src.parameter_type), operand(src.operand),
      container(src.container) 
  {
    src.parameter_type = t_none; // move data
  }

  Solver_Parameter::Solver_Parameter( const Solver_Parameter &src )
    : parameter_type(src.parameter_type)
  {
    switch( parameter_type )
    {
    case t_operand:
      operand = new Operand( *src.operand );
      break;
    case t_container:
      container = new Container( *src.container );
      break;
    case t_none:
      break;
    }
  }

  Solver_Parameter::Solver_Parameter()
    : parameter_type(t_none) {}

  Solver_Parameter &Solver_Parameter::operator=( const Operand&   op )
  {
    delete_data();
    operand = new Operand(op);
    parameter_type = t_operand;
    return *this;
  }
  Solver_Parameter &Solver_Parameter::operator=( const Container& c )
  {
    delete_data();
    container = new Container(c);
    parameter_type = t_container;
    return *this;
  }
  //! moves content
  Solver_Parameter &Solver_Parameter::operator=( Solver_Parameter& src )
  {
    delete_data();
    parameter_type = src.parameter_type;
    operand        = src.operand;
    container      = src.container;

    src.parameter_type = t_none;		// removes data in source
    return *this;
  }

  Solver_Parameter &Solver_Parameter::operator=( const Solver_Parameter& src )
  {
    delete_data();

    parameter_type = src.parameter_type;
    switch( parameter_type )
    {
    case t_operand:
      operand = new Operand( *src.operand );
      break;
    case t_container:
      container = new Container( *src.container );
      break;
    case t_none:
      break;
    }

    return *this;
  }

  Operand   &Solver_Parameter::get_operand() const
  {
    assert( parameter_type == t_operand );
    return *operand;
  }
  Container &Solver_Parameter::get_container() const
  {
    assert( parameter_type == t_container );
    return *container;
  }

  Solver_Parameter::~Solver_Parameter()
  {
    delete_data();
  }

  void Solver_Parameter::delete_data()
  {
    switch( parameter_type )
    {
    case t_operand:   delete operand; break;
    case t_container: delete container; break;
    case t_none: break;
    }
  }

  Parameter::Parameter( std::string n, std::string t ) : name(n), type(t) {}

  Event::Event( std::string name ) : name(name) {}

  Event_Group::Event_Group( std::string name ) : name(name) {}

  Event_Solver::Event_Solver() : current_event_group(0) {}

  void Context::set_parent_context( Context *parent_context )
  {
    parent = parent_context;
  }

  Context::Context( Context *parent_context )
    : parent(parent_context)
  {}

  Alias::Alias( std::string a, std::string s ) : alias(a), src(s)
  {}

  //! print, just for debug purposes
  void Constraint_Declaration::print() const 
  {
    if( !essentials.empty() )
    {
      std::cout << "      " << "if( ";
      std::list<std::string>::const_iterator i;
      bool first = true;
      for( i = essentials.begin(); 
	   i != essentials.end(); i++ )
	std::cout << (first?first=false,"":" && ") << (*i);
      std::cout << " )" << std::endl << "  ";
    }
    std::cout << "      " << constraint_code << std::endl;// should include ';'
  }

  Constraint_Declaration::Constraint_Declaration( const Expression *exp )
    : essentials( exp->essentials ), constraint_code( exp->expression_code )
  { }

  void Constraint_Code::new_constraint( const Expression *exp )
  {
    constraint_declarations.push_back( Constraint_Declaration(exp) );
  }

  void Constraint_Code::merge( const Constraint_Code &cc )
  {
    constraint_declarations.insert( constraint_declarations.end(),
				    cc.constraint_declarations.begin(),
				    cc.constraint_declarations.end() );
  }

  //! print, just for debug purposes
  void Constraint_Code::print() const
  {
    std::list<Constraint_Declaration>::const_iterator i;
    for( i = constraint_declarations.begin(); 
	 i != constraint_declarations.end(); i++ )
      i->print();
  }

  //! print, just for debug purposes
  void Solver_Declaration::print() const 
  {
    if( !essentials.empty() )
    {
      std::cout << "      " << "if( ";
      std::list<std::string>::const_iterator i;
      bool first = true;
      for( i = essentials.begin(); 
	   i != essentials.end(); i++ )
	std::cout << (first?first=false,"":" && ") << (*i);
      std::cout << " )" << std::endl << "  ";
    }
    if( is_expression )
      std::cout << "      " << dest_operand << " = " << expression_code << ";";
    else
      std::cout << "      " << solver_type << "( ... );";
  }

  void Solver_Code::new_solver( std::string solver )
  {
    // open solver call, insert it and store the iterator
    current_solver = solver_declarations.insert
	(solver_declarations.end(), Solver_Declaration());

    current_solver->solver_type = solver;
    current_solver->is_expression = false;
  }

  void Solver_Code::set_solver_identifier( std::string name )
  {
    current_solver->identifier = name;
  }
  

  void Solver_Code::add_parameter_ref( Property_Reference &ref )
  {
    current_solver->parameters.push_back( ref.code );
    current_solver->essentials.insert( current_solver->essentials.end(), 
				       ref.essentials.begin(),
				       ref.essentials.end() );
    ref.clear();
  }

  void Solver_Code::add_const_parameter( std::string parameter )
  {
    current_solver->parameters.push_back( parameter );
  }

  void Solver_Code::finish_solver()
  {
  }

  void Solver_Code::new_expression_solver( const std::string &op, 
					   const Expression *exp, 
					   Code_Translator *translator )
  {
    // open solver call, insert it and store the iterator
    current_solver = solver_declarations.insert
	(solver_declarations.end(), Solver_Declaration());

    current_solver->is_expression = true;
    current_solver->essentials = exp->essentials;
    current_solver->dest_operand = op;
    current_solver->expression_code = exp->expression_code;
  }

  void Solver_Code::merge( const Solver_Code &sc )
  {
    solver_declarations.insert( solver_declarations.end(),
                                sc.solver_declarations.begin(),
                                sc.solver_declarations.end() );
  }

  //! print, just for debug purposes
  void Solver_Code::print() const
  {
    std::list<Solver_Declaration>::const_iterator i;
    for( i = solver_declarations.begin();
         i != solver_declarations.end(); i++ )
      i->print();
  }

  //! print, just for debug purposes
  void Action_Declaration::print() const 
  {
    if( !essentials.empty() )
    {
      std::cout << "      " << "if( ";
      std::list<std::string>::const_iterator i;
      bool first = true;
      for( i = essentials.begin(); 
	   i != essentials.end(); i++ )
	std::cout << (first?first=false,"":" && ") << (*i);
      std::cout << " )" << std::endl << "  ";
    }
    std::cout << "      " << action_code << std::endl;// should include ';'
  }

  void Action_Code::new_action( std::string action, std::string priority_label,
				Code_Translator *translator )
  {
    // open action call, insert it and store the iterator
    current_action = action_declarations.insert
	(action_declarations.end(), Action_Declaration());

    current_action->action_code = translator->open_action( action, 
							   priority_label );
  }

  void Action_Code::add_parameter_ref( Property_Reference &ref, 
				       Code_Translator *translator )
  {
    current_action->action_code += translator->parameter_add( ref.code );
    current_action->essentials.insert( current_action->essentials.end(), 
				       ref.essentials.begin(),
				       ref.essentials.end() );
    ref.clear();
  }

  void Action_Code::add_parameter_str( std::string str, 
				       Code_Translator *translator )
  {
    current_action->action_code 
      += translator->parameter_add( str );
  }

  void Action_Code::add_parameter_exp( Expression *exp, 
				       Code_Translator *translator )
  {
    current_action->action_code 
      += translator->parameter_add( exp->expression_code );
    current_action->essentials.insert( current_action->essentials.end(), 
				       exp->essentials.begin(),
				       exp->essentials.end() );
  }

  void Action_Code::finish_action( Code_Translator *translator )
  {
    current_action->action_code += translator->close_function();
  }

  void Action_Code::merge( const Action_Code &ac )
  {
    action_declarations.insert( action_declarations.end(),
				ac.action_declarations.begin(),
				ac.action_declarations.end() );
  }

  //! print, just for debug purposes
  void Action_Code::print() const
  {
    std::list<Action_Declaration>::const_iterator i;
    for( i = action_declarations.begin(); 
	 i != action_declarations.end(); i++ )
      i->print();
  }

  void Solve_System_Code::merge( const Solve_System_Code &ssc )
  {
    constraints.merge( ssc.constraints );
    solvers.merge( ssc.solvers );
    actions.merge( ssc.actions );
  }

  //! print, just for debug purposes
  void Solve_System_Code::print() const
  {
    std::cout << "    constraints {" << std::endl;
    constraints.print();
    std::cout << "    }" << std::endl;
    
    std::cout << "    solvers {" << std::endl;
    solvers.print();
    std::cout << "    }" << std::endl;
    
    std::cout << "    actions {" << std::endl;
    actions.print();
    std::cout << "    }" << std::endl;
  }

  Child_Container::Child_Container( bool _max1, bool _min1, 
				    std::string _provider_type, bool _serial )
    : max1(_max1), min1(_min1), provider_type(_provider_type), serial(_serial)
  {}

  //! just for set container
  bool Child_Container::operator<( const Child_Container &cc ) const
  {
    return provider_type < cc.provider_type;
  }

  //! print, just for debug purposes
  void Child_Container::print() const
  {
    std::cout << "    ";
    if( max1 ) 
      std::cout << "max1 ";
    if( min1 ) 
      std::cout << "min1 ";

    std::cout << provider_type;

    std::cout << std::endl;
  }

  //! print, just for debug purposes
  void Result_Code::print( const Result_Type &type ) const
  {
    std::cout << "    resulting " << type.return_type << "( " 
	      << type.parameter_type << " " << parameter 
	      << " ) " << std::endl; 
    std::cout << "      requires ";
    char first = true;
    // required properties
    std::list<std::string>::const_iterator p;
    for( p = required_properties.begin(); p != required_properties.end(); p++ )
      std::cout << (first?first=false,"":", ") << (*p);

    // required children
    {
      std::list< std::pair<std::string,Result_Type> >::const_iterator c;
      for( c = required_children.begin(); c != required_children.end(); c++ )
	std::cout << (first?first=false,"":", ") 
		  << "child." << c->first << "." << c->second.return_type 
		  << "( " << c->second.parameter_type << ")";
    }

    // required results
    {
      std::list< std::pair<std::string,Result_Type> >::const_iterator c;
      for( c = required_results.begin(); c != required_results.end(); c++ )
	std::cout << (first?first=false,"":", ") 
		  << "this." << c->first << "." << c->second.return_type 
		  << "( " << c->second.parameter_type << ")";
    }
    std::cout << "    {" << std::endl;
    std::cout << code << std::endl;
    std::cout << "    }" << std::endl;
  }

  Result_Code::Result_Code()
    : defined(false)
  {}

  //! print, just for debug purposes
  void Provided_Results::print() const
  {
    std::map<Result_Type, Result_Code>::const_iterator i;
    for( i = results.begin(); i != results.end(); i++ )
      i->second.print( i->first );
  }

  void Tree_Node_Type::merge( const Tree_Node_Type &src )
  {
    // merge properties
    properties.insert( src.properties.begin(), src.properties.end() );

    // merge aliases
    aliases.insert( aliases.end(), src.aliases.begin(), src.aliases.end() );

    // merge operands
    operands.insert( src.operands.begin(), src.operands.end() );

    // merge Solve_System_Code
    common.merge( src.common );
    std::map<std::string,Solve_System_Code>::const_iterator i;
    std::map<std::string,Solve_System_Code>::iterator dest;
    // - merge first code
    for( i = src.first.begin(); i != src.first.end(); i++ )
    {
      dest = first.find( i->first );
      if( dest != first.end() ) // was the element already in dest?
      {
	dest->second.merge( i->second );
      }
      else			// else copy element
      {
	first[i->first] = i->second;
      }
    }
    // - merge last code
    for( i = src.last.begin(); i != src.last.end(); i++ )
    {
      dest = last.find( i->first );
      if( dest != last.end() ) // was the element already in dest?
      {
	dest->second.merge( i->second );
      }
      else			// else copy element
      {
	last[i->first] = i->second;
      }
    }

    // merge child_containers
    child_containers.insert( src.child_containers.begin(), 
			     src.child_containers.end() );

    // merge result code
    provided_results.insert( src.provided_results.begin(), 
			     src.provided_results.end() );
  }

  Tree_Node_Type::Tree_Node_Type() 
    : don_t_create_code(false) 
  {
  }

  //! print, just for debug purposes
  void Tree_Node_Type::print_provides() const
  {
    std::map<std::string, Provided_Results>::const_iterator i;
    if( !provided_results.empty() )
    {
      std::cout << "provides ";
      
      bool first = true;
      for( i = provided_results.begin(); i != provided_results.end(); i++ )
      {
	std::cout << (first?first=0,"":", ") // comma separated
		  <<  i->first; 
      }
    }
  }

  //! print, just for debug purposes
  void Tree_Node_Type::print() const
  {
    //***********
    // properties
    {
      std::cout << "  properties {" << std::endl;
      std::map<std::string,Property*>::const_iterator i;

      for( i = properties.begin(); i != properties.end(); i++ )
	std::cout << "    " << i->second->type << " " << i->first << ";" 
		  << std::endl;

      std::cout << "  }" << std::endl;
    }
    //*********
    // aliases
    {
      std::cout << "  aliases {" << std::endl;
      std::list<Alias>::const_iterator i;
      for( i = aliases.begin(); i != aliases.end(); i++ )
	std::cout << "    " << i->alias << " = "<< i->src << ";" << std::endl;
      std::cout << "  }" << std::endl;
    }
    //**********
    // operands
    {
      std::cout << "  operands {" << std::endl;
      std::map<std::string,Operand*>::const_iterator i;

      for( i = operands.begin(); i != operands.end(); i++ )
	std::cout << "    " << i->second->type << " " << i->first << ";" 
		  << std::endl;

      std::cout << "  }" << std::endl;
    }
    //********
    // common
    {
      std::cout << "  common {" << std::endl;
      common.print();
      std::cout << "  }" << std::endl;
    }
    //********
    // first
    {
      std::map<std::string,Solve_System_Code>::const_iterator i;
      for( i = first.begin(); i != first.end(); i++ )
      {
	std::cout << "  first " << i->first << " {" << std::endl;
	i->second.print();
	std::cout << "  }" << std::endl;
      }
    }
    //******
    // last
    {
      std::map<std::string,Solve_System_Code>::const_iterator i;
      for( i = last.begin(); i != last.end(); i++ )
      {
	std::cout << "  last " << i->first << " {" << std::endl;
	i->second.print();
	std::cout << "  }" << std::endl;
      }
    }
    //******************
    // child containers
    {
      std::set<Child_Container>::const_iterator i;
      std::cout << "  contains {" << std::endl;
      for( i = child_containers.begin(); i != child_containers.end(); i++ )
	i->print();
      std::cout << "  }" << std::endl;
    }
    //******************
    // provided results
    {
      std::map<std::string, Provided_Results>::const_iterator i;
      for( i = provided_results.begin(); i != provided_results.end(); i++ )
      {
	std::cout << "  provide " << i->first << " {" << std::endl;
	i->second.print();
	std::cout << "  }" << std::endl;
      }
    }
  }

  void Property_Reference::clear()
  {
    essentials.clear(); code = ""; 
    first = true;
  }
  void Property_Reference::add( std::string add_code, std::string concat_str )
  {
    if(!first)
      code += concat_str;    // add concat string when more then one elements
    else
      first = false;

    code += add_code;
    essentials.push_back( code );
  }
  void Property_Reference::add_unchecked( std::string add_code, 
					  std::string concat_str )
  {
    if(!first)
      code += concat_str;    // add concat point when more then one elements
    else
      first = false;

    code += add_code;
  }
  Property_Reference::Property_Reference()
    : first(true) {}

    std::list<std::string> essentials;
    std::string expression_code;

  void Expression::append( std::string code )
  {
    expression_code += code;
  }
  void Expression::append( Expression *exp )
  {
    essentials.insert( essentials.end(), 
		       exp->essentials.begin(), exp->essentials.end() );
    expression_code += exp->expression_code;
  }

  Expression::Expression() {}
  Expression::Expression( const Expression *exp )
    : essentials( exp->essentials ), expression_code(exp->expression_code){}
  Expression::Expression( const Property_Reference &ref )
    : essentials( ref.essentials ), expression_code(ref.code) {}

  //! print, just for debug purposes
  void AFD_Root::print() const
  {
    //*************
    // base types
    {
      std::map<std::string, Base_Type>::const_iterator i;
      std::cout << "base_types {" << std::endl;
      for( i = base_types.begin(); i != base_types.end(); i++ )
      {
	std::cout << "  " << i->first << " = ";
	i->second.print();
	std::cout << ";" << std::endl;
      }
      std::cout << "}" << std::endl;
    }
    //****************
    // provider types
    {
      std::map<std::string, Provider_Type>::const_iterator i;
      for( i = provider_types.begin(); i != provider_types.end(); i++ )
      {
	i->second.print(i->first);
      }
    }
    //*****************
    // tree node types
    {
      std::map<std::string, Tree_Node_Type>::const_iterator i;
      for( i = nodes.begin(); i != nodes.end(); i++ )
      {
	std::cout << "node " << i->first << " ";
	i->second.print_provides();
	std::cout << " {" << std::endl;
	i->second.print();
	std::cout << "}" << std::endl;
      }
    }
  }

  void AFD_Root::push_context( Context* context )
  {
    contexts.push( context );
  }
  void AFD_Root::pop_context()
  {
    contexts.pop();
  }
  Context *AFD_Root::get_context()
  {
    return contexts.top();
  }

  AFD_Root::AFD_Root( Code_Translator *trans )
    : translator(trans), priority_list_defined(false), 
      write_priority_list(false), 
      current_node(0), include_depth(0), current_code(0)
  {
    don_t_create_code.push( false );
    push_context( this );
  }
}

