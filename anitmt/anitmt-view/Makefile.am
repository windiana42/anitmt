SUBDIRS =		parser
INCLUDES=		-I$(srcdir)/parser/
bin_PROGRAMS=		anitmt-view tga2avi avi2tga

anitmt_view_SOURCES=	anitmt-view.cpp myavi.h mytga.h mystr.h myfile.h \
			myvektor.h common.h osdep.hpp
anitmt_view_LDADD=	parser/libparser.a

tga2avi_SOURCES=	tga2avi.cpp myavi.h mytga.h mystr.h myfile.h \
			myvektor.h common.h
avi2tga_SOURCES=	avi2tga.cpp myavi.h mytga.h mystr.h myfile.h \
			myvektor.h common.h

