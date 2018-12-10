-----------------------------------------
  HOW TO BUILD AND USE FLTK WITH AN IDE
-----------------------------------------

Since FLTK 1.4 we do no longer include IDE [1] solution files in our
source distribution. [2]

If you want to build the FLTK library with an IDE you need to use
CMake [3] to generate the IDE files from the source distribution.

The FLTK team will officially support generation of selected IDE projects,
particularly Visual C++ and Xcode. Older version support of these IDE
projects will be limited to the versions that are supported by and can be
generated with CMake.

Other IDE solutions generated with CMake may or may not work. The FLTK
team will try to support as many IDE solutions as possible, but we may
need help to adjust the CMake files to fit a particular IDE project.


Using CMake to generate IDE project files
------------------------------------------------------------------------

IDE files can easily be created using CMake and the provided CMake files.

For more informations about using CMake to build FLTK please read the file
README.CMake.txt in the root directory of the FLTK distribution.


   Current Status as of Nov 2016:
-------------------------------------

   Xcode:        Supported       Known to work.
   Visual C++:   Supported       Visual C++ 2015 generator known to work.

   Other IDE's:  Not supported   Status unknown.

Note: "Not supported" doesn't mean that a particular generator does not work,
      but the FLTK team does not put much effort into making this IDE work,
      hence it may work for you or not. Contributions welcome.

--------------------------------------------------------------------------------

[1] IDE = Integrated Development Environment,
    for instance Visual Studio, Xcode, Eclipse, ...
    https://en.wikipedia.org/wiki/Integrated_development_environment

[2] The only exception is the Android Studio IDE in ide/AndroidStudio3
    currently provided for testing. This IDE solution is likely to be
    moved elsewhere or removed entirely before FLTK 1.4 will be released.

[3] https://cmake.org/
