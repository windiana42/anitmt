CPPFLAGS=		-I$(top_srcdir)/lib -I$(top_srcdir)/anitmt-calc 
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend
noinst_LIBRARIES=	libsolve.a
libsolve_a_SOURCES= 	\
	operand.hpp operand.cpp operand_inline.cpp operand_templ.cpp \
	operator.hpp operator.cpp operator_inline.cpp operator_templ.cpp \
	solver.hpp solver.cpp solver_templ.cpp \
	event_solver.hpp event_solver.cpp \
	constraint.hpp constraint.cpp \
	priority.hpp priority.cpp priority_templ.cpp \
	caller.hpp caller.cpp \
	reference.hpp reference.cpp reference_templ.cpp 

check_PROGRAMS=		test_solve
test_solve_SOURCES=	test_solve.cpp
test_solve_LDADD=	../message/libmessage.a \
			../../anitmt-calc/functionality/libfunctionality.a \
			libsolve.a ../val/libval.a 
EXTRA_DIST=		README
