# Makefile.am for test functionality
INCLUDES=		-I$(top_srcdir)/lib 

noinst_LIBRARIES=	libfunctionality.a
libfunctionality_a_SOURCES= \
			functionality.hpp functionality.cpp 
EXTRA_DIST= 		functionality.afd

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
			../../lib/proptree/libproptree.a \
			-lefence

FUNCGEN= ../funcgen/funcgen

# exec funcgen...
$(srcdir)/functionality.cpp \
$(srcdir)/functionality.hpp: $(srcdir)/functionality.afd $(srcdir)/$(FUNCGEN)
	$(FUNCGEN) $(srcdir)/functionality.afd