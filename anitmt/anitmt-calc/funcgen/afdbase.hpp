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

#ifndef __funcgen_afdbase__
#define __funcgen_afdbase__

#include <message/message.hpp>

#include <string>
#include <set>
#include <map>
#include <list>

namespace funcgen
{
  class Basic_Type;
  class Provider_Type;
  class Result_Type;
  class Alias;
  class Constraint_Code;
  class Solver_Code;
  class Action_Code;
  class Solve_System_Code;
  class Child_Container;
  class Provided_Results;
  class Result_Code;
  class Property_Reference;
  class Bool_Expression;
  class Expression;
  class Tree_Node_Type;
  class AFD_Manager;

  class Base_Type
  {
    bool structure;
    std::string type_name;
    std::map<std::string,std::string> element_types; // name->type
  public:
    //! add element type to structure
    void add_element( std::string type, std::string name ); 

    //! returns the type name
    std::string get_type();	

    //! new type is structure
    Base_Type();		
    //! new type is same as old type
    Base_Type( std::string old_type );	
    void print() const;		// print, just for debug purposes
  };

  class Result_Type 
  {
  public:
    std::string return_type;
    std::string parameter_type;

    Result_Type( std::string ret, std::string par );
    
    //! for insertion in set/map containers:
    bool operator<( const Result_Type &rt ) const;
  };

  class Provider_Type
  {
  public:
    std::map<std::string,Result_Type> result_types;
    bool serial;

    // special functions for identification
    bool operator==( Provider_Type& );
    typedef Provider_Type* id;	// object pointer is unique ID
    inline id get_id() { return this; }
    void print( std::string ) const; // print, just for debug purposes
  };  

  class Alias
  {
  public:
    std::string alias, src;
    Alias( std::string alias, std::string src );
    void print() const;		// print, just for debug purposes
  };

  class Constraint_Declaration
  {
  public:
    std::list<std::string> essentials;
    std::string constraint_code;

    void print() const;		// print, just for debug purposes
    Constraint_Declaration( const Expression *exp );
  };

  class Constraint_Code
  {
  public:
    std::list<Constraint_Declaration> constraint_declarations;
    
    void new_constraint( const Expression *exp );
    void merge( const Constraint_Code & );
    void print() const;		// print, just for debug purposes
  };

  class Solver_Declaration
  {
  public:
    std::list<std::string> essentials;
    std::string solver_code;

    void print() const;		// print, just for debug purposes
  };

  class Solver_Code
  {
    bool first_param;
  public:
    std::list<Solver_Declaration> solver_declarations;

    std::list<Solver_Declaration>::iterator current_solver;

    void new_solver( std::string name );
    void add_parameter_ref( Property_Reference &ref );
    void finish_solver();
    void new_expression_solver( const Expression *exp );
    void merge( const Solver_Code & );
    void print() const;		// print, just for debug purposes
  };

  class Action_Declaration
  {
  public:
    std::list<std::string> essentials;
    std::string action_code;

    void print() const;		// print, just for debug purposes
  };

  class Action_Code
  {
  public:
    std::list<Action_Declaration> action_declarations;
    std::list<Action_Declaration>::iterator current_action;

    void new_action( std::string name, double level );
    void add_parameter_ref( Property_Reference &ref );
    void add_parameter_exp( Expression *ref );
    void finish_action();
    void merge( const Action_Code & );
    void print() const;		// print, just for debug purposes
  };

  class Solve_System_Code
  // must be copyable
  {
  public:
    Constraint_Code	constraints;
    Solver_Code		solvers;
    Action_Code		actions;

    void merge( const Solve_System_Code & );
    void print() const;		// print, just for debug purposes
  };

  class Child_Container
  {
  public:
    bool max1;			// container hasn't more than one element
    bool min1;			// container needs at least one element
    std::string provider_type;	// type that all elements have to provide
    void print() const;		// print, just for debug purposes

    Child_Container( bool max1, bool min1, std::string provider_type );
    //! just for set container
    bool operator<( const Child_Container &cc ) const;
  };

  class Result_Code
  {
  public:
    bool defined;		// is this code was already defined
    std::string parameter;	// parameter name
    std::list<std::string> required_properties; 
    std::list< std::pair<std::string,Result_Type> > required_children; 
    std::list< std::pair<std::string,Result_Type> > required_results; 

    int start_src_line;		// source line where this code starts
    std::string code;		// source code
    void print( const Result_Type &type ) const; 
				// print, just for debug purposes
  };

  class Provided_Results
  {
  public:
    std::map<Result_Type, Result_Code> results;
    void print() const;		// print, just for debug purposes
  };

  class Property_Reference
  {
    bool first;
  public:
    std::list<std::string> essentials;
    std::string code;		// reference code

    void clear();
    void add( std::string code );
    void add_unchecked( std::string code );

    Property_Reference();
  };

  class Expression
  {
  public:
    std::list<std::string> essentials;
    std::string expression_code;

    void append( std::string );
    void append( Expression *exp );

    Expression();
    Expression( const Expression *exp );
    Expression( const Property_Reference &exp );
  };

  class Tree_Node_Type
  {
  public:
    bool don_t_create_code;	// ... from this node
    message::Abstract_Position *pos;

    //***********
    // properties
    std::set<std::string> flag_properties;
    std::set<std::string> scalar_properties;
    std::set<std::string> vector_properties;
    std::set<std::string> matrix_properties;
    std::set<std::string> string_properties;
    std::set<std::string> *current_properties; // points to the current set

    //***********
    // aliases
    std::list<Alias> aliases;
    
    //***********
    // operands
    std::set<std::string> flag_operands;
    std::set<std::string> scalar_operands;
    std::set<std::string> vector_operands;
    std::set<std::string> matrix_operands;
    std::set<std::string> string_operands;
    std::set<std::string> *current_operands; // points to the current set

    //*****************
    // solve relations
    Solve_System_Code common;
    std::map<std::string,Solve_System_Code> first, last; 
    Solve_System_Code *current_solve_code;
    
    //**************************************
    // contained result providers (children)
    std::set<Child_Container> child_containers;
    
    //*************
    // result code
    std::map<std::string, Provided_Results> provided_results;
    Result_Code *current_result_code;

    //**********************
    // general help classes
    Property_Reference current_reference;

    void merge( const Tree_Node_Type & );

    Tree_Node_Type();
    void print_provides() const; // print, just for debug purposes
    void print() const;		 // print, just for debug purposes
  };

  // stores all information of afd files
  class AFD_Manager
  {
  public:
    std::map<std::string, Base_Type> base_types;
    Base_Type      *current_base_type;
    std::map<std::string, Provider_Type> types;
    Provider_Type  *current_type;
    std::map<std::string, Tree_Node_Type> nodes;
    Tree_Node_Type *current_node;

    //AFD_Manager(){}
    void print() const;		// print, just for debug purposes
  };

  //**********************
  // general help classes

  class Property_Reference_Handler
  {
  public:
  };
}

#endif
