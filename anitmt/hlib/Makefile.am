#CFLAGS = @CFLAGS@ -W -Wall -D_GNU_SOURCE
#CXXFLAGS = @CXXFLAGS@ -W -Wall -D_GNU_SOURCE

OPTCXXFLAGS = -W -Wall -Wformat -Woverloaded-virtual \
	-Os -fno-rtti -fno-exceptions -fomit-frame-pointer \
	-DHLIB_SIZE_OPT=1 -D_GNU_SOURCE
OPTCFLAGS = -W -Wall -Wformat \
	-Os -fomit-frame-pointer \
	-DHLIB_SIZE_OPT=1 -D_GNU_SOURCE
I386_OPT_FLAGS = -march=i486 -malign-jumps=2 -malign-loops=2 -malign-functions=2

# Pass ADDFLAGS if you want to set -fmessage-length or the like. 
ADDFLAGS=""

SUBDIRS = include cpp misc fd crypto par

noinst_LIBRARIES = libhlib.a
libhlib_a_SOURCES = 

EXTRA_DIST = BUILD hlib-config.h.in hconfig.h

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

# NOTE; - If "make opt" fails, try "make opt-generic". 
#       - All the make opt stuff needs gcc as compiler. 

opt: FORCE
	$(MAKE) opt-"`uname -m`"
check-opt: FORCE
	$(MAKE) check-"`uname -m`"

opt-i386 opt-i486 opt-i586 opt-i686: FORCE
	$(MAKE) \
		CXX="$(CXX) -pipe" \
		CC="$(CXX) -pipe" \
		CFLAGS="$(OPTCFLAGS) $(I386_OPT_FLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(I386_OPT_FLAGS) $(ADDFLAGS)" \
		all
check-i386 check-i486 check-i586 check-i686: FORCE
	$(MAKE) \
		CXX="$(CXX) -pipe" \
		CC="$(CXX) -pipe" \
		CFLAGS="$(OPTCFLAGS) $(I386_OPT_FLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(I386_OPT_FLAGS) $(ADDFLAGS)" \
		all check

# Just the same CFLAGS/CXXFLAGS as opt-generic. 
opt-alpha: FORCE
	$(MAKE) \
		CXX="$(CXX) -pipe" \
		CC="$(CXX) -pipe" \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)" \
		all
check-alpha: FORCE
	$(MAKE) \
		CXX="$(CXX) -pipe" \
		CC="$(CXX) -pipe" \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)" \
		all check

# Currently simply opt-generic. But this has potential for tweaking...
# You may add "-m64" to ADDFLAGS for 64bit environment (with gcc>=3.0). 
opt-ppc: FORCE
	$(MAKE) \
		CXX="$(CXX) -pipe" \
		CC="$(CXX) -pipe" \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS) " \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS) " \
		all
check-ppc: FORCE
	$(MAKE) \
		CXX="$(CXX) -pipe" \
		CC="$(CXX) -pipe" \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS) " \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS) " \
		all check

# You may add "-m64" to ADDFLAGS for 64bit environment (with gcc>=3.0). 
opt-generic: FORCE
	$(MAKE) \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)" \
		all
check-generic: FORCE
	$(MAKE) \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)" \
		all check

FORCE:
