#CXXFLAGS = @CXXFLAGS@ -W -Wall ...
#
OPTCXXFLAGS = -W -Wall -Wformat \
	-Os -march=i486 -fno-rtti -fno-exceptions -fomit-frame-pointer \
	-malign-jumps=2 -malign-loops=2 -malign-functions=2 \
	-DHLIB_SIZE_OPT=1
OPTCFLAGS = -W -Wall -Wformat \
	-Os -march=i486 -fomit-frame-pointer \
	-malign-jumps=2 -malign-loops=2 -malign-functions=2 \
	-DHLIB_SIZE_OPT=1

# Be careful changing this line (do not leave away any spaces). 
SUBDIRS = cpp misc par

noinst_LIBRARIES = libhlib.a

libhlib.a: FORCE
	@objs="" ; for i in $(SUBDIRS) ; do \
		for o in `$(AR) -t "$$i/lib$$i.a"` ; do \
			if test -f $$i/$$o ; then objs="$$objs $$i/$$o" ; \
			else echo "Archive: $$i/$$o missing" ; fi ; \
	done ; done ; \
	$(RM) -f libhlib.a ; \
	$(AR) -r libhlib.a $$objs ; \
	$(RANLIB) libhlib.a 

opt: FORCE
	$(MAKE) CFLAGS="$(OPTCFLAGS)" CXXFLAGS="$(OPTCXXFLAGS)"

FORCE:
