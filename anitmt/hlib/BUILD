Build instructions: 

Do the standard GNU build routine: 
bash# aclocal && autoconf && automake
bash# ./configure
bash# make
(I recommend building in a separate build directory.) 

On i386 I also recommend compiling with special optimizations; 
for this do 
bash# make opt
(Works only in the hlib build root dir, not in subdirs.)

On other platforms using gcc, I recommend adding 
-Os -fno-exceptions -fno-rtti
to the CXXFLAGS to save build time and binay size. 
Furthermore, on GNU platforms, it is highly recommended to add 
-D_GNU_SOURCE 
to CFLAGS and CXXFLAGS (for large file support, etc). 

HLIB builds without problems on 
GNU/Linux with gcc 2.95.*, 3.0.*, glibc >= 2.1 and kernel >= 2.2 

Other setups should either not make trouble at all or be supported 
with a minimal effort. 
