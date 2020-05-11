# AniTMT / AniVision

## Introduction

AniTMT and AniVision are two animation systems built on top of the raytracer
povray. AniTMT was built by Jan Theofel, Manuel Moser and Martin Trautmann as
a project for the german national competition "jugend Forscht". AniVision was
build by Wolfgang Wieser who met Martin on the german national computer
science competition.

Both systems differ a bit in concept. While AniVision primarily assumes
animation actions that are appended in time, AniTMT includes a solver network
which allows to throw in as many attributes about the movements as you like
and remaining information is calculated or default-setted as needed. In fact
both systems were finally integrated so you can use AniTMT as a description
language within AniVision.

In another way of describing the difference, AniTMT exploits the descriptive
power of a declarative language whereas AniVision allows for more complex
expressions in an imparative style. 

## Demo videos AniTMT

[![circles](/doc/imgs/circles.jpg)](http://youtu.be/kqSyDJFjLyk)

## Demo videos with older version of AniTMT

[![flight_camera](/doc/imgs/flight_camera.jpg)](http://youtu.be/bpzJQJiBTDc)
[![robot](/doc/imgs/robot.jpg)](http://youtu.be/WUZZ7e0Wn34)
[![chess](/doc/imgs/chess.jpg)](http://youtu.be/Ez9cTCQDfwA)
[![iaflight](/doc/imgs/iaflight.jpg)](http://youtu.be/RF4hHDXmrjs)
[![gotime](/doc/imgs/gotime.jpg)](http://youtu.be/Os3Z28F0_6I)
[![lego](/doc/imgs/lego.jpg)](http://youtu.be/NzevFstjBl8)
[![nuts](/doc/imgs/nuts.jpg)](http://youtu.be/JZdZi4oEnKA)

The old version has a less meta-programmed description language. However,
after finishing the really cool new meta programming description (in fact we
generate the code that generates code for generating scene descriptions that
generate images) nobody put much energy into developing the animation language
that makes use of all the generating power.
[see solver description example](/anitmt/anitmt-calc/functionality/scalar.afd).

## Install

You can download precompiled packages for many RPM based distributions from
[here](https://build.opensuse.org/project/show/home:trautm).

If you like to build the software without rpm spec, you can do this as
follows (make -j4 runs make with 4 processes, you can adjust this as you like):
```bash
pushd anitmt
   ./autogen.sh
   mkdir build
   cd build
   ../configure
   make -j4
   make install
   cd -
popd

pushd hlib
   ./autogen.sh
   mkdir build
   cd build
   ../configure
   make -j4
   make install
   cd -
popd

pushd rendview
   ./autogen.sh
   mkdir build
   cd build
   ../configure --with-hlib="$(pwd)/../../hlib/build"
   make -j4
   make install
   cd -
popd

pushd anivision
   ./autogen.sh
   mkdir build
   cd build
   ../configure --with-hlib="$(pwd)/../../hlib/build" --with-anitmt-build="$(pwd)/../../anitmt/build" --with-anitmt-src="$(pwd)/../../anitmt"
   make -j4
   make install
   cd -
popd
```

### Run AniTMT Examples

```bash
cd anitmt/examples/
   ./make_examples
cd -
```

The examples are very small. Have a look at the make_examples script. There
are some commented commands which render a much nicer version of the circles example.

### Run AniVision Examples

```bash
cd anivision/examples/
   ./make_examples
cd -
```

### Creating better compressed movies

The AniTMT Examples use tga2avi and mkavi for converting images to
videos. These days, much better tools exist as open source software. Here, one
example:

`ffmpeg -framerate 25 -i f%07d.tga -c:v libx264 -profile:v high -crf 20
-pix_fmt yuv420p output.mp4` [taken from here](https://askubuntu.com/questions/610903/how-can-i-create-a-video-file-from-a-set-of-jpg-images)

If you are a fan of free video codes, please send me another example
and I will put it here as well.

## Build and install without root permissions (replace <user> or full prefix directory)

- for each `configure` call add `--prefix=/home/<user>/progs/local`
- please bear in mind that when running make_examples or anitmt-calc you need
to set `LD_LIBRARY_PATH=/home/<user>/progs/local/lib64`

### Run AniTMT Examples with prefix in user home

```bash
cd anitmt/examples/
   LD_LIBRARY_PATH=/home/<user>/progs/local/lib64 ./make_examples
cd -
```

### Run AniVision Examples with prefix in user home

```bash
cd anivision/examples/
   LD_LIBRARY_PATH=/home/<user>/progs/local/lib64 ./make_examples
cd -
```

## Cross-compiling anitmt for Windows with a Linux Machine

Cross-compiling windows executables from linux generally works. Look in /usr
how your system compilier is called and replace `x86_64-suse-linux` in my
example (https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/html_node/Specifying-Target-Triplets.html#Specifying-Target-Triplets).

```bash
pushd anitmt
   ./autogen.sh
   mkdir build-windows
   cd build-windows
   ../configure --prefix=/usr/x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --build=x86_64-suse-linux --target=x86_64-suse-linux
   make -j4
   cd -
popd
```

This will produce build-windows/anitmt-calc/anitmt-calc-static and some error
messages since producing shared library versions fails. libtool seems to be
incompatible with cross-compiling to Windows.


## Known Problems

- 0.2.4:
  - tga2avi does not work propperly (green movies and offset to the right) but mkavi
works fine (probably some inprecise type handling when parsing tga headers or
writing avi headers)
  - anitmt should be split in anitmt-devel and anitmt; proper dependency setup
  of the sub-projects within this repo, potentially using different repos;
  this is mainly an rpmbuild issue and the packaging on opensuse build service
  - convince automake project upstream to introduce a --with-rpath flag in
  configure script generated by aclocal/automake (remove ugly solution in autogen.sh)
