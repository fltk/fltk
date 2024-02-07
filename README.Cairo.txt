README.Cairo.txt - Cairo Window Support for FLTK
----------------------------------------------------


 CONTENTS
==========

 1   INTRODUCTION

 2   CAIRO SUPPORT FOR FLTK
   2.1    Supported Features (Fl_Cairo_Window)

 3   PLATFORM SPECIFIC NOTES
   3.1    Linux
   3.1.1  Debian and Derivatives (like Ubuntu)
   3.1.2  CentOS from Greg (erco@seriss.com)
   3.2    Windows
   3.3    macOS
   3.3.1  Install Xcode Commandline Tools
   3.3.2  Install Homebrew for Cairo and other Library Support
   3.3.3  Install CMake and Build with CMake


 1 INTRODUCTION
================

Cairo is a software library used to provide a vector graphics-based,
device-independent API for software developers. It is designed to provide
primitives for 2-dimensional drawing across a number of different
backends. Cairo is designed to use hardware acceleration when available.


 2 CAIRO SUPPORT FOR FLTK
==========================

Since FLTK 1.3 we provide minimum support for Cairo. User programs can
use the class Fl_Cairo_Window which sets up a Cairo context so the user
progam can call Cairo drawing calls in their own drawing callback.

    CMake option name: FLTK_OPTION_CAIRO_WINDOW
    Configure option : --enable-cairo

Since FLTK 1.3 the library can also be configured to provide a Cairo context
in all subclasses of Fl_Window. This is called "extended" Cairo support.

    CMake option name: FLTK_OPTION_CAIRO_EXT
    Configure option : --enable-cairoext

These two options provide users with an interface to use Cairo to draw
into FLTK windows. FLTK does not use Cairo for rendering its own graphics
with these two options. Both options must be enabled explicitly.


Since FLTK 1.4 the new Wayland platform uses Cairo for all drawings.
Under X11 drawing with Cairo rather than Xlib is a build option.
The old "Fl_Cairo_Window support" is still available on all platforms.

    CMake option name: FLTK_GRAPHICS_CAIRO
    Configure option : --enable-usecairo

Full Cairo drawing is provided on Unix/Linux platforms. It is always used if
Wayland (FLTK_BACKEND_WAYLAND) is enabled during the build. It is optional
(default: OFF) if Wayland is disabled (FLTK_BACKEND_WAYLAND=OFF).

Fl_Cairo_Window support is *inactive* as long as it is not explicitly enabled
with one of the first two options mentioned above, even if FLTK uses Cairo
drawing by itself (FLTK_GRAPHICS_CAIRO).


 2.1 Supported Features (Fl_Cairo_Window)
------------------------------------------

(1) Adding a new Fl_Cairo_Window class permitting transparent and easy
    integration of a Cairo draw callback without the need to subclass Fl_Window.

(2) Adding a Fl::cairo_make_current(Fl_Window*) function only providing
    transparently a Cairo context to your custom Fl_Window derived class.
    This function is intended to be used in your overloaded draw() method.

(3) FLTK instrumentation for cairo extended use:
    Adding an optional Cairo autolink context mode support which permits
    complete and automatic synchronization of OS dependent graphical context
    and Cairo contexts, thus furthering a valid Cairo context anytime,
    in any current window.

    Usage :
    - Call Fl::cairo_autolink_context(true); before windows are shown.
    - In an overridden widget draw method, do
        cairo_t *cc = Fl::cairo_cc();
    and obtain the cairo context value adequate to draw with cairo to
    the current window.

    This feature should be only necessary in the following cases:
    - Intensive and almost systematic use of Cairo in an FLTK application
    - Creation of a new Cairo based scheme for FLTK
    - Other uses of Cairo necessitating the FLTK internal instrumentation
      to automatically making possible the use of a Cairo context
      in any FLTK window.

    This feature must be enabled with 'configure --enable-cairoext' or the
    CMake option FLTK_OPTION_CAIRO_EXT:BOOL=ON (Default: OFF).

(4) A new Cairo demo that is available in the test subdirectory.

For more details, please have a look to the doxygen documentation,
in the Modules section.


 3 PLATFORM SPECIFIC NOTES
===========================

