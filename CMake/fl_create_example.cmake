#
# A macro used by the CMake build system for the Fast Light Tool Kit (FLTK).
# Originally written by Michael Surette
#
# Copyright 1998-2023 by Bill Spitzak and others.
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

################################################################################
#
# macro CREATE_EXAMPLE - Create a test/demo (example) program
#
# Input:
#
# - NAME: program name, e.g. 'hello'
#
# - SOURCES: list of source files, separated by ';' (needs quotes)
#   Sources can be:
#   - .c/.cxx files, e.g. 'hello.cxx'
#   - .fl (fluid) files, e.g. 'radio.fl'
#   - .plist file (macOS), e.g. 'editor.plist'
#   - .icns file (macOS Icon), e.g. 'checkers.icns'
#   - .rc file (Windows resource file, e.g. icon definition)
#
#   Order of sources doesn't matter, multiple .cxx and .fl files are
#   supported, but only one .plist and one .icns file.
#
#   File name (type), e.g. '.icns' matters, it is parsed internally:
#   File types .fl, .plist, and .icns are treated specifically,
#   all other file types are added to the target's source files.
#
#   macOS specific .icns and .plist files are ignored on other platforms.
#   These files must reside in the subdirectory 'mac-resources'.
#
# - LIBRARIES:
#   List of libraries (CMake target names), separated by ';'. Needs
#   quotes if more than one library is required, e.g. "fltk_gl;fltk"
#
# CREATE_EXAMPLE can have an optional fourth argument with a list of options
# - these options are currently not used
#
################################################################################

macro (CREATE_EXAMPLE NAME SOURCES LIBRARIES)

  set (srcs)                    # source files
  set (flsrcs)                  # fluid source (.fl) files
  set (TARGET_NAME ${NAME})     # CMake target name
  set (ICON_NAME)               # macOS icon (max. one)
  set (PLIST)                   # macOS .plist file (max. one)
  set (ICON_PATH)               # macOS icon resource path

  # create macOS bundle? 0 = no, 1 = yes

  if (APPLE AND (NOT OPTION_APPLE_X11))
    set (MAC_BUNDLE 1)
  else ()
    set (MAC_BUNDLE 0)
  endif (APPLE AND (NOT OPTION_APPLE_X11))

  # rename target name "help" (reserved since CMake 2.8.12)
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

  # generate source files from .fl files, add output to sources

  if (flsrcs)
    FLTK_RUN_FLUID (FLUID_SOURCES "${flsrcs}")
    list (APPEND srcs ${FLUID_SOURCES})
    unset (FLUID_SOURCES)
  endif (flsrcs)

  # set macOS (icon) resource path if applicable

  if (MAC_BUNDLE AND ICON_NAME)
    set (ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mac-resources/${ICON_NAME}")
  endif (MAC_BUNDLE AND ICON_NAME)

  ##############################################################################
  # add executable target and set properties (all platforms)
  ##############################################################################

  if (MAC_BUNDLE)
    add_executable        (${TARGET_NAME} MACOSX_BUNDLE ${srcs} ${ICON_PATH})
  else ()
    add_executable        (${TARGET_NAME} WIN32 ${srcs} ${ICON_PATH})
  endif (MAC_BUNDLE)

  set_target_properties   (${TARGET_NAME} PROPERTIES OUTPUT_NAME ${NAME})
  target_link_libraries   (${TARGET_NAME} ${LIBRARIES})

  if (FLTK_HAVE_CAIRO)
    if (CMAKE_VERSION VERSION_LESS "3.13")
      link_directories    (${PKG_CAIRO_LIBRARY_DIRS})
    else()
      target_link_directories (${TARGET_NAME} PRIVATE ${PKG_CAIRO_LIBRARY_DIRS})
    endif()
  endif (FLTK_HAVE_CAIRO)

  if (ICON_PATH)
    set_target_properties (${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
    set_target_properties (${TARGET_NAME} PROPERTIES RESOURCE ${ICON_PATH})
  endif (ICON_PATH)

  if (PLIST)
    set_target_properties (${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                           "${CMAKE_CURRENT_SOURCE_DIR}/mac-resources/${PLIST}")
  elseif (MAC_BUNDLE)
    set_target_properties (${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${TARGET_NAME}")
    set_target_properties (${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.fltk.${TARGET_NAME}")
  endif ()

  ##############################################################################
  # Copy macOS "bundle wrapper" (shell script) to target directory.
  # The "custom command" will be executed "POST_BUILD".
  ##############################################################################

  if (MAC_BUNDLE)
    set (WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}")
    add_custom_command (
      TARGET ${TARGET_NAME} POST_BUILD
      COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/macOS-bundle-wrapper.in ${WRAPPER}
      COMMAND chmod u+x,g+x,o+x ${WRAPPER}
      BYPRODUCTS ${WRAPPER}
      # COMMENT "Creating macOS bundle wrapper script ${WRAPPER}"
      VERBATIM
    )
    unset (WRAPPER)
  endif (MAC_BUNDLE)

  ######################################################################
  # Parse optional fourth argument, see description above.
  ######################################################################

  # code left commented out as an example

  # *unused* #  if (${ARGC} GREATER 3)
  # *unused* #    foreach (OPTION ${ARGV3})
  # *unused* #      if (${OPTION} STREQUAL "xxx")
  # *unused* #        # do something ...
  # *unused* #      endif ()
  # *unused* #    endforeach ()
  # *unused* #  endif ()

endmacro ()
