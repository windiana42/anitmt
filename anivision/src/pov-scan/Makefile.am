INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
noinst_LIBRARIES = libpovscanner.a

EXTRA_DIST = test.pov test2.pov scene.pov

BUILT_SOURCES = pov_scanner.cc
libpovscanner_a_SOURCES = \
	pov_scan.cc pov_scan.h \
	pov_scanner.ll pov_scanner.h

check_PROGRAMS = test
test_SOURCES = test.cc
test_LDADD = \
	$(top_builddir)/hlib/libhlib.a $(LIBS) \
	libpovscanner.a \
	../sourcepos/libsourcepos.a \
	../ani-parser/libaniparser.a \
	$(top_builddir)/hlib/libhlib.a 

check: test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
	./test$(EXEEXT) $(srcdir)/test.pov

pov_scanner.cc: $(srcdir)/pov_scanner.ll $(srcdir)/pov_scan.h $(srcdir)/pov_scanner.h
	$(LEX) -s $(srcdir)/pov_scanner.ll

clean:
	rm -f $(BUILT_SOURCES)

xFORCE:
