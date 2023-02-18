README.CMake.txt - Building and using FLTK with CMake
-----------------------------------------------------


 CONTENTS
==========

  1     Introduction to CMake
  2     Using CMake to Build FLTK
    2.1   Prerequisites
    2.2   Options
    2.3   Building under Linux with Unix Makefiles
    2.4   Building under Windows with Visual Studio [SUGGESTED DOCS -erco]
    2.5   Building under Windows with MinGW using Makefiles
    2.6   Building under MacOS with Xcode
    2.7   Crosscompiling
  3     Using CMake with FLTK
    3.1   Library Names
    3.2   Using Fluid Files
  4     Document History


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
CMake 3.2.3 or later and a recent FLTK 1.3 release, snapshot, or Git
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

CMAKE_OSX_ARCHITECTURES (macOS only, ignored on other platforms)
    Set this to either "arm64", "x86_64", or a list of both "arm64;x86_64".
    The latter will build "universal apps" on macOS, whereas the former
    will either build Intel (x86_64) or Apple Silicon aka M1 (arm64) apps.
    The default is to build for the host processor architecture.

The following are the FLTK specific options.  Platform specific options
are ignored on other platforms.

OPTION_OPTIM - default EMPTY
   Extra optimization flags for the C and C++ compilers, for instance
   "-Wall -Wno-deprecated-declarations".

OPTION_ARCHFLAGS - default EMPTY
   Extra architecture flags.

OPTION_APPLE_X11 - default OFF
   In case you want to use X11 on macOS.
   Use this only if you know what you do, and if you have installed X11.

OPTION_USE_POLL - default OFF
   Don't use this one, it is deprecated.

OPTION_BUILD_SHARED_LIBS - default OFF
   Normally FLTK is built as static libraries which makes more portable
   binaries.  If you want to use shared libraries, this will build them too.

FLTK_BUILD_FLUID - default ON
    Builds the Fast Light User-Interface Designer ("FLUID").

FLTK_BUILD_FLTK_OPTIONS - default ON
    Builds the FLTK options editor ("fltk-options").

FLTK_BUILD_TEST - default ON
   Builds the test and demo programs in the 'test' directory.

FLTK_BUILD_EXAMPLES - default OFF
   Builds the example programs in the 'examples' directory.

OPTION_CAIRO - default OFF
   Enables support of class Fl_Cairo_Window (all platforms, requires the
   Cairo library) - see README.Cairo.txt.

OPTION_CAIROEXT - default OFF
   Enables extended libcairo support - see README.Cairo.txt.

OPTION_USE_GL - default ON
   Enables OpenGL support.

OPTION_USE_THREADS - default ON
   Enables multithreaded support.

OPTION_LARGE_FILE - default ON
   Enables large file (>2G) support.

OPTION_USE_SYSTEM_LIBJPEG - default ON (macOS: OFF)
OPTION_USE_SYSTEM_LIBPNG  - default ON (macOS: OFF)
OPTION_USE_SYSTEM_ZLIB    - default ON
   FLTK has built in jpeg, zlib, and png libraries.  These options let you
   use system libraries instead, unless CMake can't find them.  If you set
   any of these options to OFF, then the built in library will be used.

OPTION_USE_SVG - default ON
   FLTK has a built-in SVG library and can create (write) SVG image files.
   Turning this option off disables SVG (read and write) support.

OPTION_USE_XINERAMA - default ON
OPTION_USE_XFT      - default ON
OPTION_USE_XCURSOR  - default ON
OPTION_USE_XRENDER  - default ON
   These are X11 extended libraries. These libs are used if found on the
   build system unless the respective option is turned off.

OPTION_USE_CAIRO - default OFF
   Makes all drawing operations use the Cairo library (rather than Xlib)
   producing antialiased graphics (X11 platform, implies OPTION_USE_PANGO).

OPTION_USE_PANGO - default OFF
   Enables use of the Pango library for drawing text. Pango supports all
   unicode-defined scripts and gives FLTK limited support of right-to-left
   scripts. This option makes sense only under X11 or Wayland, and also
   requires Xft.

