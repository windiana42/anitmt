INCLUDES=		-I$(srcdir)/..
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend
noinst_LIBRARIES=	libsolve.a
libsolve_a_SOURCES= 	operand.hpp operand.cpp operand_inline.cpp \
			operand_templ.cpp \
			operator.hpp operator.cpp operator_inline.cpp \
			operator_templ.cpp \
			solver.hpp solver.cpp solver_templ.cpp \
			constraint.hpp constraint.cpp \
			priority.hpp priority.cpp priority_templ.cpp \
			caller.hpp caller.cpp \
			reference.hpp reference.cpp reference_templ.cpp \
			error.hpp error.cpp # should be replaced by libmessage
check_PROGRAMS=		test_solve
test_solve_SOURCES=	test_solve.cpp
test_solve_LDADD=	libsolve.a ../val/libval.a ../message/libmessage.a
EXTRA_DIST=		README
