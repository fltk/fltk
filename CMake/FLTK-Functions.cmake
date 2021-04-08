#
# FLTK-Functions.cmake
# Written by Michael Surette
#
# Copyright 1998-2021 by Bill Spitzak and others.
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
# functions used by the build system and exported for the end-user
#######################################################################

# USAGE: FLTK_RUN_FLUID TARGET_NAME "FLUID_SOURCE [.. FLUID_SOURCE]"

function (FLTK_RUN_FLUID TARGET SOURCES)
  set (CXX_FILES)
  foreach (src ${SOURCES})
    if ("${src}" MATCHES "\\.fl$")
      string(REGEX REPLACE "(.*).fl" \\1 basename ${src})
      add_custom_command(
        OUTPUT "${basename}.cxx" "${basename}.h"
        COMMAND fluid -c ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${src}
        MAIN_DEPENDENCY ${src}
      )
      list (APPEND CXX_FILES "${basename}.cxx")
    endif ("${src}" MATCHES "\\.fl$")
  endforeach ()
  set (${TARGET} ${CXX_FILES} PARENT_SCOPE)
endfunction (FLTK_RUN_FLUID TARGET SOURCES)

#######################################################################

# sets the bundle icon for OSX bundles

function (FLTK_SET_BUNDLE_ICON TARGET ICON_PATH)
  get_filename_component (ICON_NAME "${ICON_PATH}" NAME)
  set_target_properties ("${TARGET}" PROPERTIES
    MACOSX_BUNDLE_ICON_FILE "${ICON_NAME}"
    RESOURCE "${ICON_PATH}"
  )
endfunction (FLTK_SET_BUNDLE_ICON TARGET ICON_PATH)
