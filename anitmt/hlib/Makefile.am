#CFLAGS = @CFLAGS@ -W -Wall -D_GNU_SOURCE
#CXXFLAGS = @CXXFLAGS@ -W -Wall -D_GNU_SOURCE

OPTCXXFLAGS = -W -Wall -Wformat -Woverloaded-virtual \
	-Os -march=i486 -fno-rtti -fno-exceptions -fomit-frame-pointer \
	-malign-jumps=2 -malign-loops=2 -malign-functions=2 \
	-DHLIB_SIZE_OPT=1 -D_GNU_SOURCE
OPTCFLAGS = -W -Wall -Wformat \
	-Os -march=i486 -fomit-frame-pointer \
	-malign-jumps=2 -malign-loops=2 -malign-functions=2 \
	-DHLIB_SIZE_OPT=1 -D_GNU_SOURCE

# Pass ADDFLAGS if you want to set -fmessage-length or the like. 
ADDFLAGS=""

SUBDIRS = include cpp misc fd par

noinst_LIBRARIES = libhlib.a
libhlib_a_SOURCES = 

EXTRA_DIST = BUILD config.h.in hconfig.h

libhlib.a: FORCE
	@objs="" ; for i in $(SUBDIRS) ; do \
		test "$$i" = "include" && continue ; \
		for o in `$(AR) -t "$$i/lib$$i.a"` ; do \
			if test -f $$i/$$o ; then objs="$$objs $$i/$$o" ; \
			else echo "Archive: $$i/$$o missing" ; fi ; \
	done ; done ; \
	$(RM) -f libhlib.a ; \
	$(AR) -r libhlib.a $$objs ; \
	$(RANLIB) libhlib.a 

opt: FORCE
	$(MAKE) CXX="$(CXX) -pipe" CC="$(CXX) -pipe" CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)"

FORCE:
