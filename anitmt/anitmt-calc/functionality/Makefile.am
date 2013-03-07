# Makefile.am for test functionality
INCLUDES=		-I$(top_srcdir)/lib -I$(srcdir)

noinst_LTLIBRARIES=	libfunctionality.la

BUILT_SOURCES=		solver.hpp solver.cpp \
			base_func.hpp base_func.cpp \
			scene_base.hpp scene_base.cpp \
			scalar_base.hpp scalar_base.cpp \
			scalar.hpp scalar.cpp \
			object_base.hpp object_base.cpp \
			object.hpp object.cpp \
			complex_solver.hpp complex_solver.cpp 
CLEANFILES = 		solver.hpp solver.cpp \
			solver_prototypes.hpp solver_parser.yy solver_scanner.ll \
			base_func.hpp base_func.cpp \
			base_func_prototypes.hpp base_func_parser.yy base_func_scanner.ll \
			scene_base.hpp scene_base.cpp \
			scene_base_prototypes.hpp scene_base_parser.yy scene_base_scanner.ll \
			scalar_base.hpp scalar_base.cpp \
			scalar_base_prototypes.hpp scalar_base_parser.yy scalar_base_scanner.ll \
			scalar.hpp scalar.cpp \
			scalar_prototypes.hpp scalar_parser.yy scalar_scanner.ll \
			object_base.hpp object_base.cpp \
			object_base_prototypes.hpp object_base_parser.yy object_base_scanner.ll \
			object.hpp object.cpp \
			object_prototypes.hpp object_parser.yy object_scanner.ll \
			complex_solver.hpp complex_solver.cpp \
			complex_solver_prototypes.hpp complex_solver_parser.yy complex_solver_scanner.ll 
nodist_libfunctionality_la_SOURCES= solver.hpp solver.cpp \
			complex_solver.hpp complex_solver.cpp \
			base_func.hpp base_func.cpp \
			scene_base.hpp scene_base.cpp \
			scalar_base.hpp scalar_base.cpp \
			scalar.hpp scalar.cpp \
			object_base.hpp object_base.cpp \
			object.hpp object.cpp 
libfunctionality_la_SOURCES= solver.afd base_func.afd scene_base.afd   \
			scalar_base.afd scalar.afd  object_base.afd   \
			object.afd complex_solver.afd \
			sp_curve.hpp sp_curve.cpp \
			spline.hpp spline.cpp \
			make_avail.cpp make_avail.hpp \
			break_point.cpp break_point.hpp

check_PROGRAMS=		test_functionality
TESTS=			$(check_PROGRAMS)
AM_CXXFLAGS =		-Wno-non-template-friend -Wno-unused \
			-ftemplate-depth-30

test_functionality_SOURCES= \
			test_functionality.cpp

test_functionality_LDADD= \
			../../lib/message/libmessage.la \
			../../lib/val/libval.la \
			../../lib/solve/libsolve.la \
			../../lib/proptree/libproptree.la 

FUNCGEN=../funcgen/funcgen
#FUNCGEN_DEBUG=-d -s
#FUNCGEN_CODE=$(srcdir)/../funcgen/gen_cpp.cpp

AFD_COMMON=$(srcdir)/base_func.afd 
#AFD_COMMON=$(FUNCGEN) $(srcdir)/base_func.afd 

# exec funcgen...
base_func.cpp \
base_func.hpp: $(srcdir)/base_func.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then				\
	  $(FUNCGEN) -i $(srcdir)/base_func.afd -o base_func	\
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG)	\
	  || touch $(srcdir)/base_func.afd;			\
	else							\
	  echo "No funcgen availible, cannot create code.";	\
	fi

scene_base.cpp \
scene_base.hpp: $(srcdir)/scene_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then				\
	  $(FUNCGEN) -i $(srcdir)/scene_base.afd -o scene_base	\
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 	\
	  || touch $(srcdir)/scene_base.afd;			\
	else							\
	  echo "No funcgen availible, cannot create code.";	\
	fi

scalar_base.cpp \
scalar_base.hpp: $(srcdir)/scalar_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/scalar_base.afd -o scalar_base	\
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 		\
	  || touch $(srcdir)/scalar_base.afd;				\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

scalar.cpp \
scalar.hpp: $(srcdir)/scalar.afd $(srcdir)/scalar_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/scalar.afd -o scalar -n functionality	\
	    -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 				\
	  || touch $(srcdir)/scalar.afd;				\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

object_base.cpp \
object_base.hpp: $(srcdir)/object_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/object_base.afd -o object_base	\
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 		\
	  || touch $(srcdir)/object_base.afd;				\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

object.cpp \
object.hpp: $(srcdir)/object.afd $(srcdir)/object_base.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/object.afd -o object -n functionality	\
	    -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 				\
	  || touch $(srcdir)/object.afd;				\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

solver.cpp \
solver.hpp: $(srcdir)/solver.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/solver.afd -o solver -n functionality	\
	    -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 				\
	  || touch $(srcdir)/solver.afd;				\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

complex_solver.cpp \
complex_solver.hpp: $(srcdir)/complex_solver.afd $(AFD_COMMON)
	@if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/complex_solver.afd -o complex_solver	\
	    -n functionality -I+=$(srcdir)/ $(FUNCGEN_DEBUG) 		\
	  || touch $(srcdir)/complex_solver.afd;			\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi


