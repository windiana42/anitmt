INCLUDES=		-I$(srcdir)/../lib/  \
			-I$(srcdir)/functionality \
			-I$(builddir)/functionality 

SUBDIRS=		funcgen functionality input output #manu_functionality
noinst_LIBRARIES=	libanitmt-static.a
lib_LTLIBRARIES=	libanitmt.la
bin_PROGRAMS=		anitmt-calc anitmt-calc-static 
check_PROGRAMS=		test_core
TESTS=			$(check_PROGRAMS)
AM_CXXFLAGS=		-Wno-non-template-friend

libanitmt_la_SOURCES= 	animation.hpp animation.cpp \
			save_filled.hpp save_filled.cpp \
			anitmt.hpp anitmt.cpp
libanitmt_la_LIBADD=	output/liboutput.la \
			input/libinput.la \
			functionality/libfunctionality.la \
			../lib/message/libmessage.la \
			../lib/par/libpar.la \
			../lib/param/libparam.la \
			../lib/val/libval.la \
			../lib/solve/libsolve.la \
			../lib/proptree/libproptree.la \
			../lib/utl/libutl.la 

libanitmt_static_a_SOURCES=
libanitmt_static_a_LIBADD= libanitmt.la

anitmt_calc_SOURCES=	anitmt-calc.cpp	
anitmt_calc_LDADD=	libanitmt.la 

anitmt_calc_static_SOURCES=anitmt-calc.cpp	
anitmt_calc_static_LDADD=libanitmt.la 
anitmt_calc_static_LDFLAGS= -static

test_core_SOURCES=	test_core.cpp 
test_core_LDADD=	libanitmt.la 