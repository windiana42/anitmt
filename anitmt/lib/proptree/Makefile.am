# requires:
#   - libmessage
#   - libval
#   - libsolve
#   - libutl
INCLUDES=		-I$(srcdir)/..
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend
noinst_LIBRARIES=	libproptree.a
libproptree_a_SOURCES=	property.hpp property_inline.cpp property_templ.cpp \
			property.cpp \
			proptree.hpp proptree_inline.cpp proptree_templ.cpp \
			proptree.cpp 
#check_PROGRAMS=	test_message
#test_message_SOURCES=	test_message.cpp
#test_message_LDADD=	libmessage.a
