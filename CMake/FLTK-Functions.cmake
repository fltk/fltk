#
# "$Id$"
#
# FLTK-Functions.cmake
# Written by Michael Surette
#
# Copyright 1998-2016 by Bill Spitzak and others.
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

#######################################################################
# functions used by the build system and exported for the end-user
#######################################################################
# USAGE: FLTK_RUN_FLUID TARGET_NAME "FLUID_SOURCE [.. FLUID_SOURCE]"
function(FLTK_RUN_FLUID TARGET SOURCES)
    set (CXX_FILES)
    foreach(src ${SOURCES})
        if ("${src}" MATCHES "\\.fl$")
            string(REGEX REPLACE "(.*).fl" \\1 basename ${src})
            add_custom_command(
                OUTPUT "${basename}.cxx" "${basename}.h"
                COMMAND "${FLTK_FLUID_EXECUTABLE}" -c ${CMAKE_CURRENT_SOURCE_DIR}/${src}
                DEPENDS ${src}
                MAIN_DEPENDENCY ${src}
            )
            list(APPEND CXX_FILES "${basename}.cxx")
        endif ("${src}" MATCHES "\\.fl$")
        set (${TARGET} ${CXX_FILES} PARENT_SCOPE)
    endforeach(src)
endfunction(FLTK_RUN_FLUID TARGET SOURCES)

#######################################################################
# sets the bundle icon for OSX bundles
function(FLTK_SET_BUNDLE_ICON TARGET ICON_PATH)
    get_filename_component(ICON_NAME "${ICON_PATH}" NAME)
    set_target_properties("${TARGET}" PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "${ICON_NAME}"
        RESOURCE "${ICON_PATH}"
    )
endfunction(FLTK_SET_BUNDLE_ICON TARGET ICON_PATH)

#
# End of "$Id$".
#
