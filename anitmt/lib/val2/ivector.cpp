#include "internals.hpp"

// Most is done inline for speed increase. 

namespace internal_vect
{
namespace internal
{

// Suffix 1 for 1-dim array (vector). 
ostream& stream_write_array1(ostream& s,const double *x,int n)
{
	s << "<";
	if(n>0)
	{
		s << *x;
		for(int i=1; i<n; i++)
		{  s << "," << x[i] ;  }
	}
	s << ">";
	return(s);
}

}  // end of namespace internal
}  // namespace end 

