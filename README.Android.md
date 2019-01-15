README.Android.md

# Building and Running FLTK with Android Studio 3


WARNING: FLTK FOR ANDROID IS WORK IN PROGRESS IN A PRETTY EARLY STAGE.


## Contents

1. Building FLTK with Android Studio 3
2. Extensions and limitation of FLTK on Android
3. Document History


## Building FLTK with Android Studio 3

There is no need to ever write a single line of Java.

Download and install AndroidStudio on your developer machine. If you use
AndroidStudio for the first time, use the IDE to download and build the
"Native Plasm" sample app. In the process, all resources required to build
FLTK will be installed. Once Native Plasm runs on your emulator or physical
device, you are ready to install FLTK.

Build FLTK for your native platform first using CMake. AndroidStudio will need a native
version of the user interface design tool _Fluid_.

The following example is for the _Xcode_ IDE on _macOS_. The same should work for 
other IDEs and Makefiles on other platforms that run _AndroidStudio_.

```bash
git clone https://github.com/fltk/fltk.git fltk-1.4.git
cd fltk-1.4.git
mkdir build
cd build
mkdir Xcode
cd Xcode
cmake -G "Unix Makefiles" \
  -D OPTION_USE_SYSTEM_LIBJPEG=Off \
  -D OPTION_USE_SYSTEM_ZLIB=Off \
  -D OPTION_USE_SYSTEM_LIBPNG=Off \
  -D OPTION_CREATE_ANDROID_STUDIO_IDE=On \
  ../..
```
Note the last option, `-D OPTION_CREATE_ANDROID_STUDIO_IDE=On`. This option will 
create the file needed to create an FLTK demo project in 
`fltk-1.4.git/build/Xcode/AndroidStudio`.

- open the `build/Xcode/AndroidStudio/` directory as a project in AndroidStudio3

- click "run"; the project should compile and run out of the box


## Extensions and limitation of FLTK on Android

Android support for FLTK is in an early stage. As of January 2019, most
rendering works, fonts work, bitmaps and pixmaps work, clipping works, window
layering works, and mouse clicks and keyboard events are handled

When loading fonts:
 - font names starting with a $ will have the system font path inserted
 - font names starting with an @ will be loaded via the Asset Manager
 - all other names will be used verbatim

Limitations:
 - the screen size is currently fixed to 600x800 and is scaled up
 - many many big and little functions are not yet implemented


## Document History

Jan 15 2019 - matt: rewrote document to explain the new CMake process
Mar 29 2018 - matt: many graphics functions have been implemented, keyboard
Mar 17 2018 - matt: added Android extensions for fonts
Mar 12 2018 - matt: started list of limitation that serevs as information to the
                    user as much as a todo list for core developers
Mar  6 2018 - matt: moved project to ide/AndroidStudio3/
Mar  2 2018 - matt: rewriting Android port and documentation from scratch
Feb  9 2016 - matt: recreated document with more warnings and active support
