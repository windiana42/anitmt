/*****************************************************************************/
/**   This file offers solver for properties          			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __AniTMT_Solver__
#define __AniTMT_Solver__

namespace anitmt{
  class Solver;
}

#include "val.hpp"
#include "property.hpp"

namespace anitmt{

  //************************************************
  // Solver: general solver for dependent properties
  //************************************************

  class Solver{
    friend class Property;
    template<class T> friend class Type_Property;

    // Properties call that if they were solved
    virtual void prop_was_solved( Property *ID ) = 0;
    // Properties call that if they want to validate their results
    virtual bool is_prop_solution_ok
    ( Property *ID, Solve_Problem_Handler *problem_handler ) = 0;

    // Property that caused the last rejection of a solve run
    static Property *rejection_cause;
  protected:
    int n_props_available;
    int n_props_connected;
    long try_id;		// id to identify a solution try
  public:
    // returns the Property that caused the last solve run rejection
    static Property *get_rejection_cause();

    // Properties call that if they are distroyed
    void prop_disconnect();

    Solver();
    virtual ~Solver();
  };

  //*********************************************************
  // Accel_Solver: Solver for a constantly accelerated system
  //*********************************************************

  class Accel_Solver : public Solver{
    Scalar_Property &d;		// differance
    Scalar_Property &t;		// duration
    Scalar_Property &a;		// acceleration
    Scalar_Property &v0;	// startspeed
    Scalar_Property &ve;	// endspeed
    bool s_d, s_t, s_a, s_v0, s_ve; // indicates wheater a property was solved
				    // while checking a solution
  public:
    // Properties call that if they were solved
    virtual void prop_was_solved( Property *ID );
    // Properties call that if they want to validate their results
    virtual bool is_prop_solution_ok( Property *ID, 
				      Solve_Problem_Handler *problem_handler );

    Accel_Solver( Scalar_Property *d, Scalar_Property *t, Scalar_Property *a,
		  Scalar_Property *v0, Scalar_Property *ve );
  };

  //**********************************************************
  // Diff_Solver: Solver for a start, end and differance value
  //**********************************************************

  class Diff_Solver : public Solver{
    Scalar_Property &d;		// differance
    Scalar_Property &s;		// start
    Scalar_Property &e;		// end
    bool s_d, s_s, s_e;		// indicates wheater a property was solved
				// while checking a solution
  public:
    // Properties call that if they were solved
    virtual void prop_was_solved( Property *ID );
    // Properties call that if they want to validate their results
    virtual bool is_prop_solution_ok( Property *ID, Solve_Problem_Handler *problem_handler );

    Diff_Solver( Scalar_Property *d, Scalar_Property *s, Scalar_Property *e );
  };

  // Solver for a start, end and relationerance value
  class Relation_Solver : public Solver{
    Scalar_Property &q;		// quotient
    Scalar_Property &n;		// numerator
    Scalar_Property &d;		// denominator
    bool s_q, s_n, s_d;		// indicates wheater a property was solved
				// while checking a solution
  public:
    // Properties call that if they were solved
    virtual void prop_was_solved( Property *ID );
    // Properties call that if they want to validate their results
    virtual bool is_prop_solution_ok( Property *ID, Solve_Problem_Handler *problem_handler );

    Relation_Solver( Scalar_Property *q, Scalar_Property *n, 
		     Scalar_Property *d );
  };
}
#endif
