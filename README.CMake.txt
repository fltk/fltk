README.CMake.txt - 2014-09-27 - Building and using FLTK with CMake
------------------------------------------------------------------



 CONTENTS
==========

  1   INTRODUCTION TO CMAKE
  2   USING CMAKE TO BUILD FLTK
    2.1   Prerequisites
    2.2   Options
    2.3   Building under Linux with Unix Makefiles
    2.4   Crosscompiling
  3   USING CMAKE WITH FLTK
    3.1   Library names
    3.2   Using Fluid files
  4   DOCUMENT HISTORY


 INTRODUCTION TO CMAKE
=======================

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



 USING CMAKE TO BUILD FLTK
===========================


 PREREQUISITES
---------------

The prerequisites for building FLTK with CMake are staightforward:
CMake 2.6 or later and a recent FLTK 1.3 snapshot.  Installation of
CMake is covered on its web site.

This howto will cover building FLTK with the default options using cmake
under Linux with both the default Unix Makefiles and a MinGW cross compiling
toolchain.  Other platforms are just as easy to use.


 OPTIONS
---------
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
    Where everything will go on install.  Defaults are /usr/local for unix
    and C:\Program Files\FLTK for Windows.

The following are the FLTK specific options.  Platform specific options
are ignored on other platforms.

OPTION_OPTIM
   Extra optimization flags.

OPTION_ARCHFLAGS
   Extra architecture flags.

OPTION_APPLE_X11 - default OFF
   In case you want to use X11 on OSX.  Not currently supported.

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
   FLTK has built in jpeg zlib and png libraries.  These let you use
   system libraries instead, unless CMake can't find them.

OPTION_USE_XINERAMA - default ON
OPTION_USE_XFT - default ON
OPTION_USE_XDBE - default ON
   These are X11 extended libraries.

 BUILDING UNDER LINUX WITH UNIX MAKEFILES
------------------------------------------

After untaring the FLTK source, go to the root of the FLTK tree and type
the following.

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

This will build and install a default configuration FLTK.

Some flags can be changed during the 'make' command, such as:

    make VERBOSE=on

..which builds in verbose mode, so you can see all the compile/link commands.


 CROSSCOMPILING
----------------

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



 USING CMAKE WITH FLTK
=======================

The CMake Export/Import facility can be thought of as an automated
fltk-config.  For example, if you link your program to the fltk
library, it will automatically link in all of its dependencies.  This
includes any special flags.  ie on Linux it includes the -lpthread flag.

This howto assumes that you have FLTK libraries which were built using
CMake, installed.  Building them with CMake generates some CMake helper
files which are installed in standard locations, making FLTK easy to find
and use.

Here is a basic CMakeLists.txt file using FLTK.

------

cmake_minimum_required(VERSION 2.6)

project(hello)

# the following line is required only if you don't install FLTK
# or if find_package can't find your installation directory
# it points to the build directory or the base folder,
# not the library folder.
set(FLTK_DIR /path/to/fltk)

find_package(FLTK REQUIRED NO_MODULE)
include(${FLTK_USE_FILE})

add_executable(hello WIN32 hello.cxx)

target_link_libraries(hello fltk)

------

The set(FLTK_DIR ...) command is a superhint to the find_package command.
This is very useful if you don't install or have a non-standard install.
The find_package command tells CMake to find the package FLTK, REQUIRED
means that it is an error if it's not found.  NO_MODULE tells it to search
only for the FLTKConfig file, not using the FindFLTK.cmake supplied with
CMake, which doesn't work with this version of FLTK.

Once the package is found we include the ${FLTK_USE_FILE} which adds the
FLTK include directories to its knowledge base.  After that your programs
will be able to find FLTK headers.

The WIN32 in the add_executable tells your Windows compiler that this is
a gui app.  It is ignored on other platforms.

 LIBRARY NAMES
---------------

When you use the target_link_libraries command, CMake uses it's own
internal names for libraries.  The fltk library names are:

    fltk     fltk_forms     fltk_images    fltk_gl

and for the shared libraries (if built):

    fltk_SHARED     fltk_forms_SHARED     fltk_images_SHARED    fltk_gl_SHARED

The built-in libraries (if built):

    fltk_jpeg      fltk_png    fltk_z


 USING FLUID FILES
-------------------

CMake has a command named fltk_wrap_ui which helps deal with fluid *.fl
files.  An example of its use is in test/CMakeLists.txt.  Here is a short
summary on its use.

Set a variable to list your C++ files, say CPPFILES.
Set another variable to list your *.fl files, say FLFILES.
Say your executable will be called exec.

Then this is what you do...

fltk_wrap_ui(exec ${FLFILES})
add_executable(exec WIN32 ${CPPFILES} ${exec_FLTK_UI_SRCS})

fltk_wrap_ui calls fluid and generates the required C++ files from the *.fl
files.  It sets the variable, in this case exec_FLTK_UI_SRCS, to the
list of generated files for inclusion in the add_executable command.

The variable FLTK_FLUID_EXECUTABLE which is needed by fltk_wrap_ui is set
when find_package(FLTK REQUIRED NO_MODULE) succeeds.


 DOCUMENT HISTORY
==================

Dec 20 2010 - matt: merged and restructures
May 15 2013 - erco: small formatting tweaks, added some examples
Feb 23 2014 - msurette: updated to reflect changes to the CMake files
