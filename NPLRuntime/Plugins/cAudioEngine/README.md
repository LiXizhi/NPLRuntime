# Version 2.3.0 dev

This is the development branch for cAudio 2.3.0 development.
If you need a stable version of cAudio please use the 2.2.0 tag.

Author Contact
=============
Name: Raynaldo Rivera

Twitter: @R4stl1n

Email: royal.r4stl1n@gmail.com

## Change log
*Nothing yet

## Windows

To compile:

Install CMake if you don't already have it. http://www.cmake.org/cmake/resources/software.html

To generate makefiles use: `cmake . -DCAUDIO_DEPENDENCIES <Path to the Dependencies folder>`. You can also use the graphical CMake utility and set the option in the list before you run generate.

## Linux

This is the build steps for building on a cleanly installed Ubuntu 11.10 system. Other systems should be similar and the build easy to adapt. If you run in to problems post an issue here and if you adapt these steps for another Linux distribution please contribute the information back so it can be included here. Solutions for common problems can be found after the build description.

* First we need some basic build tools. The command `sudo apt-get install build-essential git cmake` will set you up with all you need for this build.

* Then lets get some place for the project to live. `mkdir -p ~/Documents/projects` will set up what I use as a root for all my projects. (You can, of cause, choose any directory you like for this. The GIT command will make a cAudio sub directory for you so you don't have to make that.)
Lets move in to the new directory `cd ~/Documents/projects`.

* Next step is to clone the Github repo for cAudio. Just run `git clone https://github.com/wildicv/cAudio.git` to get your local copy of the source.

* When the clone is done move in to the new directory `cd cAudio/CMake`.

* Install the libraries needed for cAudio to build `sudo apt-get install libopenal-dev libogg-dev`. This will get you the OpenAL and OGG development files (headers and binaries).

* You are now all set to generate the project of your choice. CMake defaults to unix makefiles but you can specify another with the -G switch. Run `cmake ../ -DCAUDIO_DEPENDENCIES_DIR=../Dependencies/` for a 32bit build or `cmake ../ -DCAUDIO_DEPENDENCIES_DIR=../Dependencies64/` for a 64bit build.

* If CMake exits cleanly, run `make -j4` (The -j switch means 'jobs' and tells make how many threads to run. Twice the number of cores you have is recommended.)
The project will now build and you can run an optional `sudo make install` to install the build .so files and headers system wide.

## Common problems and solutions

Here is a collection of problems and solutions that have come up while compiling on Linux

### Errors on config_types.h

On older versions of Linux you might get errors like:


```
In file included from /home/thijs/Downloads/cAudio/DependenciesSource/libogg-1.2.2/include/ogg/os_types.h:143,
                 from /home/thijs/Downloads/cAudio/DependenciesSource/libogg-1.2.2/include/ogg/ogg.h:25,
                 from /home/thijs/Downloads/cAudio/DependenciesSource/libvorbis-1.3.2/src/analysis.c:21:
/usr/include/ogg/config_types.h:5: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘ogg_int16_t’
/usr/include/ogg/config_types.h:6: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘ogg_uint16_t’
/usr/include/ogg/config_types.h:7: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘ogg_int32_t’
/usr/include/ogg/config_types.h:8: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘ogg_uint32_t’
/usr/include/ogg/config_types.h:9: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘ogg_int64_t’
```

If you do it's an indication that your version of libogg is to old. The version required to function with cAudio is, probably, 1.2.0 or higher (the build described here uses 1.2.2). Try finding updated packages (on Ubuntu the packages are libogg0 and libogg-dev) for your distribution or build it from source that you'll find at http://xiph.org/downloads/ ...

Building libogg from source is a simple process, it's a small library. Just follow the normal path of:

1. Download sources
2. Unpack the downloaded tar.gz somewhere
3. Change to the directory with the unpacked source
4. `./configure`
5. `make`
6. `make install` (as root)
