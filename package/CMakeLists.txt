#
# CMakeLists.txt to enable CPack for the FLTK project
#
# Copyright 2025 by Bill Spitzak and others.
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

# Note: this file is included by add_subdirectory() only if it really
# is to be used (top level project, maybe an option, etc.).
# Therefore the code below is executed unconditionally...

#######################################################################
# Configure CPack parameters
#######################################################################

set(CPACK_PACKAGE_NAME                    "FLTK")
set(CPACK_PACKAGE_VENDOR                  "The FLTK TEAM (fltk.org)")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY     "Cross-platform GUI development library")
set(CPACK_PACKAGE_INSTALL_DIRECTORY       "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_VERSION_MAJOR           "${FLTK_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR           "${FLTK_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH           "${FLTK_VERSION_PATCH}")

set(CPACK_VERBATIM_VARIABLES              TRUE)

# The following files are used by *some* installers (not yet supported by FLTK).
# These files would be located in the same directory as this file.
# CMake uses default values if these variables are not defined.

# set(CPACK_PACKAGE_DESCRIPTION_FILE        ${CMAKE_CURRENT_LIST_DIR}/Description.txt)
# set(CPACK_RESOURCE_FILE_WELCOME           ${CMAKE_CURRENT_LIST_DIR}/Welcome.txt)
# set(CPACK_RESOURCE_FILE_LICENSE           ${CMAKE_CURRENT_LIST_DIR}/License.txt)
# set(CPACK_RESOURCE_FILE_README            ${CMAKE_CURRENT_LIST_DIR}/Readme.txt)

#######################################################################
# Include the CPack module
#######################################################################

include(CPack)
