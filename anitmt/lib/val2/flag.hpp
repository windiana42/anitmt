#ifndef _NS_vect_flag_HPP_
#define _NS_vect_flag_HPP_ 1

#include <ostream>

namespace vect
{

class Flag
{
	private:
		bool x;
	public:
		Flag(const Flag &f) : x(f.x) { }
		Flag(bool f) : x(f) { }
		Flag() { }
		
		operator bool() const  {  return(x);  }
		bool val() const  {  return(x);  }
		
		friend std::ostream& operator<<(std::ostream& s,const Flag &m);
};

}  // namespace end 

#endif  /* _NS_vect_flag_HPP_*/
