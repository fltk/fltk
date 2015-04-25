#
# "$Id$"
#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2015 by Bill Spitzak and others.
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
# final config and export
#######################################################################
# Set the fluid executable path
if(CMAKE_CROSSCOMPILING)
   find_file(FLUID_PATH
      NAMES fluid fluid.exe
      PATHS ENV PATH
      NO_CMAKE_FIND_ROOT_PATH
   )
   add_executable(fluid IMPORTED)
   set(FLTK_FLUID_EXECUTABLE ${FLUID_PATH})
   set(FLUID)       # no export
   set_target_properties(fluid
      PROPERTIES IMPORTED_LOCATION ${FLUID_PATH}
   )
else()
   add_subdirectory(fluid)
   set(FLTK_FLUID_EXECUTABLE fluid)
   set(FLUID fluid) # export
endif(CMAKE_CROSSCOMPILING)

add_subdirectory(src)

# generate FLTK-Targets.cmake for build directory use
export(TARGETS ${FLUID} ${FLTK_LIBRARIES} FILE ${CMAKE_BINARY_DIR}/FLTK-Targets.cmake)

# generate FLTKConfig.cmake for build directory use
set(INCLUDE_DIRS "${FLTK_INCLUDE_DIRS}")
set(CONFIG_PATH ${FLTK_BINARY_DIR})

configure_file(
   ${FLTK_SOURCE_DIR}/CMake/FLTKConfig.cmake.in
   ${FLTK_BINARY_DIR}/FLTKConfig.cmake
   @ONLY
)

# generate UseFLTK.cmake for build directory use
configure_file(
   ${FLTK_SOURCE_DIR}/CMake/UseFLTK.cmake.in
   ${FLTK_BINARY_DIR}/UseFLTK.cmake
   @ONLY
)

# generate fltk-config for build directory use
set(prefix ${FLTK_BINARY_DIR})
set(exec_prefix "\${prefix}")
set(includedir "${FLTK_SOURCE_DIR}")
set(libdir "\${exec_prefix}/lib")
set(srcdir ".")

set(LIBNAME "${libdir}/libfltk.a")

configure_file(
   "${FLTK_SOURCE_DIR}/fltk-config.in"
   "${FLTK_BINARY_DIR}/fltk-config"
   @ONLY
)
if(UNIX)
   execute_process(COMMAND chmod 755 fltk-config
      WORKING_DIRECTORY "${FLTK_BINARY_DIR}"
   )
endif(UNIX)

# generate config.h
configure_file(
   "${FLTK_SOURCE_DIR}/configh.cmake.in"
   "${FLTK_BINARY_DIR}/config.h"
   @ONLY
)

if(OPTION_CREATE_LINKS)
   # Set PREFIX_INCLUDE to the proper value.
   if(IS_ABSOLUTE ${FLTK_INCLUDEDIR})
      set(PREFIX_INCLUDE ${FLTK_INCLUDEDIR})
   else()
      set(PREFIX_INCLUDE "${CMAKE_INSTALL_PREFIX}/${FLTK_INCLUDEDIR}")
   endif(IS_ABSOLUTE ${FLTK_INCLUDEDIR})
   configure_file(
      "${FLTK_SOURCE_DIR}/CMake/install-symlinks.cmake.in"
      "${FLTK_BINARY_DIR}/install-symlinks.cmake"
      @ONLY
   )
endif(OPTION_CREATE_LINKS)
