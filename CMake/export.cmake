#
# "$Id: CMakeLists.txt 10092 2014-02-02 00:49:50Z AlbrechtS $"
#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2010 by Bill Spitzak and others.
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
   set(FLTK_FLUID_PATH ${FLUID_PATH})
   set_target_properties(fluid
      PROPERTIES IMPORTED_LOCATION ${FLUID_PATH}
   )
else()
   add_subdirectory(fluid)
   set(FLTK_FLUID_EXECUTABLE fluid)
   set(FLTK_FLUID_PATH ${PREFIX_BIN}/fluid)
endif(CMAKE_CROSSCOMPILING)

add_subdirectory(src)

# generate FLTKConfig.cmake
string(REPLACE ";" " " EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
configure_file(
   ${FLTK_SOURCE_DIR}/CMake/FLTKConfig.cmake.in
   ${EXECUTABLE_OUTPUT_PATH}/FLTKConfig.cmake
   @ONLY
)

# generate UseFLTK.cmake
configure_file(
   ${FLTK_SOURCE_DIR}/CMake/UseFLTK.cmake.in
   ${EXECUTABLE_OUTPUT_PATH}/UseFLTK.cmake
   @ONLY
)

# generate config.h
configure_file(
   "${FLTK_SOURCE_DIR}/configh.cmake.in"
   "${FLTK_BINARY_DIR}/config.h"
   @ONLY
)

# generate fltk-config
get_filename_component(CC ${CMAKE_C_COMPILER} NAME)
get_filename_component(CXX ${CMAKE_CXX_COMPILER} NAME)

string(REPLACE ";" " " C_FLAGS "${FLTK_CFLAGS}")

if(X11_Xext_FOUND)
   list(APPEND FLTK_LDLIBS -lXext)
endif(X11_Xext_FOUND)
string(REPLACE ";" " " LD_LIBS "${FLTK_LDLIBS}")

configure_file(
   "${FLTK_SOURCE_DIR}/fltk-config.cmake.in"
   "${FLTK_BINARY_DIR}/fltk-config"
   @ONLY
)
if(UNIX)
   execute_process(COMMAND chmod 755 fltk-config
      WORKING_DIRECTORY "${FLTK_BINARY_DIR}"
   )
endif(UNIX)

if(OPTION_CREATE_LINKS)
   configure_file(
      "${FLTK_SOURCE_DIR}/CMake/install-symlinks.cmake.in"
      "${FLTK_BINARY_DIR}/install-symlinks.cmake"
      @ONLY
   )
endif(OPTION_CREATE_LINKS)
#
