REQUIREMENTS
------------

To build AniTMT, you need: 

* gcc:
  I recommend using the newest gcc version. 
  AniTMT currently will not build with gcc-3.1 (some minor issue being worked 
  on by us). 
  Building AniTMT using gcc-3.0 or above is generally a good idea since 
  these versions have a lot of template bugs fixed which were present in 
  earlier versions. (Note flex++ below, too.)
  However, AniTMT will currently also build using gcc-2.95.x (preferrably 
  2.95.3 or above). 

If you don't have a generated parser from a tar ball you need: 

* bison: 
  Version 1.28 and 1.35 are known to work. 
  Some versions in between like 1.33 definitely do NOT work. 

* flex++:
  Ugh. If you are using gcc-3.0 or above, you will get into trouble 
  when using flex-2.54a or earlier versions. 
  Sadly, a new version was not yet released (although the problem 
  has been there since the early days of a more standard conform 
  STL in gcc-3)


BUILDING ANITMT
---------------

This is easy; just follow these steps. All of them besides 
the installation should not be done by root. 
(Note: `#' is used to symbolise the shell prompt, the text left 
       of it is an example for the current directory.) 

1. In the source directory, call aclocal, autoconf and automake:
   
   /home/me/source/anitmt# aclocal && autoconf && automake

   Then, do the same in the subdir hlib/ 
   
   /home/me/source/anitmt# ( cd hlib && aclocal && autoconf && automake )
   
   Alternatively, you may simply call the script ./autogen.sh which does all 
   that for you. Pass option -f to be sure the files are really re-made. 
   
   /home/me/source/anitmt# ./autogen.sh
   
2. Create the build directory if it is not already there. 
   You may put this directory anywhere but I recomment using a 
   dir called `anitmt-build' at the same level as the anitmt 
   source root: 
   
   /home/me/source/anitmt# cd ..
   /home/me/source# mkdir anitmt-build
   
   Note: You may also build anitmt in the source directory but 
         I recomment against this. 

3. Configure anitmt: 
   
   /home/me/source# cd anitmt-build
   /home/me/source/anitmt-build# ../anitmt/configure 
   
   Configuring takes a while. You may pass quite a lot of options 
   to the configure script which can be viewed by calling configure 
   with the --help flag. But don't worry, normally everything works 
   fine without supplying any options (unless you don't have some 
   required software installed or your system is somehow non-standard.)

4. Build anitmt: 
   This is done by simply calling make in the build directory. 
   
   /home/me/source/anitmt-build# make
   
   You may pass several values to make, among them: 
   CFLAGS - set the gcc flags for C sources (there are currently none)
   CXXFLAGS - set the compiler flags for C++ sources. 
   Example: 
   /home/me/source/anitmt-build# make CXXFLAGS="-O -g"

5. Do checks: 
   You may get some internal checks done by calling `make check'. 
   This is, however, not required. 
   
   /home/me/source/anitmt-build# make check

   [!! Currently the check programs are built but not executed !!]
   
6. Install anitmt: 
   Installing normally has to be done by root 
   
   /home/me/source/anitmt-build# su 
   Password: 
   /home/me/source/anitmt-build# make install

7. You may clean up the created object files, etc by using 
   
   /home/me/source/anitmt-build# make clean
   
   Note that cleaning up will force you to re-build all the 
   code again in case you change the source (or uodate it from the CVS) 
   and thus will result in much longer build times. 

That's it. 
You should now be able to use anitmt. 

