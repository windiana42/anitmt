/*****************************************************************************/
/**   This file offers functions that tell availible basic operators	    **/
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

#ifndef __Funcgen_Basic_Operators__
#define __Funcgen_Basic_Operators__

#include <map>
#include <set>
#include <list>
#include <string>

namespace funcgen
{
  enum type{ none, boolean, result, operand, operand1, operand2, operand3, info };
  class Basic_Operator
  {
  public:
    bool is_function( std::string function ) const;
    //! get parameter types of function (first is return type)
    const std::list<type> &get_types( std::string function );
    //! get functions that are required
    const std::list<std::string> &get_required_functions() const;
    //! get real class name of the operator
    std::string get_real_name() const;
  private:
    friend class Available_Basic_Operators;
    std::map< std::string, std::list<type> > functions;
    std::list< std::string > required_functions;
    std::string real_name;
    void add_1_arg_fun( type return_type, std::string name, type arg1 );
    void add_2_arg_fun( type return_type, std::string name, type arg1, 
			type arg2 );
    void add_3_arg_fun( type return_type, std::string name, type arg1, 
			type arg2, type arg3 );
    void add_4_arg_fun( type return_type, std::string name, type arg1, 
			type arg2, type arg3, type arg4 );
  };
  class Available_Basic_Operators
  {
  public:
    Available_Basic_Operators();

    bool is_operator( std::string basic_operator ) const;
    const Basic_Operator &get_operator( std::string basic_operator );
  private:
    std::map<std::string,Basic_Operator> basic_operators;
  };
}

#endif
