README.CMake.txt - Building and using FLTK with CMake
------------------------------------------------------


 CONTENTS
==========

  1     Introduction to CMake

  2     Using CMake to Build FLTK

    2.1    Prerequisites
    2.2    Options
    2.2.1  CMake Specific Configuration Options
    2.2.2  FLTK Specific Configuration Options
    2.2.3  Documentation Options
    2.2.4  Special Options
    2.2.5  Other CMake Cache Variables
    2.3    Building FLTK with CMake (all Platforms)
    2.4    Building under Linux with Unix Makefiles
    2.5    Building under Windows with Visual Studio and/or NMake
    2.5.1  Building under Windows with Visual Studio
    2.5.2  Building under Windows with NMake
    2.6    Building under Windows with MinGW using Makefiles
    2.7    Building under Windows WSL with Clang using Makefiles
    2.8    Building under macOS with Xcode
    2.9    Crosscompiling

  3     Using CMake with FLTK

    3.1    Library Names
    3.2    Library Aliases
    3.3    Exported and Imported Targets
    3.4    Building a Simple "Hello World" Program with FLTK
    3.5    Building a Program Using Fluid Files
    3.6    Building a Program Using CMake's FetchContent Module


  4     FindFLTK.cmake and find_package(FLTK)

 1.  Introduction to CMake
===========================

CMake was designed to let you create build files for a project once and
then compile the project on multiple platforms.

Using it on any platform consists of the same steps. Create the
CMakeLists.txt build file(s). Run one of the CMake executables, picking
your source directory, build directory, and build target. The "cmake"
executable is a one-step process with everything specified on the command
line. The others let you select options interactively, then configure
and generate your platform-specific target. You then run the resulting
Makefile / project file / solution file as you normally would.

CMake can be run in up to three ways, depending on your platform. "cmake"
is the basic command line tool. "ccmake" is the curses based interactive
tool. "cmake-gui" is the gui-based interactive tool. Each of these will
take command line options in the form of -DOPTION=VALUE. ccmake and
cmake-gui will also let you change options interactively.

CMake not only supports, but works best with out-of-tree builds. This means
that your build directory is not the same as your source directory or with a
complex project, not the same as your source root directory. Note that the
build directory is where, in this case, FLTK will be built, not its final
installation point. If you want to build for multiple targets, such as
VC++ and MinGW on Windows, or do some cross-compiling you must use out-of-tree
builds exclusively. In-tree builds will gum up the works by putting a
CMakeCache.txt file in the source root.

More information on CMake can be found on its web site https://www.cmake.org.



 2.  Using CMake to Build FLTK
===============================


 2.1  Prerequisites
--------------------

The prerequisites for building FLTK with CMake are staightforward:
CMake 3.15 or later and a recent FLTK release, snapshot, or Git download
(working copy). Installation of CMake is covered on its web site.

This howto will cover building FLTK with the default options using CMake
under Linux and MinGW with Unix Makefiles. Chapter 2.5 shows how to use
a MinGW cross compiling toolchain to build a FLTK library for Windows
under Linux. Other platforms are just as easy to use.


 2.2  Options
--------------

Options can be specified to CMake with the -D flag:

    cmake -D <OPTION_NAME>=<OPTION_VALUE>

Example:

    cmake -D CMAKE_BUILD_TYPE=Debug

Notes: the space between '-D' and the option name can be omitted.
Option values must be quoted if they contain spaces.

Other CMake tools are `ccmake` and `cmake-gui` but these are not
described here.

All options have sensible defaults so you won't usually need to specify
them explicitly.


 2.2.1  CMake Specific Configuration Options
---------------------------------------------

There are only three CMake options that you may want to specify:

CMAKE_BUILD_TYPE
    This specifies what kind of build this is i.e. Release, Debug...
    Platform specific compile/link flags/options are automatically selected
    by CMake depending on this value.

CMAKE_INSTALL_PREFIX
    Where everything will go on install. Defaults are /usr/local for Unix
    and C:\Program Files\FLTK for Windows.

CMAKE_OSX_ARCHITECTURES (macOS only, ignored on other platforms)
    Set this to either "arm64", "x86_64", or a list of both "arm64;x86_64".
    The latter will build "universal apps" on macOS, whereas the former
    will either build Intel (x86_64) or Apple Silicon aka M1 (arm64) apps.
    The default is to build for the host processor architecture.

Note: the CMake variable BUILD_SHARED_LIBS is ignored by FLTK. FLTK builds
    static libs by default and can optionally build shared libs as well.
    Please see FLTK_BUILD_SHARED_LIBS instead.


 2.2.2  FLTK Specific Configuration Options
--------------------------------------------

