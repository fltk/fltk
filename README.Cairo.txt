README.Cairo.txt - Cairo rendering support for FLTK
----------------------------------------------------


 CONTENTS
==========

 1   INTRODUCTION

 2   CAIRO SUPPORT FOR FLTK
   2.1    Configuration
   2.2    Currently supported features
   2.3    Future considerations

 3   PLATFORM SPECIFIC NOTES
   3.1    Linux
   3.1.1  Debian and Derivatives (like Ubuntu)
   3.1.2  CentOS from Greg (erco@seriss.com)
   3.2    Windows
   3.3    macOS
   3.3.1  Install Xcode Commandline Tools
   3.3.2  Install Homebrew for Cairo and other Library Support
   3.3.3  Install CMake and Build with CMake

 4   DOCUMENT HISTORY


 INTRODUCTION
==============

Cairo is a software library used to provide a vector graphics-based,
device-independent API for software developers. It is designed to provide
primitives for 2-dimensional drawing across a number of different
backends. Cairo is designed to use hardware acceleration when available.


 CAIRO SUPPORT FOR FLTK
========================

It is now possible to integrate Cairo rendering in your FLTK application
more easily and transparently.
Since FLTK 1.3 we provide minimum support for Cairo; no "total" Cairo
rendering layer support is achieved.


 Configuration
---------------

All the changes are *inactive* as long as the new configuration
option --enable-cairo is not added to the configure command or the CMake
variable OPTION_CAIRO:BOOL=ON is set.


 Currently supported features
------------------------------

(1) Adding a new Fl_Cairo_Window class permitting transparent and easy
    integration of a Cairo draw callback without the need to subclass Fl_Window.

(2) Adding a Fl::cairo_make_current(Fl_Window*) function only providing
    transparently a Cairo context to your custom Fl_Window derived class.
    This function is intended to be used in your overloaded draw() method.

(3) FLTK instrumentation for cairo extended use :
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
    CMake variable OPTION_CAIROEXT:BOOL=ON (Default: OFF).

(4) A new Cairo demo that is available in the test subdirectory and has
    been used as a testcase during the multiplatform tests.

For more details, please have a look to the doxygen documentation,
in the Modules section.


 PLATFORM SPECIFIC NOTES
=========================

The following are notes about building FLTK with Cairo support
on the various supported operating systems.

    3.1 Linux
    ---------

    3.1.1 Debian and Derivatives (like Ubuntu)
    -------------------------------------------

      Run from a terminal command line:
        sudo apt install libcairo2-dev

    Then build fltk using the Cairo support option using:
      cmake -G"Unix Makefiles" -DOPTION_CAIRO:BOOL=ON -S <fltk_dir> -B <your_build_dir>
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
    as well but there are plans to implement a fallback mechanism so you can
    build FLTK w/o having to install and use pkg-config. This will be done if
    possible (but not guaranteed).


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
    to select OPTION_CAIRO (ON) to build with basic Cairo support. Finally
    click "generate" to create the build files.

    For more information on using CMake to build FLTK see README.CMake.txt.


 DOCUMENT HISTORY
==================

Dec 20 2010 - matt: restructured document
Dec 09 2011 - greg: Updates for Centos 5.5 builds
Dec 10 2011 - Albrecht: Updates for Ubuntu and Debian, fixed typos.
Jul 05 2017 - Albrecht: Added CMake config info, fixed typos.
Feb 28 2021 - Albrecht: Update for FLTK 1.4, add macOS instructions.
