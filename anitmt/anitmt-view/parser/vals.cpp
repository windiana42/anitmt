/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#include "vals.hpp"
#include "constant.hpp"

#include <cmath>
#include <strstream>

namespace values{

  std::string num2str( double num ){
    char buf[200];

    std::ostrstream str(buf,200);

    str << num;

    return buf;
  }

  /******************/
  /* Error messages */
  /******************/
  
  Type_Error::Type_Error( int number, std::string message )
    : err::Basic_Error( number, message ){
  }

  void error( int number, std::string message ) throw( Type_Error ){
    throw Type_Error( number, message );
  }

  /***********/
  /* datatyp */
  /***********/

  std::string type2str( data_types t ){
    if( t == scalar) return("scalar");
    if( t == vector) return("vector");
    if( t == string) return("string");
    if( t == undefined) return("<undefined type>");
    return("<unknown type>");
  }

  datatyp::datatyp( data_types t ) : type(t) {}
  datatyp::~datatyp() {}
  

  //****************
  //* undefined data
 
  datatyp *undef_data::get_copy() const{
    return new undef_data();
  }

  std::string undef_data::get_as_string(){
    return "<undefined>";
  }
  void undef_data::operator>>( std::ostream &os ) const{
    os << "<undefined>";
  }


  undef_data::undef_data()
    : datatyp(undefined){
  }

  //*************
  //* scalar data
 
  double scalar_data::get() const{
    return x;
  }
  void scalar_data::set( double v ){
    x = v;
  }
  datatyp *scalar_data::get_copy() const{
    return new scalar_data( x );
  }

  std::string scalar_data::get_as_string(){
    std::string str;
    str += num2str(get());
    return str;
  }
  void scalar_data::operator>>( std::ostream &os ) const{
    os << get();
  }

  scalar_data::scalar_data( double v )
    : datatyp(scalar){
    set( v );
  }

  //*************
  //* vector data

  vect::vector3 vector_data::get() const{
    return x;
  }

  void vector_data::set( vect::vector3 v ){
    x = v;
  }

  datatyp *vector_data::get_copy() const{
    return new vector_data( x );
  }

  std::string vector_data::get_as_string(){
    std::string str = "<";
    str += num2str(x.coord[0]);
    str += ',';
    str += num2str(x.coord[1]);
    str += ',';
    str += num2str(x.coord[2]);
    str += '>';
    return str;
  }
  void vector_data::operator>>( std::ostream &os ) const{
    os << "<" << x.coord[0] << "," << x.coord[1] << "," << x.coord[2] << ">";
  }

  vector_data::vector_data( vect::vector3 v )
    : datatyp(vector){
    set( v );
  }

  //*************
  //* string data
 
  std::string string_data::get() const{
    return x;
  }
  void string_data::set( std::string s ){
    x = s;
  }
  datatyp *string_data::get_copy() const{
    return new string_data( x );
  }

  std::string string_data::get_as_string(){
    return "\"" + x + "\"";
  }

  void string_data::operator>>( std::ostream &os ) const{
    os << '"' << x << '"';
  }

  string_data::string_data( std::string s )
    : datatyp(string){
    set( s );
  }

  /*************/
  /* operators */
  /*************/

  double get_scalar( const datatyp& op ){
    return ((scalar_data*)&op)->get();
  }
  double get_scalar( datatyp* op ){
    return ((scalar_data*)op)->get();
  }

  vect::vector3 get_vector( const datatyp& op ){
    return ((vector_data*)&op)->get();
  }
  vect::vector3 get_vector( datatyp *op ){
    return ((vector_data*)op)->get();
  }

  std::string get_string( const datatyp& op ){
    return ((string_data*)&op)->get();
  }
  std::string get_string( datatyp* op ){
    return ((string_data*)op)->get();
  }

  std::ostream& operator<<( std::ostream &os, const datatyp* op1 ){
    (*op1) >> os;
    return os;
  }

