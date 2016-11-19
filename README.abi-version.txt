Configuration of the ABI version for the Fast Light Toolkit (FLTK)
------------------------------------------------------------------

FLTK preserves the application binary interface (ABI) throughout
patch versions, for instance all 1.3.x versions (x = patch version).

This basically means that a program compiled and linked with FLTK 1.3.0
can run with a FLTK shared library (fltk.dll, fltk.so.1.3.x) of a later
FLTK version 1.3.x, but not with a shared library of FLTK 1.4.0 or later.

Since FLTK 1.3.1 the FLTK team began to introduce ABI-breaking features
wrapped in so-called ABI guards in the library code, e.g.

    #if FL_ABI_VERSION >= 10401
	... new, ABI breaking code ...
    #else
	... old, ABI preserving code ...
    #endif

  Note:	In FLTK 1.3.x this preprocessor macro was named FLTK_ABI_VERSION.
	In FLTK 1.4.0 FLTK_ABI_VERSION was renamed to FL_ABI_VERSION.

This documentation is written for FLTK 1.4.x, but it applies to all later
versions as well. Replace the version numbers given here with the version
numbers of the version you are using. FLTK version 1.4.1 was chosen as an
example only. As of this writing, FLTK 1.4.1 does not yet exist.


How to define the FLTK ABI version
----------------------------------

To define the ABI version the preprocessor macro FL_ABI_VERSION must be
defined as a number representing the ABI version in the form

    #define FL_ABI_VERSION 1xxyy

where xx and yy are the minor and patch versions, resp. with leading zeroes,
and '1' is the major version number.

The default ABI version for all FLTK 1.4.x versions is 10400 (the binary
version of FLTK 1.4.0), but you can configure another version, e.g.
10401 for FLTK 1.4.1 to enable the ABI features of FLTK 1.4.1 and all
previous versions. See CHANGES file.


Depending on how you build FLTK, there are two different ways to configure
the ABI version. The default is always the lowest version (e.g. 10400). All
following examples are written for FLTK 1.4.1, hence we use "10401" for
the version number.


(1) Traditional configure + make (Unix, Linux, MinGW etc.)
----------------------------------------------------------

    Run

	./configure --with-abiversion=10401
	make

    This will generate FL/abi-version.h and build FLTK as usual.


(2) CMake + make
----------------

    FLTK versions 1.4.0 and later contain full CMake support.

    Use CMake to build the Makefile's and run 'make'. To configure the
    ABI version, use ccmake, cmake-gui, or run make with the following
    command:

	cmake -D OPTION_ABI_VERSION:STRING=10401 /path/to/fltk
	make

    You can define OPTION_ABI_VERSION to the required version number using
    one of the graphical CMake tools.

    For more information on how to use CMake with FLTK see README.CMake.txt.


(3) CMake + IDE Projects: Visual C++, Xcode, other IDE's
--------------------------------------------------------

    FLTK versions 1.4.0 and later contain full CMake support.

    IDE project files are no longer included in the FLTK source distribution.
    You need to install CMake to generate the IDE files.

    Use CMake to generate the IDE project files of your choice. Currently
    the FLTK team uses some Visual C++ (Visual Studio) versions and Xcode.
    Other IDE's may work as well (not yet tested).

    For more informations on how to install and use CMake see ...
    ... *FIXME* [Add documentation how to use CMake with FLTK].


    Use CMake option OPTION_ABI_VERSION:STRING=10401 with the command line
    or any of the CMake GUI programs.

    Then start the build process in the IDE solution of your choice. This
    will run the build with the defined ABI version.


General note on CMake:
----------------------

    CMake generates FL/abi-version.h in the build tree. You may run
    'make install' to install the FLTK library including all headers in
    the chosen installation directory, although this is not necessary.

    The FLTK team recommends to use the FLTK library directly from the
    build folder. See README.CMake.txt for more information.

    Possible exception: Visual Studio IDE builds (Windows).

    ... to be continued ...
