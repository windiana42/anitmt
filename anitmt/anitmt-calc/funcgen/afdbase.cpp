/*****************************************************************************/
/**   Basic classes to store all information of afd files       	    **/
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

#include "afdbase.hpp"

#include <iostream>

#include "stdextend.hpp"

namespace funcgen
{
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
    return return_type <  rt.return_type;
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

  Alias::Alias( std::string a, std::string s )
    : alias(a), src(s)
  {
  }

  //! print, just for debug purposes
  void Alias::print() const
  {
    std::cout << "    " << alias << " = " << src << ";" << std::endl;
  }

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
  {
    constraint_code += ";";	// terminate constraint expression
  }

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
    std::cout << "      " << solver_code << std::endl;// should include ';'
  }

  void Solver_Code::new_solver( std::string solver )
  {
    // open solver call, insert it and store the iterator
    current_solver = solver_declarations.insert
	(solver_declarations.end(), Solver_Declaration());

    current_solver->solver_code = solver + "(";
    first_param = true;
  }

  void Solver_Code::add_parameter_ref( Property_Reference &ref )
  {
    if(!first_param)
      current_solver->solver_code += ',';
    else
      first_param = false;

    current_solver->solver_code += ref.code;
    current_solver->essentials.insert( current_solver->essentials.end(), 
				       ref.essentials.begin(),
				       ref.essentials.end() );
    ref.clear();
  }

  void Solver_Code::finish_solver()
  {
    current_solver->solver_code += ");";
  }

  void Solver_Code::new_expression_solver( const Expression *exp )
  {
    // open solver call, insert it and store the iterator
    current_solver = solver_declarations.insert
	(solver_declarations.end(), Solver_Declaration());

    current_solver->essentials = exp->essentials;
    current_solver->solver_code = exp->expression_code;
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

  void Action_Code::new_action( std::string action, double level )
  {
    // open action call, insert it and store the iterator
    current_action = action_declarations.insert
	(action_declarations.end(), Action_Declaration());

    current_action->action_code = action + "(" + level;
  }

  void Action_Code::add_parameter_ref( Property_Reference &ref )
  {
    current_action->action_code += ',';
    current_action->action_code += ref.code;
    current_action->essentials.insert( current_action->essentials.end(), 
				       ref.essentials.begin(),
				       ref.essentials.end() );
    ref.clear();
  }

  void Action_Code::add_parameter_exp( Expression *exp )
  {
    current_action->action_code += ',';
    current_action->action_code += exp->expression_code;
    current_action->essentials.insert( current_action->essentials.end(), 
				       exp->essentials.begin(),
				       exp->essentials.end() );
  }

  void Action_Code::finish_action()
  {
    current_action->action_code += ");";
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
				    std::string _provider_type )
    : max1(_max1), min1(_min1), provider_type(_provider_type)
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

    std::cout << code << std::endl;
  }

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
    flag_properties.insert( src.flag_properties.begin(),
			    src.flag_properties.end() );
    scalar_properties.insert( src.scalar_properties.begin(),
			      src.scalar_properties.end() );
    vector_properties.insert( src.vector_properties.begin(),
			      src.vector_properties.end() );
    matrix_properties.insert( src.matrix_properties.begin(),
			      src.matrix_properties.end() );
    string_properties.insert( src.string_properties.begin(),
			      src.string_properties.end() );
    // merge aliases
    aliases.insert( aliases.end(), src.aliases.begin(), src.aliases.end() );

    // merge operands
    flag_operands.insert( src.flag_operands.begin(),
			  src.flag_operands.end() );
    scalar_operands.insert( src.scalar_operands.begin(),
			    src.scalar_operands.end() );
    vector_operands.insert( src.vector_operands.begin(),
			    src.vector_operands.end() );
    matrix_operands.insert( src.matrix_operands.begin(),
			    src.matrix_operands.end() );
    string_operands.insert( src.string_operands.begin(),
			    src.string_operands.end() );

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
      std::set<std::string>::const_iterator i;

      std::cout << "    type flag {" << std::endl;
      for( i = flag_properties.begin(); i != flag_properties.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type scalar {" << std::endl;
      for( i = scalar_properties.begin(); i != scalar_properties.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type vector {" << std::endl;
      for( i = vector_properties.begin(); i != vector_properties.end(); i++ )
      std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type matrix {" << std::endl;
      for( i = matrix_properties.begin(); i != matrix_properties.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type string {" << std::endl;
      for( i = string_properties.begin(); i != string_properties.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

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
      std::set<std::string>::const_iterator i;

      std::cout << "    type flag {" << std::endl;
      for( i = flag_operands.begin(); i != flag_operands.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type scalar {" << std::endl;
      for( i = scalar_operands.begin(); i != scalar_operands.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type vector {" << std::endl;
      for( i = vector_operands.begin(); i != vector_operands.end(); i++ )
      std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type matrix {" << std::endl;
      for( i = matrix_operands.begin(); i != matrix_operands.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

      std::cout << "    type string {" << std::endl;
      for( i = string_operands.begin(); i != string_operands.end(); i++ )
	std::cout << "      " << (*i) << ";" << std::endl;
      std::cout << "    }" << std::endl;

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
  void Property_Reference::add( std::string add_code )
  {
    if(!first)
      code += '.';		// add concat point when more then one elements
    else
      first = false;

    code += add_code;
    essentials.push_back( code );
  }
  void Property_Reference::add_unchecked( std::string add_code )
  {
    if(!first)
      code += '.';		// add concat point when more then one elements
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

  AFD_Root::AFD_Root( Code_Translator *trans )
    : translator(trans)
  {}
}

