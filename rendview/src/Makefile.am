SUBDIRS = lib imgfmt tdriver tsource admin adminshell
INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
bin_PROGRAMS = rendview
# LIBS = -lm

rendview_SOURCES = rendview.cpp \
	database.hpp database.cpp \
	taskmanager.cpp taskmanager.hpp 
rendview_LDADD = \
	lib/libsupport.a \
	tdriver/local/libtdif_local.a \
	tdriver/libtdriver.a \
	tdriver/render/libtdrender.a \
	tdriver/filter/libtdfilter.a \
	tdriver/render/driver/libtdrender_driver.a \
	tdriver/filter/driver/libtdfilter_driver.a \
	tdriver/filter/libtdfilter.a \
	tdriver/render/libtdrender.a \
	tdriver/local/libtdif_local.a \
	tdriver/ldr/libdif_ldr.a \
	tdriver/libtdriver.a \
	tsource/libtsource.a \
	tsource/local/libtsource_local.a \
	tsource/ldr/libtsource_ldr.a \
	tsource/libtsource.a \
	imgfmt/libimgfmt.a \
	admin/libadmin.a \
	lib/libsupport.a 	
