INCLUDES =		-I$(srcdir)/../lib/include/
SUBDIRS =		parser
noinst_LIBRARIES =	libanitmt.a
bin_PROGRAMS =		anitmt-calc
check_PROGRAMS =	test_core

test_core_SOURCES =	test_core.cpp 
test_core_LDADD =	../lib/val/libval.a libanitmt.a

libanitmt_a_SOURCES = 	error.hpp error.cpp\
			operand.cpp \
			operand.hpp operand_templ.cpp operand_inline.cpp \
			operator.cpp \
			operator.hpp operator_templ.cpp operator_inline.cpp \
			property.cpp property.hpp property_templ.cpp \
			solver.cpp solver.hpp caller.hpp caller.cpp \
			priority.hpp priority.cpp priority_templ.cpp \
			proptree.hpp proptree.cpp proptree_templ.cpp \
			return.hpp return.cpp nodes.hpp nodes.cpp \
			options.hpp \
			animation.hpp animation.cpp scene.hpp scene.cpp \
			scalar.hpp scalar.cpp linear.hpp linear.cpp \
			object.hpp object.cpp move.hpp move.cpp \
			save_filled.hpp save_filled.cpp

anitmt_calc_SOURCES =	anitmt.cpp
anitmt_calc_LDADD =	
