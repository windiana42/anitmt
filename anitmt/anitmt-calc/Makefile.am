INCLUDES=		-I$(srcdir)/../lib/ 
SUBDIRS=		input output
noinst_LIBRARIES=	libanitmt.a
bin_PROGRAMS=		anitmt-calc
check_PROGRAMS=		test_core
CXXFLAGS=		@CXXFLAGS@ -Wno-non-template-friend

test_core_SOURCES=	test_core.cpp 
test_core_LDADD=	libanitmt.a \
			../lib/val/libval.a \
			../lib/utl/libutl.a \
			../lib/par/libpar.a \
			../lib/message/libmessage.a \
			../lib/solve/libsolve.a 

libanitmt_a_SOURCES= 	error.hpp error.cpp \
			tmttype.hpp tmttype.cpp tmttype_inline.cpp \
			property.cpp property.hpp \
			property_templ.cpp property_inline.cpp \
			proptree.hpp proptree.cpp \
			proptree_templ.cpp proptree_inline.cpp \
			return.hpp return.cpp nodes.hpp nodes.cpp \
			animation_classes.hpp animation.hpp animation.cpp \
			scene.hpp scene.cpp \
			scalar.hpp scalar.cpp linear.hpp linear.cpp \
			object.hpp object.cpp move.hpp move.cpp \
			save_filled.hpp save_filled.cpp \
			anitmt.hpp anitmt.cpp

anitmt_calc_SOURCES=	anitmt-calc.cpp	
anitmt_calc_LDADD=	../lib/val/libval.a  \
			../lib/par/libpar.a \
			../lib/utl/libutl.a \
			../lib/message/libmessage.a \
			../lib/solve/libsolve.a \
			input/libinput.a \
			input/adl2/libadlparser.a \
			output/raw/librawout.a \
			output/pov/libpovout.a \
			output/lib/liboutput.a \
			output/liboutput.a libanitmt.a 

