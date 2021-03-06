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

#ifndef __funcgen_afdbase__
#define __funcgen_afdbase__

#include <message/message.hpp>

#include <string>
#include <set>
#include <map>
#include <list>
#include <stack>

#include "base_operators.hpp"

namespace funcgen
{
  const int max_include_depth = 30;

  class Header_Files;
  class Priority_List;
  class Basic_Type;
  class Result_Type;
  class Provider_Type;
  class Code;
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
  class Operator_Declaration;
  class Solver_Parameter;
  class Event_Solver;
  class AFD_Root;
  /* !!! incomplete !!! */
}

#include "gen_code.hpp"

namespace funcgen
{
  class Header_Files
  {
  private:
    std::list<std::string> headers;
  public:
    void add_header( std::string header );
    std::list<std::string>::const_iterator begin();
    std::list<std::string>::const_iterator end();
  };
  class Priority_List
  {
  private:
    std::list<std::string> priority_labels;
    std::set<std::string> priority_label_set;
  public:
    void add_priority_label( std::string label );
    bool is_priority_label( std::string label );
    std::list<std::string>::const_iterator begin();
    std::list<std::string>::const_iterator end();
  };

  class Base_Type
  {
  private:
    bool structure;
    std::string type_name;
  public:
    typedef std::map<std::string,std::string> element_types_type; 
  private:
    element_types_type element_types; // map name->type
  public:
    bool don_t_create_code;	// ... from this node
    //! add element type to structure
    void add_element( std::string type, std::string name ); 

    bool is_structure() const;
    //! returns the type name
    std::string get_type() const;	
    //! returns element begin-iterator
    element_types_type::const_iterator element_begin() const;
    //! returns element end-iterator
    element_types_type::const_iterator element_end() const;
    
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
    bool don_t_create_code;	// ... from this node
    typedef std::set<Result_Type> result_types_type;
    result_types_type result_types;
    bool serial;
    message::Abstract_Position *pos;

    // special functions for identification
    bool operator==( Provider_Type& );
    typedef Provider_Type* id;	// object pointer is unique ID
    inline id get_id() { return this; }
    void print( std::string ) const; // print, just for debug purposes
  };  

  class Operand
  {
  public:
    Operand( std::string name="", std::string type="" );

    std::string name;
    std::string type;
  };

  class Property
  {
  public:
    Property( std::string name="", std::string type="" );

    std::string name;
    std::string type;
  };

  class Container
  {
  public:
    bool serial;
    std::string provider_type;
    std::string name;
    
    Container( bool serial=true, std::string provider_type="", 
	       std::string name="" );
  };

  class Variable
  {
  public:
    Variable( std::string name="", std::string type="" );

    std::string name;
    std::string type;
  };

  class Solver_Name
  {
  public:
    Solver_Name( std::string name="", std::string type="" );

    std::string name;
    std::string type;
  };

  class Context
  {
  public:
    Context( Context *parent = 0 );

    void set_parent_context( Context *parent );

    //***********
    // properties
    std::map<std::string,Property*> properties;	// for search
    std::list<Property> property_list;		// for sequence
    std::string current_property_type;
    bool is_property( std::string name );
    Property *get_property( std::string name );

    //***********
    // operands
    std::map<std::string,Operand*> operands;	// for search
    std::list<Operand> operand_list;		// for sequence
    std::string current_operand_type;
    bool is_operand( std::string name );
    Operand *get_operand( std::string name );

    //************
    // containers
    std::map<std::string,Container*> containers;// for search
    std::list<Container> container_list;	// for sequence
    bool is_container( std::string name );
    Container *get_container( std::string name );

    //***********
    // variables
    std::map<std::string,Variable*> variables;	// for search
    std::list<Variable> variable_list;		// for sequence
    std::string current_variable_type;
    bool is_variable( std::string name );
    Variable *get_variable( std::string name );

    //*******************
    // special_variables
    std::map<std::string,Variable*> special_variables;	// for search
    std::list<Variable> special_variable_list;		// for sequence
    std::string current_special_variable_type;
    bool is_special_variable( std::string name );
    Variable *get_special_variable( std::string name );

    //***********
    // base type
    std::map<std::string,Base_Type> base_types; 
    bool is_base_type( std::string name );
    Base_Type *get_base_type( std::string name );

    //****************
    // provider types
    std::map<std::string,Provider_Type>  provider_types;
    bool is_provider_type( std::string name );
    Provider_Type *get_provider_type( std::string name );

    //***********
    // operators
    std::map<std::string,Operator_Declaration> operators;
    bool is_operator( std::string name );
    Operator_Declaration *get_operator( std::string name );

    //*******
    // solver
    std::map<std::string,Event_Solver> solvers;
    bool is_solver( std::string name );
    Event_Solver *get_solver( std::string name );

    //************
    // solver name
    std::map<std::string,Solver_Name*> solver_names;
    std::list<Solver_Name> solver_name_list;		// for sequence
    bool is_solver_name( std::string name );
    Solver_Name *get_solver_name( std::string name );
  private:
    Context *parent;
  };

