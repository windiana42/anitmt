# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(top_srcdir)/lib -I$(top_srcdir)/lib/flex \
			-I$(top_builddir)/hlib/include -I$(top_srcdir)

bin_PROGRAMS=		funcgen
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend -ftemplate-depth-30

BUILT_SOURCES=		scanner.cc parser.cc tokens.h 
CLEANFILES=		$(BUILT_SOURCES)
funcgen_SOURCES=	gen_code.hpp gen_code.cpp gen_cpp.cpp gen_cpp.hpp \
			afdbase.hpp afdbase.cpp parser.yy tokens.h token.hpp \
			flexlexer_public.h flexlexer_global.h scanner.ll \
			parser_functions.hpp parser_functions.cpp \
			parser_functions_inline.cpp \
			parsinfo.hpp parsinfo.cpp parsinfo_inline.cpp \
			stdextend.hpp stdextend.cpp \
			base_operators.hpp base_operators.cpp \
			params.cpp params.hpp \
			funcgen.cpp

funcgen_LDADD=		../../lib/message/libmessage.a ../../hlib/libhlib.a 

#also includes old adl/libadlparser.a for libanitmt

YLWRAP  = 		$(top_srcdir)/ylwrap

#-- Scanner
# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
scanner.cc:	$(srcdir)/scanner.ll tokens.h 
	$(SHELL) $(YLWRAP) \
	$(srcdir)/scanner.ll lex.funcgen_.cc scanner.cc \
	-- $(LEX)

#-- Parser
parser.cc tokens.h:	$(srcdir)/parser.yy 
	$(SHELL) $(YLWRAP) $(srcdir)/parser.yy \
	y.tab.c parser.cc \
	y.tab.h tokens.h \
	-- $(YACC) -d

params.o: params.cpp params.hpp
#$(CXX) $(INCLUDES) $(CFLAGS) -fno-rtti -c $(srcdir)/params.cpp
	$(CXXCOMPILE) -fno-rtti -c $(srcdir)/params.cpp -o $@

