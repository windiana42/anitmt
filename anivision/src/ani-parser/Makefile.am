INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I..
noinst_LIBRARIES = libaniparser.a

BUILT_SOURCES = grammar.cc grammar.hh ani_scanner.cc
libaniparser_a_SOURCES = \
	strtreedump.h strtreedump.cc \
	location.h location.cc \
	coreglue.h coreglue.cc \
	ani_parser.h grammar.yy ani_parser.cc \
	scannerbase.h scannerbase.cc ani_scanner.h ani_scanner.ll \
	treenode.h treenode.cc \
	exprtype.h exprtype.cc \
	vnotify.h vnotify.cc \
	exprvalue.h exprvalue.cc \
	opfunc.h opfunc.cc \
	assfunc.h assfunc.cc \
	threadinfo.h threadinfo.cc \
	execstack.h execstack.cc \
	evalstack.h evalstack.cc \
	exprtf.h exprtf.cc \
	tree.h treedump.cc treeloc.cc expreval.cc stmtexec.cc \
		checkifeval.cc treemisc.cc treeclone.cc \
	treereg.h treereg.cc 

ani_scanner.cc: $(srcdir)/ani_scanner.ll $(srcdir)/ani_scanner.h grammar.hh \
	$(srcdir)/tree.h $(srcdir)/treenode.h
	$(LEX) -s $(srcdir)/ani_scanner.ll

grammar.cc grammar.hh: $(srcdir)/grammar.yy $(srcdir)/ani_scanner.h $(srcdir)/tree.h $(srcdir)/treenode.h $(srcdir)/ani_parser.h
	$(YACC) $(srcdir)/grammar.yy

clean:
	rm -f $(BUILT_SOURCES)
