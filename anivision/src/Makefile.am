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
	$(top_builddir)/anitmt-build/anitmt-calc/libanitmt-static.a \
	\
	-lstdc++ 
