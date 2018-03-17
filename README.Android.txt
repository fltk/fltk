
README.Android.txt - Building and using FLTK with Android Studio 3
==================================================================


WARNING: FLTK FOR ANDROID IS WORK IN PROGRESS IN A PRETTY EARLY STAGE.


CONTENTS
--------

  1	Building FLTK with Android Studio 3
  2 Extensions and limitation of FLTK on Android
  3	DOCUMENT HISTORY


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


Extensions and limitation of FLTK on Android
--------------------------------------------

Android support for FLTK is in a very early stage. As of March 2018, very basic
rendering works, text rendering work, clipping works, window layering works,
and mouse clicks (touch events) are detected.

When loading fonts:
 - font names starting with a $ will have the system font path inserted
 - font names starting with an @ will be loaded via the Asset Manager
 - all other names will be used verbatim

Limitations:
 - the screen size is currently fixed to 600x800 and is scaled up
 - many many big and little functions are not yet implemented


DOCUMENT HISTORY
----------------

Mar 17 2018 - matt: added Android extensions for fonts
Mar 12 2018 - matt: started list of limitation that serevs as information to the
                    user as much as a todo list for core developers
Mar  6 2018 - matt: moved project to ide/AndroidStudio3/
Mar  2 2018 - matt: rewriting Android port and documentation from scratch
Feb  9 2016 - matt: recreated document with more warnings and active support
