SUBDIRS = valtype sourcepos numerics objdesc pov-scan ani-parser \
	pov-core depgraph calccore core
INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
	
bin_PROGRAMS = anivision
LIBS = -lm

anivision_SOURCES = \
	anivision.h anivision.cc

anivision_LDADD = \
	$(top_builddir)/hlib/libhlib.a $(LIBS) \
	ani-parser/libaniparser.a \
	sourcepos/libsourcepos.a \
	core/libavcore.a \
	valtype/libvaltype.a \
	ani-parser/libaniparser.a \
	pov-core/libpovcore.a \
	pov-scan/libpovscanner.a \
	calccore/libcalccore.a \
	calccore/scc/libccsimple.a \
	calccore/anitmt/libccanitmt.a \
	calccore/libcalccore.a \
	numerics/libnumerics.a \
	objdesc/libobjdesc.a \
	numerics/spline/libnumerics_spline.a \
	numerics/diff_int/libnumerics_diff_int.a \
	numerics/linalg/libnumerics_linalg.a \
	numerics/libnumerics.a \
	$(top_builddir)/hlib/libhlib.a \
	\
	/home/wolfgang/projsrc/aniTMT/cvs-build/anitmt-calc/libanitmt.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/anitmt-calc/functionality/libfunctionality.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/message/libmessage.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/param/libparam.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/par/libpar.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/val/libval.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/solve/libsolve.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/proptree/libproptree.a \
	/home/wolfgang/projsrc/aniTMT/cvs-build/lib/utl/libutl.a \
	\
	-lstdc++ 
