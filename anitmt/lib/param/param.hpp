/*****************************************************************************/
/**   Parameter handling system                                   	    **/
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

#ifndef __lib_Param__
#define __lib_Param__

#include <list>

#include <solve/operand.hpp>
#include <message/message.hpp>

//wrap to old system
#include <par/params.hpp>

namespace param
{
  class Parameter_Source;
  class Parameter_Manager;
  class Parameter_Interface;
}

namespace param
{
  class Parameter_Source {
  public:
    virtual ~Parameter_Source() {}
  protected:
    Parameter_Manager *manager;
  private:
    friend class Parameter_Manager;
    void set_manager( Parameter_Manager *manager );
    virtual int read_parameters() = 0;
  };

  class Parameter_Manager : public message::Message_Reporter {
  public:
    Parameter_Manager( Parameter_Source &start_source, message::Message_Consultant &consultant );
    ~Parameter_Manager();
    //! start reading the parameters
    //! \return 0 means no errors
    int read_parameters();

    //Wrapping interface to old parameter system
    anitmt::Animation_Parameters old_sys; // all options
  private:
    Parameter_Source &start_source;
    std::list<Parameter_Source*> sources;
    std::list<Parameter_Interface*> interfaces;

    friend class Parameter_Source;
    void set_int_parameter( std::string name, int value );

    friend class Parameter_Interface;
    void register_int_parameter( solve::Operand<int>* param, std::string name, 
				 std::string description, int default_value,
				 bool needed );
  };

  class Parmeter_Interface {
    Parameter_Manager *manager;
  public:
    inline void int_parameter( solve::Operand<int>* param, std::string name, 
			       std::string description, int default_value,
			       bool needed );
  };

  class Commandline_Parameter_Source : public Parameter_Source {
  public:
    Commandline_Parameter_Source( int argc, char *argv[], char *envp[] );
  private:
    virtual int read_parameters();
    //Wrapping interface to old parameter system
    anitmt::Command_Line cmd;
  };
}

#include "param_inline.cpp"

#endif
