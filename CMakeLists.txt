#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
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
# Set CMake minimum version first: must be set before `project()`
#######################################################################

# Minimum CMake version required by FLTK 1.4

# Note 1: Linking against OBJECT libraries (fluid, test) requires
#         at least CMake 3.12.0
# Note 2: Used in fluid since 05/2023,
#         used for Windows (MSVC) shared lib builds much earlier
# Note 3: More modern CMake features require 3.13...3.15 (Nov 2023)

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Since CMake 3.4: enable symbol export from all executable targets
# cmake_policy(SET CMP0065 OLD)

#######################################################################
# define the FLTK project and version
#######################################################################

project(FLTK VERSION 1.4.0)

#######################################################################
# include macro and function definitions for general usage
#######################################################################

include(CMake/fl_debug_var.cmake)
include(CMake/fl_debug_pkg.cmake)
include(CMake/fl_add_library.cmake)

# right now we don't use compatibility functions
# include(CMake/compatibility.cmake)

if(0)
  fl_debug_var(FLTK_VERSION_MAJOR)
  fl_debug_var(FLTK_VERSION_MINOR)
  fl_debug_var(FLTK_VERSION_PATCH)
  fl_debug_var(FLTK_VERSION)
  fl_debug_var(CMAKE_VERSION)
endif()

# Set FLTK_VERSION in the cache so user projects can access it,
# for instance if FLTK is built as a subproject (FetchContent)

set(FLTK_VERSION ${FLTK_VERSION} CACHE STRING "FLTK version" FORCE)

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

set(debug_build 0) # set to 1 to show debug info

if(debug_build)
  message("")
  message(STATUS "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt: set 'debug_build' to 0 to disable the following info:")
  fl_debug_var(WIN32)
  fl_debug_var(MINGW)
  fl_debug_var(CYGWIN)
  fl_debug_var(MSVC)
  fl_debug_var(UNIX)
  fl_debug_var(APPLE)
  fl_debug_var(CMAKE_BUILD_TYPE)
  fl_debug_var(CMAKE_SIZEOF_VOID_P)
  fl_debug_var(FLTK_OPTION_OPTIM)
  fl_debug_var(CMAKE_C_FLAGS)
  fl_debug_var(CMAKE_CXX_FLAGS)
  fl_debug_var(CMAKE_EXE_LINKER_FLAGS)
  message(STATUS "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt: end of debug_build info.")
endif(debug_build)

unset(debug_build)

#######################################################################
# Build a dummy ("empty") Cairo library for backwards compatibility.
# This should be removed some time after 1.4.0 was released, maybe
# in FLTK 1.4.1, 1.4.2, 1.5.0, or whatever the next minor release is.
#######################################################################

if(FLTK_HAVE_CAIRO)
  add_subdirectory(cairo)
endif()

#######################################################################
# build the standard FLTK libraries
#######################################################################

add_subdirectory(src)

#######################################################################
# build fluid (optional)
#######################################################################

set(FLTK_FLUID_EXECUTABLE "")

if(FLTK_BUILD_FLUID)
  add_subdirectory(fluid)
endif(FLTK_BUILD_FLUID)

# Set FLTK_FLUID_EXECUTABLE in the cache for user projects.
# This can be used if FLTK has been built as a subproject.

set(FLTK_FLUID_EXECUTABLE
    "${FLTK_FLUID_EXECUTABLE}" CACHE STRING
    "FLTK's 'fluid' executable")

#######################################################################
# build fltk-options
#######################################################################

if(FLTK_BUILD_FLTK_OPTIONS)
  add_subdirectory(fltk-options)
endif(FLTK_BUILD_FLTK_OPTIONS)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/fl_config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FL/fl_config.h
  @ONLY
)

#######################################################################
# options to build test/demo and example programs
#######################################################################

if(FLTK_BUILD_TEST)
  add_subdirectory(test)
endif(FLTK_BUILD_TEST)

