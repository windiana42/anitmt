if HAVE_LIBZ
    LIBZ_DEP_SRC = gzip_cinfile.h gzip_cinfile.cc
else
    LIBZ_DEP_SRC = 
endif

if HAVE_LIBBZ2
    LIBBZ2_DEP_SRC = bzip2_cinfile.h bzip2_cinfile.cc
else
    LIBBZ2_DEP_SRC = 
endif

if HAVE_LIBPNG
    LIBPNG_DEP_SRC = \
        pngwrite.h pngwrite.cc \
        pngread.h pngread.cc
else
    LIBPNG_DEP_SRC = 
endif

INCLUDES = -I../include/
noinst_LIBRARIES = libcompress.a
libcompress_a_SOURCES = \
	base_cinfile.h \
	plain_cinfile.h plain_cinfile.cc \
	$(LIBZ_DEP_SRC) \
	$(LIBBZ2_DEP_SRC) \
	$(LIBPNG_DEP_SRC)
