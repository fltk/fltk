#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
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
# final config and export
#######################################################################

# Set the fluid executable path
if (CMAKE_CROSSCOMPILING)
  find_file(FLUID_PATH
    NAMES fluid fluid.exe
    PATHS ENV PATH
    NO_CMAKE_FIND_ROOT_PATH
  )
  add_executable(fluid IMPORTED)
  set (FLTK_FLUID_EXECUTABLE ${FLUID_PATH})
  set (FLUID)       # no export
  set_target_properties(fluid
    PROPERTIES IMPORTED_LOCATION ${FLUID_PATH}
  )
else ()
  add_subdirectory(fluid)
  set (FLTK_FLUID_EXECUTABLE fluid)
  set (FLUID fluid) # export
endif (CMAKE_CROSSCOMPILING)

add_subdirectory(src)

# generate FLTK-Targets.cmake for build directory use
export(TARGETS ${FLUID} ${FLTK_LIBRARIES} FILE ${CMAKE_CURRENT_BINARY_DIR}/FLTK-Targets.cmake)

# generate FLTK-Functions.cmake for build directory use
configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/FLTK-Functions.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/FLTK-Functions.cmake
  COPYONLY
)

# generate FLTKConfig.cmake for build directory use
set (INCLUDE_DIRS "${FLTK_INCLUDE_DIRS}")
set (CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/FLTKConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FLTKConfig.cmake
  @ONLY
)

# generate UseFLTK.cmake for build directory use
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/UseFLTK.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/UseFLTK.cmake
  @ONLY
)

# generate fltk-config for build directory use
set (prefix ${CMAKE_CURRENT_BINARY_DIR})
set (exec_prefix "\${prefix}")
set (includedir "${CMAKE_CURRENT_SOURCE_DIR}")
set (BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set (libdir "\${exec_prefix}/lib")
set (srcdir ".")

set (LIBNAME "${libdir}/libfltk.a")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/fltk-config.in"
  "${CMAKE_CURRENT_BINARY_DIR}/fltk-config"
  @ONLY
)

if (UNIX)
  execute_process(COMMAND chmod 755 fltk-config
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  )
endif (UNIX)

# prepare some variables for config.h

if (IS_ABSOLUTE "${FLTK_DATADIR}")
  set (PREFIX_DATA "${FLTK_DATADIR}/fltk")
else (IS_ABSOLUTE "${FLTK_DATADIR}")
  set (PREFIX_DATA "${CMAKE_INSTALL_PREFIX}/${FLTK_DATADIR}/fltk")
endif (IS_ABSOLUTE "${FLTK_DATADIR}")

if (IS_ABSOLUTE "${FLTK_DOCDIR}")
  set (PREFIX_DOC "${FLTK_DOCDIR}/fltk")
else (IS_ABSOLUTE "${FLTK_DOCDIR}")
  set (PREFIX_DOC "${CMAKE_INSTALL_PREFIX}/${FLTK_DOCDIR}/fltk")
endif (IS_ABSOLUTE "${FLTK_DOCDIR}")

set (CONFIG_H_IN configh.cmake.in)
set (CONFIG_H config.h)

# generate config.h

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${CONFIG_H_IN}"
  "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_H}"
  @ONLY
)

if (OPTION_CREATE_LINKS)
  # Set PREFIX_INCLUDE to the proper value.
  if (IS_ABSOLUTE ${FLTK_INCLUDEDIR})
    set (PREFIX_INCLUDE ${FLTK_INCLUDEDIR})
  else ()
    set (PREFIX_INCLUDE "${CMAKE_INSTALL_PREFIX}/${FLTK_INCLUDEDIR}")
  endif (IS_ABSOLUTE ${FLTK_INCLUDEDIR})
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMake/install-symlinks.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/install-symlinks.cmake"
    @ONLY
  )
endif (OPTION_CREATE_LINKS)
