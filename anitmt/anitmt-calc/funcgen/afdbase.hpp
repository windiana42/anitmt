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

  class Basic_Type
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
    Basic_Type();		
    //! new type is same as old type
    Basic_Type( std::string old_type );	
  };

  class Provider_Type
  {
  public:
    std::set<Result_Type> result_types;

    // special functions for identification
    bool operator==( Provider_Type& );
    typedef Provider_Type* id;	// object pointer is unique ID
    inline id get_id();
  };  

  class Result_Type : public std::pair<std::string,std::string>
  {
  public:
    std::string &return_type;
    std::string &parameter_type;

    Result_Type( std::string ret, std::string par );
  };

  class Alias
  {
  public:
    std::string alias, src;
    Alias( std::string alias, std::string src );
  };

  class Constraint_Code
  {
  public:
    std::list<std::string> constraint_expressions;
    
    void merge( const Constraint_Code & );
  };

  class Solver_Code
  {
  public:
    std::list<std::string> solver_declarations;

    std::list<std::string>::iterator current_solver;

    void merge( const Solver_Code & );
  };

  class Action_Code
  {
  public:
    std::list<std::string> action_declarations;

    void merge( const Action_Code & );
  };

  class Solve_System_Code
  {
  public:
    Constraint_Code	constraints;
    Solver_Code		solvers;
    Action_Code		actions;

    void merge( const Solve_System_Code & );
  };

  class Child_Container
  {
  public:
    bool max1;			// container hasn't more than one element
    bool min1;			// container needs at least one element
    std::string provider_type;	// type that all elements have to provide
    bool seriatim;		// elements have a serial order
  };

  class Provided_Results
  {
  public:
    std::map<Result_Type, Result_Code> result;
  };

  class Result_Code
  {
  public:
    int start_src_line;		// source line where this code starts
    std::string code;		
  };

  class Property_Reference
  {
    std::string pre_code;
    std::string code;
    std::string post_code;
  public:
    void clear();
  };
  class Bool_Expression
  {
    std::string exp;
  public:
    void clear();
    std::string get() const;
  };
  class Expression
  {
    std::string exp;
  public:
    void clear();
    std::string get() const;
  };

  class Tree_Node_Type
  {
  public:
    std::string name; 
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
    
    //*****************
    // solve relations
    Solve_System_Code common;
    std::map<std::string,Solve_System_Code> first, last; 
    Solve_System_Code *current_solve_code;
    
    //**************************************
    // contained result providers (children)
    std::map<std::string, Child_Container> child_containers;
    
    //*************
    // result code
    std::map<std::string, Provided_Results> provided_results;
    Result_Code *current_result_code;

    //**********************
    // general help classes
    Property_Reference current_reference;
    Bool_Expression current_bool_expression;
    Expression current_expression;

    void merge( const Tree_Node_Type & );
  };

  // stores all information of afd files
  class AFD_Manager
  {
  public:
    std::map<std::string, Basic_Type> base_types;
    Basic_Type     *current_base_type;
    std::map<std::string, Provider_Type> types;
    Provider_Type  *current_type;
    std::map<std::string, Tree_Node_Type> nodes;
    Tree_Node_Type *current_node;

    AFD_Manager(){}
  };

  //**********************
  // general help classes

  class Property_Reference_Handler
  {
  public:
  };
}

#endif
