INCLUDES=		-I$(srcdir)/..
AM_CXXFLAGS =		-Wno-non-template-friend
noinst_LTLIBRARIES=	libparam.la
libparam_la_SOURCES= 	param.hpp param.cpp param_inline.cpp
check_PROGRAMS=		test_param
TESTS=			$(check_PROGRAMS)
test_param_SOURCES=	test_param.cpp
test_param_LDADD=	libparam.la ../val/libval.la ../message/libmessage.la \
			 ../solve/libsolve.la
EXTRA_DIST=		README
