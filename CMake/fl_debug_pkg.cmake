#
# Macro used by the CMake build system for the Fast Light Tool Kit (FLTK).
#
# Copyright 2022 by Bill Spitzak and others.
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
# fl_debug_pkg - a macro to output pkgconfig debugging info
#######################################################################
#
# This macro displays the name and value of some CMake variables
# defined by pkg_check_modules().
#
# Syntax:
#
#   fl_debug_pkg(PREFIX NAME)
#
# Example for package "cairo":
#
#  pkg_check_modules (CAIRO cairo)
#  fl_debug_pkg(CAIRO cairo)
#
# The first command searches for pkg 'cairo' and stores the results
# in CMake variables with prefix 'CAIRO_'.
#
# The second command displays all relevant variables if the package has
# been found, otherwise only 'CAIRO_FOUND' (empty or false).
#
#######################################################################

macro(fl_debug_pkg PREFIX NAME)
  message("")
  message(STATUS "Results of pkg_check_modules(${PREFIX}, ${NAME}):")
  fl_debug_var(${PREFIX}_FOUND)
  if(${PREFIX}_FOUND)

    fl_debug_var(${PREFIX}_INCLUDE_DIRS)
    fl_debug_var(${PREFIX}_CFLAGS)
    fl_debug_var(${PREFIX}_LIBRARIES)
    fl_debug_var(${PREFIX}_LINK_LIBRARIES)
    fl_debug_var(${PREFIX}_LIBRARY_DIRS)
    fl_debug_var(${PREFIX}_LDFLAGS)
    fl_debug_var(${PREFIX}_LDFLAGS_OTHER)
    fl_debug_var(${PREFIX}_CFLAGS_OTHER)

    fl_debug_var(${PREFIX}_STATIC_INCLUDE_DIRS)
    fl_debug_var(${PREFIX}_STATIC_CFLAGS)
    fl_debug_var(${PREFIX}_STATIC_LIBRARIES)
    fl_debug_var(${PREFIX}_STATIC_LINK_LIBRARIES)
    fl_debug_var(${PREFIX}_STATIC_LIBRARY_DIRS)

    fl_debug_var(${PREFIX}_VERSION)
    fl_debug_var(${PREFIX}_PREFIX)
    fl_debug_var(${PREFIX}_INCLUDEDIR)
    fl_debug_var(${PREFIX}_LIBDIR)

  endif()
  message("")
endmacro(fl_debug_pkg)
