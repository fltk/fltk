README.CMake.txt - Building and using FLTK with CMake
-----------------------------------------------------


 CONTENTS
==========

  1	Introduction to CMake
  2	Using CMake to Build FLTK
    2.1   Prerequisites
    2.2   Options
    2.3   Building under Linux with Unix Makefiles
    2.4   Building under Windows with MinGW using Makefiles
    2.5   Crosscompiling
  3	Using CMake with FLTK
    3.1   Library Names
    3.2   Using Fluid Files
  4	Document History


 1.  INTRODUCTION TO CMAKE
===========================

CMake was designed to let you create build files for a project once and
then compile the project on multiple platforms.

Using it on any platform consists of the same steps.  Create the
CMakeLists.txt build file(s).  Run one of the CMake executables, picking
your source directory, build directory, and build target.  The "cmake"
executable is a one-step process with everything specified on the command
line.  The others let you select options interactively, then configure
and generate your platform-specific target.  You then run the resulting
Makefile / project file / solution file as you normally would.

CMake can be run in up to three ways, depending on your platform.  "cmake"
is the basic command line tool.  "ccmake" is the curses based interactive
tool.  "cmake-gui" is the gui-based interactive tool.  Each of these will
take command line options in the form of -DOPTION=VALUE.  ccmake and
cmake-gui will also let you change options interactively.

CMake not only supports, but works best with out-of-tree builds.  This means
that your build directory is not the same as your source directory or with a
complex project, not the same as your source root directory.  Note that the
build directory is where, in this case, FLTK will be built, not its final
installation point.  If you want to build for multiple targets, such as
VC++ and MinGW on Windows, or do some cross-compiling you must use out-of-tree
builds exclusively.  In-tree builds will gum up the works by putting a
CMakeCache.txt file in the source root.

More information on CMake can be found on its web site http://www.cmake.org.



 2.  Using CMake to Build FLTK
===============================


 2.1  Prerequisites
--------------------

The prerequisites for building FLTK with CMake are staightforward:
CMake 2.6.3 or later and a recent FLTK 1.3 release, snapshot, or subversion
download (working copy).  Installation of CMake is covered on its web site.

This howto will cover building FLTK with the default options using CMake
under Linux and MinGW with Unix Makefiles. Chapter 2.5 shows how to use
a MinGW cross compiling toolchain to build a FLTK library for Windows
under Linux.  Other platforms are just as easy to use.


 2.2  Options
--------------
Options can be specified to cmake with the -D flag:

    cmake -D <OPTION_NAME>=<OPTION_VALUE>

Example:

    cmake -D CMAKE_BUILD_TYPE=Debug

All options have sensible defaults so you won't usually need to touch these.
There are only two CMake options that you may want to specify:

CMAKE_BUILD_TYPE
    This specifies what kind of build this is i.e. Release, Debug...
    Platform specific compile/link flags/options are automatically selected
    by CMake depending on this value.

CMAKE_INSTALL_PREFIX
    Where everything will go on install.  Defaults are /usr/local for Unix
    and C:\Program Files\FLTK for Windows.

The following are the FLTK specific options.  Platform specific options
are ignored on other platforms.

OPTION_OPTIM
   Extra optimization flags.

OPTION_ARCHFLAGS
   Extra architecture flags.

OPTION_APPLE_X11 - default OFF
   In case you want to use X11 on OSX.
   Use this only if you know what you do, and if you have installed X11.

OPTION_USE_POLL - default OFF
   Don't use this one either.

OPTION_BUILD_SHARED_LIBS - default OFF
   Normally FLTK is built as static libraries which makes more portable
   binaries.  If you want to use shared libraries, this will build them too.

OPTION_BUILD_EXAMPLES - default ON
   Builds the many fine example programs.

OPTION_CAIRO - default OFF
   Enables libcairo support

OPTION_CAIROEXT - default OFF
   Enables extended libcairo support

OPTION_USE_GL - default ON
   Enables OpenGL support

OPTION_USE_THREADS - default ON
   Enables multithreaded support

OPTION_LARGE_FILE - default ON
   Enables large file (>2G) support

