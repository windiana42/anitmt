Build instructions for HLIB:
===========================


A) If you got hlib via CVS: 
   -----------------------

Okay, first, you will have to generate the configure script, the Makefile.in 
files and config.h.in. For that you need GNU autoconf and GNU automake. 

bash# aclocal && autoconf && autoheader && automake

If that fails, make sure you are using recent versions of automake and 
autoconf. (I'm using autoconf-2.53, automake-1.6)

Now, go on as if you got hlib as source tarball (B): 


B) If you got hlib as source tarball:
   ---------------------------------

First, I recommend creating a separate build directory. Yes, you can also 
build hlib in the source dir but I strongly recommend against that. 
So, change into the build directory and run configure: 

bash# ../<sourcedir>/configure

You may want to pass some of the standard options to configure; 
try configure --help. 

See what configure writes on the terminal. If everything works fine, 
you can actually build hlib. Please use GNU make. All that automake-generated 
stuff works best with GNU make. If you are not using GNU make, you may need 
to pass AR=ar RM=rm, etc. Note that using GNU make can be as simple as 
calling "gmake all" instead of "make all". 
First, try using 

bash# make opt

If that fails immediately, try 

bash# make opt-generic

If that also fails, you are probably in trouble. Make sure you are using 
the GNU C/C++ compiler (gcc). I am using gcc-3.0, but hlib should also build 
with gcc-2.95.x. In case gcc is unavailable, you may fall back to 

bash# make 

or check what CFLAGS/CXXFLAGS are set by make opt-generic and pass all those 
which do not make problems. (Note: exceptions and runtime type identification 
are not used by hlib; but virtual functions are)

Note that you may use ADDFLAGS to pass additional flags to both the C and C++ 
compiler. If you are using gcc version 3 or above you may find it convenient 
to use 

make ADDFLAGS="-fmessage-length=$COLUMNS"

Okay, if you have successfully build hlib, run the check programs: 
If you compiled with any of the make opt-xxx, then call 

bash# make check-xxx

Important: If you compiled with "make opt", then run 

bash# make check-opt

If you compiled with simple "make" or "make all" or passed special 
CFLAGS/CXXFLAGS, then run 

make# make [your flags] check

Running the check programs is highly recommended. If they do not fail, then 
that is a good sign, if something fails then you should look very carefully 
at what failed. I do not recommend using hlib if something failed, but if 
the feature which failed is not used at all, there should not be a problem. 
Note: If some check really fails, make should report that error as the check 
program returns an error (nonzero exit code), 


C) Troubleshooting...
   ---------------

If the configure script failed, check the reason for the failure. It may 
just be the case that some include file declaring a required function is 
not checked for (for example, on FreeBSD you may need netinet/ip_compat.h 
instead of netinet/ip.h). 

NOTE: The configure script checks for several header files but the result 
of the check may be unused. I generally do `lazy error correction´ which 
means that I only put complicated #ifdef stuff around #include alternatives 
if problems actually show up and I do NOT spend hours thinking about what 
could be checked. So, if you have problems and you know how to fix them, 
please drop me an e-mail (check the source code file for the address). 

Supported platforms include: 
* Linux/GNU: 
   - i[3456]86
   - alpha
   - ppc
   - sparc64
* FreeBSD:
   - i[3456]86
* SunOS (tested: SunOS 5.8): 
   - sparc (sparc64 should also be ok)
* Cygwin:
   - i[3456]86

NOT SUPPORTED platforms include: 
* ppc-darwin
  alias MacOS X. 
  To hell, FreeBSD is a good system, but why did they at Apple have to 
  come up with that tainted derivate? Why couldn't they leave the central 
  syscall poll(2) in the system?!
  [Please correct me in case I am just too stupid.]
* i[3456]86-win32
  M$ Windooze (any flavour)
  Sorry, not POSIX compliant at all. Please use Cygwin. 

Okay, if your system does not belong to the supported architectures: 
If hlib compiles & runs without problems, please send me an e-mail so that 
the arch can be added here. 
If you do have problems: I wish you happy porting... (And keep one thing in 
mind: the more POSIX-conform the better your chances.) 

If compilation failed: If you did not use gcc, please consider using gcc. 
If gcc failed (versions since 2.95): 
Is the issue some damn undefined symbol at linking: 
- The function may be inlined and the compiler did get that wrong somehow. 
  gcc-3.0 does not have that problem but gcc-2.95 has if you compile without 
  "make opt-generic" or "make opt". 
- Are all libraries present or does some compatibility lib need to be added?
If the issue is a real compiler error. Please track it down, fix it and send 
me the fix. 

Contact: wwieser @ gmx . de
