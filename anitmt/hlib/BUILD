Build instructions for HLIB:
===========================


A) If you got hlib via CVS: 
   -----------------------

Okay, first, you will have to generate the configure script, the Makefile.in 
files and config.h.in. For that you need GNU autoconf and GNU automake. 

bash# aclocal && autoconf && autoheader && automake

If that fails, make sure you are using recent versions of automake and 
autoconf. (I'm using autoconf-2.53, automake-1.6.3)

Now, go on as if you got hlib as source tarball (B): 


B) If you got hlib as source tarball:
   ---------------------------------

Unpack the archive. If you don't know how to do that better go doing 
something else (hint: "tar xvfz ..." or "gzip -dc ... | tar xvf -"). 


C) Configuring hlib:
   ----------------

First, I recommend creating a separate build directory. Yes, you can also 
build hlib in the source dir but I strongly recommend against that. 
So, change into the build directory and run configure: 

bash# ../<sourcedir>/configure

But wait; configuring hlib may be a bit different compared to configuring 
standard GNU sources. PLEASE READ THIS. 
You may want to pass some of the standard options to configure; 
try configure --help. 

However, the compiler flags are an issue of interest here becuase they 
are handeled differently: 

You can build hlib by issuing "make" (i.e. "make all") or by using 
"make debug" which is only available in the top build dir and probably of 
little interest. 

For "make debug", you may pass DBGCFLAGS and DBGCXXFLAGS to configure; 
these flags default to "-g": 

bash# ../<sourcedir>/configure DBGCXXFLAGS="-fno-rtti -fno-exceptions -g"

(I don't fiddle around with debug args a lot; if you need debug, make 
sure to pass the correct ones.) 

For standard "make" ("make all"), pre-setting CFLAGS and CXXFLAGS is 
of NO effect at all (if you use gcc) UNLESS you pass some magic prefix, 
so read on. 

Configure sets these flags by itself but this may, of course, not be what 
you want. So, here is some more detail: 
Configure takes optimization flags, misc flags, warning flags and 
additional flags and puts them together into CFLAGS and CXXFLAGS. 
- optimization flags: -Os -fomit-frame-pointer -fno-exceptions -fno-rtti ...
- misc flags: -D_GNU_SOURCE if on GNU platform
- warning flags: -W -Wall -Wformat ...
- additional flags: -pipe -fmessage-length=$COLUMNS $(ADDFLAGS) ...

So, the only thing you want to tweak around in configure are the 
optimization flags. Configure outputs the autodetect optimization flags 
and if you wish to use your own ones, you may pass CFLAGS and CXXFLAGS 
in a special way prepending "opt> ": 

bash# ../<sourcedir>/configure CFLAGS="opt> -O9 -fmy-secret-engenious-flag" \
    CXXFLAGS="opt> -fremove-all-c++-overhead"

For porting or if you want to change the optimization flags regularly, you 
should simply pass "opt> " for both, CFLAGS and CXXFLAGS to configure. 
Then, no optimization flags are set by configure, BUT you can use ADDFLAGS...

Being smart, configure always appends "additional flags" to CFLAGS and 
CXXFLAGS which are inserted by "make", so you can pass these 
flags to "make" in the following way: 

make ADDFLAGS="-fmessage-length=100" ADDCXXFLAGS="-frepo" ADDCFLAGS="-pedantic"

ADDFLAGS get passed to C and C++ compiler, 
ADDCFLAGS and ADDCXXFLAGS only to the C and C++ compiler, respectively. 

So, for ADDFLAGS, etc. no re-configuring is needed; you simply pass them to 
make. 

Okay, if you really want your compiler flags and no configure-generated 
flags, you can use the following trick: Prepend "zap> " instead of "opt> ": 

bash# ../<sourcedir>/configure --disable-gccpipe \
	CFLAGS="zap> -use -my" CXXFLAGS="zap> -flags -only"

Do NOT pass CFLAGS= and CXXFLAGS= to "make" unless you really know what 
you are doing. Use ADDFLAGS/ADDCFLAGS/ADDCXXFLAGS instead. 


D) Compiling hlib:
   --------------

