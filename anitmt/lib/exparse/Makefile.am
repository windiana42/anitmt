INCLUDES=		-I$(srcdir)/..
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend \
			-DTHROW_NIL=throw\(\) -D__CPLUSPLUS
noinst_LIBRARIES=	libexparse.a
libexparse_a_SOURCES=	\
	fcall.cpp fcall.hpp \
	function.cpp function.hpp \
	operator.cpp operator.hpp \
	tree.cpp tree.hpp \
	value.cpp value.hpp \
	basicparser.cpp basicparser.hpp \
	lexer.ll parse.cpp lexer.hpp \
	flexlexer_public.h flexlexer_global.h \
	linkedlist.h 
# unused: mcache.cpp mcache.hpp

check_PROGRAMS=		minicalc
minicalc_SOURCES=	minicalc.cpp
minicalc_LDADD=		libexparse.a ../val/libval.a -lstdc++ -lm


#lexer.cc: $(srcdir)/lexer.ll $(srcdir)/lexer.hpp
#	$(LEX) -s -olexer.cc $(srcdir)/lexer.ll 

YLWRAP = $(top_srcdir)/ylwrap

lexer.cc: $(srcdir)/lexer.ll $(srcdir)/lexer.hpp 
	$(SHELL) $(YLWRAP) "$(LEX)" \
	$(srcdir)/lexer.ll lex.exparse_.cc lexer.cc \
	-- -Pexparse_
 
