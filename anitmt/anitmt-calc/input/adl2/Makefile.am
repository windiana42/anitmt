# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir) -I$(top_srcdir)/lib		\
			-I$(top_srcdir)/lib/flex			\
			-I$(srcdir)/../ -I$(srcdir)/../../ -I../../	\
			-I$(builddir)/../../functionality/ -I$(srcdir)/../../functionality/
noinst_LTLIBRARIES=	libadlparser.la
check_PROGRAMS=		testadlparser
TESTS=			$(check_PROGRAMS)
AM_CXXFLAGS=		-Wno-non-template-friend 

BUILT_SOURCES=		scanner.cc parser.cc tokens.h 
CLEANFILES=		$(BUILT_SOURCES)
libadlparser_la_SOURCES=parser.yy tokens.h\
			parser_functions.hpp parser_functions.cpp \
			parser_functions_inline.cpp \
			adlparser.hpp adlparser.cpp adlparser_inline.cpp \
			token.hpp token.cpp token_inline.cpp \
			parsinfo.hpp parsinfo.cpp parsinfo_inline.cpp \
			flexlexer_public.h flexlexer_global.h scanner.ll 

testadlparser_SOURCES=	testadlparser.cpp expression.hpp expression.cpp \
			expression_inline.cpp
testadlparser_LDADD=	libadlparser.la \
			../../libanitmt.la \
			../../../lib/val/libval.la \
			../../../lib/par/libpar.la \
			../../../lib/utl/libutl.la \
			../../../lib/solve/libsolve.la \
			../../../lib/message/libmessage.la \
			../libinput.la \
			../../output/liboutput.la \
			../../functionality/libfunctionality.la 

#			-lefence

#also includes old adl/libadlparser.a for libanitmt

YLWRAP  = 		$(top_srcdir)/ylwrap

#-- Scanner
# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
scanner.cc:	$(srcdir)/scanner.ll tokens.h 
	$(SHELL) $(YLWRAP) \
	$(srcdir)/scanner.ll lex.adlparser_.cc scanner.cc \
	-- $(LEX)

#-- Parser
parser.cc tokens.h:	$(srcdir)/parser.yy 
	$(SHELL) $(YLWRAP) $(srcdir)/parser.yy \
	y.tab.c parser.cc \
	y.tab.h tokens.h \
	-- $(YACC) -d

