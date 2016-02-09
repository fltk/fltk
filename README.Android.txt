README.Android.txt - Building and using FLTK with CMake for the Android platform
--------------------------------------------------------------------------------


WARNING: BUILDING FLTK FOR ANDROID IS WORK IN PROGRESS IN A PRETTY EARLY STAGE.
WARNING: THIS FILE MERELY CONTAINS A ROUGH LAYOUT AND SOME SIMPLE IDEAS HOW
         FLTK WILL BUILD FOR ANDROID EVENTUALLY.


 CONTENTS
==========

  1	INTRODUCTION TO CMAKE
  2	USING CMAKE TO BUILD FLTK
    2.1   Prerequisites
    2.2   Options
    2.3   Building under Linux with Unix Makefiles
    2.4   Crosscompiling
  3	USING CMAKE WITH FLTK
    3.1   Library names
    3.2   Using Fluid files
  4	DOCUMENT HISTORY


 INTRODUCTION TO CMAKE
=======================

Please read README.CMake.txt in the same directory to learn about CMake.

More information on CMake can be found on its web site http://www.cmake.org.



 USING CMAKE TO BUILD FLTK FOR ANDROID ON OS X AND LINUX
=========================================================


 PREREQUISITES
---------------

Get CMake 3.2.3 or newer.

Go to https://github.com/taka-no-me/android-cmake and download the content 
as a zip file.

Go into the FLTK base directory, then:

> mkdir build
> cd build
> unzip ~/Downloads/android-cmake-master.zip
> mv android-cmake-master Android
> cd Android
> cmake -DCMAKE_TOOLCHAIN_FILE=android.toolchain.cmake -DANDROID_NDK=~/dev/android-ndk-r10e -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="armeabi-v7a" ../..
> make

At this time, compilation will fail relatively soon, but we are working hard to make porting easier,
and to prove our work, we poert FLTK to Android at the same time. Using CMake to cross-compile
is one of the many steps required to make life easier and maintenance more flexible.


 OPTIONS
---------
Options can be specified to cmake with the -D flag:

    cmake -D <OPTION_NAME>=<OPTION_VALUE>



WARNING: Old outdated information:

Building FLTK on Android (pre alpha, don't expect miracles)


1. cd to the root of its source code ('cd android/hello')

2. Run ndk-build. This builds the native code, and should result in some .so files being put into the libs directory.

3. android update project --path . --name hello

4. ant debug (or similar). This will build the Java code and create an .apk. Crucially, the build process will pick up the .so files left within the libs directory and include them into the .apk.

5. adb install bin/name-of-project.apk

6. Then launch as normal using the Android GUI or using an am start command such as you give.
6a. emulator -avd Intel_x86

7. adb shell am start -n com.example.native_activity

8. adb uninstall com.example.native_activity


 DOCUMENT HISTORY
==================

Feb 9 2016 - matt: recreated document with more warnings and active support
