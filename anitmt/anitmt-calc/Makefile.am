INCLUDES=-I$(srcdir)/../lib/include/
SUBDIRS=parser
bin_PROGRAMS=anitmt-calc
anitmt_calc_SOURCES = 	error.hpp error.cpp\
			operand.hpp operand.cpp operand_templ.cpp \
			property.hpp property_templ.cpp property.cpp \
			solver.cpp solver.hpp caller.hpp caller.cpp \
			priority.hpp priority.cpp priority_templ.cpp \
			proptree.hpp proptree.cpp proptree_templ.cpp \
			return.hpp return.cpp nodes.hpp nodes.cpp \
			options.hpp \
			animation.hpp animation.cpp scene.hpp scene.cpp \
			scalar.hpp scalar.cpp linear.hpp linear.cpp \
			object.hpp object.cpp move.hpp move.cpp \
			save_filled.hpp save_filled.cpp \
			anitmt.cpp 
anitmt_calc_LDADD=../lib/val/libval.a