if(FLTK_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(FLTK_BUILD_EXAMPLES)

#######################################################################
# installation
#######################################################################

include(CMake/install.cmake)

#######################################################################
# Generate Library Export Headers          *** EXPERIMENTAL *** WIP ***
#######################################################################

# Enable (1) or disable (0) generation of experimental headers (WIP)
set(GENERATE_EXPORT_HEADERS 0)

if(FLTK_BUILD_SHARED_LIBS AND GENERATE_EXPORT_HEADERS)

  include(GenerateExportHeader)

  generate_export_header(fltk_SHARED
    BASE_NAME fl
    EXPORT_FILE_NAME FL/fltk_export.h
    STATIC_DEFINE FL_STATIC_LIB
  )

  if(NOT MSVC)

    # Visual Studio builds only one shared lib (DLL)

    generate_export_header(fltk_images_SHARED
      BASE_NAME fl_images
      EXPORT_FILE_NAME FL/fltk_images_export.h
      STATIC_DEFINE FL_STATIC_LIB
    )

    generate_export_header(fltk_forms_SHARED
      BASE_NAME fl_forms
      EXPORT_FILE_NAME FL/fltk_forms_export.h
      STATIC_DEFINE FL_STATIC_LIB
    )

    generate_export_header(fltk_gl_SHARED
      BASE_NAME fl_gl
      EXPORT_FILE_NAME FL/fltk_gl_export.h
      STATIC_DEFINE FL_STATIC_LIB
    )

  endif(NOT MSVC)

endif(FLTK_BUILD_SHARED_LIBS AND GENERATE_EXPORT_HEADERS)

#######################################################################
# Output Configuration Summary
#######################################################################

include(FeatureSummary)

message("")
set(_descr "${PROJECT_NAME} ${FLTK_VERSION} generated by CMake ${CMAKE_VERSION}")
feature_summary(WHAT ALL DESCRIPTION "Configuration Summary for ${_descr} --\n")

message(STATUS "Static libraries   will be built in ${CMAKE_CURRENT_BINARY_DIR}/lib")

if(FLTK_BUILD_SHARED_LIBS)
  message(STATUS "Shared libraries   will be built in ${CMAKE_CURRENT_BINARY_DIR}/lib")
else()
  message(STATUS "Shared libraries   will not be built (set FLTK_BUILD_SHARED_LIBS=ON to build)")
endif()

if(FLTK_BUILD_FORMS)
  message(STATUS "The forms library  will be built in ${CMAKE_CURRENT_BINARY_DIR}/lib")
else()
  message(STATUS "The forms library  will not be built (set FLTK_BUILD_FORMS=ON to build)")
endif()

if(FLTK_USE_GL)
  message(STATUS "The OpenGL library will be built in ${CMAKE_CURRENT_BINARY_DIR}/lib")
else()
  message(STATUS "The OpenGL library will not be built (set FLTK_BUILD_GL=ON to build)")
endif()

if(FLTK_BUILD_FLUID)
  message(STATUS "fluid              will be built in ${CMAKE_CURRENT_BINARY_DIR}/bin/fluid")
else()
  message(STATUS "fluid              will not be built (set FLTK_BUILD_FLUID=ON to build)")
endif()

if(FLTK_BUILD_FLTK_OPTIONS)
  message(STATUS "fltk-options       will be built in ${CMAKE_CURRENT_BINARY_DIR}/bin/fltk-options")
else()
  message(STATUS "fltk-options       will not be built (set FLTK_BUILD_FLTK_OPTIONS=ON to build)")
endif()

if(FLTK_BUILD_TEST)
  message(STATUS "Test programs      will be built in ${CMAKE_CURRENT_BINARY_DIR}/bin/test")
else()
  message(STATUS "Test programs      will not be built (set FLTK_BUILD_TEST=ON to build)")
endif()

if(FLTK_BUILD_EXAMPLES)
  message(STATUS "Example programs   will be built in ${CMAKE_CURRENT_BINARY_DIR}/bin/examples")
else()
  message(STATUS "Example programs   will not be built (set FLTK_BUILD_EXAMPLES=ON to build)")
endif()

message(STATUS "")

if(CMAKE_BUILD_TYPE STREQUAL "")
  message(STATUS "Build configuration     : <unspecified>")
else()
  message(STATUS "Build configuration     : ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "")

if(FLTK_USE_BUNDLED_JPEG)
  message(STATUS "Image Libraries         : JPEG = Builtin")
else()
  message(STATUS "Image Libraries         : JPEG = System")
endif()

if(FLTK_USE_BUNDLED_PNG)
  message(STATUS "                        : PNG  = Builtin")
else()
  message(STATUS "                        : PNG  = System")
endif()

if(FLTK_USE_BUNDLED_ZLIB)
  message(STATUS "                        : ZLIB = Builtin")
else()
  message(STATUS "                        : ZLIB = System")
endif()

if(UNIX AND NOT (APPLE AND NOT FLTK_BACKEND_X11))

  if(FLTK_BACKEND_WAYLAND)
    if(FLTK_BACKEND_X11)
      message(STATUS "Use Wayland             : Yes (can also run as X11 client)")
    else()
      message(STATUS "Use Wayland             : Yes (cannot run as X11 client)")
    endif(FLTK_BACKEND_X11)
    if(USE_SYSTEM_LIBDECOR)
      message(STATUS "Use system libdecor     : Yes")
    else()
      message(STATUS "Use system libdecor     : No")
    endif(USE_SYSTEM_LIBDECOR)
  else()
    message(STATUS "Use Wayland             : No (therefore, X11 is used)")
  endif(FLTK_BACKEND_WAYLAND)

  if(FLTK_USE_CAIRO)
    message(STATUS "All drawing uses Cairo  : Yes")
  else()
    message(STATUS "All drawing uses Cairo  : No")
  endif()

  if(USE_PANGO)
    message(STATUS "Use Pango               : Yes")
  else()
    message(STATUS "Use Pango               : No")
    if(USE_XFT)
      message(STATUS "Use Xft                 : Yes")
    else()
      message(STATUS "Use Xft                 : No")
    endif()
  endif()

endif()

if(FLTK_HAVE_CAIROEXT)
  message(STATUS "Fl_Cairo_Window support : Yes (extended)")
elseif(FLTK_HAVE_CAIRO)
  message(STATUS "Fl_Cairo_Window support : Yes (standard)")
else()
  message(STATUS "Fl_Cairo_Window support : No")
endif()

if(FLTK_USE_STD)
  message(STATUS "Use std::               : Yes")
else()
  message(STATUS "Use std::               : No")
endif()

message("")
message(STATUS "End of Configuration Summary --\n")

# optional info for "modern CMake"

if(0) # debug built library and fluid targets
  message(STATUS "------------------------ TARGETS ------------------------")
  foreach(tgt fltk fluid fluid-cmd options options-cmd images gl forms cairo jpeg png z)
    if(TARGET fltk::${tgt})
      message("Target: fltk::${tgt}")
      # fl_debug_target(fltk::${tgt})
    endif()
    if(TARGET fltk::${tgt}-shared)
      message("Target: fltk::${tgt}-shared")
      # fl_debug_target(fltk::${tgt}-shared)
    endif()
  endforeach()
  message(STATUS "---------------------------------------------------------")
endif()
