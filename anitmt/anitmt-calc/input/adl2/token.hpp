/*****************************************************************************/
/**   This file offers a complex token class for storing all types          **/
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

#ifndef __anitmt_input_adl_token__
#define __anitmt_input_adl_token__

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <message/message.hpp>

#include <proptree/property.hpp>

namespace anitmt
{
  namespace adlparser
  {
    class Token;

    //*********************************************************************
    // Meta_Operand: stores operands and allows assigning operands without
    //               creating an Store_Operater object in between
    //*********************************************************************
      
    // copies the operand pointer when assigned to another Meta_Operand
    class Meta_Operand_Flag {
      Token *tok; void *info;
    public:
      inline Meta_Operand_Flag &operator=( solve::Operand<values::Flag> 
					   &operand );
      inline solve::Operand<values::Flag> &operator()();
      Meta_Operand_Flag( Token *t, void *i );
    };
    class Meta_Operand_Scalar {
      Token *tok; void *info;
    public:
      inline Meta_Operand_Scalar &operator=( solve::Operand<values::Scalar> 
					     &operand );
      inline solve::Operand<values::Scalar> &operator()();
      Meta_Operand_Scalar( Token *t, void *i );
    };
    class Meta_Operand_Vector {
      Token *tok; void *info;
    public:
      inline Meta_Operand_Vector &operator=( solve::Operand<values::Vector> 
					     &operand );
      inline solve::Operand<values::Vector> &operator()();
      Meta_Operand_Vector( Token *t, void *i );
    };
    class Meta_Operand_Matrix {
      Token *tok; void *info;
    public:
      inline Meta_Operand_Matrix &operator=( solve::Operand<values::Matrix> 
					     &operand );
      inline solve::Operand<values::Matrix> &operator()();
      Meta_Operand_Matrix( Token *t, void *i );
    };
    class Meta_Operand_String {
      Token *tok; void *info;
    public:
      inline Meta_Operand_String &operator=( solve::Operand<values::String> 
					     &operand );
      inline solve::Operand<values::String> &operator()();
      Meta_Operand_String( Token *t, void *i );
    };


    //******************************
    // Token: Type for lexer tokens
    //******************************

    // YACC/LEX token type
    class Token
    {
      int type;
      union{
	void *ptr;

	std::string *identifier;
	values::Flag   *flag;
	values::Scalar *scalar;
	values::Vector *vector;
	values::Matrix *matrix;
	values::String *string;
	solve::Operand<values::Flag>   *op_flag;
	solve::Operand<values::Scalar> *op_scalar;
	solve::Operand<values::Vector> *op_vector;
	solve::Operand<values::Matrix> *op_matrix;
	solve::Operand<values::String> *op_string;
	proptree::Type_Property<values::Flag>   *prop_flag;
	proptree::Type_Property<values::Scalar> *prop_scalar;
	proptree::Type_Property<values::Vector> *prop_vector;
	proptree::Type_Property<values::Matrix> *prop_matrix;
	proptree::Type_Property<values::String> *prop_string;

	// these types make problems with the functions below
	//std::string *std_string; // might be useful for anything
	//double dval;
      }u;
      // used by constructor and operator=
      friend inline void _internal_assign( Token &dest, const Token &src );
    public:
      //*********************************************************************
      // Token member funktions
      //*********************************************************************
      
      inline int get_type() const;

      // access functions by references
      inline std::string &identifier();
      inline values::Flag   &flag();
      inline values::Scalar &scalar();
      inline values::Vector &vector();
      inline values::Matrix &matrix();
      inline values::String &string();
      inline solve::Operand<values::Flag>   &op_flag  ( void* info );
      inline solve::Operand<values::Scalar> &op_scalar( void* info );
      inline solve::Operand<values::Vector> &op_vector( void* info );
      inline solve::Operand<values::Matrix> &op_matrix( void* info );
      inline solve::Operand<values::String> &op_string( void* info );
      inline Meta_Operand_Flag   meta_op_flag  ( void* info );
      inline Meta_Operand_Scalar meta_op_scalar( void* info );
      inline Meta_Operand_Vector meta_op_vector( void* info );
      inline Meta_Operand_Matrix meta_op_matrix( void* info );
      inline Meta_Operand_String meta_op_string( void* info );
      inline proptree::Type_Property<values::Flag>   &prop_flag();
      inline proptree::Type_Property<values::Scalar> &prop_scalar();
      inline proptree::Type_Property<values::Vector> &prop_vector();
      inline proptree::Type_Property<values::Matrix> &prop_matrix();
      inline proptree::Type_Property<values::String> &prop_string();

      // readonly access functions
      inline std::string get_identifier() const;
      inline values::Flag   get_flag() const;
      inline values::Scalar get_scalar() const;
      inline values::Vector get_vector() const;
      inline values::Matrix get_matrix() const;
      inline values::String get_string() const;
      inline solve::Operand<values::Flag>   *get_op_flag() const;
      inline solve::Operand<values::Scalar> *get_op_scalar() const;
      inline solve::Operand<values::Vector> *get_op_vector() const;
      inline solve::Operand<values::Matrix> *get_op_matrix() const;
      inline solve::Operand<values::String> *get_op_string() const;
      inline proptree::Type_Property<values::Flag>   *get_prop_flag() const;
      inline proptree::Type_Property<values::Scalar> *get_prop_scalar() const;
      inline proptree::Type_Property<values::Vector> *get_prop_vector() const;
      inline proptree::Type_Property<values::Matrix> *get_prop_matrix() const;
      inline proptree::Type_Property<values::String> *get_prop_string() const;
      // set operand without creating a new object
      inline void set_op_flag  ( solve::Operand<values::Flag> &f );
      inline void set_op_scalar( solve::Operand<values::Scalar> &f );
      inline void set_op_vector( solve::Operand<values::Vector> &f );
      inline void set_op_matrix( solve::Operand<values::Matrix> &f );
      inline void set_op_string( solve::Operand<values::String> &f );
      inline void set_prop_flag  ( proptree::Type_Property<values::Flag> &f );
      inline void set_prop_scalar( proptree::Type_Property<values::Scalar>&f );
      inline void set_prop_vector( proptree::Type_Property<values::Vector>&f );
      inline void set_prop_matrix( proptree::Type_Property<values::Matrix>&f );
      inline void set_prop_string( proptree::Type_Property<values::String>&f );
      // is a value stored in that token?
      inline bool has_value() const;

      // delete stored token
      inline int consumed();

      // returns this token (needed by parser)
      inline Token &tok();

      //************
      // operators
      Token &operator=(const Token &t);


      //***************************
      // constuctors/destructors

      Token();
      // copy constructor
      Token(const Token &t);

      ~Token();
    };

  }
}

//include inline implementation
#include "token_inline.cpp"

#endif
