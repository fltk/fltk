#
# Installation support for building the FLTK project using CMake (www.cmake.org)
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

#######################################################################
# installation
#######################################################################

# generate uninstall target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  @ONLY
)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

install(DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR}/FL
  DESTINATION ${FLTK_INCLUDEDIR} USE_SOURCE_PERMISSIONS
  FILES_MATCHING
    PATTERN "*.[hH]"
    PATTERN "fl_config.h" EXCLUDE
)

install(DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/FL
  DESTINATION ${FLTK_INCLUDEDIR} USE_SOURCE_PERMISSIONS
  FILES_MATCHING
    PATTERN "*.[hH]"
)

if(FLTK_INSTALL_LINKS)
  install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install-symlinks.cmake)
endif(FLTK_INSTALL_LINKS)

# generate FLTKConfig.cmake for installed directory use
set(INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
if(FLTK_HAVE_CAIRO)
  list(APPEND INCLUDE_DIRS ${PKG_CAIRO_INCLUDE_DIRS})
endif()

set(CONFIG_PATH ${CMAKE_INSTALL_PREFIX}/${FLTK_CONFIG_PATH})

install(EXPORT FLTK-Targets
  DESTINATION ${FLTK_CONFIG_PATH}
  FILE FLTK-Targets.cmake
  NAMESPACE fltk::
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/FLTKConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/etc/FLTKConfig.cmake
  @ONLY
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/etc/FLTKConfig.cmake
  DESTINATION ${FLTK_CONFIG_PATH}
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/FLTK-Functions.cmake
  DESTINATION ${FLTK_CONFIG_PATH}
)

# Generate fltk-config

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(BINARY_DIR)
set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
set(srcdir ".")

set(LIBNAME "${libdir}/libfltk.a")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/fltk-config.in"
  "${CMAKE_CURRENT_BINARY_DIR}/bin/fltk-config"
  @ONLY
)

# Install fltk-config
# Note: no need to set execute perms, install(PROGRAMS) does this

install(PROGRAMS
  ${CMAKE_CURRENT_BINARY_DIR}/bin/fltk-config
  DESTINATION ${FLTK_BINDIR}
)

if(UNIX OR MSYS OR MINGW)
  macro(INSTALL_MAN FILE LEVEL)
    install(FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/documentation/src/${FILE}.man
      DESTINATION ${FLTK_MANDIR}/man${LEVEL}
      RENAME ${FILE}.${LEVEL}
    )
  endmacro(INSTALL_MAN FILE LEVEL)

  if(FLTK_BUILD_FLUID)
    INSTALL_MAN (fluid 1)
  endif(FLTK_BUILD_FLUID)
  if(FLTK_BUILD_FLTK_OPTIONS)
    INSTALL_MAN (fltk-options 1)
  endif(FLTK_BUILD_FLTK_OPTIONS)
  INSTALL_MAN (fltk-config 1)
  INSTALL_MAN (fltk 3)

  if(FLTK_BUILD_TEST AND FLTK_BUILD_FLUID)
    # Don't (!) install man pages of games (GitHub issue #23)
    # INSTALL_MAN (blocks 6)
    # INSTALL_MAN (checkers 6)
    # INSTALL_MAN (sudoku 6)
  endif()

endif(UNIX OR MSYS OR MINGW)
