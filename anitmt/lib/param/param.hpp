/*****************************************************************************/
/**   Parameter handling system                                   	    **/
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

#ifndef __lib_Param__
#define __lib_Param__

#include <list>

#include <solve/operand.hpp>
#include <message/message.hpp>

namespace param
{
  class Parameter_Input;
  class Parameter_Manager;
  class Parameter_Interface;
}

namespace param
{
  class Parameter_Source {
    Parameter_Manager *manager;

    friend class Parameter_Manager;
    void set_manager( Parameter_Manager *manager );
    virtual void read_parameters();
  public:
    virtual ~Parameter_Source() {}
  };

  class Parameter_Manager : public message::Message_Reporter {
    std::list<Parameter_Input*> inputs;
    std::list<Parameter_Interface*> interfaces;

    friend class Parameter_Source;
    void set_int_parameter( std::string name, int value );

    friend class Parameter_Interface;
    void register_int_parameter( solve::Operand<int>* param, std::string name, 
				 std::string description, int default_value,
				 bool needed );
  public:
    void read_parameters();

    Parameter_Manager( message::Message_Consultant consultant );
  };

  class Parmeter_Interface {
    Parameter_Manager *manager;
  public:
    inline void int_parameter( solve::Operand<int>* param, std::string name, 
			       std::string description, int default_value,
			       bool needed );
  };

  class Commandline_Parameter_Source {
    Parameter_Manager *manager;

    void set_manager( Parameter_Manager *manager );
    friend class Parameter_Manager;
  public:
  };
}

#include "param_inline.cpp"

#endif