  class Code
  {
  public:
    Code();

    message::File_Position *pos;
    int start_src_line;		// source line where this code starts
    int start_src_column;	// source column where this code starts
    std::string code;		// source code
  };

  class Function_Code : public Code
  {
  public:
    std::list<std::string> parameter_names;
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
    bool is_expression;
    // for normal solvers
    std::string solver_type;
    std::string identifier;
    std::list<std::string> parameters;
    // for expression trees
    std::string dest_operand;
    std::string expression_code;

    void print() const;		// print, just for debug purposes
  };

  class Solver_Code
  {
    std::list<Solver_Declaration>::iterator current_solver;
  public:
    std::list<Solver_Declaration> solver_declarations;

    void new_solver( std::string name );
    void set_solver_identifier( std::string name );
    void add_parameter_ref( Property_Reference &ref );
    void add_const_parameter( std::string parameter );
    void finish_solver();
    void new_expression_solver( const std::string &op, const Expression *exp, 
				Code_Translator *translator );
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

    void new_action( std::string name, std::string priority_label, 
		     Code_Translator *t );
    void add_parameter_ref( Property_Reference &ref, Code_Translator *t );
    void add_parameter_exp( Expression *ref, Code_Translator *t );
    void add_parameter_str( std::string str, Code_Translator *t );
    void finish_action( Code_Translator *t );
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
    bool serial;
    void print() const;		// print, just for debug purposes

    Child_Container( bool max1, bool min1, std::string provider_type, 
		     bool serial );
    //! just for set container
    bool operator<( const Child_Container &cc ) const;
  };

  class Result_Code : public Code
  {
  public:
    bool defined;		// is this code already defined
    std::string return_type;	// return type of this code
    std::string parameter_type;	// parameter_type type of this code
    std::string parameter;	// parameter name
    std::list<std::string> required_properties; 
    std::list< std::pair<std::string,Result_Type> > required_children;
    std::list< std::pair<std::string,Result_Type> > required_results; 
    std::list< std::pair<std::string,std::string> > required_solver_functions; 

    void print( const Result_Type &type ) const; 
				// print, just for debug purposes
    Result_Code();
  };

  class Provided_Results
  {
  public:
    std::map<Result_Type, Result_Code> results;
    Result_Code *current_result_code;
    Provider_Type *type;	// provider type of these provided results
    std::string    type_name;	// name of provider type
    void print() const;		// print, just for debug purposes
  };

  class Property_Reference
  {
    bool first;
  public:
    std::list<std::string> essentials;
    std::string code;		// reference code

    // ** to store until next call
    std::string provider_type;
    std::string ret_type;
    std::string par_type;
    // ** access functions
    void clear();		// clear reference
    //! add checked reference element
    void add( std::string code, std::string concat_str ); 
    //! add unchecked reference element
    void add_unchecked( std::string code, std::string concat_str ); 

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

  class Tree_Node_Type : public Context
  {
  public:
    bool don_t_create_code;	// ... from this node
    message::Abstract_Position *pos;

    //***********
    // properties
    //Context: std::map<std::string,Property*> properties;
    //Context: std::string current_property_type;

    //***********
    // aliases
    std::list<Alias> aliases;
    
    //***********
    // operands
    //Context: std::map<std::string,Operand*> operands; 
    //Context: std::string current_operand_type;

    //*****************
    // solve relations
    Solve_System_Code common;
    std::map<std::string,Solve_System_Code> first, last; 
    
    //**************************************
    // contained result providers (children)
    std::set<Child_Container> child_containers;
    
    //*************
    // result code
    std::map<std::string, Provided_Results> provided_results;
    Provided_Results *current_provided_results;
    Provider_Type *current_provided_result_type;

    void merge( const Tree_Node_Type & );

    Tree_Node_Type();
    void print_provides() const; // print, just for debug purposes
    void print() const;		 // print, just for debug purposes
  };

  class Operator_Declaration
  {
  public:
    bool don_t_create_code;	// ... from this operator declaration

    message::File_Position *pos;
    std::string operator_name;
    std::string operator_base_type_name;
    //! information object about the base operator
    const Basic_Operator *basic_operator; 

    std::map< std::string, Function_Code > function_code;     // name->code
    std::list< std::list<std::string> > versions; // [name,ret,op1,op2,...]*
    Function_Code *current_function; 
    std::list<std::string> *current_version;
  };

  class Container_Function
  {
  public:
    std::string name;
    std::string provider_type;
    std::string return_type;
    std::string parameter_type;

    Container_Function( std::string name="", std::string provider_type="",
			std::string return_type="", 
			std::string parameter_type="" );
  };

  class Solver_Function
  {
  public:
    Solver_Function( std::string solver="", std::string function="" );

    std::string solver;
    std::string function;
  };


  class Solver_Parameter
  {
  public:
    enum type { t_operand, t_container, t_none };

