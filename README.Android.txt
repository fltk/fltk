README.Android.txt - Building and using FLTK with Android Studio 3
------------------------------------------------------------------


WARNING: FLTK FOR ANDROID IS WORK IN PROGRESS IN A PRETTY EARLY STAGE.


 CONTENTS
==========

  1	Building FLTK with Android Studio 3
  2	DOCUMENT HISTORY


 BUILDING FLTK SAMPLE WITH ANDROID STUDIO 3
============================================

There is no need to ever write a single line of Java.

Download and install AndroidStudio on your developer machine. If you use
AndroidStudio for the first time, use the IDE to download and build the
"Native Plasm" sample app. In the process, all resources required to build
FLTK will be installed. Once Native Plasm runs on your emulator or physical
device, you are ready to install FLTK.

- if your host machine runs MSWindows, unzip the file
  "ide/AndroidStudio3/gradlew.zip" first

- open the "ide/AndroidStudio3/" directory as a project in AndroidStudio

- click "run"; the project should compile and run out of the box


 DOCUMENT HISTORY
==================

Mar 6 2018 - matt: moved project to ide/AndroidStudio3/
Mar 2 2018 - matt: rewriting Android port and documentation from scratch
Feb 9 2016 - matt: recreated document with more warnings and active support
