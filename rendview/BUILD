Building RendView
-----------------

People don't want to read novels about building a source package; they simply 
want to get on with it. So, I'll try and make it short. 

Please follow the steps here carefully. 
It's a slightly non-standard build with builddir != srcdir. 

Make sure you have the following 2 directories: 
  hlib-version (version >=1.0.0)
  rendview-version

First, you need to build HLib. 

Building HLib
-------------

- Create the build dir (on the same level as the hlib source dir) 
  and change into it: 
  
  bash# mkdir hlib-build ; cd hlib-build
  
- Run HLib's configure: 
  
  bash# ../hlib-version/configure
  
  Do NOT pass your favourite CFLAGS/CXXFLAGS unless you REALLY know what you 
  are doing. REALLY know means you read the HLib build instructions about 
  this issue. HLib will NOT build with debugging info by default and use 
  nice optimizations. If you are not using Linux/ix86, you may need to 
  change C(XX)FLAGS; read HLlib build instructions in that case. 
  
- If that completes without error (ignore warning about missing GMP), 
  build HLib: 
  
  bash# make ADDFLAGS="-DAllocDebugging=1 -DUSE_GNU_MEMCHECK=0"
  
  Make sure you pass the specified ADDFLAGS as long as RendView is in beta 
  stage (although that implies a non-critical speed penalty it benefits 
  crash protection). For gcc>3.0, you may pass -fmessage-length=$COLUMNS as 
  well. 
  
  PLEASE MAKE SURE to use GNU make whenever possible. On some systems you 
  have to call it "gmake" instead of plain "make". 
  
  If there are errors during compilation, please figure out how to fix 
  them and/or report it to the author so that the incompability can be fixed. 
  HLib definitely builds on linux-ix86, AFAIK also on linux-.., freebsd, 
  sunos, darwin. 
  
- If compilation went without errors (ignore the warnings!), run the checks: 
  
  bash# make check
  
  If that fails, you may try again but if it still fails, you should not go 
  on. Check what you can do; read HLib docu or report a bug. 
  
- Do NOT install HLib. That is NOT needed and I recommend against that. 
  Simply go on building RendView: 


Building RendView
-----------------

- Create the build dir (on the same level as the hlib source and build dir 
  and the rendview source dir) and change into it: 
  
  bash# mkdir rendview-build ; cd rendview-build
  
- Run RendView's configure script: 
  
  bash# ../rendview-version/configure 
  
  (You may need to pass --with-hlib=path/to/hlib-build_dir in case hlib is 
  not found which will not happen if you followed the instructions.) 
  
  Again, do NOT pass any CFLAGS/CXXFLAGS. RendView will automatically use 
  the build flags used by HLib. 

- In case that went without errors, build RendView: 
  
  bash# make
  
- In case that completes successfully (ignore warnings), RendView is built. 
  Now run a tiny test (atually checks only LDR padding issues): 
  
  bash# make check
  
  If that fails, please report that to the author. If you are not using 
  GCC, the reason is probably compiler-generated padding in the LDR 
  data packet structures. LDR will not work. 
  
- In case no error occured, RendView is succcessfully build. 
  Do NOT install RendView. Instead, go on reding INSTALL (it's just a few 
  lines). 
  
  In case an error occured during compilation, check what you can do and/or 
  report a bug so that the incompability can be fixed. RendView definitely 
  compiles on linux-ix86. On SunOS, figure out how NAN can be set and 
  e-mail me...


Debugging
---------

If you want to build HLib and RendView with debugging info, the easiest 
way is the following: 

In hlib build dir:
bash# make clean
bash# make ADDFLAGS="-DAllocDebugging=1 -DUSE_GNU_MEMCHECK=0 -g -fno-omit-frame-pointer"

In RendView build dir:
bash# make clean
bash# make ADDFLAGS="-DDEBUG=1 -g -fno-omit-frame-pointer"

