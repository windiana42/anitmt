# Pass ADDFLAGS if you want to set -fmessage-length or the like. 
ADDFLAGS=""
ADDCFLAGS=""
ADDCXXFLAGS=""

SUBDIRS = include cpp misc fd crypto compress par
SUBLIBS = cpp/libcpp.a misc/libmisc.a fd/libfd.a crypto/libcrypto.a \
	par/libpar.a compress/libcompress.a

lib_LIBRARIES = libhlib.a
libhlib_a_SOURCES = 

EXTRA_DIST = BUILD COPYING.GPL \
	hlib-config.h.in hconfig.h config.guess config.sub hlib.spec

hlibinstalldir=$(datadir)/hlib/
hlibinstall_DATA=HConfigVars

libhlib.a: $(SUBLIBS)
	@echo "Re-creating libhlib.a..."
	@objs="" ; for i in $(SUBDIRS) ; do \
		test "$$i" = "include" && continue ; \
		for o in `$(AR) -t "$$i/lib$$i.a"` ; do \
			if test -f $$i/$$o ; then objs="$$objs $$i/$$o" ; \
			else echo "Archive: $$i/$$o missing" ; fi ; \
	done ; done ; \
	$(RM) -f libhlib.a || true ; \
	$(AR) -r libhlib.a $$objs ; \
	$(RANLIB) libhlib.a 

# NOTE: See the configure script for more info on opt. 
#       Check OPTCFLAGS, OPTCXXFLAGS. 

debug: FORCE
	$(MAKE) \
		CFLAGS="@DBGCFLAGS@" \
		CXXFLAGS="@DBGCXXFLAGS@" \
		all
check-debug: FORCE
	$(MAKE) \
		CFLAGS="@DBGCFLAGS@" \
		CXXFLAGS="@DBGCXXFLAGS@" \
		check

FORCE:
