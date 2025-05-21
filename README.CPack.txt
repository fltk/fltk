README.CPack.txt - Building Binary Packages with CPack
-------------------------------------------------------

Intended Audience
-----------------

"Packagers" (maintainers) of Linux distributions or users who want to
deploy FLTK binary packages on multiple systems, e.g. companies using
FLTK for their software development.


Introduction
------------

In previous FLTK versions binary packages could be created using EPM
(an external tool) or `rpmbuild` on Linux. Both tools used files no
longer provided by FLTK: 'fltk.list' (EPM) and 'fltk.spec' (RPM).

FLTK 1.5 and later supports CPack to create binary "packages". The new
approach using CPack is more flexible and supports many more package
formats. CPack is usually installed together with CMake.

CPack support is still experimental and may be improved in the future.
The documentation below may be enhanced later if required.


How To Build Binary Packages
----------------------------

On some platforms and with some "generators" CMake creates the target
'package' so you can execute e.g. `make package` after building FLTK.
This creates the default set of packages for the given platform.

On other platforms, or to use more flexible options, `cpack` may be run
from the commandline after building FLTK to generate a particular package
format. For details please refer to the CMake and CPack documentation.


Example Commands On Linux Using Ninja
-------------------------------------

1. Build the FLTK library:

$ cd /path-to-fltk
$ cmake -G Ninja -D CMAKE_BUILD_TYPE=Release -B build
$ cmake --build build

2. Create one or more binary packages:

$ cmake --build build --target package

... or ...

$ cd build
$ ninja package  # or `make package` if you're using Makefiles

... or to build (only) the RPM package:

$ cpack -G RPM

... or to build a .tar.gz package:

$ cpack -G TGZ

See `cpack --help` for possible package formats ("Generators").

Note: you need to install the package format specific tools on your system
to create some package formats, e.g. `rpmbuild` for RPM packages.


Further Reading
---------------

More information on this topic can be found in the CMake and CPack
documentation or by executing `cpack --help`.


Links: To Documentation
-----------------------

https://cmake.org/cmake/help/latest/index.html
https://cmake.org/cmake/help/latest/manual/cmake.1.html
https://cmake.org/cmake/help/latest/manual/cpack.1.html
