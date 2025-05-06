# README - Fast Light Tool Kit (FLTK) Version 1.5.0

## What is FLTK?

The Fast Light Tool Kit (FLTK) is a cross-platform C++ GUI toolkit
for UNIX®/Linux® (X11 or Wayland), Microsoft® Windows®, and macOS®.
FLTK provides modern GUI functionality without bloat and supports
3D graphics via OpenGL® and its built-in GLUT emulation.
It was originally developed by Mr. Bill Spitzak and is currently
maintained by a small group of developers across the world with
a central repository on GitHub.

  https://www.fltk.org/
  https://github.com/fltk/fltk/

## Licensing

FLTK comes with complete free source code.  FLTK is available
under the terms of the GNU Library General Public License with
exceptions (e.g. for static linking).
Contrary to popular belief, it can be used in commercial
software! (Even Bill Gates could use it.)

## Online Documentation

The documentation in HTML and PDF forms can be created by
Doxygen from the source files. HTML and PDF versions of this
documentation are also available from the FLTK web site at:

  https://www.fltk.org/documentation.php


## Prerequisites for Building FLTK

To build FLTK 1.5 and higher you need:

  - CMake
  - a C++11 capable compiler, e.g. gcc, clang, Visual Studio, Xcode
  - system specific build files (headers, SDK, etc.)

CMake is used to generate the build environment on your system.
It can create build environments for a lot of different build tools,
please see the CMake documentation for more info.

Since FLTK 1.5 we use C++11 features and you need at least a C++11 capable
compiler running in C++11 mode. The minimal C++ standard used for building
FLTK and your application *may* be raised in future versions.

The required header files etc. (build environment or SDK) vary across
platforms and are described in platform specific README.* files, e.g. on
Unix/Linux systems these are typically provided by the package manager.

## Building and Installing FLTK With CMake (Commandline)

On systems where a commandline `CMake` utility is available (this is
the case even on Windows), the commands to build FLTK using CMake can
be as easy as:

    cd /path/to/fltk
    cmake . -B build [ options ]
    cmake --build build

These commands create the build folder 'build' inside your source tree,
build the library and all test programs.

After successful tests you may install the library with the following
command or a similar one, but please be aware that this will install
FLTK in a system directory for system-wide use if you don't change the
default installation path. We don't recommend this unless you know
what you're doing.

    sudo cmake --install build  # from the source tree (with CMake)

or

    sudo make install           # from the build folder (with make)

Other commands (e.g. `make` or `ninja`) may be used as well after the
initial cmake command, depending on the CMake generator you used.

For more info on using CMake to build FLTK please see README.CMake.txt.

## Building HTML Documentation

FLTK uses Doxygen for documentation, so you'll at least need doxygen
installed for creating html docs, and additionally LaTeX for creating
PDF documentation.

If you want to build the documentation, change directory to your build
folder, for instance

    cd /path/to/fltk/build

To build the HTML or PDF documentation, use these CMake commands:

    cmake --build . --target html
    cmake --build . --target pdf

Note: instead of using the generic CMake commands above you can also
use equivalent commands of your build system, e.g. `make html` or
`ninja pdf`, respectively.

Note: more about building Fluid documentation to be added.

## Internet Resources

FLTK is available on the internet in a bunch of locations:

  - https://www.fltk.org/                   - homepage
  - https://github.com/fltk/fltk            - source code and discussions
  - https://www.fltk.org/bugs.php           - info for reporting bugs
  - https://www.fltk.org/software.php       - download source code
  - https://github.com/fltk/fltk/releases   - source code and documentation

Note that we don't provide pre-compiled (binary) distributions. Consult
the package manager of your (Linux, Unix, macOS) operating system.

## General Questions

To join the FLTK mailing list, go to the following web page:

  https://groups.google.com/forum/#!forum/fltkgeneral

You can find detailed instructions on how you can register for the
mailing list (even w/o a Google account) at the bottom of this page:

  https://www.fltk.org/newsgroups.php

Since July 2024 we offer GitHub Discussions on our GitHub project page.
Use the 'Q&A' section for general questions on building and using FLTK.

  https://github.com/fltk/fltk/discussions/categories/q-a

## Reporting Bugs

If you are new to FLTK, or have general questions about how to use FLTK,
or aren't sure if you found a bug, please ask first on the fltk.general
group forum at:

  https://groups.google.com/forum/#!forum/fltkgeneral

or on GitHub Discussions (Q&A) as noted above:

  https://github.com/fltk/fltk/discussions/categories/q-a

See also paragraph "General Questions" above for more info.

If you are sure you found a bug, please see the following page for
further information on how to report a bug:

  https://www.fltk.org/bugs.php

## Trademarks

- Microsoft and Windows are registered trademarks of Microsoft Corporation
- UNIX is a registered trademark of the X/Open Group, Inc.
- OpenGL is a registered trademark of Silicon Graphics, Inc.
- macOS is a registered trademark of Apple Computers, Inc.

## Copyright

FLTK is copyright 1998-2025 by Bill Spitzak and others,
see the CREDITS.txt file for more info.

This library is free software. Distribution and use rights are
outlined in the file "COPYING" which should have been included with
this file.  If this file is missing or damaged, see the license at:

  https://www.fltk.org/COPYING.php

## Further Information

For more information see README.txt:
https://github.com/fltk/fltk/blob/master/README.txt

![Build](https://github.com/fltk/fltk/actions/workflows/build.yml/badge.svg)

[![Build FLUID User Manual.](https://github.com/fltk/fltk/actions/workflows/build_fluid_docs.yml/badge.svg)](https://github.com/fltk/fltk/actions/workflows/build_fluid_docs.yml)
