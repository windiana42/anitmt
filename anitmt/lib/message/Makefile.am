CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend
noinst_LIBRARIES=	libmessage.a
libmessage_a_SOURCES= 	message.hpp message_inline.cpp message.cpp 
check_PROGRAMS=		test_message
test_message_SOURCES=	test_message.cpp
test_message_LDADD=	libmessage.a