OPTION_USE_SYSTEM_LIBJPEG - default ON
OPTION_USE_SYSTEM_ZLIB - default ON
OPTION_USE_SYSTEM_LIBPNG - default ON
   FLTK has built in jpeg, zlib, and png libraries.  These let you use
   system libraries instead, unless CMake can't find them.  If you set
   any of these options to OFF, then the built in library will be used.

OPTION_USE_XINERAMA - default ON
OPTION_USE_XFT - default ON
OPTION_USE_XDBE - default ON
OPTION_USE_XCURSOR - default ON
OPTION_USE_XRENDER - default ON
   These are X11 extended libraries.

OPTION_ABI_VERSION - default EMPTY
   Use a numeric value corresponding to the FLTK ABI version you want to
   build in the form 1xxyy for FLTK 1.x.y (xx and yy with leading zeroes).
   The default ABI version is 1xx00 (the stable ABI throughout all patch
   releases of one minor FLTK version). The highest ABI version you may
   choose is 1xxyy for FLTK 1.x.y (again with leading zeroes).
   Please see README.abi-version.txt for more information about which
   ABI version to select.


 2.3  Building under Linux with Unix Makefiles
-----------------------------------------------

After untaring the FLTK source, go to the root of the FLTK tree and type
the following.

    mkdir build
    cd build
    cmake ..
    make
    sudo make install (optional)

This will build and install a default configuration FLTK.

Some flags can be changed during the 'make' command, such as:

    make VERBOSE=on

..which builds in verbose mode, so you can see all the compile/link commands.

Hint: if you intend to build several different versions of FLTK, e.g. a Debug
and a Release version, or multiple libraries with different ABI versions,
then use subdirectories in the build directory, like this:

    mkdir build
    cd build
    mkdir Debug
    cd Debug
    cmake ../..
    make
    sudo make install (optional)


 2.4  Building under Windows with MinGW using Makefiles
--------------------------------------------------------

Building with CMake under MinGW requires you to specify the CMake Generator
with the -G command line switch. Using

  cmake -G "Unix Makefiles" /path/to/fltk

is recommended by the FLTK team if you have installed MinGW with the MSYS
environment. You can use the stock Windows CMake executables, but you must
run the CMake executables from within the MinGW environment so CMake can
use your MinGW PATH to find the compilers and build tools. Example:

  alias cmake='/c/CMake/bin/cmake'
  alias cmake-gui='/c/CMake/bin/cmake-gui'

  mkdir build
  cd build
  cmake -G "Unix Makefiles" ..

Note the path to FLTK ".." in the last command line. Depending on where you
installed CMake you may need to adjust the path's in the alias commands.


 2.5  Crosscompiling
---------------------

Once you have a crosscompiler going, to use CMake to build FLTK you need
two more things.  You need a toolchain file which tells CMake where your
build tools are.  The CMake website is a good source of information on
this file.  Here's mine for MinGW under Linux.

----
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which tools to use
set(CMAKE_C_COMPILER   /usr/bin/i486-mingw32-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/i486-mingw32-g++)

# here is where the target environment located
set(CMAKE_FIND_ROOT_PATH  /usr/i486-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# search headers and libraries in the target environment,
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_INSTALL_PREFIX ${CMAKE_FIND_ROOT_PATH}/usr CACHE FILEPATH
   "install path prefix")
----

Not too tough.  The other thing you need is a native installation of FLTK
on your build platform.  This is to supply the fluid executable which will
compile the *.fl into C++ source and header files.

So, again from the FLTK tree root.

    mkdir mingw
    cd mingw
    cmake -DCMAKE_TOOLCHAIN_FILE=~/projects/toolchain/Toolchain-mingw32.cmake ..
    make
    sudo make install

This will create a default configuration FLTK suitable for mingw/msys and
install it in the /usr/i486-mingw32/usr tree.



 3.  Using CMake with FLTK
===========================

The CMake Export/Import facility can be thought of as an automated
fltk-config.  For example, if you link your program to the FLTK
library, it will automatically link in all of its dependencies.  This
includes any special flags, i.e. on Linux it includes the -lpthread flag.

This howto assumes that you have FLTK libraries which were built using
CMake, installed.  Building them with CMake generates some CMake helper
files which are installed in standard locations, making FLTK easy to find
and use.

Here is a basic CMakeLists.txt file using FLTK.

------

cmake_minimum_required(VERSION 2.6.3)

project(hello)

