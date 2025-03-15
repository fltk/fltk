#
# A function used by the CMake build system for the Fast Light Tool Kit (FLTK).
# Originally written by Michael Surette
#
# Copyright 1998-2025 by Bill Spitzak and others.
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
# function fl_create_example - Create a test/demo (example) program
#
# Input:
#
# - NAME: program name, e.g. 'hello'
#
# - SOURCES: list of source files, separated by ';' (needs quotes)
#   Sources can be:
#   - .c/.cxx files, e.g. 'hello.cxx'
#   - .fl (fluid) files, e.g. 'radio.fl'
#   - .plist file(macOS), e.g. 'editor.plist'
#   - .icns file(macOS Icon), e.g. 'checkers.icns'
#   - .rc file(Windows resource file, e.g. icon definition)
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
#   List of libraries (CMake target names), separated by ';'. Must be
#   quoted if more than one library is required, e.g. "fltk::gl;fltk::images"
#
################################################################################

function(fl_create_example NAME SOURCES LIBRARIES)

  set(srcs)                    # source files
  set(flsrcs)                  # fluid source (.fl) files
  set(TARGET_NAME ${NAME})     # CMake target name
  set(ICON_NAME)               # macOS icon (max. one)
  set(PLIST)                   # macOS .plist file(max. one)
  set(ICON_PATH)               # macOS icon resource path

  # create macOS bundle? 0 = no, 1 = yes

  if(APPLE AND NOT FLTK_BACKEND_X11)
    set(MAC_BUNDLE 1)
  else()
    set(MAC_BUNDLE 0)
  endif()

  # filter input files for different handling (fluid, icon, plist, source)

  foreach(src ${SOURCES})
    if("${src}" MATCHES "\\.fl$")
      list(APPEND flsrcs ${src})
    elseif("${src}" MATCHES "\\.icns$")
      set(ICON_NAME "${src}")
    elseif("${src}" MATCHES "\\.plist$")
      set(PLIST "${src}")
    else()
      list(APPEND srcs ${src})
    endif("${src}" MATCHES "\\.fl$")
  endforeach(src)

  # generate source files from .fl files, add output to sources

  if(flsrcs)
    if(NOT FLTK_FLUID_EXECUTABLE)
      message(STATUS "Example app \"${NAME}\" will not be built. FLUID executable not found.")
      return ()
    endif()
    FLTK_RUN_FLUID (FLUID_SOURCES "${flsrcs}")
    list(APPEND srcs ${FLUID_SOURCES})
    unset(FLUID_SOURCES)
  endif(flsrcs)

  # set macOS (icon) resource path if applicable

  if(MAC_BUNDLE AND ICON_NAME)
    set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/mac-resources/${ICON_NAME}")
  endif(MAC_BUNDLE AND ICON_NAME)

  ##############################################################################
  # add executable target and set properties (all platforms)
  ##############################################################################

  if(MAC_BUNDLE)
    add_executable        (${TARGET_NAME} MACOSX_BUNDLE ${srcs} ${ICON_PATH})
  else()
    add_executable        (${TARGET_NAME} WIN32 ${srcs})
  endif(MAC_BUNDLE)

  set_target_properties   (${TARGET_NAME} PROPERTIES OUTPUT_NAME ${NAME})
  target_link_libraries   (${TARGET_NAME} PRIVATE ${LIBRARIES})

  # make sure we're "exporting" global symbols like 'fl_disable_wayland',
  # see also README.Wayland.txt and CMake policy CMP0065.

  set_target_properties   (${TARGET_NAME} PROPERTIES ENABLE_EXPORTS TRUE)


  # Search the current binary directory for header files created by CMake
  # or fluid and the source folder for other headers included by test programs

  target_include_directories(${TARGET_NAME} PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
  )

  if(MAC_BUNDLE)
    if(PLIST)
      set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                            "${CMAKE_CURRENT_SOURCE_DIR}/mac-resources/${PLIST}")
    endif()

    string(REPLACE "_" "-" FLTK_BUNDLE_ID "org.fltk.${TARGET_NAME}")
    set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "${TARGET_NAME}")
    set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "${FLTK_BUNDLE_ID}")
    set_target_properties(${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${FLTK_BUNDLE_ID}")

    if(ICON_NAME)
      set_target_properties(${TARGET_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
      set_target_properties(${TARGET_NAME} PROPERTIES RESOURCE ${ICON_PATH})
    endif()
  endif()

  ##############################################################################
  # Copy macOS "bundle wrapper" (shell script) to target directory.
  # The "custom command" will be executed "POST_BUILD".
  ##############################################################################

  if(MAC_BUNDLE)
    set(WRAPPER "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}")

    add_custom_command(
      TARGET ${TARGET_NAME} POST_BUILD
      COMMAND cp ${FLTK_SOURCE_DIR}/CMake/macOS-bundle-wrapper.in ${WRAPPER}
      COMMAND chmod u+x,g+x,o+x ${WRAPPER}
      BYPRODUCTS ${WRAPPER}
      # COMMENT "Creating macOS bundle wrapper script ${WRAPPER}"
      VERBATIM
    )
    unset(WRAPPER)
  endif(MAC_BUNDLE)

  ##############################################################################
  # MSVC: Add fltk-shared (DLL) path to Environment 'PATH' for debugging
  ##############################################################################

  if(MSVC AND TARGET fltk-shared)
    set(DllDir "$<SHELL_PATH:$<TARGET_FILE_DIR:fltk-shared>>")
    set_target_properties(${TARGET_NAME} PROPERTIES
      VS_DEBUGGER_ENVIRONMENT "PATH=${DllDir};$ENV{PATH}"
    )
  endif()

endfunction()
