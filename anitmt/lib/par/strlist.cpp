#include <string>
#include <iostream>

#ifndef NULL
#define NULL ((void*)0)
#endif


#include "strlist.hpp"

namespace anitmt
{

bool stringlist::next()
{
	if(!curr)
		return(false);
	curr=curr->next;
	return(curr ? true : false);
}


std::string &stringlist::current()
{
	if(curr)
		return(curr->str);
	return(empty);  // Be sure not to modify the empty string. 
}


void stringlist::_delete(stringlist::Node *n)
{
	if(!n)  return;
	if(n==curr)  curr=NULL;
	if(n==first && n==last)
	{
		first=NULL;
		last=NULL;
	}
	else if(n==first)
	{
		first=first->next;
		if(first)  first->prev=NULL;
	}
	else if(n==last)
	{
		last=last->prev;
		if(last)  last->next=NULL;
	}
	else
	{
		if(n->prev)  n->prev->next=n->next;
		if(n->next)  n->next->prev=n->prev;
	}
	delete n;
}


void stringlist::clear()
{
	//cerr << "CLEEEEEEAAAAAAAAARRRRRR!!!!\n";
	// ...you see, I had problems with this one...
	while(first)
		stringlist::_delete(first);
}


void stringlist::add(std::string &str)
{
	Node *n=new Node(str);
	if(!first)
	{  first=n;  }
	else
	{  last->next=n;  n->prev=last;  }
	last=n;
}

void stringlist::add(const char *str)
{
	std::string tmp(str);
	add(tmp);
}


void stringlist::_copy(const stringlist &sl)
{
	for(Node *i=sl.first; i; i=i->next)
	{
		add(i->str);
		if(sl.curr==i)
			curr=last;
	}
}


stringlist &stringlist::operator=(const stringlist &sl)
{
	clear();
	_copy(sl);
	return(*this);
}

stringlist &stringlist::operator+=(const stringlist &sl)
{
	for(Node *i=sl.first; i; i=i->next)
	{  add(i->str);  }
	return(*this);
}


stringlist::stringlist(const stringlist &sl) : empty()
{
	first=NULL;
	last=NULL;
	curr=NULL;
	_copy(sl);
}


stringlist::stringlist()
{
	first=NULL;
	last=NULL;
	curr=NULL;
}

stringlist::~stringlist()
{
	clear();
}


ostream& operator<<(ostream& os,const stringlist &sl)
{
	for(stringlist::Node *i=sl.first; i; i=i->next)
	{
		os << i->str;
		if(i->next)
			os << " ";
	}
	return(os);
}

}  /* end of namespace anitmt */
