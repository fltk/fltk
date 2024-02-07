#
# FLTK-Functions.cmake
# Originally written by Michael Surette
#
# Copyright 1998-2024 by Bill Spitzak and others.
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
# functions used by the build system and exported for the end-user
################################################################################

################################################################################
#
# fltk_run_fluid: Run fluid to compile fluid (.fl) files
#
# Usage: fltk_run_fluid(SOURCES "FLUID_SOURCE [.. FLUID_SOURCE]")
#
# Input:  The CMake variable "FL_FILES" must contain a list of input
#         file names. Only names ending in ".fl" are considered and
#         compiled to their ".cxx" and ".h" files which are stored
#         in the current build directory.
#
# Output: "SOURCES" is used as a CMake variable name that is assigned the
#         names of the compiled ".cxx" files as a ";" separated list in the
#         parent scope (the caller's scope).
#
################################################################################

function(fltk_run_fluid SOURCES FL_FILES)

  if(NOT FLTK_FLUID_EXECUTABLE)
    message(WARNING "Not building ${FL_FILES}. FLUID executable not found.")
    return()
  endif()

  set(CXX_FILES)
  foreach(src ${FL_FILES})
    if("${src}" MATCHES "\\.fl$")
      string(REGEX REPLACE "(.*).fl" \\1 basename ${src})
      add_custom_command(
        OUTPUT "${basename}.cxx" "${basename}.h"
        COMMAND ${FLTK_FLUID_EXECUTABLE} -c ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${src}
        MAIN_DEPENDENCY ${src}
      )
      list(APPEND CXX_FILES "${basename}.cxx")
    endif()
  endforeach()
  set(${SOURCES} ${CXX_FILES} PARENT_SCOPE)

endfunction(fltk_run_fluid SOURCES FL_FILES)


################################################################################
#
# fltk_set_bundle_icon: Set the bundle icon for macOS bundles
#
# This function sets macOS specific target properties. These properties are
# ignored on other platforms.
#
# Usage: fltk_set_bundle_icon(TARGET ICON_PATH)
#
# TARGET must be a valid CMake target that has been created before this
# function is called. It must not be an alias name.
#
################################################################################

function(fltk_set_bundle_icon TARGET ICON_PATH)
  get_filename_component(ICON_NAME "${ICON_PATH}" NAME)
  set_target_properties(${TARGET}
    PROPERTIES
      MACOSX_BUNDLE_ICON_FILE "${ICON_NAME}"
      RESOURCE "${ICON_PATH}"
  )
endfunction(fltk_set_bundle_icon TARGET ICON_PATH)
