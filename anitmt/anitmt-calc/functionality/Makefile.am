# Makefile.am for test functionality
INCLUDES=		-I$(top_srcdir)/lib -I$(srcdir)

noinst_LIBRARIES=	libfunctionality.a
BUILT_SOURCES=		base_func.hpp base_func.cpp \
			scalar.hpp scalar.cpp \
			object.hpp object.cpp \
			functionality.hpp functionality.cpp 
libfunctionality_a_SOURCES= base_func.afd base_func.hpp base_func.cpp \
			scalar.afd scalar.hpp scalar.cpp \
			object.afd object.hpp object.cpp \
			functionality.afd functionality.hpp functionality.cpp \
			make_avail.cpp

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

FUNCGEN= ../funcgen/funcgen

# exec funcgen...
# base_func isn't regenerated when funcgen is changed
base_func.cpp \
base_func.hpp: $(srcdir)/base_func.afd 
	$(FUNCGEN) $(srcdir)/base_func.afd base_func functionality $(srcdir)/

scalar.cpp \
scalar.hpp: $(srcdir)/scalar.afd $(FUNCGEN)
	$(FUNCGEN) $(srcdir)/scalar.afd scalar functionality $(srcdir)/

object.cpp \
object.hpp: $(srcdir)/object.afd $(FUNCGEN)
	$(FUNCGEN) $(srcdir)/object.afd object functionality $(srcdir)/

functionality.cpp \
functionality.hpp: $(srcdir)/functionality.afd $(FUNCGEN)
	$(FUNCGEN) $(srcdir)/functionality.afd functionality functionality \
	  $(srcdir)/

