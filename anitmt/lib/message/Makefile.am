INCLUDES=		-I$(srcdir)/..
AM_CXXFLAGS =		-Wno-non-template-friend
noinst_LTLIBRARIES=	libmessage.la
libmessage_la_SOURCES= 	message.hpp message_inline.cpp message.cpp 
check_PROGRAMS=		test_message
TESTS=			$(check_PROGRAMS)
test_message_SOURCES=	test_message.cpp
test_message_LDADD=	libmessage.la
