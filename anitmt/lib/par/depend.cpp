/*
 * depend.cpp
 * 
 * Implementation of routines dealing with parameters depending 
 * on each other. 
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs, suggestions to wwieser@gmx.de. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 * 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * See the GNU General Public License for details.
 * If you have not received a copy of the GNU General Public License,
 * write to the 
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * Revision History:
 *   Mar 5, 2001   started writing
 *
 */

#include "params.hpp"

#include <string.h>
#include <assert.h>

#include <val/val.hpp>

namespace anitmt
{

namespace
{
	struct NamedAParameter : public AParameter<double>
	{
		const char *name;
		NamedAParameter(const char *_name) : 
			AParameter<double>(),name(_name)  { }
		~NamedAParameter()  { }
		
		inline NamedAParameter &operator=(const AParameter<double> &a)
		{  AParameter<double>::operator=(a);  return(*this);  }
		inline NamedAParameter &operator=(const AParameter<int> &a)
		{
			AParameter<double>::operator=(double(a.val));
			is_set=a.is_set;  
			return(*this);
		}
	};
	
	// time/frame pars
	struct TFPars
	{
		NamedAParameter starttime;
		NamedAParameter endtime;
		NamedAParameter duration;
		NamedAParameter startframe;
		NamedAParameter endframe;
		NamedAParameter frames;
		NamedAParameter fps;
		
		TFPars();
		~TFPars() { }
		
		// Return value int IS IMPORTANT. 
		int IsSet(AParameter<double> &v)
			{  return(v.is_set ? 1 : 0);  }
		int N_Set();
		
		int Calc_All();
		void Set_Array(NamedAParameter **array);  // array of 7 pointers 
		void Add_Pars(TFPars *toadd);
		void Print_Set_Pars(std::ostream &os,int verbose);
	};
	
	TFPars::TFPars() : 
		starttime("starttime"),endtime("endtime"),duration("duration"),
		startframe("startframe"),endframe("endframe"),frames("frames"),
		fps("fps")
	{  }
	
	void TFPars::Set_Array(NamedAParameter **array)  // array of 7 pointers 
	{
		NamedAParameter *arr[7]=
		{&starttime,&endtime,&duration,&startframe,&endframe,&frames,&fps};
		for(int i=0; i<7; i++)  array[i]=arr[i];
	}
	
	
	bool equal(const AParameter<double> &a,const AParameter<double> &b)
	{
		if(a.is_set && b.is_set)
		{
			if(fabs(a.val-b.val)<vect::epsilon)
			{  return(true);  }
		}
		return(false);
	}
	bool notequal(const AParameter<double> &a,const AParameter<double> &b)
	{
		if(a.is_set && b.is_set)
		{
			if(fabs(a.val-b.val)>=vect::epsilon)
			{  return(true);  }
		}
		return(false);  // special; keep that
	}
	
	// Some calculation routines probably only needed here: 
	template<class OP> inline AParameter<double> Operation(
		AParameter<double> &a,AParameter<double> &b)
	{
		AParameter<double> rv;
		if(a.is_set && b.is_set)
		{  rv=OP::op(a,b);  }
		return(rv);
	}
	
	struct OP_MUL  {  static inline double op(double a,double b)  {  return(a*b);  }  };
	struct OP_ADD  {  static inline double op(double a,double b)  {  return(a+b);  }  };
	struct OP_SUB  {  static inline double op(double a,double b)  {  return(a-b);  }  };
	
	inline AParameter<double> mul(AParameter<double> &a,AParameter<double> &b)
	{  return(Operation<OP_MUL>(a,b));  }
	inline AParameter<double> add(AParameter<double> &a,AParameter<double> &b)
	{  return(Operation<OP_ADD>(a,b));  }
	inline AParameter<double> sub(AParameter<double> &a,AParameter<double> &b)
	{  return(Operation<OP_SUB>(a,b));  }
	
	inline AParameter<double> div(AParameter<double> &a,AParameter<double> &b)
	{
		AParameter<double> rv;
		if(a.is_set && b.is_set && !equal(b,0.0))
		{  rv=a.val/b.val;  }
		return(rv);
	}
	
	
	// Returns 1 on change, 0 if nothing was done. 
	int SetIfUnset(AParameter<double> &v,AParameter<double> nv)
	{
		if(!v.is_set && nv.is_set)
		{  v=nv;  return(1);  }
		return(0);
	}
	
	
	// Returns how many parameters are set. 
	int TFPars::N_Set()
	{
		return(IsSet(starttime)+IsSet(endtime)+IsSet(duration)+
	    	   IsSet(startframe)+IsSet(endframe)+IsSet(frames)+
	    	   IsSet(fps));
	}
	
