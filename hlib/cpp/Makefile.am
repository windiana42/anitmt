INCLUDES = -I../include/
noinst_LIBRARIES = libcpp.a
libcpp_a_SOURCES = \
	cfexit.cc cplusplusff.cc cplusplusffa.cc \
		refstring.cc refstringw.cc refstringp.cc \
		refstrlist.cc refstrhash.cc \
		growbuffer.cc \
		refnode.cc \
	cplusplus.h linkedlist.h refstrlist.h refstrhash.h refstring.h \
		growbuffer.h \
		defop.h \
		orderedarray.h cdrecorder.h btree.h binsearch.h notifierlist.h heap.h \
		refnode.h

check_PROGRAMS = test-btree test-cpp
test_btree_SOURCES = test-btree.cc
test_btree_LDADD = ../libhlib.a $(LIBS)

check: test-btree$(EXEEXT) ../libhlib.a  xFORCE
	./test-btree$(EXEEXT)

test_cpp_SOURCES = test-cpp.cc
test_cpp_LDADD = ../libhlib.a $(LIBS)

xFORCE:

