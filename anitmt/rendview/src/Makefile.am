SUBDIRS = lib imgfmt tdriver tsource
INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/../hlib/include
bin_PROGRAMS = rendview

rendview_SOURCES = rendview.cpp \
	database.hpp database.cpp \
	taskmanager.cpp taskmanager.hpp 
rendview_LDADD = \
	$(top_builddir)/../hlib/libhlib.a \
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
	tdriver/libtdriver.a \
	tsource/libtsource.a \
	tsource/local/libtsource_local.a \
	tsource/ldr/libtsource_ldr.a \
	tsource/libtsource.a \
	imgfmt/libimgfmt.a \
	lib/libsupport.a \
	$(top_builddir)/../hlib/libhlib.a 	

