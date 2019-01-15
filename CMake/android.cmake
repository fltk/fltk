#
# "$Id$"
#
# android.cmake to build the FLTK AndroidStudio project using CMake
# Written by Matthias Melcher
#
# Copyright 1998-2019 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     http://www.fltk.org/COPYING.php
#
# Please report all bugs and problems on the following page:
#
#     http://www.fltk.org/str.php
#

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/settings.gradle.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/settings.gradle"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/build.gradle.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/build.gradle"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/abi-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/FL/abi-version.h"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/app.build.gradle.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/build.gradle"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/AndroidManifest.xml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/AndroidManifest.xml"
  @ONLY
)


configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/Roboto-Regular.ttf"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/assets/fonts/Roboto-Regular.ttf"
  COPYONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/mdpi.ic_launcher.png"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/res/mipmap-mdpi/ic_launcher.png"
  COPYONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/hdpi.ic_launcher.png"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/res/mipmap-hdpi/ic_launcher.png"
  COPYONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/xhdpi.ic_launcher.png"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/res/mipmap-xhdpi/ic_launcher.png"
  COPYONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/xxhdpi.ic_launcher.png"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/res/mipmap-xxhdpi/ic_launcher.png"
  COPYONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/strings.xml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/res/values/strings.xml"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/CMakeList.txt.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/cpp/CMakeLists.txt"
  @ONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/HelloAndroid.cxx.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/app/src/main/cpp/HelloAndroid.cxx"
  @ONLY
)