Following are the FLTK specific options. Platform specific options are
ignored on other platforms. For convenience the list of options is ordered
alphabetically except "Documentation Options" and "Special Options" that
follow in their own sections below.


FLTK_ABI_VERSION - default EMPTY
    Use a numeric value corresponding to the FLTK ABI version you want to
    build in the form 1xxyy for FLTK 1.x.y (xx and yy with leading zeroes).
    The default ABI version is 1xx00 (the stable ABI throughout all patch
    releases of one minor FLTK version). The highest ABI version you may
    choose is 1xxyy for FLTK 1.x.y (again with leading zeroes).
    Please see README.abi-version.txt for more information about which
    ABI version to select.

FLTK_ARCHFLAGS - default EMPTY
    Extra "architecture" flags used as C and C++ compiler flags.
    These flags are also "exported" to fltk-config.

FLTK_BACKEND_WAYLAND - default ON (only Unix/Linux)
    Enable the Wayland backend for all window operations, Cairo for all
    graphics and Pango for text drawing (Linux+FreeBSD only). Resulting FLTK
    apps use Wayland when a Wayland compositor is available at runtime,
    and use X11 for their window operations otherwise (unless FLTK_BACKEND_X11
    is OFF), but keep using Cairo and Pango - see README.Wayland.txt.
    If FLTK_BACKEND_X11 has been turned OFF and there is no Wayland compositor
    at runtime, then FLTK programs fail to start.

FLTK_BACKEND_X11 - default ON on Unix/Linux, OFF elsewhere (Windows, macOS).
    Enable or disable the X11 backend on platforms that support it.
    - Unix/Linux: enable or disable the X11 backend when building with
      Wayland (FLTK_BACKEND_WAYLAND), otherwise this option must be ON.
    - macOS: enable the X11 backend instead of standard system graphics.
      This requires XQuartz or a similar X11 installation. This option is
      tested only with XQuartz by the FLTK team.
      Use this only if you know what you do and if you have installed X11.
    - Windows/Cygwin: enable X11 backend for Cygwin platforms. This option
      is currently (as of FLTK 1.4.0) not supported on Windows.

Note: On platforms that support Wayland you may set FLTK_BACKEND_WAYLAND=ON
    (this is the default) and FLTK_BACKEND_X11=OFF to build a Wayland-only
    library or vice versa for an X11-only library.

FLTK_BUILD_EXAMPLES - default OFF
    Build the example programs in the 'examples' directory.

FLTK_BUILD_FLTK_OPTIONS - default ON
    Build the FLTK options editor ("fltk-options").

FLTK_BUILD_FLUID - default ON
    Build the Fast Light User-Interface Designer ("fluid").

FLTK_BUILD_FORMS - default ON
    Build the (X)Forms compatibility library. This option is ON by default
    for backwards compatibility but can safely be turned OFF if you don't
    need (X)Forms compatibility.

FLTK_BUILD_GL - default ON
    Build the OpenGL support library fltk_gl (fltk::gl) and enable OpenGL
    support in user programs using fltk_gl.

FLTK_BUILD_SHARED_LIBS - default OFF
    Normally FLTK is built as static libraries which makes more portable
    binaries. If you want to use shared libraries, this will build them too.
    You can use shared FLTK libs in your own CMake projects by appending
    "-shared" to FLTK target names as described in section 3.1 and 3.2.

FLTK_BUILD_TEST - default ON in top-level build, OFF in sub-build
    Build the test and demo programs in the 'test' directory. The default
    is ON if the FLTK build is in a top-level project so all test and demo
    programs are built. If FLTK is built as a subproject only the Library
    and the tools (fluid and fltk-config) are built by default.

FLTK_GRAPHICS_CAIRO - default OFF (Unix/Linux: X11 + Wayland only).
    Make all drawing operations use the Cairo library (rather than Xlib),
    producing antialiased graphics (X11 platform: implies FLTK_USE_PANGO).
    When using Wayland this option is ignored (Wayland uses Cairo).

FLTK_GRAPHICS_GDIPLUS - default ON (Windows only).
    Make FLTK use GDI+ to draw oblique lines and curves resulting in
    antialiased graphics. If this option is OFF standard GDI is used.

FLTK_MSVC_RUNTIME_DLL - default ON (Windows: Visual Studio, NMake, clang).
    Select whether the build uses the MS runtime DLL (ON) or not (OFF).
    Default is ON: either /MD or /MDd for Release or Debug, respectively.
    Select OFF for either /MT or /MTd for Release or Debug, respectively.
    If this variable is defined on other platforms it is silently ignored.

FLTK_OPTION_CAIRO_EXT - default OFF
    Enable extended libcairo support - see README.Cairo.txt.

