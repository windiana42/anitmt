INCLUDES = -I../include/
noinst_LIBRARIES = libcrypto.a
libcrypto_a_SOURCES = \
	base64.h base64dec.c base64enc.c


check_PROGRAMS = check_crypto
check_crypto_SOURCES = check_crypto.cc
check_crypto_LDADD = ../libhlib.a $(LIBS)

check: check_crypto$(EXEEXT) ../libhlib.a  xFORCE
	./check_crypto$(EXEEXT)

xFORCE:
