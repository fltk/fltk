Hello Android
=============

Hello Android is an FLTK sample app that derives from the Android Studio sample
project Native Plasma. This sample uses Android Studio 3 and CMake.


Pre-requisites
--------------
- Android Studio 3.0+ with [NDK](https://developer.android.com/ndk/) bundle.


Getting Started
---------------
1. [Download Android Studio](http://developer.android.com/sdk/index.html)
1. Launch Android Studio.
1. Open the Android directory inside the FLTK directory.
1. Open the AndroidStudio project by loading the HelloAndroid directory.
1. Click *Run/Run 'app'*.


Micrososft Windows Users
------------------------
The file 'gradlew.zip' must be unzipped before you can use AndroidStudio. It contains the required file 'gradlew.bat'. This was neccessary because some svn setups block anything that looks like an executable file, including files that end in .bat .


Support
-------
If you've found an error in these samples, please [file an issue](http://www.fltk.org/str.php). Patches are encouraged, and may be submitted via the same FLTK Bug & Feature system.

Please visit the FLTK [Forum](http://www.fltk.org/newsgroups.php) for additional help.


License
-------
FLTK is provided under the terms of the [GNU Library Public License, Version 2 with exceptions](http://www.fltk.org/COPYING.php) that allow for static linking.


Android Shell
-------------
> am start -n org.fltk.android_hello/android.app.NativeActivity
> am force-stop org.fltk.android_hello

> stop
> setprop libc.debug.malloc 10
> // setprop libc.debug.malloc.program org.fltk.android_hello
> setprop libc.debug.malloc.options "guard fill"
> start

