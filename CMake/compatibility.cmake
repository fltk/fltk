#
# CMake compatibility functions and macros for the Fast Light Tool Kit (FLTK)
#
# Copyright 1998-2023 by Bill Spitzak and others.
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

################################################################################
#
# The functions and maybe macros in this file are defined to simplify CMake
# code that uses features not available in all supported CMake versions.
# Functions should be preferred (rather than macros) because functions
# have their own variable scope.
#
# The function must apply a workaround for older versions or not add code
# at all if there is no suitable workaround.
#
# The functions included here may be removed (with according changes of the
# calling code) or the workaround code may be removed from the implementation
# after cmake_minimum_required() has been raised to a version number greater
# than or equal to the required version.
#
# Current cmake_minimum_required() version: see <fltk-root>/CMakeLists.txt
#
################################################################################


# Right now we don't need compatibility functions
# This file is currently "empty" but left for documentation purposes
# An example function documentation follows...


################################################################################
#
# function fl_target_link_directories - add link directories to target
#
# Requires CMake version: 3.13
# https://cmake.org/cmake/help/latest/command/target_link_directories.html
#
# Input:
#
# - TARGET: target to add link directories to, e.g. fluid
#
# - SCOPE: one of <INTERFACE|PUBLIC|PRIVATE> (see CMake docs)
#
# - DIRS: quoted list of link directories (separated by ';')
#
# The 'DIRS' argument takes a standard CMake list of directories, i.e. the
# individual members must be separated by ';'. The list may be empty ("").
# If more than one directory is to be added or if the list of directories
# can be empty it *must* be quoted. This function may be called more than
# once. Each invocation adds zero, one, or more directories.
#
# Example:
#
#   fl_target_link_directories(fluid PRIVATE "${PKG_CAIRO_LIBRARY_DIRS}")
#
#   In this example 'PKG_CAIRO_LIBRARY_DIRS' is platform dependent and
#   can be an empty list.
#
################################################################################
