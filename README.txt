README - Fast Light Tool Kit (FLTK) Version 1.4.1
-------------------------------------------------

WHAT IS FLTK?

    The Fast Light Tool Kit is a cross-platform C++ GUI toolkit for
    UNIX®/Linux® (X11 or Wayland), Microsoft® Windows®, and macOS®.
    FLTK provides modern GUI functionality without bloat and
    supports 3D graphics via OpenGL® and its built-in GLUT
    emulation. It was originally developed by Mr. Bill Spitzak
    and is currently maintained by a small group of developers
    across the world with a central repository on GitHub.

        https://www.fltk.org/
        https://github.com/fltk/fltk/


LICENSING

    FLTK comes with complete free source code.  FLTK is available
    under the terms of the GNU Library General Public License with
    exceptions (e.g. for static linking).
    Contrary to popular belief, it can be used in commercial
    software! (Even Bill Gates could use it.)


ON-LINE DOCUMENTATION

    The documentation in HTML and PDF forms can be created by
    Doxygen from the source files. HTML and PDF versions of this
    documentation is also available from the FLTK web site at:

        https://www.fltk.org/documentation.php


BUILDING AND INSTALLING FLTK UNDER UNIX AND macOS

    Beginning with FLTK 1.4 the main and recommended build system
    is CMake. CMake is a "build system generator" and can be used
    to create Makefile's, Ninja build files, Xcode (macOS),
    Visual Studio (Windows) IDE files and many more.

    Please see README.CMake.txt for further information.

    Alternatively FLTK can be built with autoconf + make, the
    build system used in FLTK 1.3 and earlier versions. Please
    be aware that the following information may be outdated
    because it is no longer actively maintained.

    Note: autoconf + configure + make is still supported in FLTK 1.4.x
    but will be removed in FLTK 1.5.0 or any higher version.

    In most cases you can just type "make".  This will run configure
    with the default (no) options and then compile everything.

    FLTK uses GNU autoconf to configure itself for your UNIX
    platform. The main things that the configure script will
    look for are the X11, OpenGL (or Mesa), and JPEG header and
    library files.  Make sure that they are in the standard
    include/library locations.  If they aren't you need to
    define the CFLAGS, CXXFLAGS, and LDFLAGS environment
    variables.

    If you aren't using "gcc", "g++", "c++", or "CC" for your
    C++ compiler, you'll also need to set the CXX environment
    variable. Similarly, if you aren't using "gcc" or "cc" for
    your C compiler you'll need to set the CC environment variable.

    You can run configure yourself to get the exact setup you
    need. Type "./configure <options>".  Options include:

        --help                  - display help and exit
        --enable-cygwin         - Enable the Cygwin DLL (Cygwin only)
        --enable-debug          - Enable debugging code & symbols
        --disable-forms         - Disable generation of the forms library
        --disable-gl            - Disable OpenGL support
        --enable-shared         - Enable generation of shared libraries
        --enable-threads        - Enable multithreading support
        --enable-xft            - Enable the Xft library (anti-aliased fonts)
        --enable-pango          - Draw text with the pango library
        --disable-wayland       - Force building for X11 only (no Wayland support)
        --enable-x11            - Force building for X11 (macOS and Cygwin)
        --disable-x11           - Force building for Wayland only (Linux/Unix)
        --bindir=/path          - Set the location for executables
                                  [default = /usr/local/bin]
        --libdir=/path          - Set the location for libraries
                                  [default = /usr/local/lib]
        --includedir=/path      - Set the location for include files.
                                  [default = /usr/local/include]
        --prefix=/dir           - Set the directory prefix for files
                                  [default = /usr/local]

    For more options please see './configure --help'.

    When the configure script is done you can just run the
    "make" command. This will build the library, FLUID tool, and
    all of the test programs.

    To install the library, become root and type "make
    install".  This will copy the "fluid" executable to
    "bindir", the header files to "includedir", and the library
    files to "libdir".

    To install additional files and icons to be used by the main
    desktop environments such as KDE, GNOME and XFCE, you will also
    need to run "make install-desktop" as root.


GIT USERS

    If you've just checked out a fresh copy of FLTK from  Git (GitHub),
    you'll need to generate an initial version of 'configure'
    by running 'make makeinclude' or 'make clean' (we don't
    include a copy of configure in git).


MAKE TARGETS

    make            -- builds the library + test programs (does not install)
    make install    -- builds and installs
    make clean      -- clean for a rebuild
    make distclean  -- like 'clean', but also removes docs, configure, fltk-config
    ( cd src; make ) -- builds just the fltk libraries


BUILDING FLTK UNDER MICROSOFT WINDOWS

    There are two ways to build FLTK under Microsoft Windows.

    The first is to use CMake to create the Visual C++ project or NMake files
    in your favorite development directory, then build FLTK with Visual Studio.

    The second method is to use a GNU-based development tool. To build with
    the Cygwin or MinGW tools, use the supplied configure script as specified
    in the UNIX section above:

        ./configure ...options...

    See README.Windows.txt and README.CMake.txt for more info.


BUILDING HTML DOCUMENTATION

    If you want to build the HTML documentation:

        ( cd documentation && make html )

    If you want to build the PDF documentation:

        ( cd documentation && make pdf )

    FLTK uses doxygen for documentation, so you'll at least need doxygen
    installed for creating html docs, and LaTeX for creating PDF docs.


INTERNET RESOURCES

    FLTK is available on the 'net in a bunch of locations:

        - WWW:   https://www.fltk.org/
                 https://www.fltk.org/bugs.php [for reporting bugs]
                 https://www.fltk.org/software.php [source code]


GENERAL QUESTIONS

    To join the FLTK mailing list, go the following web page:

        https://groups.google.com/forum/#!forum/fltkgeneral

    You can find detailed instructions how you can register for the mailing
    list (even w/o a Google mail address) at the bottom of this page:

        https://www.fltk.org/newsgroups.php


REPORTING BUGS

    If you are new to FLTK, or have general questions about how to use FLTK,
    or aren't sure if you found a bug, please ask first on the fltk.general
    group forum at:

        https://groups.google.com/forum/#!forum/fltkgeneral

    See paragraph GENERAL QUESTIONS above for more info.

    If you are sure you found a bug, please see the following page for
    further information:

        https://www.fltk.org/bugs.php


TRADEMARKS

    Microsoft and Windows are registered trademarks of Microsoft
    Corporation. UNIX is a registered trademark of the X/Open
    Group, Inc.  OpenGL is a registered trademark of Silicon
    Graphics, Inc.  macOS is a registered trademark of Apple
    Computers, Inc.


COPYRIGHT

    FLTK is copyright 1998-2024 by Bill Spitzak and others,
    see the CREDITS.txt file for more info.

    This library is free software. Distribution and use rights are
    outlined in the file "COPYING" which should have been included with
    this file.  If this file is missing or damaged, see the license at:

        https://www.fltk.org/COPYING.php