    Solver_Parameter( const Operand&   operand   );
    Solver_Parameter( const Container& container );
    Solver_Parameter( Solver_Parameter& );
    Solver_Parameter( const Solver_Parameter& );
    Solver_Parameter();

    Solver_Parameter &operator=( const Operand&   operand   );
    Solver_Parameter &operator=( const Container& container );
    //! moves content
    Solver_Parameter &operator=( Solver_Parameter& );
    Solver_Parameter &operator=( const Solver_Parameter& );

    inline type get_type() const	{ return parameter_type; }
    Operand   &get_operand() const;
    Container &get_container() const;

    ~Solver_Parameter();
  private:
    type parameter_type;

    Operand   *operand;
    Container *container;

    void delete_data();
  };

  class Solver_Parameters : public Context
  {
  public:
    //***********
    // operands
    //Context: std::map<std::string,Operand*> operands; 
    //Context: std::list<Operand> operand_list;
    //Context: std::string current_operand_type;

    //************
    // containers
    //Context: std::map<std::string,Container*> containers;
    //Context: std::list<Container> container_list;	
    //Context: std::string current_container_type;

    std::list<Solver_Parameter> parameters; // operands and containers
  };

  class Parameter
  {
  public:
    Parameter( std::string name="", std::string type="" );

    std::string name;
    std::string type;
  };

  class Event
  {
  public:
    Event( std::string name="" );

    std::string name;
    std::list<std::string> required_operands;
    std::list<std::string> required_containers;
    std::list<Container_Function> required_container_functions;
    std::list<std::string> required_events;
    std::list<std::string> required_event_groups;
    std::list<Solver_Function> required_solver_functions;
    Code test_run_code;
    Code reset_code;
    Code final_code;
  };
  class Event_Group
  {
  public:
    Event_Group( std::string name="" );

    std::string name;
    Code reset_code;

    // events:
    std::list<Event> events;
    Event *current_event;
  };

  class Solver_Function_Code : public Code, public Context
  {
  public:
    std::string return_type;	// return type of this code
    std::string name;
    //Context: std::map<name,Variable*> variables; as parameters
    //Context: std::list<Variable>  variable_list; as parameter list
    std::list<std::string> required_operands; 
    std::list<std::string> required_functions; 
    std::list<Solver_Function> required_solver_functions; 
    std::list<std::string> required_events;
    std::list<std::string> required_event_groups;
    std::list<std::string> required_containers;
    std::list<Container_Function> required_container_functions;

    Solver_Function_Code( std::string name="", std::string return_type="" );
  };

  class Event_Solver : public Context
  {
  public:
    Event_Solver();

    bool don_t_create_code;	// ... from this solve definition

    Solver_Parameters parameters;

    //***********
    // operands
    //Context: std::map<std::string,Operand*> operands; 
    //Context: std::string current_operand_type;

    //*************
    // declarations
    //Context: std::map<std::string,Variable*> variables;
    //Context: std::list< Variable > variable_list;

    //*************
    // init code
    Constraint_Code     init_constraint_code;
    Solver_Code		init_solver_code;
    Code		init_code;

    //*************
    // events
    std::list<Event_Group> event_groups;
    Event_Group *current_event_group;
    
    //*************
    // provides
    std::list<Solver_Function_Code> functions;
    Solver_Function_Code *current_function_code;
  };

  // stores all information of afd files
  class AFD_Root : public Context
  {
  public:
    AFD_Root( Code_Translator *translator );

    std::stack<bool> don_t_create_code; // whether source shouldn't be written
    Code_Translator *translator; // translates code peaces 

    std::list<std::string> included_basenames;
    Header_Files header_files;

    Priority_List priority_list;
    bool priority_list_defined;
    bool write_priority_list;
    
    //Context: std::map<std::string, Base_Type*> base_types;
    Base_Type      *current_base_type;
    typedef 
    std::list< std::pair<std::string, Base_Type*> > base_types_list_type; 
    base_types_list_type base_types_list;
				// list in original sequence

    typedef std::map<std::string, Provider_Type> provider_types_type;
    //Context: provider_types_type provider_types;
    Provider_Type  *current_type;

    typedef std::map<std::string, Tree_Node_Type> nodes_type;
    nodes_type nodes;
    Tree_Node_Type *current_node;

    typedef std::map<std::string, 
		     Operator_Declaration> operators_type;
    //Context: operators_type operators;
    Operator_Declaration *current_operator_declaration;

    typedef std::map<std::string, Event_Solver> event_solver_type;
    //Context: event_solver_type solvers;
    Event_Solver *current_event_solver;

    int include_depth;
    std::set<std::string> avoid_recursion_of;

    void print() const;		// print, just for debug purposes

    void push_context( Context* );
    void pop_context();
    Context *get_context();

    //**********************
    // general help objects
    Code *current_code;		// any type of plain code
    std::string return_type;	// main return type of current code
    Property_Reference current_reference;
    Solve_System_Code *current_solve_code;
    std::list<std::string> string_list;
    std::string store;		// to store a string
  private:
    std::stack<Context*> contexts;
  };

}

#endif
