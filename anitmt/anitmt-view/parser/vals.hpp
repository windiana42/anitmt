/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/*****************************************************************************/

#ifndef __aniTMT_values__
#define __aniTMT_values__

#include <string>
#include <map>

#include "error.hpp"
#include "vector.hpp"
#include "matrix.hpp"

namespace values{

  /******************/
  /* Error messages */
  /******************/
  
  class Type_Error : public err::Basic_Error{
  public:
    Type_Error( int number, std::string message );
  };

  void error( int number, std::string message ) throw(Type_Error); 

  /***********/
  /* datatyp */
  /***********/

  enum data_types { scalar, vector, string, undefined };

  std::string type2str( data_types t );

  class datatyp{
  public:
    data_types type;

    virtual datatyp *get_copy() const = 0;

    virtual std::string get_as_string() = 0;
    virtual void operator>>( std::ostream &os ) const = 0;

    datatyp( data_types t );
    virtual ~datatyp();
  };

  /*******************/
  /* data operations */
  /*******************/
  double get_scalar( const datatyp& op );
  double get_scalar( datatyp* op );
  vect::vector3 get_vector( const datatyp& op );
  vect::vector3 get_vector( datatyp *op );
  std::string get_string( const datatyp& op );
  std::string get_string( datatyp* op );

  std::ostream& operator<<( std::ostream &os, const datatyp* op1 );

  datatyp *operator+ ( const datatyp& op1, datatyp *op2 );
  datatyp *operator- ( const datatyp& op1, datatyp *op2 );
  datatyp *operator* ( const datatyp& op1, datatyp *op2 );
  datatyp *operator/ ( const datatyp& op1, datatyp *op2 );
  datatyp *operator% ( const datatyp& op1, datatyp *op2 );

  bool     operator! ( const datatyp& op1 );
  bool     operator==( const datatyp& op1, datatyp *op2 );
  bool     operator< ( const datatyp& op1, datatyp *op2 );
  bool     operator> ( const datatyp& op1, datatyp *op2 );
  bool     operator||( const datatyp& op1, datatyp *op2 );
  bool     operator&&( const datatyp& op1, datatyp *op2 );
  
  datatyp *operator^ ( const datatyp& op1, datatyp *op2 );
  datatyp *pow( const datatyp& op1, datatyp *op2 );
  datatyp *abs( const datatyp& op1 );
  datatyp *sqrt( const datatyp& op1 );
  
  //****************
  //* undefined data
 
  class undef_data : public datatyp{
  public:
    virtual datatyp *get_copy() const;
    virtual std::string get_as_string();
    virtual void operator>>( std::ostream &os ) const;

    undef_data();
  };

  //*************
  //* scalar data
 
  class scalar_data : public datatyp{
    double x;
  public:
    double get() const;
    void set( double v );
    virtual datatyp *get_copy() const;

    virtual std::string get_as_string();
    virtual void operator>>( std::ostream &os ) const;

    scalar_data( double v );
  };

  //*************
  //* vector data

  class vector_data : public datatyp{
    vect::vector3 x;
  public:
    vect::vector3 get() const;
    void set( vect::vector3 v );
    virtual datatyp *get_copy() const;

    virtual std::string get_as_string();
    virtual void operator>>( std::ostream &os ) const;

    vector_data( vect::vector3 v );
  };

  //*************
  //* string data
 
  class string_data : public datatyp{
    std::string x;
  public:
    std::string get() const;
    void set( std::string s );
    virtual datatyp *get_copy() const;

    virtual std::string get_as_string();
    virtual void operator>>( std::ostream &os ) const;

    string_data( std::string s );
  };

  /********************************************************/
  /* Valtype: reference interface to pointer type datatyp */ 
  /********************************************************/  

  class Valtype{
  public:
    datatyp *x;

    datatyp *copy_data() const;
    data_types get_type() const;

    Valtype& operator=(const Valtype &op);
    Valtype& operator+=(const Valtype &op );
    Valtype& operator-=(const Valtype &op );
    Valtype& operator*=(const Valtype &op );
    Valtype& operator/=(const Valtype &op );
    Valtype& operator%=(const Valtype &op );
    Valtype& operator^=(const Valtype &op );

    std::string get_as_string();

    double get_scalar() const;
    vect::vector3 get_vector() const;
    std::string get_string() const;

    //    operator bool(); 

    Valtype();
    Valtype( datatyp *data );
    Valtype( const Valtype &op );

    Valtype( int v );
    Valtype( double v );
    Valtype( vect::vector3 v );
    Valtype( std::string s );
    Valtype( char *s );

    virtual ~Valtype();
  };

  /********************/
  /* value operations */
  /********************/

  std::ostream& operator<<( std::ostream &os, const Valtype& op1 );

  Valtype operator+ (const Valtype& op1, const Valtype& op2 );
  Valtype operator- (const Valtype& op1, const Valtype& op2 );
  Valtype operator* (const Valtype& op1, const Valtype& op2 );
  Valtype operator/ (const Valtype& op1, const Valtype& op2 );
  Valtype operator% (const Valtype& op1, const Valtype& op2 );

  Valtype operator- (const Valtype& op1 );
  bool    operator! (const Valtype& op1 );

  bool    operator==(const Valtype& op1, const Valtype& op2 );
  bool    operator!=(const Valtype& op1, const Valtype& op2 );
  bool    operator< (const Valtype& op1, const Valtype& op2 );
  bool    operator> (const Valtype& op1, const Valtype& op2 );
  bool    operator<=(const Valtype& op1, const Valtype& op2 );
  bool    operator>=(const Valtype& op1, const Valtype& op2 );
  bool    operator||(const Valtype& op1, const Valtype& op2 );
  bool    operator&&(const Valtype& op1, const Valtype& op2 );

  Valtype operator^ (const Valtype& op1, const Valtype& op2 );
  Valtype pow(const Valtype& op1, const Valtype& op2 );
  Valtype sqrt(const Valtype& op1);
  Valtype abs(const Valtype& op1 );
  Valtype sign(const Valtype& op1 );

  /************************************************/
  /* additional operations with specialized types */
  /************************************************/

  // for vectors:
  Valtype operator*(const vect::matrix4& op1, const Valtype& op2 );
  Valtype cross(const Valtype& op1, const Valtype& op2 );
  Valtype norm(const Valtype& op1 );
  Valtype length(const Valtype& op1 );
  Valtype angle(const Valtype& op1, const Valtype& op2 );

  /*************************/
  /* automatic conversions */
  /*************************/
  class convert{
    Valtype val;
  public:
    operator bool() const{
      return !(val == 0);
    }
    operator double() const{
      return val.get_scalar();
    }
    operator int() const{
      return int(val.get_scalar());
    }
    operator std::string() const{
      return val.get_string();
    }
    operator vect::vector3() const{
      return val.get_vector();
    }

    convert( const Valtype &v ) : val(v){}
  };

}

#endif