FLTK_OPTION_CAIRO_WINDOW - default OFF
    Enable support of class Fl_Cairo_Window (all platforms, requires the
    Cairo library) - see README.Cairo.txt.

FLTK_OPTION_FILESYSTEM_SUPPORT - default ON

FLTK_OPTION_LARGE_FILE - default ON
    Enables large file (>2G) support.

FLTK_OPTION_OPTIM - default EMPTY
    Extra optimization flags for the C and C++ compilers, for instance
    "-Wall -Wno-deprecated-declarations". Example:
    cmake -D FLTK_BUILD_EXAMPLES=on -D FLTK_OPTION_OPTIM="-Wall -Wextra -pedantic" ..

FLTK_OPTION_PRINT_SUPPORT - default ON
    When turned off, the Fl_Printer class does nothing and the
    Fl_PostScript_File_Device class cannot be used, but the FLTK library
    is somewhat smaller. This option makes sense only on the Unix/Linux
    platform or on macOS when FLTK_BACKEND_X11 is ON.

FLTK_OPTION_STD - default OFF
    This option allows FLTK to use some specific features of modern C++
    like std::string in the public API of FLTK 1.4.x. Users turning this
    option ON can benefit from some new functions and methods that return
    std::string or use std::string as input parameters.
    Note: This option will be removed in the next minor (1.5.0) or major
    release which will use std::string and other modern C++ features.

FLTK_OPTION_SVG - default ON
    FLTK has a built-in SVG library and can create (write) SVG image files.
    Turning this option off disables SVG (read and write) support.

FLTK_USE_LIBDECOR_GTK - default ON (Wayland only).
    Meaningful only under Wayland and if FLTK_USE_SYSTEM_LIBDECOR is 'OFF'.
    Allows to use libdecor's GTK plugin to draw window titlebars. Otherwise
    FLTK does not use GTK and apps will not need linking to GTK.

FLTK_USE_PANGO - default OFF (see note below)
    This option is highly recommended under X11 if FLTK is expected to draw
    text that does not use the latin alphabet.
    Enables use of the Pango library for drawing text. Pango supports all
    unicode-defined scripts and gives FLTK limited support of right-to-left
    scripts. This option makes sense only under X11 or Wayland, and also
    requires Xft.
    This option is ignored (always enabled) if Wayland or FLTK_GRAPHICS_CAIRO
    is ON.

FLTK_USE_POLL - default OFF
    Deprecated: don't turn this option ON.

FLTK_USE_PTHREADS - default ON except on Windows.
    Enables multithreaded support with pthreads if available.
    This option is ignored (switched OFF internally) on Windows except
    when using Cygwin.

FLTK_USE_SYSTEM_LIBDECOR - default ON (Wayland only)
    This option makes FLTK use package libdecor-0-dev to draw window titlebars
    under Wayland. When OFF or when this package has a version < 0.2.0, FLTK
    uses its bundled copy of libdecor to draw window titlebars.

FLTK_USE_SYSTEM_LIBJPEG - default ON (macOS and Windows: OFF)
FLTK_USE_SYSTEM_LIBPNG  - default ON (macOS and Windows: OFF)
FLTK_USE_SYSTEM_ZLIB    - default ON (macOS and Windows: OFF)
    FLTK has built in jpeg, zlib, and png libraries. These options let you
    use system libraries instead, unless CMake can't find them. If you set
    any of these options to OFF, then the built in library will be used.
    The default is ON on Linux/Unix platforms but OFF on Windows and macOS
    because of potential incompatibilities on Windows and macOS whereas
    the system libraries can typically be used on Linux/Unix.
    Note: if any one of libpng or zlib is not found on the system, both
    libraries are built using the bundled ones and a warning is issued.

FLTK_USE_XCURSOR  - default ON
FLTK_USE_XFIXES   - default ON
FLTK_USE_XFT      - default ON
FLTK_USE_XINERAMA - default ON
FLTK_USE_XRENDER  - default ON
    These are X11 extended libraries. These libs are used if found on the
    build system unless the respective option is turned off.


 2.2.3  Documentation Options
------------------------------

  These options are only available if `doxygen' is installed and found.
  PDF related options require also `latex'.

FLTK_BUILD_HTML_DOCS - default ON
FLTK_BUILD_PDF_DOCS  - default ON
    These options can be used to enable HTML documentation generation with
    doxygen. If these are ON the build targets 'html', 'pdf', and 'docs'
    are generated but must be built explicitly. Technically the build targets
    are generated but excluded from 'ALL'.
    You can safely leave these two options ON if you want to save build time
    because the docs are not built automatically.

