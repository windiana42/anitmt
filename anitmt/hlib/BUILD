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

First, I recommend creating a separate build directory. Yes, you can also 
build hlib in the source dir but I strongly recommend against that. 
So, change into the build directory and run configure: 

bash# ../<sourcedir>/configure

You may want to pass some of the standard options to configure; 
try configure --help. 

There are two ways to build hlib: 
Using "make opt" and using "make all" (or simple "make"). 
In the latter case, CFLAGS and CXXFLAGS are passed to the compiler. 
In the first case, OPTCFLAGS and OPTCXXFLAGS take their place. 
I recommend using some nice optimization, especially to switch off 
RTTI (gcc: -fno-rtti) and exceptions (gcc: -fno-exceptions). 
(Do not switch off virtual functions if you compiler allows that). 
If you do not pass OPTCFLAGS and OPTCXXFLAGS, then configure will choose 
nice ones for some special architectures only. Configure dumps the used 
flags quite early and you should stop and re-run configure with more 
suitable compiler flags if you are not satisfied, especially if you are 
not using the GNU C/C++ compiler. (Try your favourite ones.)

Note that if you are using gcc-3.0 or above, you may export COLUMNS so 
that -fmessage-length=$COLUMNS is passed to the compiler. 

See what configure writes on the terminal. If everything works fine, 
you can actually build hlib. Please use GNU make. All that automake-generated 
stuff works best with GNU make. If you are not using GNU make, you may need 
to pass AR=ar RM=rm, etc. Note that using GNU make can be as simple as 
calling "gmake all" instead of "make all". 
First, try using 

bash# make opt

If that fails, you may consider changing your OPTCFLAGS / OPTCXXFLAGS in 
case the error is related to them. 
Otherwise you are probably in trouble. Make sure you are using the 
GNU C/C++ compiler (gcc). I am using gcc-3.2, but hlib should also build 
with any version since gcc-2.95. 
In case gcc is unavailable you are already using gcc, you may fall back to 

bash# make 

Note that you may use ADDFLAGS to pass additional flags to both the C and C++ 
compiler during "make opt". 

Okay, if you have successfully build hlib, run the check programs: 
If you compiled with any of the make opt, then call 

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

SEMI-SUPPORTED platforms include: 
* ppc-darwin
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
  gcc-3.0 does not have that problem but gcc-2.95 has if you compile without 
  "make opt". 
- Are all libraries present or does some compatibility lib need to be added?
If the issue is a real compiler error: Please track it down, fix it and send 
me the fix. 

Contact: wwieser AT gmx DOT de
