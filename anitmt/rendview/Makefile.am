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

SUBDIRS = src cfg

#EXTRA_DIST = BUILD

opt: FORCE
	$(MAKE) opt-"`uname -m`"

# NOTE that CXXLD is NOT g++ but gcc. 
# NOTE ALSO that any "make opt" requires (GNU) gcc. 
opt-i386 opt-i486 opt-i586 opt-i686: FORCE
	$(MAKE) \
		CFLAGS="$(OPTCFLAGS) $(I386_OPT_FLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(I386_OPT_FLAGS) $(ADDFLAGS)" \
		CXX="$(CXX) -pipe" \
		CXXLD="gcc"

opt-alpha opt-ppc opt-generic: FORCE
	$(MAKE) \
		CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" \
		CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)" \
		CXX="$(CXX) -pipe" \
		CXXLD="gcc"

FORCE:
