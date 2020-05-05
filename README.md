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

## Demo videos with older version of AniTMT

[![flight_camera](/doc/imgs/flight_camera.jpg)](http://youtu.be/bpzJQJiBTDc)
[![robot](/doc/imgs/robot.jpg)](http://youtu.be/WUZZ7e0Wn34)
[![chess](/doc/imgs/chess.jpg)](http://youtu.be/Ez9cTCQDfwA)
[![iaflight](/doc/imgs/iaflight.jpg)](http://youtu.be/RF4hHDXmrjs)
[![gotime](/doc/imgs/gotime.jpg)](http://youtu.be/Os3Z28F0_6I)
[![lego](/doc/imgs/lego.jpg)](http://youtu.be/NzevFstjBl8)
[![nuts](/doc/imgs/nuts.jpg)](http://youtu.be/JZdZi4oEnKA)

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


## Known Problems

0.2.4: tga2avi does not work propperly (green movies and offset to the right) but mkavi
works fine (probably some inprecise type handling when parsing tga headers or
writing avi headers)
