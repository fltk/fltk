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

# all target modules will be added here later
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/settings.gradle.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/settings.gradle"
  @ONLY
)

# configuration file for all modules
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android/build.gradle.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/build.gradle"
  @ONLY
)

# create a custom abi file for this setup
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/abi-version.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/AndroidStudio/FL/abi-version.h"
  @ONLY
)

# TODO: where should we create config.h?

macro(CREATE_ANDROID_IDE_FOR_TEST NAME SOURCES LIBRARIES)

  # message(STATUS "Creating Android IDE for ${NAME}")

  set (ANDROID_APP_NAME ${NAME})
  set (ANDROID_FLTK_DIR ${CMAKE_SOURCE_DIR})

  set (srcs)
  set (ANDROID_APP_SOURCES)
  set (ANDROID_FLUID_COMMANDS)
  set (flsrcs)        # fluid source files
  foreach(src ${SOURCES})
    if ("${src}" MATCHES "\\.fl$")
      list(APPEND flsrcs ${src})
      string(REGEX REPLACE "(.*).fl" \\1 basename ${src})
      string(APPEND ANDROID_FLUID_COMMANDS
        "add_custom_command(\n"
        "  OUTPUT \"\${FLTK_DIR}/test/${basename}.cxx\" \"\${FLTK_DIR}/test/${basename}.h\"\n"
        "  COMMAND fluid -c \"${src}\"\n"
        "  WORKING_DIRECTORY \"\${FLTK_DIR}/test\"\n"
        "  DEPENDS \"\${FLTK_DIR}/test/${src}\"\n"
        "  MAIN_DEPENDENCY \"\${FLTK_DIR}/test/${src}\"\n"
        ")\n\n"
      )
      file(TOUCH "${basename}.cxx")
      file(TOUCH "${basename}.h")
      file(TOUCH "${basename}.fl")
      set(src_cxx ${basename}.cxx)
    else ()
      list(APPEND srcs ${src})
      set(src_cxx ${src})
    endif ("${src}" MATCHES "\\.fl$")
    string(APPEND ANDROID_APP_SOURCES "    \"\${FLTK_DIR}/test/${src_cxx}\"\n")
  endforeach(src)

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/app.build.gradle.in"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/build.gradle"
    @ONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/AndroidManifest.xml.in"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/AndroidManifest.xml"
    @ONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/Roboto-Regular.ttf"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/assets/fonts/Roboto-Regular.ttf"
    COPYONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/mdpi.ic_launcher.png"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/res/mipmap-mdpi/ic_launcher.png"
    COPYONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/hdpi.ic_launcher.png"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/res/mipmap-hdpi/ic_launcher.png"
    COPYONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/xhdpi.ic_launcher.png"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/res/mipmap-xhdpi/ic_launcher.png"
    COPYONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/xxhdpi.ic_launcher.png"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/res/mipmap-xxhdpi/ic_launcher.png"
    COPYONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/strings.xml.in"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/res/values/strings.xml"
    @ONLY
  )

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/CMakeList.txt.in"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/cpp/CMakeLists.txt"
    @ONLY
  )

  file(APPEND "${CMAKE_BINARY_DIR}/AndroidStudio/settings.gradle" "include ':${ANDROID_APP_NAME}'\n")

endmacro(CREATE_ANDROID_IDE_FOR_TEST NAME SOURCES LIBRARIES)


macro(CREATE_ANDROID_IDE_WRAPUP)

  # message(STATUS "Wrapping up Android IDE creation")

endmacro(CREATE_ANDROID_IDE_WRAPUP)


