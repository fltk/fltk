README.bundled-libs.txt - Developer information for bundled libraries
---------------------------------------------------------------------

******************************************************
*** NOTICE *** This file is still work in progress ***
******************************************************

This file is mainly intended for FLTK developers and contains information
about the current versions of all bundled libraries and about how to
upgrade these bundled libraries.


Current versions of bundled libraries:

  ** work in progress -- not yet completely upgraded **

  Library       Version            Release date		FLTK Version
  ------------------------------------------------------------------
  jpeg          jpeg-9c            2018-01-14           1.4.0
  nanosvg       [2017-07-09]       n.a.                 1.4.0
  png           libpng-1.6.34      2017-09-29           1.4.0
  zlib          zlib-1.2.11        2017-01-15           1.4.0


Previous versions of bundled libraries:

  Library       Version            Release date		FLTK Version
  ------------------------------------------------------------------
  jpeg          jpeg-9a            2014-01-19           1.3.4
  png           libpng-1.6.16      2014-12-22           1.3.4
  zlib          zlib-1.2.8         2013-04-28           1.3.4


General information:

  FLTK does not include the entire library distributions. We only provide
  the source files necessary to build the library itself. There are no
  test programs or other contributed files.

  We use our own build files, hence a few files MUST NOT be upgraded when
  the library source files are upgraded. We strive to keep changes to the
  library source files as small as possible. Patching library code to
  work with FLTK should be a rare exception.

  If patches are necessary all changes in the library files should be
  marked with "FLTK" in a comment so a developer that upgrades the library
  later is aware of changes in the source code for FLTK. Additional comments
  should be added to show the rationale, i.e. why a particular change was
  necessary. If applicable, add a reference to a Software Trouble Report
  like "STR #3456".


How to update the bundled libraries:

  It is generally advisable to use a graphical merge program. I'm using
  'meld' under Linux, but YMMV.

  Do not add any source files unless they are required to build the library.

  Some config header files may be pre-generated in the FLTK sources. These
  header files should be left untouched, but it may be necessary to update
  them if new items were added to the new library version. In this case
  the new header should be pre-generated on a Linux system with default
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
  as well, usually to both the Makefile and the CMakeLists.txt to be
  used in configure/make and in CMake based builds.


Upgrade order:

  There is only one dependency between all bundled libraries: libpng
  depends on zlib. Hence zlib should be upgraded first, then all other
  libs can be upgraded in arbitrary order.


Tests after merge:

  Tests should be done on as many platforms as possible, both with
  autotools (configure/make) and CMake. Windows (Visual Studio) and
  macOS (Xcode) build need CMake to generate the IDE files.


Upgrade notes for specific libraries:

  The following chapters contain information of specific files and how
  they are upgraded. Since the changes in all bundled libraries can't
  be known in advance this information can change in the future. Please
  very that no other changes are necessary.


zlib:

  Website:    http://zlib.net/
  Download:   See website and follow links.
  Repository: git clone https://github.com/madler/zlib.git


  The following files need special handling:

    CMakeLists.txt: Keep FLTK version, update manually if necessary.

    Makefile: Same as CMakeLists.txt.

    zconf.h: Merge changes.

      As of zlib 1.2.11: two small sections marked with "FLTK" comments
      that need to be kept.

    makedepend: Keep this file. Run `make depend' in the zlib folder
      on a Linux system after the upgrade to update this file.


png:

  Website:    http://libpng.org/pub/png/libpng.html
  Download:   See website and follow links.
  Repository: git clone https://git.code.sf.net/p/libpng/code libpng

  libpng should be upgraded after zlib because it depends on zlib.

  The following files need special handling:

    CMakeLists.txt: Keep FLTK version, update manually if necessary.

    Makefile: Same as CMakeLists.txt.

    Note: more to come...

    makedepend: Keep this file. Run `make depend' in the zlib folder
      on a Linux system after the upgrade to update this file.


jpeg:

  Website:    http://ijg.org/
  Download:   See website and follow links.
  Repository: <unknown>

  The following files need special handling:

    CMakeLists.txt: Keep FLTK version, update manually if necessary.

    Makefile: Same as CMakeLists.txt.

    Note: more to come...

    makedepend: Keep this file. Run `make depend' in the zlib folder
      on a Linux system after the upgrade to update this file.


nanosvg:

  Website:    https://github.com/memononen/nanosvg
  Download:   See website and follow links.
  Repository: git clone https://github.com/memononen/nanosvg.git
  FLTK Clone: git clone https://github.com/fltk/nanosvg.git

  FLTK has its own GitHub clone of the original repository (see above).

  The intention is to update this clone from time to time so the FLTK
  specific patches are up-to-date with the original library. Hopefully
  the FLTK patches will be accepted upstream at some time in the future
  so we no longer need our own patches.
  AlbrechtS, 04 Feb 2018.

  Use this clone (branch 'fltk') to get the nanosvg library with FLTK
  specific patches:

    $ git clone https://github.com/fltk/nanosvg.git nanosvg-fltk
    $ cd nanosvg-fltk
    $ git checkout fltk
    $ cd src
    $ cp nanosvg.h nanosvgrast.h /path/to/fltk-1.4/nanosvg/

  This library does not have its own build files since it is a
  header-only library. The build is included in FLTK where necessary.

  The following files need special handling:

    nanosvg.h:     Merge or download from FLTK's clone (see above).
    nanosvgrast.h: Merge or download from FLTK's clone (see above).
