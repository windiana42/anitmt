# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir)/lib -I$(top_srcdir)/lib/flex 

bin_PROGRAMS=		funcgen
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend -DYYDEBUG \
			-D__CPLUSPLUS -DTHROW_NIL=throw\(\) \
			-ftemplate-depth-30

funcgen_SOURCES=	gen_code.hpp gen_code.cpp gen_cpp.cpp gen_cpp.hpp \
			afdbase.hpp afdbase.cpp parser.yy token.hpp \
			flexlexer_public.h flexlexer_global.h scanner.ll \
			parser_functions.hpp parser_functions.cpp \
			parser_functions_inline.cpp \
			parsinfo.hpp parsinfo.cpp parsinfo_inline.cpp \
			funcgen.cpp stdextend.hpp stdextend.cpp 

funcgen_LDADD=		../../lib/message/libmessage.a 

#also includes old adl/libadlparser.a for libanitmt

YLWRAP  = 		$(top_srcdir)/ylwrap

#-- Scanner
# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
scanner.cc:	$(srcdir)/scanner.ll tokens.h 
	$(SHELL) $(YLWRAP) "$(LEX)" \
	$(srcdir)/scanner.ll lex.funcgen_.cc scanner.cc 

#-- Parser
parser.cc tokens.h:	$(srcdir)/parser.yy 
	$(SHELL) $(YLWRAP) "$(YACC)" $(srcdir)/parser.yy \
	y.tab.c parser.cc \
	y.tab.h tokens.h -- -d