	// Return value: 
	//  0 -> OK
	// -1 -> too few parameters set (underdetermined)
	// +1 -> too many parameters set (overdetermined)
	//  2 -> wrong set of parameters 
	int TFPars::Calc_All()
	{
		// See how many parameters are set: 
		int ns0=N_Set();
		// IMPORTANT: ns0<3 check is performed below. 
		
		int change;
		do
		{
			change=0;
			//Print_Set_Pars(std::cerr,verbose());  std::cerr << "\n";
			
			change+=SetIfUnset(starttime,sub(endtime,duration));
			change+=SetIfUnset(starttime,div(startframe,fps));
			change+=SetIfUnset(endtime,add(starttime,duration));
			change+=SetIfUnset(endtime,div(endframe,fps));
			change+=SetIfUnset(duration,sub(endtime,starttime));
			change+=SetIfUnset(duration,div(frames,fps));
			
			change+=SetIfUnset(startframe,sub(endframe,frames));
			change+=SetIfUnset(startframe,mul(starttime,fps));
			change+=SetIfUnset(endframe,add(startframe,frames));
			change+=SetIfUnset(endframe,mul(endtime,fps));
			change+=SetIfUnset(frames,sub(endframe,startframe));
			change+=SetIfUnset(frames,mul(duration,fps));
			
			change+=SetIfUnset(fps,div(frames,duration));
			change+=SetIfUnset(fps,div(startframe,starttime));
			change+=SetIfUnset(fps,div(endframe,endtime));
		}
		while(change);
		
		int ns=N_Set();
		//cerr << "ns0=" << ns0 << "; ns=" << ns << "\n";
		if(ns<7)
		{
			if(ns0<3)  return(-1);  // surely too few 
			// Check if this is a wrong set of pars: 
			// notequal() returns false if one of the args is not set. 
			// notequal(a,b) returns true if a.is_set && b.is_set && a!=b 
			if(notequal(sub(endtime,starttime),duration) || 
			   notequal(sub(endframe,startframe),frames) ||
			   notequal(div(frames,duration),fps) )
			{  return(2);  }  // wrong set of parameters
			return(-1);  // redundant but okay and underdetermined 
		}
		
		// Check for overdeterminmation: 
		if(!equal(sub(endtime,starttime),duration) || 
		   !equal(sub(endframe,startframe),frames) || 
		   !equal(div(frames,duration),fps)         )
		{  return(+1);  }   // overdetermined
		
		return(0);  // Okay. 
	}
	
	// TFPars::Calc_All()' return value as string: 
	const char *Calc_Stat_Str(int rv)
	{
		switch(rv)
		{
			case -1: return("underdetermined");
			case  0: return("success");
			case +1: return("overdetermined");
			case  2: return("illegal");
		}
		assert(0);	// shouldn't reach that
	}
	
	// rv: value returned by TFPars::Calc_All()
	// tfp: pointer to current TFPars (NON-SOLVED) 
	// levelname: config, ini, ...
	// warn: 1 -> warning; 0 -> error
	// Returns 1 on error; 0 on warning or no error 
	int Solve_Tell_User(std::ostream &os,int rv,TFPars *tfp,
		const char *levelname,bool warn,int verbose)
	{
		if(!rv)  return(0);
		os << (warn ? "Warning" : "Error") << 
			" in level " << levelname << ": " <<
			Calc_Stat_Str(rv) <<
			" parameter set (";
		tfp->Print_Set_Pars(os,verbose);
		os << ")" << std::endl;
		return(warn ? 0 : 1);
	}
	
	void TFPars::Print_Set_Pars(std::ostream &os,int verbose)
	{
		NamedAParameter *nap[7];
		Set_Array(nap);
		bool first=true;
		for(int i=0; i<7; i++)
			if(nap[i]->is_set)
			{
				if(first)  first=false;
				else  os << ", ";
				os << nap[i]->name;
				if(fabs(nap[i]->val)<vect::epsilon)
				{  os << "=0";  }
				else if(verbose>1)
				{  os << "=" << nap[i]->val;  }
			}
		if(first)  os << "(none)";
	}
	
