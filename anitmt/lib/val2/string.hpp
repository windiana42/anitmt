#ifndef _NS_vect_string_HPP_
#define _NS_vect_string_HPP_ 1

#include <string>

namespace vect
{

class String : public std::string
{
	public:
		String() : std::string() { }
		String(std::string s) : std::string(s) { }
		String(const String &s) : std::string(s) { }
};

inline bool operator!(const String &a)
	{  return(a == std::string());  }

}  // namespace end 

#endif  /* _NS_vect_string_HPP_*/
