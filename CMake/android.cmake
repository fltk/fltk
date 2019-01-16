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


macro(CREATE_ANDROID_IDE_FOR_TEST NAME SOURCES LIBRARIES)

  message(STATUS "Creating Android IDE for ${NAME}")

  set (ANDROID_APP_NAME ${NAME})

  set (srcs)
  set (ANDROID_APP_SOURCES)
  set (ANDROID_APP_COPY_SOURCES)
  set (ANDROID_FLUID_COMMANDS)
  set (flsrcs)        # fluid source files
  foreach(src ${SOURCES})
    if ("${src}" MATCHES "\\.fl$")
      list(APPEND flsrcs ${src})
      string(REGEX REPLACE "(.*).fl" \\1 basename ${src})
      string(APPEND ANDROID_FLUID_COMMANDS
        "add_custom_command( OUTPUT \"${basename}.cxx\" \"${basename}.h\"\n"
        "  OUTPUT \"${basename}.cxx\" \"${basename}.h\"\n"
        "  COMMAND fluid -c \"\${CMAKE_CURRENT_SOURCE_DIR}/${src}\"\n"
        "  DEPENDS ${src}\n"
        "  MAIN_DEPENDENCY ${src}\n"
        ")\n\n"
      )
      set(src_cxx ${basename}.cxx)
    else ()
      list(APPEND srcs ${src})
      set(src_cxx ${src})
    endif ("${src}" MATCHES "\\.fl$")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/cpp/")
    # FIXME: Unix only for older version of CMake
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
      "${CMAKE_CURRENT_SOURCE_DIR}/${src}"
      "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/cpp/${src}")
    string(APPEND ANDROID_APP_SOURCES "    ${src_cxx}\n")
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

  configure_file(
    "${CMAKE_SOURCE_DIR}/CMake/Android/HelloAndroid.cxx.in"
    "${CMAKE_BINARY_DIR}/AndroidStudio/${ANDROID_APP_NAME}/src/main/cpp/HelloAndroid.cxx"
    @ONLY
  )

  file(APPEND "${CMAKE_BINARY_DIR}/AndroidStudio/settings.gradle" "include ':${ANDROID_APP_NAME}'\n")

endmacro(CREATE_ANDROID_IDE_FOR_TEST NAME SOURCES LIBRARIES)


macro(CREATE_ANDROID_IDE_WRAPUP)

  message(STATUS "Wrapping up Android IDE creation")

endmacro(CREATE_ANDROID_IDE_WRAPUP)


