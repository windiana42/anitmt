#ifndef __stringlist_hpp__
#define __stringlist_hpp__

#include <string>
#include <list>

namespace anitmt
{

class stringlist : public std::list<std::string>
{
	private:
		void _addlist(const stringlist &sl);
	public:
		stringlist() { }
		stringlist(const stringlist &sl)  {  _addlist(sl);  }
		stringlist(const char *str0,...);  // NULL-terminated!!!
		~stringlist()  {  clear();  }
		
		stringlist &operator=(const stringlist &sl)
			{  clear();  _addlist(sl);  }
		stringlist &operator+=(const stringlist &sl)
			{  _addlist(sl);  }
		
		bool is_empty()
			{  return((begin()==end()) ? true : false);  }
		
		// Add an entry at the end of the list: 
		void add(const std::string &str)  {  push_back(str);  }
		void add(const char *str)  {  add(std::string(str));  }
		
		// To write the string list to a stream: 
		friend ostream& operator<<(ostream& os,const stringlist &sl);
};

extern ostream& operator<<(ostream& os,const stringlist &sl);

}  /* end of namespace anitmt */
#endif  /* __stringlist_hpp__ */
