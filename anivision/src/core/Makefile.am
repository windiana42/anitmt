INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I..
noinst_LIBRARIES = libavcore.a

EXTRA_DIST = grammar-test.txt

libavcore_a_SOURCES = \
	aniscope.h aniscope.cc \
	registertn.cc fixinctype.cc \
	animation.h \
	a_anisetobj.h a_anisetobj.cc a_animation.cc a_setting.cc a_object.cc \
	anicontext.h anicontext.cc \
	a_function.h a_function.cc a_userfunc.cc a_internalfunc.cc \
		a_ifuncsimple.cc a_ifuncpov.cc a_ifuncwait.cc \
	a_anonscope.h a_anonscope.cc \
	a_variable.h a_variable.cc a_uservar.cc a_internalvar.cc a_ivarobj.cc \
		a_ivaraniset.cc a_ivaradb.cc \
	a_incomplete.h a_incomplete.cc \
	execthread.h execthread.cc \
	aniinstance.h aniinstance.cc i_object.cc

check: ../anivision$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
	../anivision$(EXEEXT) -ani TheAni $(srcdir)/grammar-test.txt

xFORCE:

