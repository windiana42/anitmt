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

#include "param.hpp"

#warning only implemented as wrapper for the old parameter system lib/

namespace param
{
  // *****************************************
  // Basic Parameter Source
  // *****************************************

  void Parameter_Source::set_manager( Parameter_Manager *m ) 
  {    
    manager = m;
  }

  // *****************************************
  // Parameter Manager
  // *****************************************

  Parameter_Manager::Parameter_Manager( Parameter_Source &source, message::Message_Consultant &consultant )
    : message::Message_Reporter(&consultant), start_source(source)
  {
    start_source.set_manager(this);
    sources.push_back( &start_source );
  }
  Parameter_Manager::~Parameter_Manager()
  {}


  //! start reading the parameters
  //! \return 0 means no errors
  int Parameter_Manager::read_parameters()
  {
    return start_source.read_parameters();
  }

  void Parameter_Manager::set_int_parameter( std::string name, int value ) 
  {
    //!!!
  }

  void Parameter_Manager::register_int_parameter( solve::Operand<int>* param, std::string name, 
				 std::string description, int default_value,
				 bool needed )
  {
    //!!!
  }

  // *****************************************
  // Commandline_Parameter_Source
  // *****************************************

  Commandline_Parameter_Source::Commandline_Parameter_Source( int argc, char *argv[], char *envp[] )
    //Wrapping interface to old parameter system
    : cmd( argc, argv, envp )
  {}

  int Commandline_Parameter_Source::read_parameters()
  {
    //Wrapping interface to old parameter system
    if(!manager->old_sys.Parse_Command_Line(&cmd) )
      return -1;
  
    if(!cmd.Check_Unused() )
      return -2;

    return 0;
  }
}
