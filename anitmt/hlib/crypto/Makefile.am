if HAVE_GMP
RSA_SRC_C = rsa.cc primetable.cc gmpalloc.cc
RSA_SRC_H = rsa.h primetable.h gmpalloc.h
GMP_CHECK_LDADD = /usr/lib/libgmp.a
else
RSA_SRC_C = 
RSA_SRC_H = 
GMP_CHECK_LDADD = 
endif

INCLUDES = -I../include/
noinst_LIBRARIES = libcrypto.a
libcrypto_a_SOURCES = \
	bcipher.cc nullcipher.cc \
		blowfishcipher.cc ideacipher.cc twofishcipher.cc \
		shash.cc sha0hash.cc sha1hash.cc rmd160hash.cc \
		$(RSA_SRC_C) \
		rndmanager.cc \
	base64.h base64dec.c base64enc.c \
	bcipherbase.h bcipher.h \
		nullcipher.h blowfishcipher.h ideacipher.h twofishcipher.h \
		shashbase.h shash.h sha0hash.h sha1hash.h rmd160hash.h \
		$(RSA_SRC_H) \
		rndmanager.h rndbase.h 


check_PROGRAMS = check_crypto
check_crypto_SOURCES = check_crypto.cc
check_crypto_LDADD = ../libhlib.a $(GMP_CHECK_LDADD) -lm

check: check_crypto$(EXEEXT) ../libhlib.a  xFORCE
	./check_crypto$(EXEEXT)

xFORCE:
