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

SUBDIRS = src cfg

#EXTRA_DIST = BUILD

opt: FORCE
	$(MAKE) CFLAGS="$(OPTCFLAGS) $(ADDFLAGS)" CXXFLAGS="$(OPTCXXFLAGS) $(ADDFLAGS)"

FORCE:
