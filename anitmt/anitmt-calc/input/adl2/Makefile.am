# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir) -I$(top_srcdir)/lib		\
			-I$(top_srcdir)/lib/flex			\
			-I$(srcdir)/../ -I$(srcdir)/../../ -I../../	\
			-I$(builddir)/../../functionality/ -I$(srcdir)/../../functionality/
noinst_LIBRARIES=	libadlparser.a
check_PROGRAMS=		testadlparser
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend 

BUILT_SOURCES=		scanner.cc parser.cc tokens.h 
libadlparser_a_SOURCES=	parser.yy tokens.h\
			parser_functions.hpp parser_functions.cpp \
			parser_functions_inline.cpp \
			adlparser.hpp adlparser.cpp adlparser_inline.cpp \
			token.hpp token.cpp token_inline.cpp \
			parsinfo.hpp parsinfo.cpp parsinfo_inline.cpp \
			flexlexer_public.h flexlexer_global.h scanner.ll 

testadlparser_SOURCES=	testadlparser.cpp expression.cpp
testadlparser_LDADD=	libadlparser.a \
			../../libanitmt.a \
			../../../lib/val/libval.a \
			../../../lib/par/libpar.a \
			../../../lib/utl/libutl.a \
			../../../lib/solve/libsolve.a \
			../../../lib/message/libmessage.a \
			../libinput.a \
			../../output/lib/liboutput.a \
			../../output/raw/librawout.a \
			../../output/pov/libpovout.a \
			../../functionality/libfunctionality.a 

#			-lefence

#also includes old adl/libadlparser.a for libanitmt

YLWRAP  = 		$(top_srcdir)/ylwrap

clean:
	rm -f $(BUILT_SOURCES)

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

