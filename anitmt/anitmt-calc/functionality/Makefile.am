# Makefile.am for test functionality
INCLUDES=		-I$(top_srcdir)/lib -I$(srcdir)

noinst_LIBRARIES=	libfunctionality.a
BUILT_SOURCES=		solver.hpp solver.cpp \
			base_func.hpp base_func.cpp \
			scene_base.hpp scene_base.cpp \
			scalar_base.hpp scalar_base.cpp \
			scalar.hpp scalar.cpp \
			object_base.hpp object_base.cpp \
			object.hpp object.cpp \
			complex_solver.hpp complex_solver.cpp 
libfunctionality_a_SOURCES= solver.afd solver.hpp solver.cpp \
			base_func.afd base_func.hpp base_func.cpp \
			scene_base.afd scene_base.hpp scene_base.cpp \
			scalar_base.afd scalar_base.hpp scalar_base.cpp \
			scalar.afd scalar.hpp scalar.cpp \
			object_base.afd object_base.hpp object_base.cpp \
			object.afd object.hpp object.cpp \
			complex_solver.hpp complex_solver.cpp \
			sp_curve.hpp sp_curve.cpp \
			make_avail.cpp make_avail.hpp 

check_PROGRAMS=		test_functionality
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend -DYYDEBUG \
			-D__CPLUSPLUS -DTHROW_NIL=throw\(\) \
			-ftemplate-depth-30

test_functionality_SOURCES= \
			test_functionality.cpp

test_functionality_LDADD= \
			../../lib/message/libmessage.a \
			../../lib/val/libval.a \
			../../lib/solve/libsolve.a \
			../../lib/proptree/libproptree.a 

clean:
	rm -f $(BUILT_SOURCES)

FUNCGEN=../funcgen/funcgen
#FUNCGEN_DEBUG=-d -s
FUNCGEN_CODE=$(srcdir)/../funcgen/gen_cpp.cpp

AFD_COMMON=$(FUNCGEN) $(srcdir)/base_func.afd 

# exec funcgen...
base_func.cpp \
base_func.hpp: $(srcdir)/base_func.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/base_func.afd -o base_func	 	\
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG);		\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

scene_base.cpp \
scene_base.hpp: $(srcdir)/scene_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					     \
	  $(FUNCGEN) -i $(srcdir)/scene_base.afd -o scene_base		     \
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG);		     \
	else								     \
	  echo "No funcgen availible, cannot create code.";		     \
	fi

scalar_base.cpp \
scalar_base.hpp: $(srcdir)/scalar_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/scalar_base.afd -o scalar_base 	    \
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG);  		    \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

scalar.cpp \
scalar.hpp: $(srcdir)/scalar.afd $(srcdir)/scalar_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/scalar.afd -o scalar -n functionality     \
	    -I+=$(srcdir)/ $(FUNCGEN_DEBUG); 				    \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

object_base.cpp \
object_base.hpp: $(srcdir)/object_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/object_base.afd -o object_base 	    \
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG); 		    \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

object.cpp \
object.hpp: $(srcdir)/object.afd $(srcdir)/object_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/object.afd -o object -n functionality     \
	    -I+=$(srcdir)/ $(FUNCGEN_DEBUG); 				    \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

solver.cpp \
solver.hpp: $(srcdir)/solver.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/solver.afd -o solver -n functionality     \
	    -I+=$(srcdir)/ $(FUNCGEN_DEBUG); 				    \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

complex_solver.cpp \
complex_solver.hpp: $(srcdir)/complex_solver.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/complex_solver.afd -o complex_solver      \
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG); 		    \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi


