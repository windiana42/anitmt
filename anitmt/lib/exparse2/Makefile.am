# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir)/lib/ -I$(top_srcdir)/lib/flex
noinst_LIBRARIES=	libexparser.a
check_PROGRAMS=		testexparser
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend \
			-D__CPLUSPLUS -DTHROW_NIL=throw\(\)

libexparser_a_SOURCES=	scanner.ll parser.yy \
			exparser.hpp tokens.h \
			flexlexer_public.h flexlexer_global.h

testexparser_SOURCES=	testexparser.cpp
testexparser_LDADD=	libexparser.a \
			../val/libval.a \
			../par/libpar.a \
			../solve/libsolve.a \
			../message/libmessage.a 

YLWRAP  = 		$(top_srcdir)/ylwrap

#-- Scanner
# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
$(srcdir)/scanner.cc:	$(srcdir)/scanner.ll $(srcdir)/tokens.h 
	$(SHELL) $(YLWRAP) "$(LEX)" \
	$(srcdir)/scanner.ll $(srcdir)/lex.exparser_.cc $(srcdir)/scanner.cc \
	-- -Pexparser_

#-- Parser
$(srcdir)/parser.cc $(srcdir)/tokens.h:	$(srcdir)/parser.yy 
	$(SHELL) $(YLWRAP) "$(YACC)" $(srcdir)/parser.yy \
	$(srcdir)/y.tab.c $(srcdir)/parser.cc \
	$(srcdir)/y.tab.h $(srcdir)/tokens.h -- -d
