/*
 * val.hpp
 * 
 * This file offers basic data classes 
 * (Flag, Scalar, Vector, Matrix, String). 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser   (wwieser@gmx.de) 
 *                         and Martin Trautmann  (martintrautmann@gmx.de)
 * Report bugs and suggestions to wwieser@gmx.de .
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_values_val_HPP_
#define _NS_values_val_HPP_ 1

#include <config.h>

#include "neutral.hpp"
#include "flag.hpp"
#include "scalar.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include "string.hpp"

namespace values
{

const double PI = 3.14159265358979323846;

typedef vect::Neutral0 Neutral0;
typedef vect::Neutral1 Neutral1;
typedef vect::Flag Flag;
typedef vect::Scalar Scalar;
typedef vect::Vector<3> Vector;
typedef vect::Matrix<4,4> Matrix;
typedef vect::String String;


//#warning Valtype needed?  ##### FIXME
class Valtype
{
	public:
		enum Types { neutral0, neutral1, scalar, vector, matrix, string, flag };
	private:
		Types type;
	public:
		Valtype(Types t) : type(t) {}
		Types get_type() const {  return(type);  }
};

}  // namespace end 

#endif  /* _NS_values_val_HPP_ */
