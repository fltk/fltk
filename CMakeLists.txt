#
# Main CMakeLists.txt to build the FLTK project using CMake (www.cmake.org)
# Originally written by Michael Surette
#
# Copyright 1998-2025 by Bill Spitzak and others.
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
# Prevent building in the source tree - don't pollute the sources
#######################################################################

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message("")
  message(STATUS "=======================================================================")
  message(STATUS " NOTE: In-source builds of FLTK are disabled to prevent writing to the")
  message(STATUS " NOTE: source tree. Please create a subfolder (e.g. 'build') and use")
  message(STATUS " NOTE: `cmake ..` inside it or use for instance `cmake . -B build`.")
  message(STATUS " NOTE: CMake will, however, create CMakeCache.txt and CMakeFiles/* .")
  message(STATUS " NOTE: You must delete CMakeCache.txt and CMakeFiles/* manually.")
  message(STATUS "=======================================================================")
  message("")
  message(FATAL_ERROR "*** In-source builds are disabled, please read notes above! ***")
endif()

#######################################################################
# Set CMake minimum version first: must be set before `project()`
#######################################################################

# Minimum CMake version required by FLTK 1.5

# Note 1: Currently the same as for FLTK 1.4.x but will likely be raised
#         depending on new features introduced in FLTK 1.5.

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

# Since CMake 3.4: enable symbol export from all executable targets
# cmake_policy(SET CMP0065 OLD)

#######################################################################
# define the FLTK project and version
#######################################################################

project(FLTK VERSION 1.5.0)

#######################################################################
# set the required C++ standard -- may be overridden by the user
#######################################################################

# Note 1: We don't check if the user sets a standard older than C++11,
#         but if they do, the build will fail.
# Note 2: For optimal portability we disable compiler specific
#         extensions, but this can also be overridden by the user.
# Note 3: This code is experimental and intentionally undocumented.
#         It may be removed or changed w/o notice.

if(DEFINED CMAKE_CXX_STANDARD)
  if(CMAKE_CXX_STANDARD EQUAL 98 OR CMAKE_CXX_STANDARD LESS 11)
    message(FATAL_ERROR           "The FLTK project requires at least C++11")
  endif()
else()
  set(CMAKE_CXX_STANDARD          11 CACHE STRING "Minimal C++ Standard")
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
  set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "Minimal C++ Standard Required?")
endif()

if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS        OFF CACHE BOOL "Use Compiler Specific C++ Extensions")
endif()

#######################################################################
# include macro and function definitions for general usage
#######################################################################

include(CMake/fl_debug_var.cmake)
include(CMake/fl_debug_pkg.cmake)
include(CMake/fl_add_library.cmake)

# right now we don't use compatibility functions
# include(CMake/compatibility.cmake)

if(0) # debug most important CMake variables and features
  # FLTK version
  fl_debug_var(FLTK_VERSION_MAJOR)
  fl_debug_var(FLTK_VERSION_MINOR)
  fl_debug_var(FLTK_VERSION_PATCH)
  fl_debug_var(FLTK_VERSION)
  # CMake version and C++ standard
  fl_debug_var(CMAKE_VERSION)
  fl_debug_var(CMAKE_CXX_STANDARD)
  fl_debug_var(CMAKE_CXX_STANDARD_REQUIRED)
  fl_debug_var(CMAKE_CXX_EXTENSIONS)
  # Build platform
  fl_debug_var(MSVC)
  fl_debug_var(MINGW)
  fl_debug_var(MSYS)
  fl_debug_var(UNIX)
  fl_debug_var(APPLE)
  fl_debug_var(CMAKE_CROSSCOMPILING)
endif()

# Set FLTK_VERSION in the cache so user projects can access it,
# for instance if FLTK is built as a subproject (FetchContent)

set(FLTK_VERSION ${FLTK_VERSION} CACHE STRING
    "FLTK version: generated, do not change" FORCE)

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
# Create and install version config file 'FLTKConfigVersion.cmake'
#######################################################################

include(CMakePackageConfigHelpers)

write_basic_package_version_file(FLTKConfigVersion.cmake
  # [VERSION requiredVersion]       # defaults to project version
  COMPATIBILITY SameMinorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/FLTKConfigVersion.cmake
  DESTINATION ${FLTK_CONFIG_PATH}
)

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
# Output Build Configuration Summary
#######################################################################

