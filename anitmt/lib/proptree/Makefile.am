# requires:
#   - libmessage
#   - libval
#   - libsolve
#   - libutl
INCLUDES=		-I$(srcdir)/..
AM_CXXFLAGS =		-Wno-non-template-friend
noinst_LTLIBRARIES=	libproptree.la
libproptree_la_SOURCES=	property.hpp property_inline.cpp property_templ.cpp \
			property.cpp \
			proptree.hpp proptree_inline.cpp proptree_templ.cpp \
			proptree.cpp 

#check_PROGRAMS=	test_message
#test_message_SOURCES=	test_message.cpp
#test_message_LDADD=	libmessage.a
