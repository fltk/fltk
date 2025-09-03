lib/README.txt
--------------

This README file is a placeholder for FLTK library files on your system
if FLTK is built using 'configure' and 'make'.


Building FLTK with CMake

If FLTK is built using CMake all static and shared libraries are created
in the 'lib' subdirectory of the build tree. We strongly recommend to
build with CMake outside the source tree ("out-of-tree") as described
in README.CMake.txt in the root folder of the FLTK distribution.

If FLTK is built out-of-tree as recommended this folder will not be touched.


Building FLTK with configure + make

Under UNIX/Linux and other systems that support 'configure' and 'make'
(e.g. MinGW, MSYS) a single set of library files will be built, with
or without debug information depending on the options you provided to
the configure script.

The FLTK build system based on configure does not support out-of-tree
builds, hence the static libraries will be built in this 'lib' folder
and the shared libraries (if enabled) can be found in the 'src' folder.
