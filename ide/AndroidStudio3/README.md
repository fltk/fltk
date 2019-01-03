# Hello Android

Hello Android is an FLTK sample app that derives from the Android Studio sample
project Native Plasma. This sample uses Android Studio 3 and CMake.


## Pre-requisites

* Android Studio 3.0+ with [NDK](https://developer.android.com/ndk/) bundle.


## Getting Started

1. [Download Android Studio](http://developer.android.com/sdk/index.html)
1. Launch Android Studio.
1. Open the `IDE` directory inside the FLTK directory.
1. Open the Android Studio project by loading the `AndroidStudio3` directory.
1. Click *Run/Run 'app'*.


## Support

If you've found an error in these samples, please [file an issue](http://www.fltk.org/str.php). Patches are encouraged, and may be submitted via the same FLTK Bug & Feature system.

Please visit the FLTK [Forum](http://www.fltk.org/newsgroups.php) for additional help.


## License

FLTK is provided under the terms of the [GNU Library Public License, Version 2 with exceptions](http://www.fltk.org/COPYING.php) that allow for static linking.


## Android Shell

List of short little helpers:

```bash
am start -n org.fltk.android_hello/android.app.NativeActivity
am force-stop org.fltk.android_hello

stop
setprop libc.debug.malloc 10
// setprop libc.debug.malloc.program org.fltk.android_hello
setprop libc.debug.malloc.options "guard fill"
start
```