The following are notes about building FLTK with Cairo support
on the various supported operating systems.

    3.1 Linux
    ---------

    3.1.1 Debian and Derivatives (like Ubuntu)
    -------------------------------------------

      Run from a terminal command line:
        sudo apt install libcairo2-dev

    Then build fltk using the Cairo support option using:
      cmake -G "Unix Makefiles" -D FLTK_OPTION_CAIRO_WINDOW:BOOL=ON -S <fltk_dir> -B <your_build_dir>
      cd <your_build_dir>
      make

    3.1.2 CentOS from Greg (erco@seriss.com)
    -----------------------------------------

    To get FLTK 1.3.x (r9204) to build on Centos 5.5, I found that
    I only needed to install the "cairo-devel" package, ie:

        sudo yum install cairo-devel

    ..and then rebuild FLTK:

        make distclean
        ./configure --enable-cairo
        make

    If you get this error:

        [..]
        Linking cairo_test...
        /usr/bin/ld: cannot find -lpixman-1
        collect2: ld returned 1 exit status
        make[1]: *** [cairo_test] Error 1

    ..remove "-lpixman-1" from FLTK's makeinclude file, i.e. change this line:

        -CAIROLIBS      = -lcairo -lpixman-1
        +CAIROLIBS      = -lcairo

    ..then another 'make' should finish the build without errors.
    You should be able to then run the test/cairo_test program.

    According to the Cairo site, "For Debian and Debian derivatives including
    Ubuntu" you need to install libcairo2-dev, i.e.

        sudo apt-get install libcairo2-dev

    This has been tested and works with Ubuntu 11.10. Note that this also
    installs libpixman-1-dev, so that dependencies on this should be resolved
    as well.

    As of Feb 2021 (FLTK 1.4.0) dependencies like pixman-1 will be detected by
    configure or CMake automatically using pkg-config.

    Note 1: CMake builds *require* the use of pkg-config.

    Note 2: As of Feb 2021 autoconf/configure/make builds require pkg-config
    as well.


    3.2 Windows
    ------------
    TBD


    3.3 macOS
    ----------
    As under Linux you can use both build options, i.e. autoconf/make or CMake
    to build FLTK with Cairo support. One option is to install Homebrew and
    add the required libraries with `brew install ...'. It is always required
    to install the "Xcode commandline tools" but a full installation of Xcode
    is not necessary. If you choose to use Xcode you can generate the Xcode
    IDE files with CMake.

    The following instructions are intentionally terse. More detailed
    information can be found in README.macOS.md and README.CMake.txt.


    3.3.1 Install Xcode Commandline Tools
    --------------------------------------
    Launch a "Terminal" and execute `xcode-select --install', then select to
    install the Xcode commandline tools. This installs the required compilers
    and build tools, for instance `git' (commandline only) and `clang'.

    With these tools installed it is already possible to download and build
    a current FLTK tarball or snapshot. All you need to do is unpack the
    tarball and run `make' in the root directory. This will run `configure',
    generate the necessary files, and build FLTK in its standard configuration.

    Note 1: this requires an existing `configure' file which is included in
    FLTK releases and snapshots but not in the FLTK Git repository.

    Note 2: to build current FLTK downloaded from Git using configure + make
    you need to run `autoconf' to generate 'configure'. If autoconf is not
    available on your system you can download a FLTK snapshot, unpack it,
    and copy the 'configure' file from the snapshot to your Git worktree.


    3.3.2 Install Homebrew for Cairo and other Library Support
    -----------------------------------------------------------
    There are several options to install additional library support under
    macOS. macOS does not provide a package manager to install such software.
    This README is about installing and using Homebrew as a package manager
    to build FLTK with Cairo, everything else (for instance "Fink") is beyond
    the scope of this README.

    This was tested on a MacBook Air 2020 (M1) with the new Apple Silicon M1
    (ARM) processor.

    Go to https://brew.sh/ and follow the instructions under "Install Homebrew".
    Before you do this, read about the security implications - the FLTK team
    is not responsible for your system security. Read their docs on how to
    install and use Homebrew.

    If you installed Homebrew correctly you can use `brew search cairo' or any
    other software (package) name to see if it can be installed. If yes, run
    `brew install cairo' (or other name) to install it.

    Other helpful packages are 'autoconf' and other Unix (Linux) tools, for
    instance 'git-gui' (installs 'gitk' and 'git gui' commands). The Xcode
    commandline tools mentioned above install only the git commandline tool.


    3.3.3 Install CMake and Build with CMake
    -----------------------------------------
    Either use `brew install cmake' or go to https://cmake.org/download/
    and download the latest stable CMake release for macOS, for instance
    'cmake-3.19.6-macos-universal.dmg' (at the time of this writing). Open
    (mount) this .dmg file in the Finder. In the shown window you can either
    run CMake directly or drag the CMake application to the "Applications"
    folder to install it (recommended). Then you can run CMake from your
    Applications folder or using "launchpad". See README.macOS.md for
    instructions to install the downloaded CMake GUI tool for use with the
    commandline.

    Once you start the CMake GUI version you can select your source folder
    (where you downloaded FLTK) and the build folder (either a subdirectory,
    e.g. 'build' or another folder anywhere else) and click "configure".
    Follow the instructions and select either "native compilers" or Xcode or
    whatever you like to build your FLTK library. In the CMake GUI you need
    to select FLTK_OPTION_CAIRO_WINDOW (ON) to build with basic Cairo support.
    Finally click "generate" to create the build files.

    For more information on using CMake to build FLTK see README.CMake.txt.
