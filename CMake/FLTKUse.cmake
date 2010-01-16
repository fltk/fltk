#
# "$Id$"
#
# CMake support file to build the FLTK project using CMake (www.cmake.org)
#
# Copyright 1998-2010 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems on the following page:
#
#     http:#www.fltk.org/str.php
#

IF(NOT FLTK11_FOUND)
  MESSAGE(FATAL_ERROR "Something went wrong. You are including FLTKUse.cmake but FLTK was not found")
ENDIF(NOT FLTK11_FOUND)

# -------------------------------------------------------------
# This macro automates wrapping of Fluid files
# Specify the output variable name and the list of sources
# The output variable can be directly added to the target.
#
# For example:
#   FLTK_WRAP_FLUID(CubeView_SRCS CubeViewUI.fl)
#   ADD_EXECUTABLE(CubeView CubeMain.cxx CubeView.cxx CubeView.h ${CubeView_SRCS})
# -------------------------------------------------------------
MACRO(FLTK_WRAP_FLUID VARIABLE)
  FOREACH(src ${ARGN})
    IF("${src}" MATCHES ".fl$")
      GET_FILENAME_COMPONENT(fname ${src} NAME_WE)
      GET_FILENAME_COMPONENT(fpath ${src} PATH)
      GET_SOURCE_FILE_PROPERTY(gen ${src} GENERATED)
      IF(gen)
        SET(fluid_name "${src}")
      ELSE(gen)
        SET(fluid_name "${CMAKE_CURRENT_SOURCE_DIR}/${fpath}/${fname}.fl")
        IF(NOT EXISTS "${fluid_name}")
          SET(fluid_name "${CMAKE_CURRENT_BINARY_DIR}/${fpath}/${fname}.fl")
          IF(NOT EXISTS "${fluid_name}")
            SET(fluid_name "${fpath}/${fname}.fl")
            IF(NOT EXISTS "${fluid_name}")
              MESSAGE(SEND_ERROR "Cannot find Fluid source file: ${fpath}/${fname}.fl")
            ENDIF(NOT EXISTS "${fluid_name}")
          ENDIF(NOT EXISTS "${fluid_name}")
        ENDIF(NOT EXISTS "${fluid_name}")
      ENDIF(gen)
      SET(cxx_name "${CMAKE_CURRENT_BINARY_DIR}/${fname}.cxx")
      SET(h_name "${CMAKE_CURRENT_BINARY_DIR}/${fname}.h")
      SET(${VARIABLE} "${${VARIABLE}};${cxx_name}")
      ADD_CUSTOM_COMMAND(
        OUTPUT ${cxx_name}
        DEPENDS "${fluid_name}" "${FLUID_COMMAND}"
        COMMAND ${FLUID_COMMAND}
        ARGS -c ${fluid_name})
      ADD_CUSTOM_COMMAND(
        OUTPUT ${h_name}
        DEPENDS "${fluid_name}" "${FLUID_COMMAND}"
        COMMAND ${FLUID_COMMAND}
        ARGS -c ${fluid_name})
    ENDIF("${src}" MATCHES ".fl$")
  ENDFOREACH(src)
ENDMACRO(FLTK_WRAP_FLUID VARIABLE)


# Make FLTK easier to use
INCLUDE_DIRECTORIES(${FLTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${FLTK_LIBRARY_DIRS})

# Load the compiler settings used for FLTK.
IF(FLTK_BUILD_SETTINGS_FILE)
  INCLUDE(CMakeImportBuildSettings)
  CMAKE_IMPORT_BUILD_SETTINGS(${FLTK_BUILD_SETTINGS_FILE})
ENDIF(FLTK_BUILD_SETTINGS_FILE)

# Add compiler flags needed to use FLTK.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLTK_REQUIRED_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLTK_REQUIRED_CXX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLTK_REQUIRED_EXE_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FLTK_REQUIRED_SHARED_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${FLTK_REQUIRED_MODULE_LINKER_FLAGS}")

#
# End of "$Id$".
#
