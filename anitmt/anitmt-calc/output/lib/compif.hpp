/*
 * compif.hpp
 * 
 * Component Interface routines for POV output. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   Feb 2002   started writing
 *   Mar 2002   moved from pov/ to lib/ ; ITFype now independent type
 *
 */

#ifndef _Inc_IO_PovComonentIF_H_
#define _Inc_IO_PovComonentIF_H_ 1

#include <animation.hpp>

namespace output_io
{

// This class holds one of the 
// Scalar_Component_Interface, Object_Comonent_Interface,...
class ComponentInterface
{
	public:
		#if 1
		enum IFType
		{
			IFNone=0,
			IFScalar,
			IFObject
		};
		#else
		// Used to let the compiler check for problems with 
		// transition from tokID to IFType. 
		static const int IFNone;   //=(175876535);
		static const int IFScalar; //=(216684673);
		static const int IFObject; //=(352489745);
		class IFType
		{
			int x;
			public:
			IFType()  {  x=-19876438;  }
			IFType(int v)  {  x=v;  }
			bool operator==(const IFType &b) const  {  return(x==b.x);  }
			bool operator!=(const IFType &b) const  {  return(x!=b.x);  }
			//operator int() {  return(x);  }
		};
		#endif
	private:
		IFType type;
		void *cif;
		
		void _Assign(const void *src_cif,IFType src_type);
	public:
		ComponentInterface()
			{  type=IFNone;  cif=NULL;  }
		ComponentInterface(const ComponentInterface &);
		ComponentInterface(const anitmt::Scalar_Component_Interface &);
		ComponentInterface(const anitmt::Object_Component_Interface &);
		~ComponentInterface()
			{  Clear();  }
		
		ComponentInterface &operator=(const ComponentInterface &src)
			{  Clear();  _Assign(src.cif,src.type);  return(*this);  }
		
		void Clear();
		
		IFType GetType() const
			{  return(type);  }
		
		anitmt::Scalar_Component_Interface *CVScalar() const
			{  return((type==IFScalar) ? (anitmt::Scalar_Component_Interface*)cif : NULL);  }
		anitmt::Object_Component_Interface *CVObject() const
			{  return((type==IFObject) ? (anitmt::Object_Component_Interface*)cif : NULL);  }
		
		// Call functions that *_Component_Interface provide: 
		std::string get_name() const;
};

}  // namespace end 

#endif  /* _Inc_IO_PovComonentIF_H_ */
