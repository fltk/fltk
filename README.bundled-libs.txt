README.bundled-libs.txt - Developer information for bundled libraries
---------------------------------------------------------------------

This file is mainly intended for FLTK developers and contains information
about the current versions of all bundled libraries and about how to
upgrade these bundled libraries.


Current versions of bundled libraries (as of Nov. 5, 2021):

  Library       Version            Release date         FLTK Version
  --------------------------------------------------------------------------
  jpeg          jpeg-9d            2020-01-12           1.3.6 - 1.3.8
  png           libpng-1.6.37      2019-04-14           1.3.6 - 1.3.8
  zlib          zlib-1.2.11        2017-01-15           1.3.6 - 1.3.8
  --------------------------------------------------------------------------

Previous versions of bundled libraries:

  Library       Version            Release date         FLTK Version
  ------------------------------------------------------------------
  jpeg          jpeg-9a            2014-01-19           1.3.5
  png           libpng-1.6.16      2014-12-22           1.3.5
  zlib          zlib-1.2.8         2013-04-28           1.3.5
  --------------------------------------------------------------------------


General information:

  FLTK does not include the entire library distributions. We only provide the
  source files necessary to build the FLTK library and some README and/or
  CHANGELOG files. There are no test programs or other contributed files.

  We use our own build files, hence a few files MUST NOT be upgraded when
  the library source files are upgraded. We strive to keep changes to the
  library source files as small as possible. Patching library code to
  work with FLTK should be a rare exception.

  If patches are necessary all changes in the library files should be
  marked with "FLTK" in a comment so a developer who upgrades the library
  later is aware of changes in the source code for FLTK. Additional comments
  should be added to show the rationale, i.e. why a particular change was
  necessary. If applicable, add a reference to a Software Trouble Report,
  GitHub Issue or PR like "STR 3456", "Issue #123", or "PR #234".


How to update the bundled libraries:

  It is generally advisable to use a graphical merge program. I'm using
  'meld' under Linux, but YMMV.

  Do not add any source files unless they are required to build the library.

  Some config header files may be pre-generated in the FLTK sources. These
  header files should be left untouched, but it may be necessary to update
  these files if new items were added to the new library version. In this
  case the new header should be pre-generated on a Linux system with default
  options unless otherwise mentioned below for a specific library.
  Currently there are no known exceptions.


Merging source files:

  Please check if some source and header files contain "FLTK" comments
  to be aware of necessary merges. It is also good to get the distribution
  tar ball of the previous version and to run a (graphical) diff or
  merge tool on the previous version and the bundled version of FLTK
  to see the "previous" differences.

  Files that were not patched in previous versions should be copied to
  the new version w/o changes. Files that had FLTK specific patches must
  be merged manually. FLTK patches should be verified (if still necessary)
  and should be kept in the new source files.

  Source and header files that have been added in the new library version
  should be added in FLTK as well if they are necessary to build the
  library. A simple "trial and error" should be sufficient to find files
  that need to be added. Added files must be added to FLTK's build files
  as well, usually to both `Makefile' and `CMakeLists.txt' to be used in
  configure/make and in CMake based builds, respectively.


Upgrade order:

  There is only one dependency between all bundled libraries: libpng
  depends on zlib. Hence zlib should be upgraded first, then all other
  libs can be upgraded in arbitrary order.


Tests after merge:

  Tests should be done on as many platforms as possible, both with
  autotools (configure/make) and CMake. Windows (Visual Studio) and
  macOS (Xcode) builds need CMake to generate the IDE files.


Upgrade notes for specific libraries:

  The following chapters contain information of specific files and how
  they are upgraded. Since the changes in all bundled libraries can't
  be known in advance this information may change in the future. Please
  verify that no other changes are necessary.


zlib:

  Website:    https://zlib.net/
  Download:   See website and follow links.
  Repository: git clone https://github.com/madler/zlib.git


  The following files need special handling:

    CMakeLists.txt: Keep FLTK version, update manually if necessary.

    Makefile: Same as CMakeLists.txt.

    zconf.h: Merge changes.

      As of zlib 1.2.11: two small sections marked with "FLTK" comments
      that need to be kept.

    makedepend: Keep this file.

      Run `make depend' in the zlib folder on a Linux system after
      the upgrade to update this file.


png:

  Website:    http://libpng.org/pub/png/libpng.html
  Download:   See website and follow links.
  Repository: git clone https://git.code.sf.net/p/libpng/code libpng

  libpng should be upgraded after zlib because it depends on zlib.

  The following files need special handling:

    CMakeLists.txt: Keep FLTK version, update manually if necessary.

    Makefile: Same as CMakeLists.txt.

    pnglibconf.h: Generate on a Linux system and merge.

    makedepend: Keep this file.

      Run `make depend' in the png folder on a Linux system after
      the upgrade to update this file.


jpeg:

  Website:    https://ijg.org/
  Download:   See website and follow links.
  Repository: <unknown>

  The following files need special handling:

    CMakeLists.txt: Keep FLTK version, update manually if necessary.

    Makefile: Same as CMakeLists.txt.

    Note: more to come...

    makedepend: Keep this file.

      Run `make depend' in the jpeg folder on a Linux system after
      the upgrade to update this file.
