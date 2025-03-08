README - Fast Light Tool Kit (FLTK) Version 1.5.0
-------------------------------------------------

WHAT IS FLTK?

    The Fast Light Tool Kit is a cross-platform C++ GUI toolkit for
    UNIX®/Linux® (X11 or Wayland), Microsoft® Windows®, and macOS®.
    FLTK provides modern GUI functionality without bloat and supports
    3D graphics via OpenGL® and its built-in GLUT emulation.
    It was originally developed by Mr. Bill Spitzak and is currently
    maintained by a small group of developers across the world with
    a central repository on GitHub.

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

    Since FLTK 1.5.0 the only available build system is CMake. CMake is a
    "build system generator" and can be used to create Makefile's, Ninja
    build files, Xcode (macOS), Visual Studio (Windows) IDE files, and
    many more. Use `cmake --help` to display the available generators
    on your platform.

    Please see also README.CMake.txt for further details. There are *many*
    options to configure the build as you need.

    In a nutshell:

      $ cd /path/to/fltk
      $ cmake . -B build
      $ cmake --build build

    These commands create the build folder 'build' inside your source tree,
    build the library and all test programs. You may want to test the demo
    programs by running `build/bin/test/demo`.

    After successful tests you may install the library with the following
    command, but please be aware that this will install FLTK in a system
    directory for system-wide use. We don't recommend this unless you know
    what you're doing.

      $ sudo cmake --install build  # default: to /usr/local/...


BUILDING FLTK UNDER MICROSOFT WINDOWS

    There are two ways to build FLTK under Microsoft Windows.

    The first is to use CMake to create the Visual C++ project or NMake files
    in your favorite development directory, then build FLTK with Visual Studio.

    The second method is to use a GNU-based development tool. To build with
    the Cygwin, MinGW, or MSYS2 tools, use CMake to create the build files
    as described above.

    In most cases it's useful to install a binary CMake distribution from
    https://cmake.org/download/ .

    Then execute `cmake-gui` and generate Visual Studio project files or any
    other build files of your choice, e.g. "Unix Makefiles" for MinGW, but
    note that there are other options as well.

    See README.Windows.txt and README.CMake.txt for more info.


BUILDING HTML DOCUMENTATION

    FLTK uses Doxygen for documentation, so you'll at least need doxygen
    installed for creating html docs, and LaTeX for creating PDF docs.

    If you want to build the documentation, change directory to your build
    folder, for instance

        cd /path/to/fltk/build

    To build the HTML or PDF documentation, use these CMake commands:

        cmake --build . --target html
        cmake --build . --target pdf

    Note: instead of using the generic CMake commands above you can also
    use equivalent commands of your build system, e.g. `make html` or
    `ninja pdf`.


INTERNET RESOURCES

    FLTK is available on the internet in a bunch of locations:

    - https://www.fltk.org/                   - homepage
    - https://github.com/fltk/fltk            - source code and discussions
    - https://www.fltk.org/bugs.php           - info for reporting bugs
    - https://www.fltk.org/software.php       - download source code
    - https://github.com/fltk/fltk/releases   - source code and documentation

    Note that we don't provide pre-compiled (binary) distributions. Consult
    the package manager of your (Linux, Unix, macOS) operating system.


GENERAL QUESTIONS

    To join the FLTK mailing list, go to the following web page:

        https://groups.google.com/forum/#!forum/fltkgeneral

    You can find detailed instructions on how you can register for the
    mailing list (even w/o a Google account) at the bottom of this page:

        https://www.fltk.org/newsgroups.php

    Since July 2024 we offer GitHub Discussions on our GitHub project page.
    Use the 'Q&A' section for general questions on building and using FLTK.

        https://github.com/fltk/fltk/discussions/categories/q-a


REPORTING BUGS

    If you are new to FLTK, or have general questions about how to use FLTK,
    or aren't sure if you found a bug, please ask first on the fltk.general
    group forum at:

        https://groups.google.com/forum/#!forum/fltkgeneral

    or on GitHub Discussions (Q&A) as noted above:

        https://github.com/fltk/fltk/discussions/categories/q-a


    See also paragraph GENERAL QUESTIONS above for more info.

    If you are sure you found a bug, please see the following page for
    further information on how to report a bug:

        https://www.fltk.org/bugs.php


TRADEMARKS

    Microsoft and Windows are registered trademarks of Microsoft Corporation.
    UNIX is a registered trademark of the X/Open Group, Inc.
    OpenGL is a registered trademark of Silicon Graphics, Inc.
    macOS is a registered trademark of Apple Computers, Inc.


COPYRIGHT

    FLTK is copyright 1998-2025 by Bill Spitzak and others,
    see the CREDITS.txt file for more info.

    This library is free software. Distribution and use rights are
    outlined in the file "COPYING" which should have been included with
    this file.  If this file is missing or damaged, see the license at:

        https://www.fltk.org/COPYING.php
