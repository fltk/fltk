#
# A macro used by the CMake build system for the Fast Light Tool Kit (FLTK).
# Written by Michael Surette
#
# Copyright 1998-2020 by Bill Spitzak and others.
#
# This library is free software. Distribution and use rights are outlined in
# the file "COPYING" which should have been included with this file.  If this
# file is missing or damaged, see the license at:
#
#     https://www.fltk.org/COPYING.php
#
# Please see the following page on how to report bugs and issues:
#
#     https://www.fltk.org/bugs.php
#

#######################################################################
#
# macro CREATE_EXAMPLE - Create a test/demo program
#
# Input:
#
# - NAME: program name, e.g. 'hello'
#
# - SOURCES: list of source files, separated by ';' (needs quotes)
#   Sources can be:
#   - .c/.cxx files, e.g. 'hello.cxx'
#   - .fl (fluid) files, e.g. 'radio.fl'
#   - .plist file (macOS), e.g. 'editor-Info.plist'
#   - .icns file (macOS Icon), e.g. 'checkers.icns'
#   File name (type), e.g. '.icns' matters, it is parsed internally.
#   Order of sources doesn't matter, multiple .cxx and .fl files are
#   supported, but only one .plist and .icns file.
#   macOS specific files are ignored on other platforms.
#
# - LIBRARIES:
#   List of libraries (CMake target names), separated by ';'. Needs
#   quotes if more than one library is needed, e.g. "fltk_gl;fltk"
#
# CREATE_EXAMPLE can have an optional fourth argument with a list of options
# - the option ANDROID_OK is set if CREATE_EXAMPLE creates code for Android
#   builds in addition to the native build
#
#######################################################################

macro (CREATE_EXAMPLE NAME SOURCES LIBRARIES)

  set (srcs)                    # source files
  set (flsrcs)                  # fluid source (.fl) files
  set (TARGET_NAME ${NAME})     # CMake target name
  set (FLUID_SOURCES)           # generated sources
  set (ICON_NAME)               # macOS icon (max. one)
  set (PLIST)                   # macOS .plist file (max. one)
  set (RESOURCE_PATH)           # macOS resource path

  # rename target name "help" (reserved since CMake 2.8.12 and later)
  # FIXME: not necessary in FLTK 1.4 but left for compatibility (06/2020)

  if (${TARGET_NAME} STREQUAL "help")
    set (TARGET_NAME "test_help")
  endif (${TARGET_NAME} STREQUAL "help")

  # filter input files for different handling (fluid, icon, plist, source)

  foreach (src ${SOURCES})
    if ("${src}" MATCHES "\\.fl$")
      list (APPEND flsrcs ${src})
    elseif ("${src}" MATCHES "\\.icns$")
      set (ICON_NAME "${src}")
    elseif ("${src}" MATCHES "\\.plist$")
      set (PLIST "${src}")
    else ()
      list (APPEND srcs ${src})
    endif ("${src}" MATCHES "\\.fl$")
  endforeach (src)

  if (flsrcs)
    FLTK_RUN_FLUID (FLUID_SOURCES "${flsrcs}")
  endif (flsrcs)

  ## FIXME ## #############################################################
  ## FIXME ## The macOS specific code needs reorganization/simplification
  ## FIXME ## -- Albrecht (06/2020)
  ## FIXME ## =============================================================
  ## FIXME ## Use "new" function `FLTK_SET_BUNDLE_ICON()` (?)
  ## FIXME ## -- see CMake/FLTK-Functions.cmake (may need some tweaks)
  ## FIXME ## -- ported from FLTK 1.3 (06/2020)
  ## FIXME ## #############################################################

  if (APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))

    ## -- Code from FLTK 1.3 for reference (variable names adjusted):
    ##    add_executable (${TARGET_NAME} MACOSX_BUNDLE ${srcs} ${FLUID_SOURCES} ${ICON_NAME})
    ##    if (ICON_NAME)
    ##      FLTK_SET_BUNDLE_ICON (${TARGET_NAME} ${ICON_NAME})
    ##    endif ()
    ## -- End of code from FLTK 1.3

    if (ICON_NAME)
      set (RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_NAME}.app/Contents/Resources/${ICON_NAME}")
    elseif (${TARGET_NAME} STREQUAL "demo")
      set (RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/demo.menu")
    endif (ICON_NAME)

    if (RESOURCE_PATH)
      add_executable (${TARGET_NAME} MACOSX_BUNDLE ${srcs} ${FLUID_SOURCES} ${RESOURCE_PATH})
      if (${TARGET_NAME} STREQUAL "demo")
        target_compile_definitions (demo PUBLIC USING_XCODE)
      endif (${TARGET_NAME} STREQUAL "demo")
    else ()
      add_executable (${TARGET_NAME} MACOSX_BUNDLE ${srcs} ${FLUID_SOURCES})
    endif (RESOURCE_PATH)
  else ()
    add_executable (${TARGET_NAME} WIN32 ${srcs} ${FLUID_SOURCES})
  endif (APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL))

  set_target_properties (${TARGET_NAME}
    PROPERTIES OUTPUT_NAME ${NAME}
  )

  if (APPLE AND RESOURCE_PATH)
    if (ICON_NAME)
      set_target_properties (${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
    endif (ICON_NAME)
    set_target_properties (${TARGET_NAME} PROPERTIES RESOURCE ${RESOURCE_PATH})
  endif (APPLE AND RESOURCE_PATH)

  if (APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL) AND PLIST)
    set_target_properties (${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/${PLIST}")
  endif (APPLE AND (NOT OPTION_APPLE_X11) AND (NOT OPTION_APPLE_SDL) AND PLIST)

  target_link_libraries (${TARGET_NAME} ${LIBRARIES})

  # Parse optional fourth argument 'ANDROID_OK', see description above.

  if (${ARGC} GREATER 3)
    foreach (OPTION ${ARGV3})
      if (${OPTION} STREQUAL ANDROID_OK AND OPTION_CREATE_ANDROID_STUDIO_IDE)
        CREATE_ANDROID_IDE_FOR_TEST (${NAME} ${SOURCES} ${LIBRARIES})
      endif ()
    endforeach ()
  endif ()

endmacro (CREATE_EXAMPLE NAME SOURCES LIBRARIES)