  datatyp *operator+( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return new scalar_data( get_scalar(op1) + 
			      get_scalar(op2) );
      
    if( (op1.type == string) && (op2->type == string) )
      return new string_data( get_string(op1) + 
			      get_string(op2) );
      
    if( (op1.type == vector) && (op2->type == vector) )
      return new vector_data( get_vector(op1) + 
			      get_vector(op2) );

    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" + "+type2str(op2->type));
    return 0;
  }

  datatyp *operator-( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return new scalar_data( get_scalar(op1) - 
			      get_scalar(op2) );
      
    if( (op1.type == vector) && (op2->type == vector) )
      return new vector_data( get_vector(op1) - 
			      get_vector(op2) );

    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" - "+type2str(op2->type));
    return 0;
  }

  datatyp *operator*( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return new scalar_data( get_scalar(op1) * 
			      get_scalar(op2) );
      
    if( (op1.type == scalar) && (op2->type == vector) )
      return new vector_data( get_scalar(op1) * 
			      get_vector(op2) );

    if( (op1.type == vector) && (op2->type == scalar) )
      return new vector_data( get_scalar(op2) * 
			      get_vector(op1) );

    if( (op1.type == vector) && (op2->type == vector) )
      return new scalar_data( vect::dot( get_vector(op1),
					 get_vector(op2) ) );
      
    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" * "+type2str(op2->type));
    return 0;
  }

  datatyp *operator/( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return new scalar_data( get_scalar(op1) / 
			      get_scalar(op2) );
      
    if( (op1.type == scalar) && (op2->type == vector) )
      return new vector_data( get_scalar(op1) / 
			      get_vector(op2) );

    if( (op1.type == vector) && (op2->type == scalar) )
      return new vector_data( get_vector(op1) /
			      get_scalar(op2) );

    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" / "+type2str(op2->type));
    return 0;
  }

  datatyp *operator%( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      {
	double res = get_scalar(op1) / get_scalar(op2);
	res -= int(res);
	res *= get_scalar(op2);
	  
	return new scalar_data( res );
      }
      
    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" % "+type2str(op2->type));
    return 0;
  }

  bool operator!( const datatyp& op ){
    return op == 0;
  }

  bool operator==( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == string) && (op2->type == string) )
      return get_string(op1) == get_string(op2);

    return abs( op1-op2 ) < const_EPSILON;
  }

  bool operator<( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return get_scalar(op1) < get_scalar(op2);

    if( (op1.type == vector) && (op2->type == vector) )
      return get_vector(op1).length2() < get_vector(op2).length2();

    if( (op1.type == string) && (op2->type == string) )
      return get_string(op1) < get_string(op2);
      
    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" < "+type2str(op2->type));
    return false;
  }

  bool operator>( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return get_scalar(op1) > get_scalar(op2);
      
    if( (op1.type == vector) && (op2->type == vector) )
      return get_vector(op1).length2() > get_vector(op2).length2();

    if( (op1.type == string) && (op2->type == string) )
      return get_string(op1) > get_string(op2);

    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" > "+type2str(op2->type));
    return false;
  }

  bool operator||( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return get_scalar(op1) || get_scalar(op2);
      
    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" || "+type2str(op2->type));
    return false;
  }

  bool operator&&( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return get_scalar(op1) && get_scalar(op2);
      
    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" && "+type2str(op2->type));
    return false;
  }


  datatyp *operator^( const datatyp& op1, datatyp *op2 ){
    if( (op1.type == scalar) && (op2->type == scalar) )
      return new scalar_data( std::pow( get_scalar(op1),
					get_scalar(op2) ) );
      
    error(201,"Cannot calculate: "+
	  type2str(op1.type)+" ^ "+type2str(op2->type));
    return 0;
  }

  datatyp *pow( const datatyp& op1, datatyp *op2 ){
    return op1 ^ op2;
  }

  datatyp *abs( const datatyp& op1 ){
    if( (op1.type == scalar) )
      return new scalar_data( std::abs( get_scalar(op1) ) );
      
    if( (op1.type == vector) )
      return new scalar_data( get_vector(op1).length() );

    error(201,"Cannot calculate: abs( "+
	  type2str(op1.type)+" ) ");
    return 0;
  }

  datatyp *sqrt( const datatyp& op1 ){
    if( (op1.type == scalar) )
      return new scalar_data( std::sqrt( get_scalar(op1) ) );
      
    error(201,"Cannot calculate: abs( "+
	  type2str(op1.type)+" ) ");
    return 0;
  }

  /********************************************************/
  /* Valtype: reference interface to pointer type datatyp */ 
  /********************************************************/  
  
  datatyp *Valtype::copy_data() const{
    return x->get_copy();
  }
  data_types Valtype::get_type() const{
    return x->type;
  }
  
  Valtype& Valtype::operator=(const Valtype &op){
    delete x;
    x = op.copy_data();
    return *this;
  }
  Valtype& Valtype::operator+=(const Valtype &op){
    datatyp *y = (*x) + op.x;
    delete x;
    x = y;
    return *this;
  }
  Valtype& Valtype::operator-=(const Valtype &op ){
    datatyp *y = (*x) - op.x;
    delete x;
    x = y;
    return *this;
  }
  Valtype& Valtype::operator*=(const Valtype &op ){
    datatyp *y = (*x) * op.x;
    delete x;
    x = y;
    return *this;
  }
  Valtype& Valtype::operator/=(const Valtype &op ){
    datatyp *y = (*x) / op.x;
    delete x;
    x = y;
    return *this;
  }
  Valtype& Valtype::operator%=(const Valtype &op ){
    datatyp *y = (*x) % op.x;
    delete x;
    x = y;
    return *this;
  }
  Valtype& Valtype::operator^=(const Valtype &op ){
    datatyp *y = (*x) ^ op.x;
    delete x;
    x = y;
    return *this;
  }
  

  std::string Valtype::get_as_string(){
    char buf[200];

    std::strstream str(buf,100);

    str << *this;

    std::string out;
    getline( str, out );
    return out;
  }

  double Valtype::get_scalar() const{
    if( get_type() != scalar )
      error( 202, "I thought this would be a scalar" );
    return values::get_scalar(x);
  }
  vect::vector3 Valtype::get_vector() const{
    if( get_type() != vector )
      error( 202, "I thought this would be a vector" );
    return values::get_vector(x);
  }
  std::string Valtype::get_string() const{
    if( get_type() != string )
      error( 202, "I thought this would be a string" );
    return values::get_string(x);
  }

  Valtype::Valtype(){
    x = new undef_data(); // undefined data
  }
  Valtype::Valtype( datatyp *data ){
    x = data;
  }
  Valtype::Valtype( const Valtype &op ){
    x = op.copy_data();
  }

  Valtype::Valtype( int v ){
    x = new scalar_data( v );
  }
  Valtype::Valtype( double v ){
    x = new scalar_data( v );
  }
  Valtype::Valtype( vect::vector3 v ){
    x = new vector_data( v );
  }
  Valtype::Valtype( std::string s ){
    x = new string_data( s );
  }
  Valtype::Valtype( char *s ){
    x = new string_data( s );
  }

  Valtype::~Valtype(){ 
    delete x; 
  }

  /*******************************************/
  /* value operations (uses data operations) */
  /*******************************************/

  std::ostream& operator<<( std::ostream &os, const Valtype& op1 ){
    return os << op1.x;
  }

  Valtype operator+(const Valtype& op1, const Valtype& op2){
    return Valtype( (*(op1.x)) + op2.x );
  }
  Valtype operator-(const Valtype& op1, const Valtype& op2 ){
    return Valtype( (*(op1.x)) - op2.x );
  }
  Valtype operator*(const Valtype& op1, const Valtype& op2 ){
    return Valtype( (*(op1.x)) * op2.x );
  }
  Valtype operator/(const Valtype& op1, const Valtype& op2 ){
    return Valtype( (*(op1.x)) / op2.x );
  }
  Valtype operator%(const Valtype& op1, const Valtype& op2 ){
    return Valtype( (*(op1.x)) % op2.x );
  }
  
  Valtype operator-(const Valtype& op1){
    return -1 * op1;
  }
  bool operator!(const Valtype& op1){
    return !(*(op1.x));
  }

  bool operator==(const Valtype& op1, const Valtype& op2 ){
    return (*(op1.x)) == op2.x;
  }
  bool operator!=(const Valtype& op1, const Valtype& op2 ){
    return !((*(op1.x)) == op2.x);
  }
  bool operator<(const Valtype& op1, const Valtype& op2 ){
    return (*(op1.x)) < op2.x ;
  }
  bool operator>(const Valtype& op1, const Valtype& op2 ){
    return (*(op1.x)) > op2.x;
  }
  bool operator<=(const Valtype& op1, const Valtype& op2 ){
    return !((*(op1.x)) > op2.x);
  }
  bool operator>=(const Valtype& op1, const Valtype& op2 ){
    return !((*(op1.x)) < op2.x);
  }
  bool operator||(const Valtype& op1, const Valtype& op2 ){
    return (*(op1.x)) || op2.x;
  }
  bool operator&&(const Valtype& op1, const Valtype& op2 ){
    return (*(op1.x)) && op2.x;
  }
  
  Valtype operator^(const Valtype& op1, const Valtype& op2 ){
    return Valtype( values::pow( *(op1.x), op2.x ) );
  }
  Valtype pow(const Valtype& op1, const Valtype& op2 ){
    return Valtype( values::pow( *(op1.x), op2.x ) );
  }
  Valtype sqrt(const Valtype& op1 ){
    return Valtype( values::sqrt( *(op1.x) ) );
  }
  Valtype abs(const Valtype& op1 ){
    return Valtype( values::abs( *(op1.x) ) );
  }
  Valtype sign(const Valtype& op1 ){
    if( op1 == 0 ) return 0;
    return op1 > 0 ? 1 :-1;
  }

  /******************************************/
  /* additional operations with other types */
  /******************************************/

  Valtype operator*(const vect::matrix4& op1, const Valtype& op2 ){
    return op1 * op2.get_vector();
  }

  Valtype cross(const Valtype& op1, const Valtype& op2 ){
    return cross( op1.get_vector(), op2.get_vector() );
  }
  
  Valtype norm(const Valtype& op1 ){
    return op1.get_vector().normalize();
  }

  Valtype length(const Valtype& op1 ){
    if( op1.get_type() == vector )
      return op1.get_vector().length();
    
    if( op1.get_type() == string )
      return int(op1.get_string().size());

    return -1;
  }

  Valtype angle(const Valtype& op1, const Valtype& op2 ){
    return get_angle( op1.get_vector(), op2.get_vector() );
  }
  
}