Okay, when configure finally ran through without errors, compile hlib. 
Simply run 

bash# make

If you chose to pass additional flags, you may use 

bash# make ADDFLAGS="..." ADDCFLAGS="..." ADDCXXFLAGS="..."

To compile with debugging info, use 

bash# make debug

You can use ADDFLAGS for "make debug", too. 

Hey, don't be confused about the #warning messages from the compiler. 
That's okay. Special ones are maked by a box of asterisks. 

If compiling fails, consider changing the optimization flags (e.g. using 
configure with C[XX]FLAGS="opt> " and using ADDFLAGS to test which flags 
work). 
If compile failure is not related to optimization flags, you are probably in trouble. Make sure you are using the 
GNU C/C++ compiler (gcc). I am using gcc-3.2, but hlib should also build 
with any version since gcc-2.95. 

Okay, if you have successfully build hlib, run the check programs: 

If you compiled with "make" or "make all", run 

bash# make [your additional flags] check

If you built with "make debug", you should use "make check-debug" instead. 

Running the check programs is highly recommended. If they do not fail, then 
that is a good sign, if something fails then you should look very carefully 
at what failed. I do not recommend using hlib if something failed, but if 
the feature which failed is not used at all, there should not be a problem. 
Note: If some check really fails, make should report that error as the check 
program returns an error (nonzero exit code), 


D) Troubleshooting...
   ---------------

If the configure script failed, check the reason for the failure. It may 
just be the case that some include file declaring a required function is 
not checked for (for example, on FreeBSD you may need netinet/ip_compat.h 
instead of netinet/ip.h). 

NOTE: The configure script checks for several header files but the result 
of the check may be unused. I generally do `lazy error correction' which 
means that I only put complicated #ifdef stuff around #include alternatives 
if problems actually show up and I do NOT spend hours thinking about what 
could be checked. So, if you have problems and you know how to fix them, 
please drop me an e-mail (check the source code file for the address). 

Supported platforms include: 

* Linux/GNU: 
	- i[3456]86
	- alpha      (alphaev67-unknown-linux-gnu)  [64 bit]
	- sparc64    (sparc64-unknown-linux-gnu)    [32 bit (!)]
	- powerpc    (powerpc-unknown-linux-gnu)
* FreeBSD:
   - i[3456]86   (i386-unknown-freebsd4.6)
* SunOS: 
   - sparc       (sparc-sun-solaris2.8)
* Cygwin:
   - i[3456]86   [untested for 0.9.7, worked with 0.9.6]

SEMI-SUPPORTED platforms include: 
* powerpc-darwin     (powerpc-apple-darwin5.5)
  alias MacOS X. 
  [To hell, FreeBSD is a good system, but why did they at Apple have to 
  come up with that tainted derivate? Why couldn't they leave the central 
  syscall poll(2) in the system?!]
  So, hlib has an emulation for poll(2) using select(2). 
  But note that the emulation makes hlib slower and that it is not 
  capable of emulating all aspets of poll(2). So, things MAY work but 
  again, applications may also NOT work as expected. 

NOT SUPPORTED platforms include: 
* i[3456]86-win32
  M$ Windooze (any flavour)
  Sorry, not POSIX compliant at all. Please use Cygwin. 

Okay, if your system does not belong to the supported architectures: 
If hlib compiles & runs without problems, please send me an e-mail so that 
the arch can be added here. 
If you do have problems: I wish you happy porting... (And keep one thing in 
mind: the more POSIX-conform the better your chances.) 
If you were successful, please e-mail me and send me fixes to the configure 
script and/or source code if available. 

If compilation failed: If you did not use gcc, please consider using gcc. 
If gcc failed (versions since 2.95): 
Is the issue some damn undefined symbol at linking: 
- The function may be inlined and the compiler did get that wrong somehow. 
  gcc>=3.0 does not have that problem but gcc-2.95 has if you do not pass 
  optimization flags. 
- Are all libraries present or does some compatibility lib need to be added?
If the issue is a real compiler error: Please track it down, fix it and send 
me the fix. 

Contact: wwieser AT gmx DOT de
