INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src \
	-I/home/wolfgang/projsrc/aniTMT/cvs-anitmt/ \
	-I/home/wolfgang/projsrc/aniTMT/cvs-anitmt/anitmt-calc/funcgen \
	-I/home/wolfgang/projsrc/aniTMT/cvs-anitmt/anitmt-calc \
	-I/home/wolfgang/projsrc/aniTMT/cvs-anitmt/anitmt-calc/functionality \
	-I/home/wolfgang/projsrc/aniTMT/cvs-anitmt/lib \
	-I/home/wolfgang/projsrc/aniTMT/cvs-anitmt/lib/flex \
	-I/home/wolfgang/projsrc/aniTMT/cvs-build/anitmt-calc \
	-I/home/wolfgang/projsrc/aniTMT/cvs-build/anitmt-calc/functionality

noinst_LIBRARIES = libccanitmt.a

libccanitmt_a_SOURCES = \
	ccif_anitmt.h \
	factory.h factory.cc \
	object.cc setting.cc

#check_PROGRAMS = test
#test_SOURCES = test.cc
#test_LDADD = \
#	$(top_builddir)/hlib/libhlib.a $(LIBS) \
#	libpovcore.a \
#	../pov-scan/libpovscanner.a \
#	../ani-parser/libaniparser.a \
#	../core/libavcore.a \
#	../sourcepos/libsourcepos.a \
#	../valtype/libvaltype.a \
#	$(top_builddir)/hlib/libhlib.a \
#	libpovcore.a 
#
#check: test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
#	./test$(EXEEXT) $(srcdir)/../pov-scan/test.pov

xFORCE:

