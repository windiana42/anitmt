/*****************************************************************************/
/**   parameter handling						    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann, Wolfgang Wieser				    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de, wwieser@gmx.de			    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include <iostream>
#include <message/message.hpp>

#include "funcgen.hpp"


FuncgenParameters::FuncgenParameters(par::ParameterManager *manager) : 
    par::ParameterConsumer_Overloaded(manager)
{
  // Set up defaults for the parameters: 
#if YYDEBUG
  yydebug=0;
#endif
  stdebug=0;
  
  // Then, register the parameters: 
  SetSection(NULL);
#if YYDEBUG
  AddOpt("yydebug|d","enable parser debugging output (LOTS of text)",&yydebug);
#endif
  AddOpt("stdebug|s","dump structure debug output",&stdebug);
  
  AddParam("input|i","input file",&in_file,STExactlyOnce);
  AddParam("output|o","output basename",&out_basename,STMaxOnce);
  AddParam("namespace|n","namespace",&namesp,STExactlyOnce);
  AddParam("I","include path (use -I+=path to add a path)",&include_path);
}

int FuncgenParameters::CheckParams()
{
  int missing=0;
  if(!out_basename.str() && in_file.str())
  {
    const char *in=in_file.str();
    if(in_file.len()>4 && strcmp(in+in_file.len()-5,".afd"))
    {
      std::string tmp(in_file,in_file.len()-4);
      out_basename.set(tmp.c_str());  // copies the c_str. 
    }
    else
    {
      std::cerr << "You did not specify the output file." << std::endl;
      ++missing;
    }
  }
  return(missing);
}

FuncgenParameters::~FuncgenParameters()
{
  // nothing to do...
}


// Holds the name of the program: 
char *prg_name=NULL;  // Set early in main(). 