OPTION_USE_KDIALOG - default ON
   Under the KDE desktop, allows class Fl_Native_File_Chooser to use the
   kdialog utility program to construct its file dialog windows, when that
   utility is available at run time on the system. This option makes sense
   only under X11 or Wayland.

OPTION_USE_WAYLAND - default ON
   Enables the use of Wayland for all window operations, of Cairo for all
   graphics and of Pango for text drawing (Linux+FreeBSD only). Resulting FLTK
   apps use Wayland when a Wayland compositor is available at run-time,
   and use X11 for their window operations otherwise, but keep using
   Cairo and Pango - see README.Wayland.txt.

OPTION_WAYLAND_ONLY - default OFF
   In conjunction with OPTION_USE_WAYLAND, restricts FLTK to support the
   Wayland backend only.

OPTION_ABI_VERSION - default EMPTY
   Use a numeric value corresponding to the FLTK ABI version you want to
   build in the form 1xxyy for FLTK 1.x.y (xx and yy with leading zeroes).
   The default ABI version is 1xx00 (the stable ABI throughout all patch
   releases of one minor FLTK version). The highest ABI version you may
   choose is 1xxyy for FLTK 1.x.y (again with leading zeroes).
   Please see README.abi-version.txt for more information about which
   ABI version to select.

OPTION_PRINT_SUPPORT - default ON
   When turned off, the Fl_Printer class does nothing and the
   Fl_PostScript_File_Device class cannot be used, but the FLTK library
   is somewhat smaller. This option makes sense only on the Unix/Linux
   platform or when OPTION_APPLE_X11 is ON.

OPTION_USE_GDIPLUS - default ON
   Makes FLTK use GDI+ to draw oblique lines and curves resulting in
   antialiased graphics (Windows platform only).

OPTION_USE_SYSTEM_LIBDECOR - default OFF
   This option makes FLTK use package libdecor-0 to draw window titlebars
   under Wayland. It's mainly meant for future use, when that package
   and its plugins will be part of major Linux distributions.

Documentation options: these options are only available if `doxygen' is
   installed and found by CMake. PDF related options require also `latex'.

OPTION_BUILD_HTML_DOCUMENTATION - default ON
OPTION_BUILD_PDF_DOCUMENTATION  - default ON
   These options can be used to switch HTML documentation generation with
   doxygen on. The build targets ('html', 'pdf', or 'docs') need still to
   be executed explicitly.

OPTION_INCLUDE_DRIVER_DOCUMENTATION - default OFF
   This option adds driver documentation to HTML and PDF docs (if ON). This
   option is marked as "advanced" since it is only useful for FLTK developers
   or advanced users.

