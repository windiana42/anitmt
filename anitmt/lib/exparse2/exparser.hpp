#ifndef __exparser__
#define __exparser__

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <message/message.hpp>

namespace exparser
{
#include "tokens.h" //include token defines TOK_...

  // YACC/LEX token type
  class Token
  {
    int type;
    union{
      void *ptr;

      solve::Operand<values::Flag>   *op_flag;
      solve::Operand<values::Scalar> *op_scalar;
      solve::Operand<values::Vector> *op_vector;
      solve::Operand<values::Matrix> *op_matrix;
      solve::Operand<values::String> *op_string;
      values::Flag   *flag;
      values::Scalar *scalar;
      values::Vector *vector;
      values::Matrix *matrix;
      values::String *string;

      // these types make problems with the functions below 
      //std::string *std_string; // might be useful for anything
      //double dval;
    }u;
  public:
    inline int get_type() const { return type; }

    // access functions by references
    inline values::Flag   &flag()   
    {
      if(type==TOK_INVALID_ID)
      {
	u.flag=new values::Flag; type=TOK_FLAG;
      }
      else
	assert(type==TOK_FLAG  ); 
      return *u.flag;   
    }
    inline values::Scalar &scalar() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.scalar=new values::Scalar; type=TOK_SCALAR;
      }
      else
	assert(type==TOK_SCALAR  ); 
      return *u.scalar;   
    }
    inline values::Vector &vector() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.vector=new values::Vector; type=TOK_VECTOR;
      }
      else
	assert(type==TOK_VECTOR  ); 
      return *u.vector;   
    }
    inline values::Matrix &matrix() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.matrix=new values::Matrix; type=TOK_MATRIX;
      }
      else
	assert(type==TOK_MATRIX  ); 
      return *u.matrix;   
    }
    inline values::String &string() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.string=new values::String; type=TOK_STRING;
      }
      else
	assert(type==TOK_STRING  ); 
      return *u.string;   
    }
    inline solve::Operand<values::Flag>   &op_flag()   
    {
      if(type==TOK_INVALID_ID)
      {
	u.op_flag=new solve::Operand<values::Flag>(); type=TOK_OP_FLAG;
      }
      else
	assert(type==TOK_OP_FLAG  ); 
      return *u.op_flag;   
    }
    inline solve::Operand<values::Scalar> &op_scalar() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.op_scalar=new solve::Operand<values::Scalar>(); 
	type=TOK_OP_SCALAR;
      }
      else
	assert(type==TOK_OP_SCALAR  ); 
      return *u.op_scalar;   
    }
    inline solve::Operand<values::Vector> &op_vector() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.op_vector=new solve::Operand<values::Vector>(); 
	type=TOK_OP_VECTOR;
      }
      else
	assert(type==TOK_OP_VECTOR  ); 
      return *u.op_vector;   
    }
    inline solve::Operand<values::Matrix> &op_matrix() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.op_matrix=new solve::Operand<values::Matrix>(); 
	type=TOK_OP_MATRIX;
      }
      else
	assert(type==TOK_OP_MATRIX  ); 
      return *u.op_matrix;   
    }
    inline solve::Operand<values::String> &op_string() 
    {
      if(type==TOK_INVALID_ID)
      {
	u.op_string=new solve::Operand<values::String>(); type=TOK_OP_STRING;
      }
      else
	assert(type==TOK_OP_STRING  ); 
      return *u.op_string;   
    }

    // access functions by references
    inline values::Flag   get_flag() const   
    {
      assert(type==TOK_FLAG  ); 
      return *u.flag;   
    }
    inline values::Scalar get_scalar() const 
    {
      assert(type==TOK_SCALAR  ); 
      return *u.scalar;   
    }
    inline values::Vector get_vector() const 
    {
      assert(type==TOK_VECTOR  ); 
      return *u.vector;   
    }
    inline values::Matrix get_matrix() const 
    {
      assert(type==TOK_MATRIX  ); 
      return *u.matrix;   
    }
    inline values::String get_string() const 
    {
      assert(type==TOK_STRING  ); 
      return *u.string;   
    }
    inline solve::Operand<values::Flag>   *get_op_flag() const   
    {
      assert(type==TOK_OP_FLAG  ); 
      return u.op_flag;   
    }
    inline solve::Operand<values::Scalar> *get_op_scalar() const 
    {
      assert(type==TOK_OP_SCALAR  ); 
      return u.op_scalar;   
    }
    inline solve::Operand<values::Vector> *get_op_vector() const 
    {
      assert(type==TOK_OP_VECTOR  ); 
      return u.op_vector;   
    }
    inline solve::Operand<values::Matrix> *get_op_matrix() const 
    {
      assert(type==TOK_OP_MATRIX  ); 
      return u.op_matrix;   
    }
    inline solve::Operand<values::String> *get_op_string() const 
    {
      assert(type==TOK_OP_STRING  ); 
      return u.op_string;   
    }

    inline bool has_value() const { return type != TOK_INVALID_ID; }

    // delete stored token
    inline int consumed() {
      if( type == TOK_INVALID_ID ) return -1; // no value stored ?!?

      switch( type )
      {
	case TOK_FLAG:   assert( u.ptr != 0 ); delete u.flag;   break;
	case TOK_SCALAR: assert( u.ptr != 0 ); delete u.scalar; break;
	case TOK_VECTOR: assert( u.ptr != 0 ); delete u.vector; break;
	case TOK_MATRIX: assert( u.ptr != 0 ); delete u.matrix; break;
	case TOK_STRING: assert( u.ptr != 0 ); delete u.string; break;
	case TOK_OP_FLAG:   assert( u.ptr != 0 ); break;
	case TOK_OP_SCALAR: assert( u.ptr != 0 ); break;
	case TOK_OP_VECTOR: assert( u.ptr != 0 ); break;
	case TOK_OP_MATRIX: assert( u.ptr != 0 ); break;
	case TOK_OP_STRING: assert( u.ptr != 0 ); break;
	default: assert(0);
      };
      type = TOK_INVALID_ID;
      return 0;
    }

    //************
    // operators
    Token &operator=(const Token &t)
    {
      // is current type different from type of t
      if( (type != TOK_INVALID_ID) && (type != t.get_type()) )
	consumed();		// delete old value 

      switch( t.get_type() )
      {
	case TOK_INVALID_ID: break; // do nothing
	case TOK_FLAG:   flag()   = t.get_flag();   break;
	case TOK_SCALAR: scalar() = t.get_scalar(); break;
	case TOK_VECTOR: vector() = t.get_vector(); break;
	case TOK_MATRIX: matrix() = t.get_matrix(); break;
	case TOK_STRING: string() = t.get_string(); break;
	case TOK_OP_FLAG:   u.op_flag   = t.get_op_flag();   break;
	case TOK_OP_SCALAR: u.op_scalar = t.get_op_scalar(); break;
	case TOK_OP_VECTOR: u.op_vector = t.get_op_vector(); break;
	case TOK_OP_MATRIX: u.op_matrix = t.get_op_matrix(); break;
	case TOK_OP_STRING: u.op_string = t.get_op_string(); break;
	default: assert(0);
      };
      type = t.get_type();
      return *this;
    }


    //***************************
    // constuctors/destructors 

    Token() : type(TOK_INVALID_ID) { u.ptr = 0; }
    // copy constructor
    Token(const Token &t) : type(TOK_INVALID_ID)
    { 
      u.ptr = 0;
      switch( t.get_type() )
      {
	case TOK_INVALID_ID: break; // do nothing
	case TOK_FLAG:   flag()   = t.get_flag();   break;
	case TOK_SCALAR: scalar() = t.get_scalar(); break;
	case TOK_VECTOR: vector() = t.get_vector(); break;
	case TOK_MATRIX: matrix() = t.get_matrix(); break;
	case TOK_STRING: string() = t.get_string(); break;
	case TOK_OP_FLAG:   u.op_flag   = t.get_op_flag();   break;
	case TOK_OP_SCALAR: u.op_scalar = t.get_op_scalar(); break;
	case TOK_OP_VECTOR: u.op_vector = t.get_op_vector(); break;
	case TOK_OP_MATRIX: u.op_matrix = t.get_op_matrix(); break;
	case TOK_OP_STRING: u.op_string = t.get_op_string(); break;
	default: assert(0);
      };
      type = t.get_type();
    }

    ~Token() { consumed(); }
  };

  // virtual class to resolve identifiers in expressions
  class Identifier_Resolver 
  {
  public:
    virtual Token get_identifier( std::string s ) = 0;
  };

  // Info opbject that is passed to the parser
  class parser_info
  {
  public:
    typedef std::list<Identifier_Resolver*> id_resolver_type;
    id_resolver_type id_resolver;	// used by scanner
    message::Message_Reporter msg;	// used by scanner & parser
    Token result;			// needed by parser
    
    parser_info( message::Message_Consultant *consultant )
      : msg(consultant)
    {}
  };

  // starts the expression parser
  Token get_expression( parser_info &info );
  solve::Operand<values::Scalar> &get_scalar_expression( parser_info &info );
  solve::Operand<values::Vector> &get_vector_expression( parser_info &info );
}

#endif
