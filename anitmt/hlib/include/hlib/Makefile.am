HlibHeaders = config.h \
	cpp/cplusplus.h cpp/refstring.h cpp/linkedlist.h cpp/strlist.h cpp/growbuffer.h \
	fd/procbase.h fd/procmanager.h fd/fdmanager.h fd/htime.h fd/intprocbase.h fd/fdbase.h \
	par/cmdline.h par/parsource.h par/parintrnl.h par/paramarg.h par/parconsumerbase.h \
		par/parconsumerovl.h par/parmanager.h par/sindentcout.h par/srccmd.h par/srcfile.h \
	misc/prototypes.h misc/hlib.h 

all: LinkHeaders
opt: LinkHeaders

clean: FORCE
	rm -f $(HlibHeaders)
# get rid of all headers here...
	rm -f *.h

# `:;' is a bash NOP (No Operation) 
LinkHeaders: 
	@echo -n "Checking for dangling header links: " ; \
	headers="`echo *.h`" ; \
	if test "$$headers" != "*.h" ; then \
		for i in $$headers ; do \
			test -f "$$i" && continue ; \
			rm -f "$$i" ; \
			echo -n "*" ; \
		done \
	fi ; \
	echo " OK"
	@echo -n "Setting up header links: " ; \
	notfound="" ; \
	for i in $(HlibHeaders) ; do \
		incname="$${i/*\//}" ; \
		test -L "$$incname" && continue ; \
		if test -f "$(top_srcdir)/$$i" ; then \
			ln -s "$(top_srcdir)/$$i" "$$incname" ; \
			echo -n "." ; \
		else \
			echo -n "?" ; \
			notfound="$$notfound $$incname" ; \
		fi \
	done ; \
	if test "x$$notfound" = "x" ; then \
		echo " OK" ; exit 0 ; \
	else \
		echo " Oops" ; \
		echo "The following headers could not be linked: $$notfound" ; \
		echo "They are needed so please fix that." ; \
		exit 1 ; \
	fi

FORCE:

