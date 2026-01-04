#
# CMakeLists.txt to create screenshot programs for FLTK documentation
#
# Copyright 2026 by Bill Spitzak and others.
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

########################################################################
# Screenshot programs for FLTK documentation
########################################################################

# The programs in this subdirectory are intended to be used by
# developers to create screenshots for our Doxygen documentation.
# See README.txt for more info.

# These programs are not "installed" on target systems, they are only
# built in the FLTK build tree.

########################################################################
# Define a list of programs that will be built w/o extension.
# All programs must use the 'scr_' prefix and the '.cxx' extension.
# Define the names in the list below w/o prefix and extension.
########################################################################

set(NAMES
    unicode      # Unicode text example
    # add more programs here ...
)

########################################################################
# Build a special CMake "object library" for common (screenshot) code
########################################################################

# not yet implemented

########################################################################
# Build all programs with common options
########################################################################

set(EXECUTABLE_OUTPUT_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/../bin/screenshots)

set(PREFIX scr_) # will be prepended to target names

foreach(_prog ${NAMES})

  set(_target ${PREFIX}${_prog}) # enforce the target prefix !

  add_executable(${_target} WIN32 MACOSX_BUNDLE ${_prog}.cxx)

  target_link_libraries(${_target} PRIVATE fltk::images)

  set_target_properties(${_target} PROPERTIES
    OUTPUT_NAME ${_target}
    EXCLUDE_FROM_ALL TRUE
  )

endforeach()
