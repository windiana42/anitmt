#ifndef __stringlist_hpp__
#define __stringlist_hpp__

namespace anitmt
{

class stringlist
{
	private:
		struct Node
		{
			Node *next,*prev;
			std::string str;
			Node(const std::string &_str) : next(NULL),prev(NULL)
				{  str.assign(_str);  }
			~Node() { }
			private:
			Node(const Node &) { }
		} *first,*last,*curr;
		std::string empty;
		
		void _delete(Node *n);
		void _copy(const stringlist &);
	public:
		stringlist();
		stringlist(const stringlist &);
		~stringlist();
		
		stringlist &operator=(const stringlist &);
		stringlist &operator+=(const stringlist &);
		
		bool is_empty()  {  return(first ? false : true);  }
		
		/*** Dealing with current pointer: ***/
		// rewind() sets the current pointer to the first string. 
		//   return value: false: no first element; else: true 
		// next() sets the current pointer to the next string in the list. 
		//   return value: false: no next string; else: true 
		// current() returns the current string
		//   The return value may be midified affecting the list. 
		// delete_current() removes the current entry from the list. 
		bool         rewind()   {  curr=first;  return(curr ? true : false);  }
		bool         next();
		std::string &current();
		void         delete_current();
		
		/*** Dealing with string entries: ***/
		
		// Add an entry at the end of the list: 
		void add(std::string &str);
		void add(const char *str);
		
		// Clear the list deleting all entries. 
		void clear();
		
		// To write the string list to a stream (strings separated 
		// by a space. 
		friend ostream& operator<<(ostream& os,const stringlist &sl);
};

extern ostream& operator<<(ostream& os,const stringlist &sl);

}  /* end of namespace anitmt */
#endif  /* __stringlist_hpp__ */
