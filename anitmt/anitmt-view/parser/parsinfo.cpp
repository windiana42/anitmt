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

#include "parsinfo.hpp"
#include "parser.hpp"
#include "vals.hpp"
#include "error.hpp"

#include "constant.hpp"

#include <stdio.h>
#include <cmath>

namespace parser{
  values::Valtype Constants::get( Parser &p, std::string word ) 
    const throw(unknown, Parser_Error){
    if( word == "pi" )    return const_PI;
    if( word == "e" )     return const_E;
    if( word == "true" )  return 1;
    if( word == "yes" )   return 1;
    if( word == "on" )    return 1;
    if( word == "false" ) return 0;
    if( word == "no" )    return 0;
    if( word == "off" )   return 0;
    if( word == "x" )     return vect::vector3(1,0,0);
    if( word == "y" )     return vect::vector3(0,1,0);
    if( word == "z" )     return vect::vector3(0,0,1);
    
    throw unknown();
  }

  values::Valtype Functions::get( Parser &p, std::string word ) 
    const throw(unknown, Parser_Error){
    //*****************
    // Scalar functions

    if( word == "sin" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::sin(par.get_scalar()) );
      }
    if( word == "cos" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::cos(par.get_scalar()) );
      }
    if( word == "tan" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::tan(par.get_scalar()) );
      }
    if( word == "asin" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::asin(par.get_scalar()) );
      }
    if( word == "acos" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::acos(par.get_scalar()) );
      }
    if( word == "atan" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::atan(par.get_scalar()) );
      }
    if( word == "atan2" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getScalar();
	p.expect(",");
	values::Valtype par2 = p.getScalar();
	p.expect(")");
	return values::Valtype( std::atan2(par1.get_scalar(),
					   par2.get_scalar()) );
      }
    if( word == "abs" ) 
      {
	p.expect("(");
	values::Valtype par = p.getTerm();
	p.expect(")");

	if( par.get_type() == values::scalar )
	  return values::Valtype( std::abs(par.get_scalar()) );

	if( par.get_type() == values::vector )
	  return values::Valtype( par.get_vector().length() );

	values::error( errTypeMismatch, 
		       "abs("+values::type2str(par.get_type())+
		       ") is not defined" );
      }
    if( word == "ceil" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::ceil(par.get_scalar()) );
      }
    if( word == "div" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getScalar();
	p.expect(",");
	values::Valtype par2 = p.getScalar();
	p.expect(")");
	return int( (par1 / par2).get_scalar() );
      }
    if( word == "degrees" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( par.get_scalar() / const_PI * 180 );
      }
    if( word == "exp" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::exp(par.get_scalar()) );
      }
    if( word == "floor" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::floor(par.get_scalar()) );
      }
    if( word == "int" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( int(par.get_scalar()) );
      }
    if( word == "log" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::log(par.get_scalar()) );
      }
    if( word == "max" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getTerm();
	p.expect(",");
	values::Valtype par2 = p.getTerm();
	p.expect(")");
	return  (par1 > par2) ? par1 : par2;
      }
    if( word == "min" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getTerm();
	p.expect(",");
	values::Valtype par2 = p.getTerm();
	p.expect(")");
	return  (par1 < par2) ? par1 : par2;
      }
    if( word == "mod" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getTerm();
	p.expect(",");
	values::Valtype par2 = p.getTerm();
	p.expect(")");
	return par1 % par2; 
      }
    if( word == "pow" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getTerm();
	p.expect(",");
	values::Valtype par2 = p.getTerm();
	p.expect(")");
	return par1 ^ par2; 
      }
    if( word == "radians" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( par.get_scalar() / 180 * const_PI  );
      }
    if( word == "sqrt" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::sqrt(par.get_scalar()) );
      }

    //*****************
    // Vector functions

    if( word == "vnormalize" ) 
      {
	p.expect("(");
	values::Valtype par = p.getVector();
	p.expect(")");
	return values::Valtype( par.get_vector().normalize() );
      }
    if( word == "vlength" ) 
      {
	p.expect("(");
	values::Valtype par = p.getVector();
	p.expect(")");
	return values::Valtype( par.get_vector().length() );
      }
    if( word == "vdot" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getVector();
	p.expect(",");
	values::Valtype par2 = p.getVector();
	p.expect(")");
	return par1*par2;
      }
    if( word == "vcross" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getVector();
	p.expect(",");
	values::Valtype par2 = p.getVector();
	p.expect(")");
	return par1.get_vector().cross( par2.get_vector() );
      }
    if( word == "vrotate" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getVector();
	p.expect(",");
	values::Valtype par2 = p.getVector();
	p.expect(")");

	par2 = par2 / 180 * const_PI; // convert to radians

	return rotate( par2.get_vector() ) * par1.get_vector();
      }
    if( word == "vaxis_rotate" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getVector();
	p.expect(",");
	values::Valtype par2 = p.getVector();
	p.expect(",");
	values::Valtype par3 = p.getScalar();
	p.expect(")");
	
	par3 = par3 / 180 * const_PI; // convert to radians

	return rotate_around( par2.get_vector(), par3.get_scalar() ) 
	  * par1.get_vector();
      }

    //*****************
    // String functions

    if( word == "asc" ) 
      {
	p.expect("(");
	values::Valtype par = p.getString();
	p.expect(")");
	return values::Valtype( par.get_string()[0] );
      }
    if( word == "chr" ) 
      {
	p.expect("(");
	values::Valtype par = p.getScalar();
	p.expect(")");
	return values::Valtype( std::string(1, char(par.get_scalar()) ) );
      }
    if( word == "concat" ) 
      {
	values::Valtype res = "";
	p.expect("(");
	do{
	  res += p.getString();
	  word = p.get_Word();
	}while( word == "," );
	p.unget_Word();
	p.expect(")");

	return res;
      }
    if( word == "str" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getScalar();
	p.expect(",");
	values::Valtype par2 = p.getScalar();
	p.expect(",");
	values::Valtype par3 = p.getScalar();
	p.expect(")");
	
	int len     = int(par2.get_scalar());
	int decimal = int(par3.get_scalar());
	char *str = (char*) malloc( abs(len) + 1 + 50 ); // +50 is to be sure
	if( len < 0 )
	  sprintf(str, "%0*.*lf", abs(len), decimal, par1.get_scalar() );
	else
	  sprintf(str, "%*.*lf", abs(len), decimal, par1.get_scalar() );
  
	return str;
      }

    if( word == "strcmp" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getString();
	p.expect(",");
	values::Valtype par2 = p.getString();
	p.expect(")");
	
	return strcmp( par1.get_string().c_str(), par2.get_string().c_str() );
      }

    if( word == "strlen" ) 
      {
	p.expect("(");
	values::Valtype par = p.getString();
	p.expect(")");
	
	return double(strlen( par.get_string().c_str() ));
      }

    if( word == "strlwr" ) 
      {
	p.expect("(");
	std::string par = p.getString().get_string();
	p.expect(")");
	
	for( int i=0; i<par.size(); i++ )
	  {
	    par[i] = tolower( par[i] );
	  }

	return par;
      }
    if( word == "substr" ) 
      {
	p.expect("(");
	values::Valtype par1 = p.getString();
	p.expect(",");
	values::Valtype par2 = p.getScalar();
	p.expect(",");
	values::Valtype par3 = p.getScalar();
	p.expect(")");
	
	return par1.get_string().substr( par2.get_scalar(),par3.get_scalar() );
      }
    if( word == "strupr" ) 
      {
	p.expect("(");
	std::string par = p.getString().get_string();
	p.expect(")");
	
	for( int i=0; i<par.size(); i++ )
	  {
	    par[i] = toupper( par[i] );
	  }

	return par;
      }
    if( word == "val" ) 
      {
	p.expect("(");
	values::Valtype par = p.getString();
	p.expect(")");
	
	return atof( par.get_string().c_str() );
      }
    
    throw unknown();
  }
}