FLTK_BUILD_FLUID_DOCS - default OFF
    If this option is ON, the FLUID user documentation will be built. If
    FLTK_BUILD_PDF_DOCS is ON, the FLUID documentation will be generated
    in PDF form. To generate the screen shots used in the handbook,
    the CMake build mode must be set to "Debug".

FLTK_INCLUDE_DRIVER_DOCS - default OFF
    This option adds driver documentation to HTML and PDF docs (if ON). This
    option is marked as "advanced" since it is only useful for FLTK developers
    and advanced users. It is only used if at least one of the documentation
    options above is ON as well.

FLTK_INSTALL_HTML_DOCS - default OFF
FLTK_INSTALL_FLUID_DOCS - default OFF
FLTK_INSTALL_PDF_DOCS  - default OFF
    If these options are ON then the HTML, FLUID, and/or PDF docs are installed
    when the 'install' target is executed, e.g. with `make install'. You
    need to select above options FLTK_BUILD_*_DOCS as well.


 2.2.4  Special Options
------------------------

FLTK_INSTALL_LINKS - default OFF
    Deprecated: install "compatibility" links to compensate for typos in
    include statements (for case sensitive file systems only).
    You should not use this option, please fix the sources instead for
    better cross-platform compatibility.


 2.2.5  Other CMake Cache Variables
------------------------------------

The following CMake cache variables can be used to view their computed values
in the CMake cache or to change the build behavior in special cases. To view
the variables

 - use `cmake -LA` or
 - use `cmake-gui` (switch 'Advanced' view ON) or
 - use `ccmake` (hit 't' to "Toggle advanced mode")
 - search the CMake cache 'CMakeCache.txt' with your favorite tool.

Use either the `cmake` commandline, `cmake-gui`, or `ccmake` to change these
variables if needed.

CMake cache variables can also be preset using a toolchain file (see below)
and on the commandline.


FLTK_FLUID_EXECUTABLE - default = fltk::fluid (see exceptions below)

    This represents the `fluid` executable or CMake target that is used
    to "compile" fluid `.fl` files to source (.cxx) and header (.h) files.

    The default `fltk::fluid` is used when `fluid` is built and not
    cross-compiled, i.e. the fluid executable that is built can be used.
    On Windows and macOS `fltk::fluid-cmd` (the console program) is used
    instead.

    When cross-compiling this variable should be a compatible `fluid`
    executable on the build host. For details see chapter 2.9.

FLTK_FLUID_HOST - default = fluid executable on the build host

    This variable is used if `fluid` is not built (FLTK_BUILD_FLUID=OFF)
    or if cross-compiling. It can be preset by the CMake commandline to
    point the build at a compatible `fluid` executable.

FLUID_PATH - obsolete (FLTK 1.3.x): predecessor of FLTK_FLUID_HOST

    This variable can safely be deleted from the CMake cache if it exists.


 2.3  Building FLTK with CMake (all Platforms)
-----------------------------------------------

CMake is used to generate a build system that will subsequently be used
to build and install the FLTK library and test and demo applications.

Note that "installing" FLTK is optional: you can skip this step if you
like to build your own applications directly from the FLTK build tree.
This has advantages if you are building FLTK with different options or
are developing FLTK (changing sources) or if you pull new FLTK versions
from git frequently.

The following generic commands may need some changes on Windows where
you may not have an adequate (POSIX) shell (command window).

(1) Generate the build system in the FLTK root directory:

    cmake -B build [ -G "Generator" -D "Options" … ]

  This command creates the 'build' subdirectory if it does not exist yet
  and generates the build (project) files in the 'build' directory.
  See above for usable options.

  Note: Although this 'build' directory is part of the source tree it
  is considered an out-of-source build because CMake does not create
  any files in source directories. You can also use CMake to build FLTK
  in an arbitrary build folder elsewhere on the system:

    cmake -B /path/to/my-fltk-build [ -G "Generator" -D "Options" … ]

  Commandline elements in […] are optional.

  Use `cmake --help` to find out which generators are available on your
  platform. The default generator is marked with '*'.

(2) Build FLTK with the generated build system:

  No matter which generator you selected in (1), the following CMake
  command can always be used to build the library:

    cmake --build build

  This uses the previously generated build system in the 'build' folder.
  This works even with Visual Studio where the build will be executed
  without opening the Visual Studio GUI, similar to NMake.

  Instead of using the above command you can also `cd build` and run
  the native build command, for instance `make -j7` or `ninja`, or
  you can open the IDE project (Xcode, Visual Studio, ...).

(3) Install FLTK (optional):

    cmake --install build

  This command installs the previously built library and headers in the
  installation folder. On Unix/Linux/macOS systems this requires root
  privileges if the target is a system directory.

The following chapters describe some special cases in more detail. Skip
chapters you don't need...


 2.4  Building under Linux with Unix Makefiles
-----------------------------------------------

After unpacking the FLTK source, go to the root of the FLTK tree and type
the following.

    cmake -B build -G "Unix Makefiles" [options]
    cd build
    make [ -j 3 ]
    sudo make install (optional)

This will build and optionally install a default configuration FLTK.

Some flags can be changed during the 'make' command, such as:

    make VERBOSE=on

which builds in verbose mode, so you can see all the compile/link commands.

Hint: if you intend to build several different versions of FLTK, e.g. a Debug
and a Release version, or multiple libraries with different ABI versions or
options, then use subdirectories in the build directory, like this:

    mkdir build
    cd build
    mkdir debug
    cd debug
    cmake -D 'CMAKE_BUILD_TYPE=Debug' ../..
    make
    sudo make install (optional)


 2.5  Building under Windows with Visual Studio and/or NMake
-------------------------------------------------------------

Building with CMake under Visual Studio may require to specify the CMake
generator with the -G"Visual Studio ..." command line switch, or the
generator can be selected interactively in the GUI (cmake-gui). If you
are not sure which one to select use `cmake --help` which lists all
generators known to CMake on your system.


 2.5.1 Building under Windows with Visual Studio
-------------------------------------------------

CMake often finds an installed Visual Studio generator and uses it w/o
using the commandline switch, particularly if you are using a special
"Visual Studio Command Prompt":

  - Hit the "Windows" key

  - Type "developer command ..."
    ... until you see something like "Developer Command Prompt for VS xxxx"
    (replace 'xxxx' with your installed Visual Studio version)

  - Activate the "app" to execute the command prompt (like an old "DOS" shell)

  - Inside this command prompt window, run your installed `cmake` (command
    line) or `cmake-gui` (GUI) program. You may need to specify the full path
    to this program.

  If you use `cmake-gui` you can select the source and the build folders in
  the GUI, otherwise change directory to where you downloaded and installed
  the FLTK sources and execute:

      `cmake` -G "Visual Studio xxx..." -B build
      cd build

  This creates the Visual Studio project files (FLTK.sln and more) in the
  'build' directory.

  Open Visual Studio, choose File -> Open -> Project, and pick the "FLTK.sln"
  created in the previous step.

  (Or, if only one version of the Visual Studio compiler is installed,
  you can just run from DOS: .\FLTK.sln)

  Make sure the pulldown menu has either "Release" or "Debug" selected
  in the "Solution Configurations" pulldown menu.

  In the "Solution Explorer", right click on:

      Solution 'FLTK' (## projects)

      ... and in the popup menu, choose "Build Solution"

  or choose 'Build/Build Solution' or 'Build/Rebuild Solution' from the
  menu at the top of the window.

  That's it, that should build FLTK.

  The test programs (*.exe) can be found relative to the 'build' folder in

      build\bin\test\Release\*.exe
      build\bin\test\Debug\*.exe

  ... and the FLTK include files (*.H & *.h) your own apps can
  compile with can be found in:

      build\FL

  *and* [1] in the source folder where you downloaded FLTK, e.g. in

      C:\fltk-1.4.x\FL

  ... and the FLTK library files (*.lib) which your own apps can
  link with can be found in:

      Release: build\lib\Release\*.lib
        Debug: build\lib\Debug\*.lib

  [1] If you want to build your own FLTK application directly using
      the build directories (i.e. without "installation") you need
      to include both the build tree (first) and then the FLTK source
      tree in the compiler's header search list.


 2.5.2 Building under Windows with NMake
-----------------------------------------

  This example uses cmake to generate + build FLTK in Release mode using nmake,
  using purely the command line (never need to open the Visual Studio IDE)
  using the static Multithreaded runtime (/MT):

    mkdir build-nmake
    cd build-nmake
    cmake -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=Release -D FLTK_MSVC_RUNTIME_DLL=off ..
    nmake

  which results in a colorful percentage output crawl similar to what we see
  with unix 'make'.

  Instead of running `nmake` directly you can also use cmake to build:

    cmake --build .


 2.6  Building under Windows with MinGW using Makefiles
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


 2.7  Building under Windows WSL with Clang and Makefiles
----------------------------------------------------------

WSL, the Windows Subsystem for Linux allows developers to run a Linux
environment without the need for a separate virtual machine or dual booting.
WSL 2 runs inside a managed virtual machine that implements the full
Linux kernel. WSL requires Windows 11.

FLTK apps generated using WSL are Linux compatible binaries. To run those
binaries on Windows, WSL comes with a limited built-in X11 server. Third
party X11 servers can be installed that better support all features of FLTK.

1) Install WSL from PowerShell with admin privileges:
     > wsl --install

2) Reboot and open the Linux terminal. You will need to install the following
   Linux packages to compile FLTK
     > sudo apt update
     > sudo apt install clang cmake freeglut3-dev

3) Change to the directory containing the FLTK project. For example:
     > cd ~/dev/fltk-1.4.x

4) Use CMake to configure the build system
     > cmake -B build

5) Use CMake to build the demo app and all dependencies
     > cmake --build build

6) Run the demo app
     > ./build/bin/test/demo


2.8  Building under macOS with Xcode
-------------------------------------

Building with CMake under Xcode requires the CMake generator with
the -G command line switch. This step need to be done only once. If any
of the CMake related files are updated, Xcode will rerun CMake for you.

1) Open the macOS Terminal

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

6) Launch Xcode from the Finder or from the Terminal:
     > open ./FLTK.xcodeproj
   When Xcode starts, it asks if it should "Autocreate Schemes". Click on
   "Automatically Create Schemes" to confirm.

7) To build and test FLTK, select the scheme "ALL_BUILD" and hit Cmd-B to
   build. Then select the scheme "demo" and hit Cmd-R to run the FLTK Demo.

8) The interactive user interface tool "Fluid" will be located in
   build/Xcode/bin/Debug. The example apps are in .../bin/examples/Debug.
   Static libraries are in .../lib/Debug/.
   Replace 'Debug' with 'Release' for a Release build.

9) The "install" Scheme may fail because it is run with user permissions.
   You may want to configure the build to install in a folder below your
   home directory.


 2.9  Crosscompiling
---------------------

Once you have a crosscompiler going, to use CMake to build FLTK you need
two more things. You need a toolchain file which tells CMake where your
build tools are. The CMake website is a good source of information on
this file. Here's one for MinGW (64-bit) under Linux.

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

# initialize required linker flags to build compatible Windows programs
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")

# end of toolchain file
----

Not too tough. The other thing you need is a native installation of FLTK
on your build platform. This is to supply the fluid executable which will
compile the *.fl into C++ source and header files. This is only needed if
the test/* and/or example/* demo programs are built.

CMake finds the fluid executable on the build host automatically when
cross-compiling if it exists and is in the user's PATH. On systems that
provide multiple fluid versions (e.g. 1.3.x and 1.4.x) or if only an older
version is installed (e.g. 1.3.x) you can set the variable `FLTK_FLUID_HOST`
on the cmake commandline like

    cmake -B mingw -S . [ -G "Ninja" ] \
      -D CMAKE_BUILD_TYPE=Debug \
      -D CMAKE_TOOLCHAIN_FILE=<toolchain-file> \
      -D FLTK_FLUID_HOST=/path/to/fluid [.. more parameters ..]

Note: replace '-G "Ninja" ' with the build tool of your choice or omit this
argument to use the default generator of your platform.

Theoretically the variable FLTK_FLUID_HOST can also be set in a toolchain
file. This has been tested successfully but is not recommended because the
toolchain file should be independent of the project using it.

After generating the build system (above), build and optionally install the
library:

    cmake --build mingw
    [sudo] cmake --install mingw  # optional

This will create a default configuration FLTK suitable for mingw/msys and
install it in the /usr/x86_64-w64-mingw32/usr tree.

Note: replace 'x86_64-w64-mingw32' with your cross toolchain location as
required.


 3.  Using CMake with FLTK
===========================

The CMake Export/Import facility can be thought of as an automated
fltk-config. For example, if you link your program to the FLTK
library, it will automatically link in all of its dependencies. This
includes any special flags, i.e. on Linux it includes the -lpthread flag.

This howto assumes that you have FLTK libraries which were built using CMake,
installed. Building them with CMake generates some CMake helper files which
are installed in standard locations, making FLTK easy to find and use.

If FLTK is not installed in a standard system location where it is found
automatically, you may need to set a CMake variable to point CMake to the
right location.

In the following examples we set the CMake cache variable 'FLTK_DIR' so
CMake knows where to find the FLTK configuration file 'FLTKConfig.cmake'.
It is important (recommended practice) to set this as a CMake cache variable
which enables the user executing 'cmake' to override this path either on the
commandline or interactively using the CMake GUI 'cmake-gui' or 'ccmake' on
Unix/Linux, for instance like this:

  $ cd my-project
  $ mkdir build
  $ cd build
  $ cmake -G "Unix Makefiles" -S.. -D "FLTK_DIR=/home/me/fltk"


 3.1  Library Names
--------------------

When you use the target_link_libraries() command, CMake uses its own internal
"target names" for libraries. The original fltk library names in the build
tree are:

    fltk    fltk_forms    fltk_images    fltk_gl

The bundled image and zlib libraries (if built):

    fltk_jpeg    fltk_png    fltk_z

Append suffix "-shared" for shared libraries (Windows: DLL's).

These library names are used to construct the filename on disk with system
specific prefixes and postfixes. For instance, on Linux/Unix 'fltk' is libfltk.a
and the shared library (fltk-shared) is libfltk.so.1.4.0 (in FLTK 1.4.0) with
additional system specific links.

Note: since FLTK 1.4.0 the library fltk_cairo is no longer necessary and
should be removed from CMake files of user projects. fltk_cairo is now an
empty library solely for backwards compatibility and will be removed in the
future.


 3.2  Library Aliases
----------------------

Since FLTK 1.4.0 "aliases" for all libraries in the FLTK build tree are
created in the namespace "fltk::". These aliases should always be used by
consumer projects (projects that use FLTK) for several reasons which are
beyond the scope of this README file. The following table shows the FLTK
libraries and their aliases in the FLTK build tree.

  Library Name     Alias          Shared Library Alias    Notes
  --------------------------------------------------------------
    fltk           fltk::fltk     fltk::fltk-shared       [1]
    fltk_forms     fltk::forms    fltk::forms-shared      [2]
    fltk_gl        fltk::gl       fltk::gl-shared         [2]
    fltk_images    fltk::images   fltk::images-shared     [2]
    fltk_jpeg      fltk::jpeg     fltk::jpeg-shared       [3]
    fltk_png       fltk::png      fltk::png-shared        [3]
    fltk_z         fltk::z        fltk::z-shared          [3]

  [1] The basic FLTK library. Use this if you don't need any of the other
      libraries for your application.
  [2] Use one or more of these libraries if you have specific needs,
      e.g. if you need to read images (fltk::images), OpenGL (fltk::gl),
      or (X)Forms compatibility (fltk::forms). If you use one of these
      libraries in your CMakeLists.txt then fltk::fltk will be included
      automatically.
  [3] The bundled libraries are only built if requested and are usually
      not needed in user projects. They are linked in with fltk::images
      automatically if they were built together with FLTK.
      The only reason you may need them would be if you used libpng,
      libjpeg, or zlib functions directly in your application and need
      to use the bundled FLTK libs (e.g. on Windows).


 3.3  Exported and Imported Targets
------------------------------------

CMake terminology is to "export" and "import" library "targets". FLTK's
CMake files export targets and its CONFIG module FLTKConfig.cmake imports
targets so user projects can use them. Hence, if you use CMake's CONFIG
mode to find FLTK all library targets will be defined using the namespace
convention listed above in the "Alias" column. This is what user projects
are recommended to use.

In addition to the library targets FLTK defines the "imported target"
'fltk::fluid' which can be used to generate source (.cxx) and header (.h)
files from fluid (.fl) files.

Another target fltk::fltk-config can be used to set (e.g.) system or user
specific FLTK options. This would usually be executed in the installation
process of a user project but should rarely be needed and is beyound the
scope of this documentation.


 3.4  Building a Simple "Hello World" Program with FLTK
--------------------------------------------------------

Here is a basic CMakeLists.txt file using FLTK. It is important that
this file can only be used as simple as it is if you use find_package()
in `CONFIG` mode as shown below. This requires that the FLTK library
itself has been built with CMake.

---
cmake_minimum_required(VERSION 3.15)

project(hello)

# optional (see below):
set(FLTK_DIR "/path/to/fltk"
    CACHE FILEPATH "FLTK installation or build directory")

find_package(FLTK 1.4 CONFIG REQUIRED)

add_executable       (hello WIN32 MACOSX_BUNDLE hello.cxx)
target_link_libraries(hello PRIVATE fltk::fltk)
---

We recommend to use `cmake_minimum_required(VERSION 3.15)` or higher for
building projects that use FLTK. Lower CMake versions may work for user
projects but this is not tested by FLTK developers.

The optional `set(FLTK_DIR ...)` command is a superhint to the find_package
command. This is useful if you don't install FLTK or have a non-standard
install location. The path you give to it must be that of a directory that
contains the file FLTKConfig.cmake.

You can omit this statement if CMake finds the required FLTK version
without it. This variable is stored in the CMake Cache so users can change
it with the ususal CMake GUI interfaces (ccmake, cmake-gui) or on the
CMake commandline (-D FLTK_DIR=...).

The find_package command tells CMake to find the package FLTK, '1.4' says
that we want FLTK 1.4.x: any patch version of 1.4 will march. 'REQUIRED'
means that it is an error if it's not found. 'CONFIG' tells it to search
only for the FLTKConfig.cmake file, not using the FindFLTK.cmake "module"
supplied with CMake, which doesn't work with this version of FLTK. Since
we specify a version (1.4) the file 'FLTKConfigVersion.cmake' must also
be found. This file is created since FLTK 1.3.10.

"WIN32 MACOSX_BUNDLE" in the add_executable() command tells CMake that
this is a GUI app. It is ignored on other platforms than Windows or macOS,
respectively, and should always be present with FLTK GUI programs for
better portability - unless you explicitly need to build a "console program"
on Windows.

Once the package is found (in CONFIG mode, as described above) all built
FLTK libraries are "imported" as CMake "targets" or aliases and can be used
directly. These CMake library targets contain all necessary informations to
be used without having to know about additional include directories or
other library dependencies. This is what is called "Modern CMake".

Older FLTK versions required to use the variables FLTK_INCLUDE_DIRS and
FLTK_LIBRARIES (among others). These variables and related commands are
no longer necessary if your project (CMakeLists.txt) uses CMake's
CONFIG mode as described in this file.

The target_link_libraries() command is used to specify all necessary FLTK
libraries. Thus you may use fltk::fltk, fltk::images, fltk::gl, fltk::forms,
or any combination. fltk::fltk is linked automatically if any of the other
libs is included.


 3.5  Building a Program Using Fluid Files
-------------------------------------------

CMake has a command named fltk_wrap_ui which helps deal with fluid *.fl
files. Unfortunately it is broken in CMake 3.4.x but it seems to work in
3.5 and later CMake versions. We recommend to use add_custom_command()
to achieve the same result in a more explicit and well-defined way.
This is a more basic approach and should work for all CMake versions.
It is described below.

Here is a sample CMakeLists.txt which compiles the CubeView example from
a directory you've copied the test/Cube* files to.

---
cmake_minimum_required(VERSION 3.15)

project(CubeView)

# change this to your fltk build directory
set(FLTK_DIR "/path/to/fltk"
    CACHE FILEPATH "FLTK installation or build directory")

find_package(FLTK 1.4 CONFIG REQUIRED)

# run fluid -c to generate CubeViewUI.cxx and CubeViewUI.h files
add_custom_command(
    OUTPUT "CubeViewUI.cxx" "CubeViewUI.h"
    COMMAND fltk::fluid -c ${CMAKE_CURRENT_SOURCE_DIR}/CubeViewUI.fl
)

add_executable(CubeView WIN32 MACOSX_BUNDLE
               CubeMain.cxx CubeView.cxx CubeViewUI.cxx)

target_include_directories(CubeView PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(CubeView PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries     (CubeView PRIVATE fltk::gl)
---

You can repeat the add_custom_command for each fluid file or if you
have a large number of them see the fltk_run_fluid() function in
CMake/FLTK-Functions.cmake for an example of how to run it in a loop.

The two lines

  target_include_directories(CubeView PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_include_directories(CubeView PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add the current build ("binary") and source directories as include directories.
This is necessary for the compiler to find the local header files since the
fluid-generated files (CubeViewUI.cxx and CubeViewUI.h) are created in the
current build directory and other header files may be in the source directory
(depending on your project).


 3.6  Building a Program Using CMake's FetchContent Module
-----------------------------------------------------------

FLTK can be downloaded and built within a user project using CMake's
FetchContent module. A sample CMakeLists.txt file follows.

You may need to adjust it to your configuration.

---
cmake_minimum_required(VERSION 3.15)
project(hello)

include(FetchContent)

FetchContent_Declare(FLTK
  GIT_REPOSITORY  https://github.com/fltk/fltk
  GIT_TAG         master
  GIT_SHALLOW     TRUE
)

message(STATUS "Download and build FLTK if necessary, please wait...")
FetchContent_MakeAvailable(FLTK)
message(STATUS "Download and build FLTK - done.")

add_executable       (hello WIN32 MACOSX_BUNDLE hello.cxx)
target_link_libraries(hello PRIVATE fltk::fltk)
---

This is as simple as it can be. The CMake FetchContent module is used to
download the FLTK sources from their Git repository and to build them.
Note that this will download and build the FLTK library during the CMake
configure phase which can take some time. Therefore the statement
`FetchContent_MakeAvailable()` is wrapped in `message(STATUS "...")`
commands to let the user know what's going on.


 4  FindFLTK.cmake and find_package(FLTK)
==========================================

The FindFLTK.cmake module provided by CMake which is also used in the
CMake command find_package(FLTK) does not yet support FLTK's new "Modern
CMake" features.

Unfortunately this module has to be used if the FLTK library wasn't built
with CMake and thus CONFIG mode can't be used. In this case CMake falls back
to MODULE mode and find_package() uses this old CMake module.

There are plans to provide a FindFLTK.cmake module with a later FLTK release.
Look here for further info if you need it...
