INCLUDES=		-I$(srcdir)/../lib/ 
SUBDIRS=		funcgen functionality input output
noinst_LIBRARIES=	libanitmt.a
bin_PROGRAMS=		anitmt-calc
check_PROGRAMS=		test_core
CXXFLAGS=		@CXXFLAGS@ -Wno-non-template-friend

test_core_SOURCES=	test_core.cpp 
test_core_LDADD=	libanitmt.a \
			../lib/par/libpar.a \
			../lib/val/libval.a \
			../lib/utl/libutl.a \
			../lib/message/libmessage.a \
			../lib/solve/libsolve.a 

libanitmt_a_SOURCES= 	animation.hpp animation.cpp \
			save_filled.hpp save_filled.cpp \
			anitmt.hpp anitmt.cpp

anitmt_calc_SOURCES=	anitmt-calc.cpp	
anitmt_calc_LDADD=	../lib/message/libmessage.a \
			input/libinput.a \
			input/adl2/libadlparser.a \
			output/raw/librawout.a \
			output/lib/liboutput.a \
			output/liboutput.a \
			libanitmt.a \
			functionality/libfunctionality.a \
			../lib/message/libmessage.a \
			../lib/par/libpar.a \
			../lib/val/libval.a \
			../lib/solve/libsolve.a \
			../lib/proptree/libproptree.a \
			../lib/utl/libutl.a 
#			output/pov/libpovout.a \
