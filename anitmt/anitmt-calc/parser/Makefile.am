# Makefile.am for parser of anitmt adl - animation description language (right?)
noinst_LIBRARIES=		libadlparser.a

libadlparser_a_SOURCES=		scanner.ll \
				support.hpp

YLWRAP  = $(top_srcdir)/ylwrap

# A .cc-File is a must here. The ylwrap script doesn't accept other file
# suffixes.
scanner.cc:	$(srcdir)/scanner.ll $(srcdir)/support.hpp
	$(SHELL) $(YLWRAP) "$(LEX)" scanner.ll lex.ADL.cc scanner.cc -- -PADL
