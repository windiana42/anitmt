REQUIREMENTS
------------

To build RendView, you need: 

* gcc:
  tested with version 4.7.1
  If you see any problems with later versions, please email windiana@users.sf.net

* hlib:
  this is also provided on the AniTMT website www.anitmt.de and in the CVS of 
  sf.net/projects/anitmt: http://anitmt.cvs.sf.net/viewvc/anitmt/
  
  Installing the RPM should work. Otherwise, please make sure the executable 
  hlib_config is reachable from the PATH and 
  libhlib is installed as a library such that -lhlib will work


BUILDING RENDVIEW
---------------

This is easy; just follow these steps. All of them besides 
the installation should not be done by root. 
(Note: `#' is used to symbolise the shell prompt, the text left 
       of it is an example for the current directory.) 

1. In the source directory, call aclocal, autoconf and automake:
   
   /home/me/source/rendview# aclocal && autoconf && autoheader && automake

   Alternatively, you may simply call the script ./autogen.sh which does all 
   that for you. Pass option -f to be sure the files are really re-made. 
   (autogen.sh will NOT run configure for you.) 
   
   /home/me/source/rendview# ./autogen.sh
   
2. Create the build directory if it is not already there. 
   You may put this directory anywhere but I recomment using a 
   dir called `rendview-build' at the same level as the rendview 
   source root: 
   
   /home/me/source/rendview# cd ..
   /home/me/source# mkdir rendview-build
   
   Note: You may also build rendview in the source directory but 
         I recomment against this. 

3. Configure rendview: 
   
   /home/me/source# cd rendview-build
   /home/me/source/rendview-build# ../rendview/configure 
   
   Configuring takes a while. You may pass quite a lot of options 
   to the configure script which can be viewed by calling configure 
   with the --help flag. But don't worry, normally everything works 
   fine without supplying any options (unless you don't have some 
   required software installed or your system is somehow non-standard.)
   
   NOTE: On slower computers, and especially if you are using gcc-2.95.x, 
         I highly recommend configuring with --ebable-debug=fast. 
         (I'm using that most of the time.) 

4. Build rendview: 
   This is done by simply calling make in the build directory. 
   
   /home/me/source/rendview-build# make
   
   You may pass several values to make, among them: 
   CFLAGS - set the gcc flags for C sources (there are currently none)
   CXXFLAGS - set the compiler flags for C++ sources. 
   Example: 
   /home/me/source/rendview-build# make CXXFLAGS="-O -g"
   
   NOTE: Passing CXXFLAGS is tricky. Different versions of gcc may  
         need additional flags. (E.g. -DGCC_HACK=1 for gcc versions 
         prior to 3.0)
   HOWEVER, you may save a LOT of build time when disabling optimization 
         (no -O) and debugging symbols (no -g) 

5. Do checks: 
   You may get some internal checks done by calling `make check'. 
   This is, however, not required. 
   
   /home/me/source/rendview-build# make check

   [!! Currently the check programs are built but not executed !!]
   
6. Install rendview: 
   Installing normally has to be done by root 
   
   /home/me/source/rendview-build# su 
   Password: 
   /home/me/source/rendview-build# make install

7. You may clean up the created object files, etc by using 
   
   /home/me/source/rendview-build# make clean
   
   Note that cleaning up will force you to re-build all the 
   code again in case you change the source (or uodate it from the CVS) 
   and thus will result in much longer build times. 

That's it. 
You should now be able to use rendview. 

