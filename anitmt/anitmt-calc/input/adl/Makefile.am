# Makefile.am for parser of anitmt adl - animation description language 
INCLUDES=		-I$(srcdir)/../.. -I$(srcdir)/../../../lib/
noinst_LIBRARIES=	libadlparser.a
check_PROGRAMS=		adltest
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend

libadlparser_a_SOURCES=	scanner.ll support.hpp support.cpp \
			adlparser.hpp adlparser.cpp

adltest_SOURCES=	adltest.cpp
adltest_LDADD=		libadlparser.a \
			../../libanitmt.a \
			../../../lib/val/libval.a \
			../../../lib/par/libpar.a \
			../../../lib/solve/libsolve.a \
			../../../lib/message/libmessage.a -lfl

YLWRAP  = 		$(top_srcdir)/ylwrap

# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
scanner.cc:	$(srcdir)/scanner.ll $(srcdir)/support.hpp
	$(SHELL) $(YLWRAP) "$(LEX)" scanner.ll lex.ADL.cc scanner.cc -- -PADL