	void TFPars::Add_Pars(TFPars *toadd)
	{
		NamedAParameter *src[7],*dest[7];
		Set_Array(dest);  toadd->Set_Array(src);
		for(int i=0; i<7; i++)
		{
			if(!dest[i]->is_set && src[i]->is_set)
			{  *dest[i]=*src[i];  }
		}
	}
}  /* namespace */


// Solves start/endtime, start/endframe, duration, frames, fps 
// Returns 0 on success. 
int Animation_Parameters::Solve_TimeFrame_Net(Override_Pars *ovp,int nlevels,
	std::ostream &os,bool warnings)
{
	int errors=0;
	std::ostream &vstream=std::cerr;  // verbose stream
	
	// We need our pars and we need them all double (against 
	// integer rounding). 
	TFPars tfp[nlevels];
	for(int i=0; i<nlevels; i++)
	{
		Animation_Parameters *ap=ovp[i].ap;
		tfp[i].starttime= ap->par_double[PID::starttime];
		tfp[i].endtime=   ap->par_double[PID::endtime];
		tfp[i].duration=  ap->par_double[PID::duration];
		tfp[i].startframe=ap->par_int[PID::startframe];
		tfp[i].endframe=  ap->par_int[PID::endframe];
		tfp[i].frames=    ap->par_int[PID::frames];
		tfp[i].fps=       ap->par_double[PID::fps];
	}
	
	// First, we check all levels for overdetermination and warn 
	// about that (for cmd line level, this is an error). 
	for(int i=0; i<nlevels; i++)
	{
		TFPars tmp;  // construct
		tmp=tfp[i];  // assign
		int rv=tmp.Calc_All();
		if(rv>0)  // overdefinition / wrong set
		{  errors+=Solve_Tell_User(os,rv,&tfp[i],
			ovp[i].type,(i!=nlevels-1),verbose());  }
	}
	
	if(errors)  return(errors);
	
	// Okay, cmd line level is not overdetermined. 
	// Let's see if it provides all pars: 
	int rv=0;
	//**TFPars tfpar;
	TFPars accup;  //**
	if(verbose())
	{  vstream << "Solving dependent parameters (time/frame/fps):" << std::endl;  }
	for(int calclevel=nlevels-1; calclevel>=0; --calclevel)
	{
		//**TFPars accup;
		//**// accumulate the pars: 
		//**for(int cl=nlevels-1; cl>=calclevel; --cl)
		//**{
			TFPars newp;
			newp=tfp[calclevel];
			accup.Add_Pars(&newp);
		//**}
		TFPars before_calc=accup;
		rv=accup.Calc_All();
		if(verbose())
		{
			vstream << "[" << ovp[calclevel].type << "]   \t";
			before_calc.Print_Set_Pars(vstream,verbose());
			vstream << "  (" << Calc_Stat_Str(rv) << ")" << std::endl;
			vstream << "[" << ovp[calclevel].type << "]==>\t";
			accup.Print_Set_Pars(vstream,verbose());
			vstream << std::endl;
		}
		
		if(rv==0)   // Okay, we have it. 
		{
			Solve_TimeFrame_Done(&accup);   // Copy pars into *this 
			if(verbose())
			{  vstream << "Solving time/frame/fps done." << std::endl;  }
			return(errors);
		}
		else if(rv==1 || rv==2)  // overdetermined / wrong set 
		{
			errors+=Solve_Tell_User(os,rv,&before_calc,
				ovp[calclevel].type,/*warn=*/false,verbose());
			return(errors);
		}
		
		// What if we first calc all new parameters?! 
		#warning Fixme? (Does it behave as we expect?) 
		//**tfpar=accup;
	}
	
	// If we reach here, the pars are underdetermined. 
	os << "Error: time/frame/fps underdetermined (computed: "; 
	accup.Print_Set_Pars(os,verbose());
	os << ")" << std::endl;
	++errors;
	
	return(errors);
}


void Animation_Parameters::Solve_TimeFrame_Done(void *_tmp)
{
	// This is an ugly cast but otherwise I had to make TFPars 
	// public in the anitmt namespace although nobody else uses it. 
	TFPars *tmp=(TFPars *)_tmp;
	par_double[PID::starttime]=tmp->starttime;
	par_double[PID::endtime]=  tmp->endtime;
	par_double[PID::duration]= tmp->duration;
	par_double[PID::fps]=      tmp->fps;
	par_int[PID::startframe]=  int(tmp->startframe+0.5);
	par_int[PID::endframe]=    int(tmp->endframe+0.5);
	par_int[PID::frames]=par_int[PID::endframe]-par_int[PID::startframe];
}

}  /* namespace anitmt */
