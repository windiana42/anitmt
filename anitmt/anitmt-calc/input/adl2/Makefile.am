# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir)/lib -I$(top_srcdir)/lib/flex \
			-I$(srcdir)/../../
noinst_LIBRARIES=	libadlparser.a
check_PROGRAMS=		testadlparser
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend

libadlparser_a_SOURCES=	scanner.ll parser.yy tokens.h \
			adlparser.hpp adlparser.cpp adlparser_inline.cpp \
			flexlexer_public.h flexlexer_global.h

testadlparser_SOURCES=	testadlparser.cpp 
testadlparser_LDADD=	libadlparser.a \
			../libanitmt.a \
			../../lib/val/libval.a \
			../../lib/par/libpar.a \
			../../lib/solve/libsolve.a \
			../../lib/message/libmessage.a \
			../adl/libadlparser.a \
			../libinput.a \
			../output/lib/liboutput.a \
			../output/raw/librawout.a \
			../output/pov/libpovout.a 
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
