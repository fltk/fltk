Configuration of the ABI version for the Fast Light Toolkit (FLTK)
------------------------------------------------------------------

FLTK preserves the application binary interface (ABI) throughout
patch versions, for instance all 1.3.x versions (x = patch version).

This basically means that a program compiled and linked with FLTK 1.3.0
can run with a FLTK shared library (fltk.dll, fltk.so.1.3.x) of a later
FLTK version.

Since FLTK 1.3.1 the FLTK team began to introduce ABI-breaking features
wrapped in so-called ABI guards in the library code, e.g.

		#if FLTK_ABI_VERSION >= 10304
		 ... new, ABI breaking code ...
		#else
		 ... old, ABI preserving code ...
		#endif

In versions older than 1.3.4 the user had to edit FL/Enumerations.H, but
since FLTK 1.3.4 the ABI version can be configured when building the
FLTK library.

Note: This documentation is written for FLTK 1.3.x. It does not apply to
later FLTK versions. Replace the version numbers given here by the
version numbers of the version you are using.


How to define the FLTK ABI version
----------------------------------

To define the ABI version the preprocessor macro FL_ABI_VERSION must be
defined as a number representing the ABI version in the form

	#define FL_ABI_VERSION 1xxyy

where xx and yy are the minor and patch versions, resp. with leading zeroes,
and '1' is the major version number.

The default ABI version for all FLTK 1.3.x versions is 10300 (the binary
version of FLTK 1.3.0), but you can configure another version, e.g.
10304 for FLTK 1.3.4 to enable the ABI features of FLTK 1.3.4 and all
previous versions. See CHANGES file.


Note: the now deprecated macro FLTK_ABI_VERSION has been used before
FLTK 1.3.4. The FLTK 1.3.4 code still uses FLTK_ABI_VERSION, but since
FLTK 1.3.4 this is identical to FL_ABI_VERSION.
FLTK_ABI_VERSION will be removed entirely in FLTK 1.4.0.


Depending on how you build FLTK, there are three different ways to configure
the ABI version. The default is always the lowest version (e.g. 10300). All
following examples are written for FLTK 1.3.4, hence we use "10304" for
the version number.


(1) Traditional configure + make (Unix, Linux, MinGW etc.)
----------------------------------------------------------

    Run
	make clean
	./configure --with-abiversion=10304
	make

    This will generate FL/abi-version.h and build FLTK as usual.

    Note: you should always make sure that you compile everything from
    scratch if you change the ABI version or any other configuration
    options, e.g. with `make clean'.

(2) CMake + make
----------------

	FLTK 1.3.4 contains experimental CMake support. This is known to be
	incomplete, but you can use it for test builds. It "almost works"
	in many cases, but it is not yet officially supported. Use on your
	own risk.

	Use CMake to build the Makefile's and run 'make'. To configure the
	ABI version, use ccmake, cmake-gui, or run make with the following
	command:

		cmake -D OPTION_ABI_VERSION:STRING=10304 /path/to/fltk
		make

	You can define OPTION_ABI_VERSION to the required version number using
	one of the graphical CMake tools.

	For more information on how to use CMake with FLTK see README.CMake.txt.


(3) Bundled IDE Projects: Visual C++ and Xcode4
-----------------------------------------------

	To use the bundled IDE projects with a different ABI version you need
	to edit the supplied file abi-version.ide in the FLTK root directory.

	Instructions are given in the file, just make sure that

		#define FL_ABI_VERSION 10304

	is used outside the comment - just replace "#undef FL_ABI_VERSION" with
	the appropriate definition.

	Then start the build process in the IDE solution of your choice. This
	will copy the file abi-version.ide to FL/abi-version.h and run the
	build with the defined ABI version.

	Note: you should always make sure that you compile everything from
	scratch if you change the ABI version or any other configuration
	options, e.g. with "Clean all" or "Rebuild all" menu functions.

Note on CMake:

	CMake generates FL/abi-version.h in the build tree. You may run
	'make install' to install the FLTK library including all headers in
	the chosen installation directory, although this is not necessary.

	The FLTK team recommends to use the FLTK library directly from the
	build folder. See README.CMake.txt for more information.
