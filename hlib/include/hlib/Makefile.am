HlibHeaders = hconfig.h hlib-config.h \
	cpp/cplusplus.h cpp/refstring.h cpp/linkedlist.h cpp/refstrlist.h \
		cpp/refstrhash.h cpp/growbuffer.h cpp/defop.h cpp/orderedarray.h \
		cpp/cdrecorder.h cpp/btree.h cpp/binsearch.h cpp/heap.h \
		cpp/notifierlist.h cpp/refnode.h \
	fd/procbase.h fd/procmanager.h fd/fdmanager.h fd/htime.h fd/intprocbase.h fd/fdbase.h \
		fd/fdfifobuf.h fd/fdcopybase.h fd/timeoutbase.h fd/timeoutmanager.h fd/searchpath.h \
	par/cmdline.h par/parsource.h par/parintrnl.h par/paramarg.h par/parconsumer.h \
		par/parconsumerovl.h par/parmanager.h par/sindentcout.h par/srccmd.h par/srcfile.h par/srcenv.h \
		par/valhdl.h par/valuehandler.h par/parbase.h par/secthdl.h \
	misc/prototypes.h misc/hlib.h misc/lmalloc.h \
	crypto/base64.h \
	compress/base_cinfile.h compress/gzip_cinfile.h compress/bzip2_cinfile.h \
		compress/pngwrite.h compress/pngread.h

headersincludedir= $(pkgincludedir)/
headersinclude_HEADERS = $(patsubst %.h,../../%.h,$(HlibHeaders))


all: LinkHeaders
opt: LinkHeaders
cplusplus.h: LinkHeaders

clean: FORCE
	rm -f $(HlibHeaders)
# get rid of all headers here...
	rm -f *.h

# `:;' is a bash NOP (No Operation) 
# Newer bash can use "$${i/*\//}" instead of basename (faster!)
LinkHeaders: 
	@echo -n "Checking for dangling header links: " ; \
	headers="`echo *.h`" ; \
	if test "$$headers" != "*.h" ; then \
		for i in $$headers ; do \
			test -f "$$i" && continue ; \
			$(RM) -f "$$i" ; \
			echo -n "*" ; \
		done \
	fi ; \
	echo " OK"
	@echo -n "Setting up header links: " ; \
	notfound="" ; \
	for i in $(HlibHeaders) ; do \
		incname="`basename "$$i"`" ; \
		test -L "$$incname" && continue ; \
		if test -f "$(top_srcdir)/$$i" ; then \
			$(LN_S) "$(top_srcdir)/$$i" "$$incname" ; \
			echo -n "." ; \
		elif test -f "$(top_builddir)/$$i" ; then \
			$(LN_S) "$(top_builddir)/$$i" "$$incname" ; \
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