# The following line is required only if (a) you didn't install FLTK
# or if (b) find_package can't find your installation directory because
# you installed FLTK in a non-standard location.  It points to
# (a) the base folder of the build directory, or
# (b) <fltk-install-prefix>/share/fltk
# resp., where <fltk-install-prefix> is the installation prefix you
# used to install FLTK.
# (The file FLTKConfig.cmake and others must be found in that path.)

set(FLTK_DIR /path/to/fltk)

find_package(FLTK REQUIRED NO_MODULE)

include_directories(${FLTK_INCLUDE_DIRS})

add_executable(hello WIN32 hello.cxx)
# target_include_directories(hello PUBLIC ${FLTK_INCLUDE_DIRS})

target_link_libraries(hello fltk)

------

The set(FLTK_DIR ...) command is a superhint to the find_package command.
This is very useful if you don't install or have a non-standard install.
The find_package command tells CMake to find the package FLTK, REQUIRED
means that it is an error if it's not found.  NO_MODULE tells it to search
only for the FLTKConfig file, not using the FindFLTK.cmake supplied with
CMake, which doesn't work with this version of FLTK.

Once the package is found the CMake variable FLTK_INCLUDE_DIRS is defined
which can be used to add the FLTK include directories to the definitions
used to compile your program. In older CMake versions you may need to use
`include_directories()` as shown above. In more recent CMake versions you
can use the (commented) `target_include_directories()` command. The latter
should be preferred (YMMV, see the CMake docs).

The WIN32 in the add_executable tells your Windows compiler that this is
a Windows GUI app.  It is ignored on other platforms and should always be
present with FLTK GUI programs for better portability.

Note: the variable ${FLTK_USE_FILE} used to include another file in
previous FLTK versions is deprecated since FLTK 1.3.4 and will be removed
in FLTK 1.4.0.


 3.1  Library Names
--------------------

When you use the target_link_libraries command, CMake uses its own
internal names for libraries.  The fltk library names are:

    fltk     fltk_forms     fltk_images    fltk_gl

and for the shared libraries (if built):

    fltk_SHARED     fltk_forms_SHARED     fltk_images_SHARED    fltk_gl_SHARED

The built-in libraries (if built):

    fltk_jpeg      fltk_png    fltk_z


 3.2  Using Fluid Files
------------------------

CMake has a command named fltk_wrap_ui which helps deal with fluid *.fl
files. Unfortunately it is broken in CMake 3.4.x. You can however use
add_custom_command to achieve the same result.
This is a more basic approach and should work for all CMake versions.

Here is a sample CMakeLists.txt which compiles the CubeView example from
a directory you've copied the test/Cube* files to.

---
cmake_minimum_required(VERSION 2.6.3)

project(CubeView)

# change this to your fltk build directory
set(FLTK_DIR /home/msurette/build/fltk-release/)

find_package(FLTK REQUIRED NO_MODULE)
include_directories(${FLTK_INCLUDE_DIRS})

#run fluid -c to generate CubeViewUI.cxx and CubeViewUI.h files
add_custom_command(
	OUTPUT "CubeViewUI.cxx" "CubeViewUI.h"
	COMMAND fluid -c ${CMAKE_CURRENT_SOURCE_DIR}/CubeViewUI.fl
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(CubeView WIN32 CubeMain.cxx CubeView.cxx CubeViewUI.cxx)

target_link_libraries(CubeView fltk fltk_gl)
---

You can repeat the add_custom_command for each fluid file or if you have
a large number of them see the CMake/macros.cmake function FLTK_RUN_FLUID
for an example of how to run it in a loop.

The two lines

  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add the current build ("binary") and source directories as include directories.
This is necessary for the compiler to find the local header files since the
fluid-generated files (CubeViewUI.cxx and CubeViewUI.h) are created in the
current build directory.


 DOCUMENT HISTORY
==================

Dec 20 2010 - matt: merged and restructures
May 15 2013 - erco: small formatting tweaks, added some examples
Feb 23 2014 - msurette: updated to reflect changes to the CMake files
Apr 07 2015 - AlbrechtS: update use example and more docs
Jan 31 2016 - msurette: custom command instead of fltk_wrap_ui
Nov 01 2016 - AlbrechtS: remove deprecated FLTK_USE_FILE, add MinGW build