include(FeatureSummary)         # CMake Feature Summary
include(CMake/fl_summary.cmake) # special FLTK summary macros

# CMake Feature Summary

message("")
set(_descr "${PROJECT_NAME} ${FLTK_VERSION} generated by CMake ${CMAKE_VERSION}")
feature_summary(WHAT ALL DESCRIPTION "Configuration Summary for ${_descr} --\n")

# FLTK specific build configuration and options (see macros in fl_summary.cmake)

#                "title"               dir  build (bool)             option to set
fl_summary_build("Static libraries"    lib  TRUE                     "n/a")
fl_summary_build("Shared libraries"    lib  FLTK_BUILD_SHARED_LIBS   FLTK_BUILD_SHARED_LIBS)
fl_summary_build("The forms library"   lib  FLTK_BUILD_FORMS         FLTK_BUILD_FORMS)
fl_summary_build("The OpenGL library"  lib  FLTK_USE_GL              FLTK_BUILD_GL)
fl_summary_build("fluid"               bin  FLTK_BUILD_FLUID         FLTK_BUILD_FLUID)
fl_summary_build("fltk-options"        bin  FLTK_BUILD_FLTK_OPTIONS  FLTK_BUILD_FLTK_OPTIONS)

fl_summary_build("Test programs"     bin/test      FLTK_BUILD_TEST      FLTK_BUILD_TEST)
fl_summary_build("Example programs"  bin/examples  FLTK_BUILD_EXAMPLES  FLTK_BUILD_EXAMPLES)

message(STATUS "")

if(CMAKE_BUILD_TYPE STREQUAL "")
  fl_summary("Build configuration" "<unspecified>")
else()
  fl_summary("Build configuration" "${CMAKE_BUILD_TYPE}")
endif()

fl_summary("Installation prefix" "${CMAKE_INSTALL_PREFIX}")

message("")
#                "title"           name  system library if used
fl_summary_image("Bundled Libraries" JPEG  LIB_jpeg)
fl_summary_image(""                  PNG   LIB_png)
fl_summary_image(""                  ZLIB  LIB_zlib)

if(FLTK_BACKEND_WAYLAND)
  if(USE_SYSTEM_LIBDECOR)
    fl_summary("" "Libdecor = System: ${SYSTEM_LIBDECOR_LINK_LIBRARIES}")
  else()
    fl_summary("" "Libdecor = Bundled")
  endif()
endif(FLTK_BACKEND_WAYLAND)

message("")

if(UNIX AND NOT (APPLE AND NOT FLTK_BACKEND_X11))

  if(FLTK_BACKEND_WAYLAND)
    if(FLTK_BACKEND_X11)
      fl_summary("Use Wayland" "Yes (can also run as X11 client)")
    else()
      fl_summary("Use Wayland" "Yes (cannot run as X11 client)")
    endif(FLTK_BACKEND_X11)
  else()
    fl_summary("Use Wayland" "No (X11 is used)")
  endif(FLTK_BACKEND_WAYLAND)

  fl_summary_yn("All drawing uses Cairo" FLTK_USE_CAIRO)

  fl_summary_yn("Use Pango" USE_PANGO)
  if(NOT USE_PANGO)
    fl_summary_yn("Use Xft" USE_XFT)
  endif()

endif()

set(title "Fl_Cairo_Window support")
if(FLTK_HAVE_CAIROEXT)
  fl_summary("${title}" "Yes (extended)")
elseif(FLTK_HAVE_CAIRO)
  fl_summary("${title}" "Yes (standard)")
else()
  fl_summary("${title}" "No")
endif()

message("")

fl_summary(   "CMAKE_CXX_STANDARD"          "${CMAKE_CXX_STANDARD}")
fl_summary_yn("CMAKE_CXX_STANDARD_REQUIRED" CMAKE_CXX_STANDARD_REQUIRED)
fl_summary_yn("CMAKE_CXX_EXTENSIONS"        CMAKE_CXX_EXTENSIONS)

message("")
message(STATUS "End of Configuration Summary --\n")

# optional info for "modern CMake"

if(0) # debug built library and fluid targets
  message(STATUS "------------------------ TARGETS ------------------------")
  foreach(tgt fltk fluid fluid-cmd options options-cmd images gl forms jpeg png z)
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
