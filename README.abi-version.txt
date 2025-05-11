Configuration of the ABI Version for the Fast Light Toolkit (FLTK)
------------------------------------------------------------------

FLTK preserves the application binary interface (ABI) throughout
patch versions, for instance all 1.4.x versions (x = patch version).

This basically means that a program compiled and linked with FLTK 1.4.0
can run with a FLTK shared library (fltk.dll, fltk.so.1.4.x) of a later
FLTK version 1.4.x, but not with a shared library of FLTK 1.5.0 or later.

Since FLTK 1.3.1 the FLTK team began to introduce ABI-breaking features
wrapped in so-called ABI guards in the library code, using a preprocessor
macro. Since FLTK 1.4.0 the macro name is FL_ABI_VERSION:

    #if FL_ABI_VERSION >= 10401
        ... new, ABI breaking code ...
    #else
        ... old, ABI preserving code ...
    #endif

This documentation was written for FLTK 1.4.x but it applies to all later
versions as well. Replace the version numbers given here with the version
numbers of the version you are using. FLTK version 1.4.1 was chosen as an
example only.


How to Define the FLTK ABI Version
----------------------------------

To define the ABI version the preprocessor macro FL_ABI_VERSION must be
defined as a number representing the ABI version in the form

    #define FL_ABI_VERSION 1xxyy

where xx and yy are the minor and patch versions, resp. with leading zeroes,
and '1' is the major version number. CMake generates the file FL/fl_config.h
in the build folder given the version you select (see below).

For instance, the default ABI version for all FLTK 1.4.x versions is 10400
(the binary version of FLTK 1.4.0) but you can select another version,
e.g. 10401 for FLTK 1.4.1 to enable the ABI features of FLTK 1.4.1 and all
previous versions. The same applies to all higher FLTK versions.

The default ABI version is always the lowest version (e.g. 10400). All
following examples are written for FLTK 1.4.1, hence we use "10401" for
the version number.

Note: Since FLTK 1.4.3 (Git branch-1.4 after release 1.4.2) the highest
selectable ABI version is FL_API_VERSION + 1 so you can use ABI features
designated for the *next* FLTK release when using FLTK from Git with new
ABI features included for the next release.


How to Select the ABI Version with CMake
----------------------------------------

Use CMake to build the Makefile's and run 'make' or use any other CMake
generator of your choice. To select the ABI version use one of the CMake
configuration tools (cmake-gui or ccmake), or run CMake with these or
similar commands:

    cd /path/to/fltk
    cmake . -B build [-G <GENERATOR>] -D FLTK_ABI_VERSION:STRING=10401

The optional part '[-G <GENERATOR>]' can be used to select a particular
build tool that is not the default for the build platform, for instance
'-G Ninja'. Further CMake options can be appended.

Then execute

    cmake --build build

or the selected build tool (-G <GENERATOR>), e.g. `make`.

For more information on how to use CMake with FLTK see README.CMake.txt.


General Note on CMake
---------------------

CMake generates FL/fl_config.h in the build tree. You may run
'make install' to install the FLTK library including all headers in
the chosen installation directory (set CMAKE_INSTALL_PREFIX to do this),
although this is not necessary.

The FLTK team recommends to use the FLTK library directly from the
build folder. See README.CMake.txt for more information.


Checking FLTK Branches for ABI Breaking Issues
----------------------------------------------

Please see 'misc/abi-compliance-checker.txt' for further information on
how to check a branch, for instance 1.4.x, for ABI breaking changes.

Since FLTK 1.5 there is a convenient script for doing this with a single
command in 'misc/abi-check'. This can be done easily during development
to find and fix potential ABI breaking changes early.
