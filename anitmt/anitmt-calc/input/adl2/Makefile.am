# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir)/lib/ -I$(top_srcdir)/lib/flex \
			-I$(srcdir)/../../
noinst_LIBRARIES=	libadlparser.a
check_PROGRAMS=		testadlparser
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend

libadlparser_a_SOURCES=	scanner.ll parser.yy tokens.h \
			adlparser.hpp adlparser.cpp adlparser_inline.cpp \
			flexlexer_public.h flexlexer_global.h

testadlparser_SOURCES=	testadlparser.cpp 
testadlparser_LDADD=	libadlparser.a \
			$(srcdir)/../../libanitmt.a \
			$(top_srcdir)/lib/val/libval.a \
			$(top_srcdir)/lib/par/libpar.a \
			$(top_srcdir)/lib/solve/libsolve.a \
			$(top_srcdir)/lib/message/libmessage.a \
			$(srcdir)/../adl/libadlparser.a \
			$(srcdir)/../libinput.a \
			$(srcdir)/../../output/lib/liboutput.a \
			$(srcdir)/../../output/raw/librawout.a \
			$(srcdir)/../../output/pov/libpovout.a 
#also includes old adl/libadlparser.a for libanitmt

YLWRAP  = 		$(top_srcdir)/ylwrap

#-- Scanner
# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
$(srcdir)/scanner.cc:	$(srcdir)/scanner.ll $(srcdir)/tokens.h 
	$(SHELL) $(YLWRAP) "$(LEX)" \
	$(srcdir)/scanner.ll $(srcdir)/lex.adlparser_.cc $(srcdir)/scanner.cc 

#-- Parser
$(srcdir)/parser.cc $(srcdir)/tokens.h:	$(srcdir)/parser.yy 
	$(SHELL) $(YLWRAP) "$(YACC)" $(srcdir)/parser.yy \
	$(srcdir)/y.tab.c $(srcdir)/parser.cc \
	$(srcdir)/y.tab.h $(srcdir)/tokens.h -- -d