OPTION_INSTALL_HTML_DOCUMENTATION - default OFF
OPTION_INSTALL_PDF_DOCUMENTATION  - default OFF
   If these options are ON then the HTML and/or PDF docs get installed
   when the 'install' target is executed, e.g. with `make install'. You
   need to select above options OPTION_BUILD_*_DOCUMENTATION as well.

 2.3  Building under Linux with Unix Makefiles
-----------------------------------------------

After unpacking the FLTK source, go to the root of the FLTK tree and type
the following.

    mkdir build
    cd build
    cmake ..
    make
    sudo make install (optional)

IMPORTANT: The trailing ".." on the cmake command must be specified
(it is NOT an ellipsis).                          ^^^^^^^^^^^^^^^^^

This will build and install a default configuration FLTK.

Some flags can be changed during the 'make' command, such as:

    make VERBOSE=on

..which builds in verbose mode, so you can see all the compile/link commands.

Hint: if you intend to build several different versions of FLTK, e.g. a Debug
and a Release version, or multiple libraries with different ABI versions or
options, then use subdirectories in the build directory, like this:

    mkdir build
    cd build
    mkdir Debug
    cd Debug
    cmake -D 'CMAKE_BUILD_TYPE=Debug' ../..
    make
    sudo make install (optional)


 2.4  Building under Windows with Visual Studio
------------------------------------------------

Building with CMake under Visual Studio requires the CMake generator with
the -G command line switch, or the generator can be selected interactively
in the GUI (cmake-gui).

     2.4.1 Visual Studio 7 / .NET
    ------------------------------

    1) Open a "Visual Studio .NET command prompt" window, e.g.

        Start > All Programs > Microsoft Visual Studio .NET >
                Visual Studio .NET Tools > Command Prompt

    2) In the DOS window created above, change the current directory
       to where you've extracted an fltk distribution tar file (or
       snapshot tar file), and run the following commands:

           cd C:\fltk-1.4.x             <-- change to your FLTK directory
           mkdir build                  <-- create an empty directory
           cd build
           cmake -G "Visual Studio 7" -D CMAKE_BUILD_TYPE=Release ..

       IMPORTANT: The trailing ".." on the cmake command must be specified
       (it is NOT an ellipsis).                          ^^^^^^^^^^^^^^^^^

        This will create the file FLTK.sln in the current 'build' directory.

    3) Open Visual Studio 7, and choose File -> Open -> Project,
       and pick the "FLTK.sln" created by step #2 in the 'build' directory.

       (Or, if only one version of the Visual Studio compiler is installed,
       you can just run from DOS: .\FLTK.sln)

    4) Make sure the pulldown menu has either "Release" or "Debug" selected
       in the "Solution Configurations" pulldown menu.

    5) In the "Solution Explorer", right click on:

            Solution 'FLTK' (## projects)

       ..and in the popup menu, choose "Build Solution"

    5) That's it, that should build FLTK.

       The test programs (*.exe) can be found in e.g.

            Release: C:\fltk-1.4.x\build\bin\examples\release\*.exe
              Debug: C:\fltk-1.4.x\build\bin\examples\debug\*.exe

       ..and the FLTK include files (*.H & *.h) your own apps can
       compile with can be found in:

            Release & Debug: C:\fltk-1.4.x\build\FL
            *and* [1] in:    C:\fltk-1.4.x\FL

       ..and the FLTK library files (*.lib) which your own apps can
       link with can be found in:

            Release: C:\fltk-1.4.x\build\lib\release\*.lib
              Debug: C:\fltk-1.4.x\build\lib\debug\*.lib

      [1] If you want to build your own FLTK application directly using
          the build directories (i.e. without "installation") you need
          to include both the build tree (first) and then the FLTK source
          tree in the compiler's header search list.

     2.4.2 Visual Studio 2019 / NMake
    --------------------------------------
     This uses cmake to generate + build FLTK in Release mode using nmake,
     using purely the command line (never need to open the Visual Studio IDE)
     using Multithreaded (/MT) and optimizer level 2 (/O2):

         mkdir build-nmake
         cd build-nmake
         cmake -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /Ob2 /D NDEBUG" -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /Ob2 /D NDEBUG" ..
         nmake all

     ..which results in a colorful percentage output crawl similar to what
     we see with unix 'make'.
                                                -erco@seriss.com
                                                 fltk.coredev - Mar 12 2022


 2.5  Building under Windows with MinGW using Makefiles
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
  cmake -G "Unix Makefiles" -D 'CMAKE_BUILD_TYPE=Debug' ..

Note the path to FLTK ".." in the last command line. Depending on where you
installed CMake you may need to adjust the path's in the alias commands.


2.6  Building under MacOS with Xcode
------------------------------------

Building with CMake under Xcode requires the CMake generator
with the -G command line switch. This step need to be done only once. If any
of the cmake related files are updated, Xcode will rerun cmake for you.

1) Open the MacOS Terminal

2) Change to the directory containing the FLTK project. For example:
     > cd ~/dev/fltk-1.4.x

3) Create a build directory
     > mkdir build
     > cd build

4) If you plan different build versions, it is useful to create another
   subdirectory level
     > mkdir Xcode
     > cd Xcode

5) Let CMake create the required IDE files
     > cmake -G Xcode ../..
   This step should end in the message:
     -- Build files have been written to: .../dev/fltk-1.4.x/build/Xcode

5a) To build the Release version of FLTK, use
      > cmake -G Xcode -D CMAKE_BUILD_TYPE=Release ../..

5b) To create all included libraries instead of using those that come
    with MacOS, use:
      > cmake -G Xcode -D OPTION_USE_SYSTEM_LIBJPEG=Off \
                       -D OPTION_USE_SYSTEM_ZLIB=Off \
                       -D OPTION_USE_SYSTEM_LIBPNG=Off \
                       ../..

6) Launch Xcode from the Finder or from the Terminal:
     > open ./FLTK.xcodeproj
   When Xcode starts, it asks if it should "Autocreate Schemes". Click on
   "Automatically Create Schemes" to confirm.

7) To build and test FLTK, select the scheme "ALL_BUILD" and hit Cmd-B to
   build. Then select the scheme "demo" and hit Cmd-R to run the FLTK Demo.

8) The interactive user interface tool "Fluid" will be located in
   build/Xcode/bin/Debug. The example apps are in .../bin/examples/Debug.
   Static libraries are in .../lib/Debug/

9) The "install" Scheme currently fails because it is run with user permission.


 2.7  Crosscompiling
---------------------

Once you have a crosscompiler going, to use CMake to build FLTK you need
two more things.  You need a toolchain file which tells CMake where your
build tools are.  The CMake website is a good source of information on
this file.  Here's one for MinGW (64-bit) under Linux.

----
# CMake Toolchain File for MinGW-w64 (64-bit) Cross Compilation

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

# which tools to use
set(CMAKE_C_COMPILER   /usr/bin/x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  /usr/bin/x86_64-w64-mingw32-windres)

# here is where the target environment located
set(CMAKE_FIND_ROOT_PATH  /usr/x86_64-w64-mingw32)

# adjust the default behavior of the FIND_XXX() commands:

# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_INSTALL_PREFIX ${CMAKE_FIND_ROOT_PATH}/usr CACHE FILEPATH
   "install path prefix")

# initialize required linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")

# end of toolchain file
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

IMPORTANT: The trailing ".." on the cmake command must be specified
(it is NOT an ellipsis).                          ^^^^^^^^^^^^^^^^^

This will create a default configuration FLTK suitable for mingw/msys and
install it in the /usr/x86_64-w64-mingw32/usr tree.

Note: replace 'x86_64-w64-mingw32' with your cross toolchain location as
required.


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

cmake_minimum_required(VERSION 3.2.3)

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

add_executable(hello WIN32 MACOSX_BUNDLE hello.cxx)
if (APPLE)
  target_link_libraries(hello "-framework cocoa")
endif (APPLE)

target_include_directories(hello PUBLIC ${FLTK_INCLUDE_DIRS})

target_link_libraries(hello fltk)

------

The set(FLTK_DIR ...) command is a superhint to the find_package command.
This is very useful if you don't install or have a non-standard install.
The find_package command tells CMake to find the package FLTK, REQUIRED
means that it is an error if it's not found.  NO_MODULE tells it to search
only for the FLTKConfig file, not using the FindFLTK.cmake supplied with
CMake, which doesn't work with this version of FLTK.

The "WIN32 MACOSX_BUNDLE" in the add_executable tells this is
a GUI app.  It is ignored on other platforms and should always be
present with FLTK GUI programs for better portability.

Once the package is found the CMake variable FLTK_INCLUDE_DIRS is defined
which can be used to add the FLTK include directories to the definitions
used to compile your program using the `target_include_directories()` command.

The target_link_libraries() command is used to specify all necessary FLTK
libraries. Thus, you may have to add fltk_images, fltk_gl, etc…

Note: the variable FLTK_USE_FILE used to include another file in
previous FLTK versions was deprecated since FLTK 1.3.4 and will be
removed in FLTK 1.4.0 (this version) or later (maybe 1.4.1).


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
files. Unfortunately it is broken in CMake 3.4.x but it seems to work in
3.5 and later CMake versions. You can however use add_custom_command()
to achieve the same result. This is a more basic approach and should
work for all CMake versions.

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

You can repeat the add_custom_command for each fluid file or if you
have a large number of them see the CMake/FLTK-Functions.cmake function
FLTK_RUN_FLUID for an example of how to run it in a loop.

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
Jul 05 2017 - matt: added instructions for MacOS and Xcode
Dec 29 2018 - AlbrechtS: add documentation option descriptions
Apr 29 2021 - AlbrechtS: document macOS "universal apps" build setup
