/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/*****************************************************************************/

#ifndef __utils__
#define __utils__

#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <map>

#include <functional>

namespace utils{

  /******************************************************************/
  /* functions for containers with one pointer as template argument */
  /******************************************************************/

  // for use with for_each()
  // deletes a pointer
  template< class T > struct delete_ptr_t : public std::unary_function<T,void>{
    void operator() (T &x ) const{
      delete x;
      x = 0;			// mark that pointer was deleted
    }
  };

  /*
  template< class T, template<class> class TT > 
  delete_ptr_t<T> delete_ptr( TT<T> c){
    return delete_ptr_t<T>();
  }
  */

  template< class T > 
  delete_ptr_t<T> delete_ptr( std::list<T> c){
    return delete_ptr_t<T>();
  }

  template< class T > 
  delete_ptr_t<T> delete_ptr( std::vector<T> c){
    return delete_ptr_t<T>();
  }

  template< class T > 
  delete_ptr_t<T> delete_ptr( std::stack<T> c){
    return delete_ptr_t<T>();
  }

  template< class T > 
  delete_ptr_t<T> delete_ptr( std::queue<T> c){
    return delete_ptr_t<T>();
  }

  /*****************/
  /* map functions */
  /*****************/

  // for use with for_each()
  // executes memberfunction of the valuetype of a map
  template< class R, class T, class U> class map_mem_fun_t
    :public std::unary_function<T*,R>
  {
    R (T::*pmf)();
  public:
    explicit map_mem_fun_t (R (T::*p)()) : pmf(p) {}
    R operator() (std::pair<U,T*> p) const { return (p.second->*pmf)(); }
  };

  // for use with for_each()
  // executes memberfunction of the valuetype of a map
  template< class R, class T, class U, class A> class map_mem_fun1_t
    :public std::binary_function<T*,A,R>
  {
    R (T::*pmf)(A);
  public:
    explicit map_mem_fun1_t (R (T::*p)(A)) : pmf(p) {}
    R operator() (std::pair<U,T*> p, A x) const { return (p.second->*pmf)(x); }
  };

  template< class R, class T, class U>
  map_mem_fun_t<R,T,U> map_mem_fun (R (T::*f)(), std::map<U,T*> m ){
    return map_mem_fun_t<R,T,U>(f);
  }

  // specialization for std::string as first argument
  template< class R, class T>
  map_mem_fun_t<R,T, std::string> map_mem_fun (R (T::*f)()){
    return map_mem_fun_t<R,T, std::string>(f);
  }

  template< class R, class T, class U, class A>
  map_mem_fun1_t<R,T,U,A> map_mem_fun (R (T::*f)(A), std::map<U,T*> m ){
    return map_mem_fun1_t<R,T,U,A>(f);
  }

  // specialization for string as first argument
  template< class R, class T, class A>
  map_mem_fun1_t<R,T, std::string,A> map_mem_fun (R (T::*f)(A)){
    return map_mem_fun1_t<R,T, std::string,A>(f);
  }

  // for use with for_each()
  // deletes the value of a pair in a map
  template<class K, class T> struct delete_maped_t
    : public std::unary_function< std::pair<K,T*> ,void>
  {
    void operator() (std::pair<K,T*> x){
      delete x.second;
      //      x.second = 0;	// doesn't work right now (only a copy?)
      //      x = *(new pair<K,T*>);
    }
  };

  template<class K, class T> 
  delete_maped_t<K,T> delete_maped( std::map<K,T*> m ){
    return delete_maped_t<K,T>();
  }

  /**********************/
  /* multimap functions */
  /**********************/

  // uses class for map 
  template<class K, class T> 
  delete_maped_t<K,T> delete_maped( std::multimap<K,T*> m ){
    return delete_maped_t<K,T>();
  }


}
#endif
