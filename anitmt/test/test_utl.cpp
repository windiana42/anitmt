#include <iostream>
#include <map>

#include <utl/utl.hpp>

int main(){
  utl::maped_string st1("hallo");
  utl::maped_string st2("hallo");
  utl::maped_string st3("hallo");
  utl::maped_string st4("hallo");
  utl::maped_string st5("hallo");
  utl::maped_string st6("hallo");
  utl::maped_string st7("hallo");
  utl::maped_string st8("hallo");
  utl::maped_string st9("hallo");
  utl::maped_string st0("hallo");
  utl::maped_string str("hallo");

  if( str == (const char *)"hallo" )
    std::cout << st1 << str << std::endl;

  std::map< utl::maped_string, int > m;

  m[str] = 5;
  m["hallo"] = 6;

  std::cout << m.find( "hallo" )->second  << std::endl;
  std::cout << m.find( utl::maped_string("hallo") )->second  << std::endl;

  std::string s = "ha ";
  s += str;
  std::cout << s << std::endl;

  return 0;
}
