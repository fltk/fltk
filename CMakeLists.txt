#
# "$Id$"
#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Written by Michael Surette
#
# Copyright 1998-2018 by Bill Spitzak and others.
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

# Prevent annoying warning under Cygwin; this must be before project().
# Remove when CMake >= 2.8.4 is required
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project(FLTK)
cmake_minimum_required(VERSION 2.6.3)

#######################################################################
# define some macros
#######################################################################
include(CMake/macros.cmake)

#######################################################################
# basic setup
#######################################################################
include(CMake/setup.cmake)

#######################################################################
# check for headers, libraries and functions
#######################################################################
include(CMake/resources.cmake)

#######################################################################
# options
#######################################################################
include(CMake/options.cmake)

#######################################################################
# print (debug) several build variables and options
#######################################################################

set (debug_build 0) # set to 1 to show debug info
if (debug_build)
  message ("")
  message (STATUS "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt: set debug_build to 0 to disable the following info:")
  fl_debug_var(WIN32)
  fl_debug_var(MINGW)
  fl_debug_var(CYGWIN)
  fl_debug_var(MSVC)
  fl_debug_var(UNIX)
  fl_debug_var(APPLE)
  fl_debug_var(CMAKE_BUILD_TYPE)
  fl_debug_var(CMAKE_SIZEOF_VOID_P)
  fl_debug_var(OPTION_OPTIM)
  fl_debug_var(CMAKE_C_FLAGS)
  fl_debug_var(CMAKE_CXX_FLAGS)
  fl_debug_var(CMAKE_EXE_LINKER_FLAGS)
  message (STATUS "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt: end of debug_build info.")
endif (debug_build)
unset (debug_build)

#######################################################################
# variables shared by export and install
# export.cmake creates configuration files for direct use in a built but uninstalled FLTK
# install.cmake creates these files for an installed FLTK
# these two would only differ in paths, so common variables are set here
#######################################################################
include(CMake/variables.cmake)

#######################################################################
# final config and export
#######################################################################
include(CMake/export.cmake)

configure_file(
 ${CMAKE_CURRENT_SOURCE_DIR}/abi-version.cmake.in
 ${CMAKE_CURRENT_BINARY_DIR}/FL/abi-version.h
 @ONLY
 )

#######################################################################
# build examples - these have to be built after fluid is built/imported
#######################################################################
if(OPTION_BUILD_EXAMPLES)
   add_subdirectory(test)
endif(OPTION_BUILD_EXAMPLES)

#######################################################################
# installation
#######################################################################
include(CMake/install.cmake)
