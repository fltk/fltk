
Using Watcom to build and use FLTK 1.1.5

Supported targets: Win32 only, static builds (no DLLs). Can be used from any Watcom
   supported host (DOS, OS/2, Windows).

1. Unzip the include file in the root of the fltk 1.1.5 directory. It will create a
   new directory called watcom, put a makefile.wat in source directories, and update
   two sources files which need to be different from the 1.1.5 versions (STR updated),
   and put a watcom.mif file in the fltk root directory.

2. To build: set the environment variable fltk to the root directory of fltk, go
   to the Watcom directory, run wmake. Both debug and release versions of all libs,
   test programs and FLUID will be built.

3. To create you own programs: use the supplied watcom.mif file. If you use fluid,
   move the two fluid rules from test/makefile.wat into the watcom.mif
   file. Look in test/makefile.wat also for rules about building a non-fluid program
   with one object (source) file, or with multiple. if you use FLUID, take care in
   the order of object files specified (see tes/makefile.wat keyboard.exe).

Questions about the watcom port please to the fltk.general newsgroup.

Mat Nieuwenhoven, Hilversum, 2004-10-28


