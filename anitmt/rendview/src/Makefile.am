SUBDIRS = lib imgfmt render filter tsource
INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/../hlib/include
bin_PROGRAMS = rendview

rendview_SOURCES = rendview.cpp \
	database.hpp database.cpp \
	gdriver.hpp gdriver.cpp gdriverfact.cpp \
	taskmanager.cpp taskmanager.hpp 
rendview_LDADD = \
	$(top_builddir)/../hlib/libhlib.a \
	lib/libsupport.a \
	render/librender.a \
	filter/libfilter.a \
	render/driver/librenderdriver.a \
	filter/driver/libfilterdriver.a \
	filter/libfilter.a \
	render/librender.a \
	tsource/libtsource.a \
	tsource/local/libtsource_local.a \
	tsource/libtsource.a \
	imgfmt/libimgfmt.a 

