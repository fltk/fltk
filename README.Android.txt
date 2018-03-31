
README.Android.txt - Building and using FLTK with Android Studio 3
==================================================================


WARNING: FLTK FOR ANDROID IS WORK IN PROGRESS IN A PRETTY EARLY STAGE.


CONTENTS
--------

  1 Building FLTK with Android Studio 3
  2 Building Apps on an Android device with C4Droid
  3 Extensions and limitation of FLTK on Android
  4 DOCUMENT HISTORY


BUILDING FLTK SAMPLE WITH ANDROID STUDIO 3
------------------------------------------

There is no need to ever write a single line of Java.

Download and install AndroidStudio on your developer machine. If you use
AndroidStudio for the first time, use the IDE to download and build the
"Native Plasm" sample app. In the process, all resources required to build
FLTK will be installed. Once Native Plasm runs on your emulator or physical
device, you are ready to install FLTK.

- if your host machine runs MSWindows, rename the file
  "ide/AndroidStudio3/gradlew.rename_to_bat" to "gradlew.bat" first

- open the "ide/AndroidStudio3/" directory as a project in AndroidStudio

- click "run"; the project should compile and run out of the box


Building Apps on an Android device with C4Droid
-----------------------------------------------

WORK IN PROGRESS:
C4Droid is a minimal IDE that comes with gcc/g++ and runs directly on your
Android device. C4Droid supports Native Activities, SDL, SDL2, and even Qt.
FLTK uses the Native Activity mechanism, so why not write FLTK apps right
on your phone?

 - compile and test the Android test app in ide/HelloAndroid
 - create /sdcard/include/ and /sdcard/lib/ on your Android device
 - copy (fltk)/FL/ and its content into /sdcard/include/
 - copy (fltk)/ide/AndroidStudio3/FL/abi-version.h to /sdcard/include/FL/
 - copy (fltk)/ide/AndroidStudio3/./app/.externalNativeBuild/cmake/debug/arm64-v8a/fltk/lib/libfltk.a
   to /sdcard/lib/
 - ... change Native Activity settings in c4Droid preferences
 - ... remove native glue
 - ... add include and library path, add library
 - ... add support for (which?) STL library


Extensions and limitation of FLTK on Android
--------------------------------------------

Android support for FLTK is in an early stage. As of March 2018, most
rendering works, fonts work, bitmaps and pixmaps work, clipping works, window
layering works, and mouse clicks and keyboard events are handled

When loading fonts:
 - font names starting with a $ will have the system font path inserted
 - font names starting with an @ will be loaded via the Asset Manager
 - all other names will be used verbatim

Limitations:
 - the screen size is currently fixed to 600x800 and is scaled up
 - many many big and little functions are not yet implemented


DOCUMENT HISTORY
----------------

Mar 29 2018 - matt: many graphics functions ahve been implemented, keyboard
Mar 17 2018 - matt: added Android extensions for fonts
Mar 12 2018 - matt: started list of limitation that serevs as information to the
                    user as much as a todo list for core developers
Mar  6 2018 - matt: moved project to ide/AndroidStudio3/
Mar  2 2018 - matt: rewriting Android port and documentation from scratch
Feb  9 2016 - matt: recreated document with more warnings and active support
