#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
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
# set CMake minimum version (must be before `project()`
#######################################################################

# Note: previous (1.3.5) minimum version
# cmake_minimum_required (VERSION 2.6.3)

# Note: minimum CMake version required by GNUInstallDirs
# cmake_minimum_required (VERSION 2.8.4)

# Minimum CMake version required by FLTK 1.4 (06/2020, work in progress)
cmake_minimum_required (VERSION 3.2.3 FATAL_ERROR)

#######################################################################

# Use "legacy mode" of FindOpenGL (avoid CMake developer warning).
# Note: we're using FindOpenGL with `OPENGL_LIBRARIES` and not (yet)
# the `OpenGL::GL` target. This may be changed in the future.
# See https://cmake.org/cmake/help/latest/policy/CMP0072.html
# Update Feb 28, 2021: To avoid a warning about "OLD" policies we set
# OpenGL_GL_PREFERENCE directly to "LEGACY" (other option: "GLVND").

# if (POLICY CMP0072)
#   cmake_policy (SET CMP0072 OLD)
# endif ()

set (OpenGL_GL_PREFERENCE LEGACY)

#######################################################################
# define the FLTK project and version
#######################################################################

project (FLTK VERSION 1.3.8)

#######################################################################
# include macro definitions of generally used macros
#######################################################################

include (CMake/fl_debug_var.cmake)
include (CMake/fl_add_library.cmake)

if (false)
  fl_debug_var (FLTK_VERSION_MAJOR)
  fl_debug_var (FLTK_VERSION_MINOR)
  fl_debug_var (FLTK_VERSION_PATCH)
  fl_debug_var (FLTK_VERSION)
endif (false)

#######################################################################
# basic setup
#######################################################################
include (CMake/setup.cmake)

#######################################################################
# check for headers, libraries and functions
#######################################################################
include (CMake/resources.cmake)

#######################################################################
# options
#######################################################################
include (CMake/options.cmake)

#######################################################################
# print (debug) several build variables and options
#######################################################################

set (debug_build 0) # set to 1 to show debug info

if (debug_build)
  message ("")
  message (STATUS "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt: set debug_build to 0 to disable the following info:")
  fl_debug_var (WIN32)
  fl_debug_var (MINGW)
  fl_debug_var (CYGWIN)
  fl_debug_var (MSVC)
  fl_debug_var (UNIX)
  fl_debug_var (APPLE)
  fl_debug_var (ANDROID)
  fl_debug_var (CMAKE_BUILD_TYPE)
  fl_debug_var (CMAKE_SIZEOF_VOID_P)
  fl_debug_var (OPTION_OPTIM)
  fl_debug_var (CMAKE_C_FLAGS)
  fl_debug_var (CMAKE_CXX_FLAGS)
  fl_debug_var (CMAKE_EXE_LINKER_FLAGS)
  message (STATUS "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt: end of debug_build info.")
endif (debug_build)

unset (debug_build)

#######################################################################
# variables shared by export and install
# export.cmake creates configuration files for direct use in a built but uninstalled FLTK
# install.cmake creates these files for an installed FLTK
# these two would only differ in paths, so common variables are set here
#######################################################################
include (CMake/variables.cmake)

#######################################################################
# final config and export
#######################################################################
include (CMake/export.cmake)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/abi-version.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FL/abi-version.h
  @ONLY
)

#######################################################################
# options to build test/demo and example programs
#######################################################################

if (FLTK_BUILD_TEST)
  add_subdirectory (test)
endif (FLTK_BUILD_TEST)

if (FLTK_BUILD_EXAMPLES)
  add_subdirectory (examples)
endif (FLTK_BUILD_EXAMPLES)

#######################################################################
# installation
#######################################################################
include (CMake/install.cmake)

#######################################################################
# Output Configuration Summary
#######################################################################

include (FeatureSummary)

message ("")
set (_descr "${PROJECT_NAME} ${FLTK_VERSION} generated by CMake ${CMAKE_VERSION}")
feature_summary (WHAT ALL DESCRIPTION "Configuration Summary for ${_descr} --\n")

message (STATUS "Static libraries will be built in ${CMAKE_CURRENT_BINARY_DIR}/lib")

if (OPTION_BUILD_SHARED_LIBS)
  message (STATUS "Shared libraries will be built in  ${CMAKE_CURRENT_BINARY_DIR}/lib")
else ()
  message (STATUS "Shared libraries will not be built (set OPTION_BUILD_SHARED_LIBS=ON to build)")
endif ()

if (FLTK_BUILD_TEST)
  message (STATUS "Test programs    will be built in ${CMAKE_CURRENT_BINARY_DIR}/bin/test")
endif ()

if (FLTK_BUILD_EXAMPLES)
  message (STATUS "Example programs will be built in ${CMAKE_CURRENT_BINARY_DIR}/bin/examples")
else ()
  message (STATUS "Example programs will not be built (set FLTK_BUILD_EXAMPLES=ON to build)")
endif ()

message ("")
message (STATUS "End of Configuration Summary --\n")
