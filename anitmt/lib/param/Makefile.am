INCLUDES=		-I$(srcdir)/..
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend
noinst_LIBRARIES=	libparam.a
libparam_a_SOURCES= 	param.hpp param.cpp param_inline.cpp
check_PROGRAMS=		test_param
test_param_SOURCES=	test_param.cpp
test_param_LDADD=	libparam.a ../val/libval.a ../message/libmessage.a \
			 ../solve/libsolve.a
EXTRA_DIST=		README
